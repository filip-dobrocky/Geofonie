/*
 * ESPRoto.cpp
 * Part of Geofonie project
 * Copyright (C) 2024 Filip Dobrocky, Trychtyr collective
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <Arduino.h>
#include <MIDI.h>
#include <Wire.h>
#include <VL53L4CD.h>
#include <ESP_FlexyStepper.h>
#include <ESP32Servo.h>

// GeoNode.h (painlessMesh) must pull in TaskScheduler: it sets _TASK_STD_FUNCTION
// first, and including TaskScheduler.h ahead of it locks in a Task without
// std::function callbacks, which mesh.addTask() then fails to construct.
#include "GeoNode.h"
#include "GeoIdentity.h"
#include "GeoOta.h"
#include "GEOUtils.h"
#include "Smoothing.h"
#include "Sequencer.h"

// Constants
const char* TAG = "geo_roto";

#define FW_VERSION 1

// Object id at runtime: read from NVS by the generic OTA image, seeded over USB
// by the per-node provisioning envs (-DOBJ_ID=n). -1 means unprovisioned.
int g_obj_id = -1;

#ifndef BAD_DRIVER
#define BAD_DRIVER 0
#endif

#ifdef BOARD_XIAO
#define MS1_PIN D4
#define MS2_PIN D5
#define MS3_PIN D6

#define RX_PIN D7
#define TX_PIN D8

#define I2C_SDA D9
#define I2C_SCL D10

#define STEP_PIN D0
#define DIR_PIN D1
#define EN_PIN D2

#define SERVO_PIN D3
#endif

#define MIN_ANGLE 40
#define MAX_ANGLE 99

#define MIN_DISTANCE 120
#define MAX_DISTANCE 400

#define STEPS_PER_ROTATION 13500
#define DEGREES_PER_ROTATION 10

// ---- Auto-calibration ----
// Scans the object once per boot: the servo is held still for a full rotation at
// each angle, so only one axis moves at a time. Tuning knobs for a physical rig.
// Upper bound, not a haste setting: a rotation takes STEPS_PER_ROTATION/CAL_SPEED
// seconds and samples every SENSOR_PERIOD, so faster thins the azimuthal scan
// (4000 -> 3.4 s -> ~140 samples/rotation -> one every 2.6 degrees).
#define CAL_SPEED 4000
#define CAL_ANGLE_STEP 4
#define CAL_ANGLE_COUNT (((MAX_ANGLE - MIN_ANGLE) / CAL_ANGLE_STEP) + 1)
// Servo travel between two adjacent stops.
#define CAL_SETTLE_MS 200
// Below this span across a rotation an angle is the pedestal, not the object.
#define CAL_FLAT_SPAN_MM 8
// Widens the measured window so the extremes don't sit exactly on 0 and 127.
#define CAL_MARGIN_MM 5
// Below this share of in-range samples the sensor is pointing past the object.
#define CAL_MIN_VALID_PCT 50

#define MISC_PARAM_NUM 6

// Sensor telemetry period. Every reading is a mesh broadcast, so five objects at
// 500 ms is already ~10 messages/s on a shared bus -- raise before lowering.
#define READING_PERIOD 500

// Sensor integration window (10..200 ms). Longer = quieter, and cheaper than
// averaging it back out in software. Trade against NUM_READINGS in Smoothing.h.
#define SENSOR_TIMING_BUDGET 20
// Poll margin: must stay above the budget or the blocking read() stalls loop().
#define SENSOR_PERIOD (SENSOR_TIMING_BUDGET + 4)

#ifndef NO_SENSOR
#define NO_SENSOR 0
#endif


// ---- Enums ----
enum TransportMode {
  STATIC,
  LINEAR,
  ALTERNATING
};

// ---- Globals ----
const char *base_address = "/toRoto";
const char *broadcast_address = "/fromRoto";

GeoConfig geo_cfg{-1, false, base_address, broadcast_address, "roto"};
GeoNode node(geo_cfg);
GeoOta ota(node.mesh(), "roto");

// ---- Misc OSC params  ----
constexpr int NUM_MISC = MISC_PARAM_NUM;
constexpr const char* misc_param_names[NUM_MISC] = {
  "/misc/1", "/misc/2", "/misc/3", "/misc/4", "/misc/5", "/misc/6"
};
constexpr const char* misc_param_names_global[NUM_MISC] = {
  "/global/misc/1", "/global/misc/2", "/global/misc/3", "/global/misc/4", "/global/misc/5", "/global/misc/6"
};
OSC_receive_msg rcv_misc[NUM_MISC] = {
  OSC_receive_msg(misc_param_names[0]),
  OSC_receive_msg(misc_param_names[1]),
  OSC_receive_msg(misc_param_names[2]),
  OSC_receive_msg(misc_param_names[3]),
  OSC_receive_msg(misc_param_names[4]),
  OSC_receive_msg(misc_param_names[5])
};

OSC_receive_msg rcv_misc_global[NUM_MISC] = {
  OSC_receive_msg(misc_param_names_global[0]),
  OSC_receive_msg(misc_param_names_global[1]),
  OSC_receive_msg(misc_param_names_global[2]),
  OSC_receive_msg(misc_param_names_global[3]),
  OSC_receive_msg(misc_param_names_global[4]),
  OSC_receive_msg(misc_param_names_global[5])
};

float misc_osc[NUM_MISC] = {1.0f};

// ---- Named OSC params ----
OSC_receive_msg rcv_rotation_speed("/rotation/speed");
OSC_receive_msg rcv_rotation_direction("/rotation/direction");
OSC_receive_msg rcv_global_rotation_speed("/global/rotation/speed");
OSC_receive_msg rcv_global_rotation_direction("/global/rotation/direction");

OSC_receive_msg rcv_servo_center("/servo/center");
OSC_receive_msg rcv_servo_angle("/servo/angle");
OSC_receive_msg rcv_servo_mode("/servo/mode");
OSC_receive_msg rcv_servo_direction("/servo/direction");
OSC_receive_msg rcv_servo_calibrate("/servo/calibrate");

OSC_receive_msg rcv_min_distance("/calibration/minDist");
OSC_receive_msg rcv_max_distance("/calibration/maxDist");
OSC_receive_msg rcv_calibration_auto("/calibration/auto");
OSC_receive_msg rcv_global_min_distance("/global/calibration/minDist");
OSC_receive_msg rcv_global_max_distance("/global/calibration/maxDist");
OSC_receive_msg rcv_global_calibration_auto("/global/calibration/auto");

OSC_send_msg snd_ping("/ping");
OSC_send_msg snd_reading("/reading");
OSC_send_msg snd_calibration("/calibration");

// ---- MIDI ----
HardwareSerial MidiSerial(1);

MIDI_CREATE_INSTANCE(HardwareSerial, MidiSerial, midi1);

// ---- Sensor ----
VL53L4CD sensor;
Smoothing sensor_filter;

int sensor_value = 0;
float sensor_value_osc;


float min_distance_osc = 0;
float max_distance_osc = 1;

// ---- Auto-calibration state ----
enum CalPhase {
  CAL_OFF,
  CAL_SETTLE,   // servo moving to the next angle, nothing recorded
  CAL_MEASURE   // servo still, recording one full rotation
};

struct CalRow {
  uint16_t min_mm;
  uint16_t max_mm;
  uint16_t valid;
  uint16_t total;
};

CalPhase cal_phase = CAL_OFF;
CalRow cal_rows[CAL_ANGLE_COUNT];
uint8_t cal_index = 0;
int32_t cal_start_steps = 0;
uint32_t cal_settle_until = 0;
uint32_t cal_deadline = 0;

// Transport state the scan takes over, restored when it ends. Messages arriving
// mid-run are parked here instead of applied.
int cal_saved_speed = 0;
int cal_saved_direction = 1;
enum TransportMode cal_saved_mode = STATIC;

// ---- Speed settings ----
const int SPEED_MIN = 2;
const int SPEED_MAX = 4000; // max freq 0.22 Hz
const int ACCELERATION = 1000;
const int DECELERATION = 1000;

int stepper_speed = SPEED_MIN;

// ---- Transport params----
int angle_dif = ((MAX_ANGLE - MIN_ANGLE) / 2);
int center_angle = MIN_ANGLE + angle_dif;
int angle = center_angle;
int servo_direction = 1;
enum TransportMode transport_mode = STATIC;
float loop_length = 0.1;
bool calibrating = false;

// ---- Midi controls ----
uint8_t start_pos_midi = 0;
uint8_t end_pos_midi = 0;
uint8_t loop_pos = 0;
uint8_t speed_midi = 0;
uint8_t sensor_midi = 0;
uint8_t misc_midi[MISC_PARAM_NUM] = {127};


ESP_FlexyStepper stepper;
int rotation_direction = 1;

Servo servo;

// ---- TaskScheduler ----
Scheduler runner;

// Forward declaration for the stepper disable task callback
void disable_stepper_driver();

Task t_disable_stepper(1000UL, TASK_ONCE, &disable_stepper_driver, &runner, false);

// For direction change: schedule stepper_start after 2s instead of blocking delay
void start_stepper_after_direction_change();
Task t_start_stepper_dir(2000UL, TASK_ONCE, &start_stepper_after_direction_change, &runner, false);
int pending_rotation_direction = 1;


// ---- Sequencer ----
// Object 0 plays the score; started at runtime, once the id is known.
Sequencer sequencer(node);

// ---- Function declarations ----

void stepper_start(int dir);
void stepper_stop();
void servo_tilt(int angle);
void transport();
void midi_send();
void ping();
void send_info();
void sense();

void start_calibration();
void update_calibration(uint16_t raw_mm, bool raw_valid);
void finish_calibration(bool completed);

void servo_osc_callback(OSCMessage& m);
void rotation_osc_callback(OSCMessage& m);
void misc_osc_callback(OSCMessage& m);
void calibration_osc_callback(OSCMessage& m);


// ---- Function definitions ----

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);


  MidiSerial.setPins(RX_PIN, TX_PIN);
  midi1.begin(MIDI_CHANNEL_OMNI);
  
  Serial.begin(115200);
  delay(1000);

  // Sensor init
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000); // use 400 kHz I2C

  sensor.setTimeout(500);
  if (!sensor.init())
  {
    ESP_LOGE(TAG, "Failed to detect and initialize sensor!");
  #if NO_SENSOR == 0
    while (1) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(250);
      digitalWrite(LED_BUILTIN, LOW);
      delay(250);
    }
  #endif
  }

  sensor.setRangeTiming(SENSOR_TIMING_BUDGET, 0);
  sensor.startContinuous();

  // Motors init
  servo.attach(SERVO_PIN);

#if BAD_DRIVER
  // legacy for A4988
  pinMode(MS1_PIN, OUTPUT);
  pinMode(MS2_PIN, OUTPUT);
  pinMode(MS3_PIN, OUTPUT); 
  digitalWrite(MS1_PIN, HIGH);
  digitalWrite(MS2_PIN, HIGH);
  digitalWrite(MS3_PIN, HIGH);
#endif

  runner.startNow();

  stepper.connectToPins(STEP_PIN, DIR_PIN);
  stepper.setSpeedInStepsPerSecond(SPEED_MIN);
  stepper.setAccelerationInStepsPerSecondPerSecond(ACCELERATION);
  stepper.setDecelerationInStepsPerSecondPerSecond(DECELERATION);
  stepper.startAsService(1);
  stepper_start(rotation_direction);

#ifdef OBJ_ID
  // Provisioning build (per-node env): seed the id into NVS over USB.
  g_obj_id = OBJ_ID;
  GeoIdentity::save(OBJ_ID);
  // A USB flash bypasses the OTA bookkeeping; drop the stale installed-md5
  // record so the next mesh OTA offer is not wrongly deduped.
  if (LittleFS.begin(true)) LittleFS.remove("/ota_fw.json");
#else
  // Generic OTA image: identity comes from NVS only.
  g_obj_id = GeoIdentity::load(-1);
#endif
  ESP_LOGI(TAG, "Roto starting up, obj id %d, fw v%d", g_obj_id, FW_VERSION);

  node.setIdentity(g_obj_id, g_obj_id == 0);
  node.begin(NetworkConfig::ssid, NetworkConfig::password);
  ota.begin();

  // Setup OSC receive handlers for misc params
  for (int i = 0; i < NUM_MISC; ++i) {
    rcv_misc[i].init(misc_osc_callback);
    rcv_misc_global[i].init(misc_osc_callback);
  }

  rcv_rotation_speed.init(rotation_osc_callback);
  rcv_rotation_direction.init(rotation_osc_callback);
  rcv_global_rotation_speed.init(rotation_osc_callback);
  rcv_global_rotation_direction.init(rotation_osc_callback);
  rcv_servo_center.init(servo_osc_callback);
  rcv_servo_angle.init(servo_osc_callback);
  rcv_servo_mode.init(servo_osc_callback);
  rcv_servo_direction.init(servo_osc_callback);
  rcv_servo_calibrate.init(servo_osc_callback);
  rcv_min_distance.init(calibration_osc_callback);
  rcv_max_distance.init(calibration_osc_callback);
  rcv_calibration_auto.init(calibration_osc_callback);
  rcv_global_min_distance.init(calibration_osc_callback);
  rcv_global_max_distance.init(calibration_osc_callback);
  rcv_global_calibration_auto.init(calibration_osc_callback);

  snd_ping.init(broadcast_address);
  snd_reading.init(broadcast_address);
  snd_calibration.init(broadcast_address);

  if (g_obj_id == 0) sequencer.start();

  start_calibration();
}

void loop() {
  runner.execute();
  node.update();
  ota.update();
  sequencer.update();

  sense();
  transport();
  midi_send();
  ping();
  send_info();

  // Unprovisioned board (generic image, empty NVS): fast blink. It still joins
  // the mesh and still takes OTA, it just ignores every id-addressed message.
  if (g_obj_id < 0) {
    static uint32_t blink = 0;
    if (millis() - blink >= 150) {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      blink = millis();
    }
  }

#if CORE_DEBUG_LEVEL > 0
  static uint32_t print_elapsed = millis();
  if (millis() - print_elapsed >= 1000) {
    Serial.printf("Sensor: %d, Servo angle: %d, Stepper speed: %d\n", sensor_value, angle, stepper_speed);
    // ESP_LOGD(TAG, "Free Heap: %d, Stack: %d", ESP.getFreeHeap(), uxTaskGetStackHighWaterMark(NULL));
    print_elapsed = millis();
  }
#endif
}

// ---- Function definitions ----

void sense() {
  static uint32_t last_time = 0;
  if (millis() - last_time < SENSOR_PERIOD) return;
  last_time = millis();

#if NO_SENSOR == 0
  sensor.read();
  uint16_t raw_mm = sensor.ranging_data.range_mm;
  bool raw_valid = sensor.ranging_data.range_status == 0;
  sensor_value = sensor_filter.filter(raw_mm);
#else
  sensor_value = (MIN_DISTANCE + MAX_DISTANCE) / 2;
  uint16_t raw_mm = sensor_value;
  bool raw_valid = true;
#endif
  update_calibration(raw_mm, raw_valid);

  sensor_value_osc = constrain((float)(sensor_value - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE), 0.0f, 1.0f);
  int min_d = (int)floor(MIN_DISTANCE + min_distance_osc * (MAX_DISTANCE - MIN_DISTANCE));
  int max_d = (int)floor(MIN_DISTANCE + max_distance_osc * (MAX_DISTANCE - MIN_DISTANCE));
  if (max_d <= min_d)
    min_d--;
  sensor_midi = SCALE_TO_MIDI(sensor_value, min_d, max_d);

  // Update misc_midi from misc_osc
  for (int i = 0; i < NUM_MISC; ++i) {
    misc_midi[i] = FLOAT_TO_MIDI(misc_osc[i]);
  }
}

// === Auto-calibration ===

static inline int cal_angle(uint8_t i) { return MIN_ANGLE + i * CAL_ANGLE_STEP; }

void start_calibration() {
  if (cal_phase != CAL_OFF) return;

  cal_saved_speed = stepper_speed;
  cal_saved_direction = rotation_direction;
  cal_saved_mode = transport_mode;

  // STATIC is what keeps transport() off the servo while the scan holds each angle
  transport_mode = STATIC;
  stepper_speed = CAL_SPEED;
  stepper.setSpeedInStepsPerSecond(CAL_SPEED);
  t_disable_stepper.disable();
  stepper_start(rotation_direction);

  for (int i = 0; i < CAL_ANGLE_COUNT; ++i) cal_rows[i] = {UINT16_MAX, 0, 0, 0};

  cal_index = 0;
  angle = cal_angle(0);
  servo.write(angle);
  cal_settle_until = millis() + CAL_SETTLE_MS;
  cal_phase = CAL_SETTLE;
  cal_deadline = millis() + 2UL * CAL_ANGLE_COUNT *
                 ((1000UL * STEPS_PER_ROTATION) / CAL_SPEED + CAL_SETTLE_MS);

  ESP_LOGI(TAG, "Calibration started, %d angles from %d to %d deg",
           CAL_ANGLE_COUNT, cal_angle(0), cal_angle(CAL_ANGLE_COUNT - 1));
}

void update_calibration(uint16_t raw_mm, bool raw_valid) {
  if (cal_phase == CAL_OFF) return;

  if ((int32_t)(millis() - cal_deadline) >= 0) {
    ESP_LOGW(TAG, "Calibration timed out at angle %d", cal_angle(cal_index));
    finish_calibration(false);
    return;
  }

  // The moving average still carries samples from the previous angle until it has
  // refilled, so a reading only counts once the whole window is in range.
  static uint8_t valid_streak = 0;
  const bool in_range = raw_valid && raw_mm >= MIN_DISTANCE && raw_mm <= MAX_DISTANCE;
  if (in_range) {
    if (valid_streak < NUM_READINGS) valid_streak++;
  } else {
    valid_streak = 0;
  }

  if (cal_phase == CAL_SETTLE) {
    if (millis() < cal_settle_until) return;
    // Latched after settling, so the recorded window is a full 360 degrees
    cal_start_steps = stepper.getCurrentPositionInSteps();
    cal_phase = CAL_MEASURE;
    return;
  }

  CalRow &row = cal_rows[cal_index];
  row.total++;
  if (in_range && valid_streak >= NUM_READINGS) {
    row.valid++;
    if (sensor_value < row.min_mm) row.min_mm = sensor_value;
    if (sensor_value > row.max_mm) row.max_mm = sensor_value;
  }

  // Read once: abs() is a macro and would poll the stepper service twice
  const int32_t turned = stepper.getCurrentPositionInSteps() - cal_start_steps;
  if (abs(turned) < STEPS_PER_ROTATION) return;

  ESP_LOGD(TAG, "Calibration angle %d: %d..%d mm, %d/%d valid",
           cal_angle(cal_index), row.min_mm, row.max_mm, row.valid, row.total);

  if (++cal_index >= CAL_ANGLE_COUNT) {
    finish_calibration(true);
    return;
  }

  angle = cal_angle(cal_index);
  servo.write(angle);
  valid_streak = 0;
  cal_settle_until = millis() + CAL_SETTLE_MS;
  cal_phase = CAL_SETTLE;
}

void finish_calibration(bool completed) {
  int run_start = -1, run_len = 0, best_start = -1, best_len = 0;

  // ponytail: flatness alone can't tell a rotationally symmetric object (cylinder,
  // vase) from the pedestal -- both read constant, and the band collapses to the
  // defaults. Upgrade path is ranging_data.signal_rate_kcps, which the driver
  // already fills in: sky returns no signal whatever the object's shape.
  for (int i = 0; completed && i < CAL_ANGLE_COUNT; ++i) {
    const CalRow &r = cal_rows[i];
    const bool object = r.total > 0 &&
                        r.valid * 100 >= r.total * CAL_MIN_VALID_PCT &&   // else sky
                        (r.max_mm - r.min_mm) >= CAL_FLAT_SPAN_MM;        // else pedestal
    if (!object) {
      run_start = -1;
      continue;
    }
    if (run_start < 0) { run_start = i; run_len = 0; }
    if (++run_len > best_len) { best_len = run_len; best_start = run_start; }
  }

  if (best_len > 0) {
    uint16_t min_mm = UINT16_MAX, max_mm = 0;
    for (int i = best_start; i < best_start + best_len; ++i) {
      if (cal_rows[i].min_mm < min_mm) min_mm = cal_rows[i].min_mm;
      if (cal_rows[i].max_mm > max_mm) max_mm = cal_rows[i].max_mm;
    }
    const int lo_mm = constrain((int)min_mm - CAL_MARGIN_MM, MIN_DISTANCE, MAX_DISTANCE - 1);
    const int hi_mm = constrain((int)max_mm + CAL_MARGIN_MM, lo_mm + 1, MAX_DISTANCE);
    min_distance_osc = (float)(lo_mm - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE);
    max_distance_osc = (float)(hi_mm - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE);

    const int lo_ang = cal_angle(best_start);
    const int hi_ang = cal_angle(best_start + best_len - 1);
    center_angle = (lo_ang + hi_ang) / 2;
    angle_dif = (hi_ang - lo_ang) / 2;

    ESP_LOGI(TAG, "Calibration done: %d..%d mm, angles %d..%d", lo_mm, hi_mm, lo_ang, hi_ang);
  } else {
    ESP_LOGW(TAG, "Calibration found no object band, keeping previous values");
  }

  cal_phase = CAL_OFF;
  transport_mode = cal_saved_mode;
  angle = center_angle;
  servo.write(center_angle);

  stepper_speed = cal_saved_speed;
  stepper.setSpeedInStepsPerSecond(stepper_speed > 0 ? stepper_speed : 1);
  if (stepper_speed == 0) {
    stepper_stop();
  } else if (cal_saved_direction != rotation_direction) {
    rotation_direction = cal_saved_direction;
    stepper_stop();
    pending_rotation_direction = rotation_direction;
    t_start_stepper_dir.restartDelayed(2000UL);
  } else {
    stepper_start(rotation_direction);
  }

  snd_calibration.m.add((float)g_obj_id);
  snd_calibration.m.add(min_distance_osc);
  snd_calibration.m.add(max_distance_osc);
  snd_calibration.m.add((float)center_angle);
  snd_calibration.m.add((float)angle_dif);
  node.send_info(snd_calibration.m);
}

void stepper_start(int dir) {
  digitalWrite(EN_PIN, LOW);
  stepper.startJogging(dir);
  ESP_LOGD(TAG, "Stepper started with speed %d", stepper_speed);
}

void disable_stepper_driver() {
    digitalWrite(EN_PIN, HIGH);
    ESP_LOGD(TAG, "Stepper disabled (TaskScheduler)");
}

void stepper_stop() {
    stepper.stopJogging();
    t_disable_stepper.restartDelayed(1000UL);
    ESP_LOGD(TAG, "Stepper stop requested (non-blocking)");
}


void start_stepper_after_direction_change() {
    stepper_start(pending_rotation_direction);
    ESP_LOGD(TAG, "Stepper started after direction change (TaskScheduler)");
}

void transport() {
  // calculate servo speed based on stepper speed
  uint16_t servo_period = (1000 * STEPS_PER_ROTATION) / ((stepper_speed > 0 ? stepper_speed : 1) * DEGREES_PER_ROTATION);
  const uint16_t min_angle = center_angle - angle_dif;
  const uint16_t max_angle = center_angle + angle_dif;

  // servo control
  static uint32_t servo_elapsed = millis();
  if (millis() - servo_elapsed >= servo_period && transport_mode != STATIC && !calibrating) {

    servo.write(angle);
    angle += servo_direction;

    if (transport_mode == LINEAR) {
      if (servo_direction > 0 && angle >= max_angle) {
        angle = min_angle;
      } else if (servo_direction < 0 && angle <= min_angle) {
        angle = max_angle;
      }
    } else if (transport_mode == ALTERNATING) {
      if (abs(center_angle - angle) >= angle_dif) {
        servo_direction = -servo_direction;
      }
    }

    servo_elapsed = millis();
  }

  start_pos_midi = SCALE_TO_MIDI(angle, min_angle, max_angle);
  end_pos_midi = (start_pos_midi + FLOAT_TO_MIDI(loop_length)) % 128;
}

// === OSC Callbacks ===

void servo_osc_callback(OSCMessage& m) {
  if (m.size() < 2) {
    ESP_LOGW(TAG, "Servo message with insufficient args");
    return;
  }
  if (m.getFloat(0) != (float)g_obj_id) {
    return;
  }

  String adr = m.getAddress();

  // The scan holds the servo itself and derives both from what it measures
  if (cal_phase != CAL_OFF && (adr.endsWith("/center") || adr.endsWith("/angle"))) return;

  // Parked while calibrating: takes effect when the scan restores the transport
  enum TransportMode &mode_target = (cal_phase != CAL_OFF) ? cal_saved_mode : transport_mode;

  if (adr.endsWith("/center")) {
    center_angle = MIN_ANGLE + int(floor(m.getFloat(1) * (MAX_ANGLE - MIN_ANGLE)));
    servo.write(center_angle);
    ESP_LOGD(TAG, "Center angle: %d", center_angle);
  } 
  else if (adr.endsWith("/angle")) {
    int a = int(m.getFloat(1) * ((MAX_ANGLE - MIN_ANGLE) / 2));

    if (center_angle + a > MAX_ANGLE) a = MAX_ANGLE - center_angle;
    else if (center_angle - a < 0)    a = center_angle - MIN_ANGLE;
    angle_dif = a;
    ESP_LOGD(TAG, "Angle dif: %d", angle_dif);
  } 
  else if (adr.endsWith("/mode")) {
    switch ((int)m.getFloat(1)) {
      case 0:
        mode_target = STATIC;
        ESP_LOGD(TAG, "Transport mode: STATIC");
        break;
      case 1:
        mode_target = LINEAR;
        ESP_LOGD(TAG, "Transport mode: LINEAR");
        break;
      case 2:
        mode_target = ALTERNATING;
        ESP_LOGD(TAG, "Transport mode: ALTERNATING");
        break;
    }
  } 
  else if (adr.endsWith("/direction")) {
    servo_direction = m.getFloat(1) > 0 ? -1 : 1;
    ESP_LOGD(TAG, "Direction: %d", servo_direction);
  } 
  else if (adr.endsWith("/calibrate")) {
    calibrating = m.getFloat(1) > 0;
    ESP_LOGD(TAG, "Calibrating: %d", calibrating ? 1 : 0);
  }
}

void rotation_osc_callback(OSCMessage& m) {
  String adr = m.getAddress();
  bool global = adr.startsWith("/toRoto/global/");
  int val_idx = global ? 0 : 1;

  if (!global) {
    if (m.size() < 2) {
        ESP_LOGW(TAG, "Param message with insufficient args");
        return;
    }
    if (m.getFloat(0) != (float)g_obj_id) {
        // not global and not for this object
        return;
    }
  } else {
    if (m.size() < 1) {
        ESP_LOGW(TAG, "Invalid global rotation message");
        return;
    }
  }

  static int last_speed = 1;

  // The scan needs a fixed speed and direction; park what arrives mid-run so it
  // takes effect when the scan restores the transport. last_speed has to move with
  // it, or the restored speed and the start/stop edge detection disagree.
  if (cal_phase != CAL_OFF) {
    if (adr.endsWith("/speed")) {
      cal_saved_speed = (int)(floor(m.getFloat(val_idx) * SPEED_MAX));
      last_speed = cal_saved_speed;
      speed_midi = FLOAT_TO_MIDI((float)(cal_saved_speed) / (SPEED_MAX));
    } else if (adr.endsWith("/direction")) {
      cal_saved_direction = m.getFloat(val_idx) > 0 ? -1 : 1;
    }
    return;
  }

  if (adr.endsWith("/speed")) {
    stepper_speed = (int)(floor(m.getFloat(val_idx) * SPEED_MAX));
    ESP_LOGD(TAG, "Speed: %d", stepper_speed);
    stepper.setSpeedInStepsPerSecond(stepper_speed);
    
    if (stepper_speed == 0 && last_speed > 0) {
      stepper_stop();
    } else if (stepper_speed > 0 && last_speed == 0 ) {
      stepper_start(rotation_direction);
    }
    last_speed = stepper_speed;

    speed_midi = FLOAT_TO_MIDI((float)(stepper_speed) / (SPEED_MAX));
  }
  else if (adr.endsWith("/direction")) {
    rotation_direction = m.getFloat(val_idx) > 0 ? -1 : 1;
    ESP_LOGD(TAG, "Rotation direction: %d", rotation_direction);

    stepper_stop();
    pending_rotation_direction = rotation_direction;
    t_start_stepper_dir.restartDelayed(2000UL);
  }
}


void calibration_osc_callback(OSCMessage& m) {
  String adr = m.getAddress();
  bool global = adr.startsWith("/toRoto/global/");
  int val_idx = global ? 0 : 1;

  if (!global) {
    if (m.size() < 2) {
      ESP_LOGW(TAG, "Invalid calibration message");
      return;
    }
    if (m.getFloat(0) != (float)g_obj_id) {
      // not global and not for this object
      return;
    }
  } else {
    if (m.size() < 1) {
      ESP_LOGW(TAG, "Invalid global calibration message");
      return;
    }
  }

  if (adr.endsWith("/auto")) {
    if (m.getFloat(val_idx) > 0) start_calibration();
  }
  else if (adr.endsWith("/minDist")) {
    min_distance_osc = constrain(m.getFloat(val_idx), 0.0f, 1.0f);
    ESP_LOGD(TAG, "Min distance: %d", (int)floor(MIN_DISTANCE + min_distance_osc * (MAX_DISTANCE - MIN_DISTANCE)));
  }
  else if (adr.endsWith("/maxDist")) {
    max_distance_osc = constrain(m.getFloat(val_idx), 0.0f, 1.0f);
    ESP_LOGD(TAG, "Max distance: %d", (int)floor(MIN_DISTANCE + max_distance_osc * (MAX_DISTANCE - MIN_DISTANCE)));
  }
}


void misc_osc_callback(OSCMessage& msg) {
  String addr = msg.getAddress();
  bool global = addr.startsWith("/toRoto/global/");
  int val_idx = global ? 0 : 1;

  if (!global) {
    if (msg.size() < 2) {
      ESP_LOGW(TAG, "Invalid misc message");
      return;
    }
    if (msg.getFloat(0) != (float)g_obj_id) {
      return;
    }
  } else {
    if (msg.size() < 1) {
      ESP_LOGW(TAG, "Invalid global misc message");
      return;
    }
  }

  for (int i = 0; i < NUM_MISC; ++i) {
    if (addr.endsWith(misc_param_names[i])) {
      misc_osc[i] = msg.getFloat(val_idx);
      ESP_LOGD(TAG, "Misc param %s: %f", misc_param_names[i], misc_osc[i]);
      break;
    }
  } 
}


void ping() {
  static uint32_t last_time = 0;
  if (millis() - last_time < 5000) return;
  last_time = millis();
  ESP_LOGD(TAG, "Ping broadcast");

  snd_ping.m.add((float)g_obj_id);
  snd_ping.m.add((float)FW_VERSION);
  node.send_info(snd_ping.m);
}

void send_info() {
  // Every reading crosses the whole mesh, so don't chatter: skip the send while
  // the sensor is sitting still.
  const float READING_EPSILON = 0.005f;
  static uint32_t last_time = 0;
  static float last_sent = -1;
  if (millis() - last_time < READING_PERIOD) return;
  if (fabsf(sensor_value_osc - last_sent) < READING_EPSILON) return;
  last_time = millis();
  last_sent = sensor_value_osc;

  snd_reading.m.add((float)g_obj_id);
  snd_reading.m.add((float)sensor_value_osc);

  node.send_info(snd_reading.m);
}


// === MIDI sending ===

void midi_send() {
  const uint8_t params_period = 60;       // change-poll rate for the slow params
  const uint16_t refresh_period = 1000;   // full resend, so a late receiver syncs
  const uint8_t NUM_PARAMS = 3 + MISC_PARAM_NUM;
  static uint32_t params_time = 0;
  static uint32_t refresh_time = 0;
  static uint8_t last_sensor = 0xFF;
  static uint8_t last_params[NUM_PARAMS] = {0};

  // CC 1 - sensor value. Sent the moment sense() produces a new one; a timer
  // here would only add latency to the one value that can least afford it.
  if (sensor_midi != last_sensor) {
    midi1.sendControlChange(1, sensor_midi, 1);
    last_sensor = sensor_midi;
  }

  if (millis() - params_time < params_period) return;
  params_time = millis();

  // At 31250 baud each CC costs ~1 ms of wire time, so resending nine unchanged
  // params every tick would sit in front of the sensor value.
  const bool refresh = millis() - refresh_time >= refresh_period;
  if (refresh) refresh_time = millis();

  // CC 2 - loop start, CC 3 - loop end, CC 4 - speed, CC 5... - misc
  uint8_t params[NUM_PARAMS] = {start_pos_midi, end_pos_midi, speed_midi};
  for (int i = 0; i < MISC_PARAM_NUM; i++) {
    params[3 + i] = misc_midi[i];
  }

  for (int i = 0; i < NUM_PARAMS; i++) {
    if (!refresh && params[i] == last_params[i]) continue;
    midi1.sendControlChange(2 + i, params[i], 1);
    last_params[i] = params[i];
  }
}
