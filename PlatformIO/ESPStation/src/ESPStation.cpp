#include <Arduino.h>
#include <painlessMesh.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <MIDI.h>
#include <Wire.h>
#include <VL53L4CD.h>
#include <ESP_FlexyStepper.h>
#include <ESP32Servo.h>
#include "osc_control.h"

#include "NetworkConfig.h"
#include "GEOUtils.h"
#include "Smoothing.h"

// Constants

#define OBJ_ID 1
#define BAD_DRIVER 0

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

#define MIN_ANGLE 40
#define MAX_ANGLE 99

#define MIN_DISTANCE 200
#define MAX_DISTANCE 400

#define STEPS_PER_ROTATION 13500
#define DEGREES_PER_ROTATION 10

#define MISC_PARAM_NUM 6


// ---- Enums ----
enum TransportMode {
  STATIC,
  LINEAR,
  ALTERNATING
};

// ---- Globals ----
painlessMesh mesh;
WiFiUDP udp;
const char *base_address = "/toRoto";
const char *broadcast_address = nullptr;

// ---- Misc OSC params  ----
constexpr int NUM_MISC = MISC_PARAM_NUM;
constexpr const char* misc_param_names[NUM_MISC] = {
  "/misc/1", "/misc/2", "/misc/3", "/misc/4", "/misc/5", "/misc/6"
};
OSC_receive_msg rcv_misc[NUM_MISC] = {
  OSC_receive_msg(misc_param_names[0]),
  OSC_receive_msg(misc_param_names[1]),
  OSC_receive_msg(misc_param_names[2]),
  OSC_receive_msg(misc_param_names[3]),
  OSC_receive_msg(misc_param_names[4]),
  OSC_receive_msg(misc_param_names[5])
};

float misc_osc[NUM_MISC] = {1.0f};

// ---- Named OSC params ----
OSC_receive_msg rcv_rotation_speed("/rotation/speed");
OSC_receive_msg rcv_rotation_direction("/rotation/direction");

OSC_receive_msg rcv_servo_center("/servo/center");
OSC_receive_msg rcv_servo_tilt("/servo/tilt");
OSC_receive_msg rcv_servo_angle("/servo/angle");
OSC_receive_msg rcv_servo_mode("/servo/mode");
OSC_receive_msg rcv_servo_direction("/servo/direction");
OSC_receive_msg rcv_servo_calibrate("/servo/calibrate");

OSC_receive_msg rcv_min_distance("/calibration/min_distance");
OSC_receive_msg rcv_max_distance("/calibration/max_distance");

// ---- MIDI ----
HardwareSerial MidiSerial(1);

MIDI_CREATE_INSTANCE(HardwareSerial, MidiSerial, midi1);

// ---- Sensor ----
VL53L4CD sensor;

Smoothing sensor_filter;

// --- Network ---
const IPAddress ip(GET_ROTO_IP(OBJ_ID));
const IPAddress gateway(NetworkConfig::gateway);
const IPAddress subnet(NetworkConfig::subnet);

int sensor_value = 0;
float sensor_value_osc;


float min_distance_osc = 0;
float max_distance_osc = 1;

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
bool calibration = false;

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

// ---- Function declarations ----

int normalize_distance(int distance, int radius, int angle);
void stepper_start(int dir);
void stepper_stop();
void servo_tilt(int angle);
void transport();
void midi_send();

void servo_osc_callback(OSCMessage& m);
void rotation_osc_callback(OSCMessage& m);
void misc_osc_callback(OSCMessage& m);



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
    Serial.println("Failed to detect and initialize sensor!");
    while (1) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(250);
      digitalWrite(LED_BUILTIN, LOW);
      delay(250);
    }
  }

  sensor.setRangeTiming(10, 0);
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

  stepper.connectToPins(STEP_PIN, DIR_PIN);
  stepper.setSpeedInStepsPerSecond(SPEED_MIN);
  stepper.setAccelerationInStepsPerSecondPerSecond(ACCELERATION);
  stepper.setDecelerationInStepsPerSecondPerSecond(DECELERATION);
  stepper.startAsService(1);
  stepper_start(rotation_direction);

  Serial.println("Motors initialized");

  // Mesh network init
  mesh.init(
    NetworkConfig::ssid, NetworkConfig::password,
    NetworkConfig::mesh_port, WIFI_AP_STA, 1, 0, NetworkConfig::max_conn
  );
  udp.begin(NetworkConfig::osc_from_ap);

  // Setup OSC receive handlers for misc params
  for (int i = 0; i < NUM_MISC; ++i) {
    rcv_misc[i].init(misc_osc_callback);
  }

  // Setup OSC receive handlers for named messages
  // These remain as your original callbacks:
  //   servo_osc, rotation_osc, min_distance_osc, max_distance_osc, misc_osc_callback
  // Example using osc_control API:
  //   OSC_receive_msg rcv_servo("/servo/*");
  //   rcv_servo.init(servo_osc);
  //   ...
  //   (repeat for other named messages and callbacks)
  //   (You may need to adapt the callback signatures if needed)
}

void loop() {
  mesh.update();
  osc_control_loop(udp, base_address, broadcast_address);

  sensor.read();
  sensor_value = sensor_filter.filter(sensor.ranging_data.range_mm);
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

  Serial.print("Sensor distance: ");
  Serial.print(sensor_value);
  Serial.print(" Stepper speed ");
  Serial.print(stepper_speed);
  Serial.print(" Servo angle ");
  Serial.println(angle);

  transport();
  midi_send();
}

// ---- Function definitions ----

int normalize_distance(int distance, int radius, int angle) {
  int difference = radius - radius / cos(DEG_TO_RAD * angle);
  return distance - difference;
}

void stepper_start(int dir) {
  digitalWrite(EN_PIN, LOW);
  stepper.startJogging(dir);
}

void stepper_stop() {
  stepper.stopJogging();
  delay(1000);
  digitalWrite(EN_PIN, HIGH);
  Serial.println("stop");
}

void transport() {
  // calculate servo speed based on stepper speed
  uint16_t servo_period = (1000 * STEPS_PER_ROTATION) / ((stepper_speed > 0 ? stepper_speed : 1) * DEGREES_PER_ROTATION);
  const uint16_t min_angle = center_angle - angle_dif;
  const uint16_t max_angle = center_angle + angle_dif;

  // Serial.print("Servo period: ");
  // Serial.println(servo_period);

  // servo control
  static uint32_t servo_elapsed = millis();
  if (millis() - servo_elapsed >= servo_period && !calibration) {
    Serial.println("Servo period");
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

void servo_osc(const OSCMessage& m) {
  String adr = m.address();

  if (adr.endsWith("/center")) {
    center_angle = MIN_ANGLE + int(floor(m.arg<float>(0) * (MAX_ANGLE - MIN_ANGLE)));
    servo.write(center_angle);
  } 
  else if (adr.endsWith("/angle")) {
    int a = int(m.arg<float>(0) * ((MAX_ANGLE - MIN_ANGLE) / 2));
    
    if (center_angle + a > MAX_ANGLE) a = MAX_ANGLE - center_angle;
    else if (center_angle - a < 0)    a = center_angle - MIN_ANGLE;
    angle_dif = a;
  } 
  else if (adr.endsWith("/mode")) {
    switch ((int)m.arg<float>(0)) {
      case 0:
        transport_mode = STATIC;
        break;
      case 1:
        transport_mode = LINEAR;
        break;
      case 2:
        transport_mode = ALTERNATING;
        break;
    }
  } 
  else if (adr.endsWith("/direction")) {
    servo_direction = m.arg<float>(0) > 0 ? -1 : 1;
  } 
  else if (adr.endsWith("/calibrate")) {
    calibration = m.arg<float>(0) > 0;
  }
}

void rotation_osc(const OSCMessage& m) {
  String adr = m.address();
  static int last_speed = 1;

  if (adr.endsWith("/speed")) {
    stepper_speed = (int)(floor(m.arg<float>(0) * SPEED_MAX));
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
    rotation_direction = m.arg<float>(0) > 0 ? -1 : 1;
    stepper_stop();
    delay(1000);
    stepper_start(rotation_direction);
  }
}


void misc_osc_callback(const OSCMessage& msg) {
  if (msg.size() < 2) return;
  const char* addr = msg.getAddress();

  for (int i = 0; i < NUM_MISC; ++i) {
    if (strcmp(addr, misc_param_names[i]) == 0) {
      if (msg.getFloat(0) == (float)OBJ_ID) {
        misc_osc[i] = msg.getFloat(1);
      }
      break;
    }
  } 
}

void midi_send() {
  const uint8_t sensor_period = 10;
  const uint8_t params_period = 60;
  static uint16_t sensor_time = 0;
  static uint16_t params_time = 0;

  // CC 1 - sensor value
  if (millis() - sensor_time >= sensor_period) {
    midi1.sendControlChange(1, sensor_midi, 1);
    sensor_time = millis();
  }

  if (millis() - params_time >= params_period) {
    // CC 2 - loop start position
    midi1.sendControlChange(2, start_pos_midi, 1);
    // CC 3 - loop end position
    midi1.sendControlChange(3, end_pos_midi, 1);
    // CC 4 - loop speed
    midi1.sendControlChange(4, speed_midi, 1);
    // CC 5... - misc
    for (int i = 0; i < MISC_PARAM_NUM; i++) {
      midi1.sendControlChange(5 + i, misc_midi[i], 1);
    }
    params_time = millis();
  }
}