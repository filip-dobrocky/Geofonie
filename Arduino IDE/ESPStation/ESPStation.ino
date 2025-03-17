#include <ArduinoOSCWiFi.h>
#include <MIDI.h>
#include <Wire.h>
#include <VL53L4CD.h>
#include <ESP_FlexyStepper.h>
#include <ESP32Servo.h>

#include "../NetworkConfig.h"
#include "Smoothing.h"

// Constants

#define OBJ_ID 3
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

#define MAX_ANGLE 110
#define MIN_ANGLE 70

#define MIN_DISTANCE 100
#define MAX_DISTANCE 500

#define STEPS_PER_ROTATION 13500
#define DEGREES_PER_ROTATION 10

// Macros

#define SCALE_TO_MIDI(val, min, max) map(constrain((val), (min), (max)), (min), (max), 0, 127)
#define FLOAT_TO_MIDI(val) (int)constrain(floor((val) * 127.0), 0, 127)

// Enums

enum TransportMode {
  LINEAR,
  ALTERNATING
};

// Globals

bool connected = false;

HardwareSerial MidiSerial(1);

MIDI_CREATE_INSTANCE(HardwareSerial, MidiSerial, midi1);

VL53L4CD sensor;

Smoothing sensor_filter;

hw_timer_t *midi_timer = NULL;

const IPAddress ip(GET_ROTO_IP(OBJ_ID));
const IPAddress gateway(NetworkConfig::gateway);
const IPAddress subnet(NetworkConfig::subnet);

int sensor_value = 0;
float sensor_value_osc;

float min_distance_osc = 0;
float max_distance_osc = 1;
float misc1_osc = 0;
float misc2_osc = 0;
float misc3_osc = 0;

// Speed settings
const int SPEED_MIN = 2;
const int SPEED_MAX = 4000; // max freq 0.22 Hz
const int ACCELERATION = 1000;
const int DECELERATION = 1000;

int stepper_speed = SPEED_MIN;

// Transport params
int angle_dif = ((MAX_ANGLE - MIN_ANGLE) / 2);
int center_angle = MIN_ANGLE + angle_dif;
int angle = center_angle;
int servo_direction = 1;
enum TransportMode transport_mode = LINEAR;
float loop_length = 0.1;
bool calibration = false;

// Midi controls
uint8_t start_pos_midi = 0;
uint8_t end_pos_midi = 0;
uint8_t loop_pos = 0;
uint8_t speed_midi = 0;
uint8_t sensor_midi = 0;
uint8_t misc1_midi = 0;
uint8_t misc2_midi = 0;
uint8_t misc3_midi = 0;

ESP_FlexyStepper stepper;
int rotation_direction = 1;

Servo servo;

//Functions

int normalize_distance(int distance, int radius, int angle) {
  int difference = radius - radius / cos(DEG_TO_RAD * angle);
  return distance - difference;
}

void wifi_init() {
  // WiFi init
  WiFi.disconnect(true, true);
  delay(1000);
  WiFi.mode(WIFI_STA);

  WiFi.begin(NetworkConfig::ssid, NetworkConfig::password);
  WiFi.config(ip, gateway, subnet);

  uint8_t timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 10) {

    Serial.print(".");
    delay(500);
    static int count = 0;
    if (count++ > 20) {
      Serial.println("WiFi connection timeout, retry");
      WiFi.begin(NetworkConfig::ssid, NetworkConfig::password);
      count = 0;
    }
    timeout++;
  }
  
  if (timeout < 10) {
    Serial.print("WiFi connected, IP = ");
    Serial.println(WiFi.localIP());
    digitalWrite(LED_BUILTIN, LOW);
    connected = true;
  } else {
    Serial.print("WiFi connection failed");
    digitalWrite(LED_BUILTIN, HIGH);
    connected = false;
  }
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

void servo_tilt(int angle) {
  // while (!servo.attached()) {
  //   servo.attach(SERVO_PIN);
  // }

  servo.write(angle);
  // delay(5);
  // servo.detach();
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
    servo_tilt(angle);
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


void servo_osc(const OscMessage& m) {
  String adr = m.address();

  if (adr.endsWith("/center")) {
    center_angle = MIN_ANGLE + int(floor(m.arg<float>(0) * (MAX_ANGLE - MIN_ANGLE)));
    servo_tilt(center_angle);
  } 
  else if (adr.endsWith("/angle")) {
    int a = int(m.arg<float>(0) * ((MAX_ANGLE - MIN_ANGLE) / 2));
    
    if (center_angle + a > MAX_ANGLE) a = MAX_ANGLE - center_angle;
    else if (center_angle - a < 0)    a = center_angle - MIN_ANGLE;
    angle_dif = a;
  } 
  else if (adr.endsWith("/mode")) {
    transport_mode = m.arg<float>(0) > 0 ? ALTERNATING : LINEAR;
  } 
  else if (adr.endsWith("/direction")) {
    servo_direction = m.arg<float>(0) > 0 ? -1 : 1;
  } 
  else if (adr.endsWith("/calibrate")) {
    calibration = m.arg<float>(0) > 0;
  }
}

void rotation_osc(const OscMessage& m) {
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


void IRAM_ATTR midi_ISR() {
  // NO CALCULATIONS HERE
  // CC 1 - sensor value
  midi1.sendControlChange(1, sensor_midi, 1);
  // CC 2 - loop start position
  midi1.sendControlChange(2, start_pos_midi, 1);
  // CC 3 - loop end position
  midi1.sendControlChange(3, end_pos_midi, 1);
  // CC 4 - loop speed
  midi1.sendControlChange(4, speed_midi, 1);
  // CC 5 - misc1
  midi1.sendControlChange(5, misc1_midi, 1);
  // CC 6 - misc2
  midi1.sendControlChange(6, misc2_midi, 1);
  // CC 7 - misc3
  midi1.sendControlChange(7, misc3_midi, 1);
}


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  pinMode(MS1_PIN, OUTPUT);
  pinMode(MS2_PIN, OUTPUT);
  pinMode(MS3_PIN, OUTPUT);

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

  if (BAD_DRIVER) {
    digitalWrite(MS1_PIN, HIGH);
    digitalWrite(MS2_PIN, HIGH);
    digitalWrite(MS3_PIN, HIGH);
  } else {
    digitalWrite(MS1_PIN, LOW);
    digitalWrite(MS2_PIN, LOW);
    digitalWrite(MS3_PIN, LOW);
  }
  stepper.connectToPins(STEP_PIN, DIR_PIN);
  stepper.setSpeedInStepsPerSecond(SPEED_MIN);
  stepper.setAccelerationInStepsPerSecondPerSecond(ACCELERATION);
  stepper.setDecelerationInStepsPerSecondPerSecond(DECELERATION);
  stepper.startAsService(1);
  stepper_start(rotation_direction);

  Serial.println("Motors initialized");

  wifi_init();

  if (connected) {
    // Subscribe to OSC messages
    OscWiFi.subscribe(NetworkConfig::osc_from_ap, "/toRoto/servo/*", servo_osc);
    OscWiFi.subscribe(NetworkConfig::osc_from_ap, "/toRoto/rotation/*", rotation_osc);
    OscWiFi.subscribe(NetworkConfig::osc_from_ap, "/toRoto/calibration/minDist", min_distance_osc);
    OscWiFi.subscribe(NetworkConfig::osc_from_ap, "/toRoto/calibration/maxDist", max_distance_osc);
    OscWiFi.subscribe(NetworkConfig::osc_from_ap, "/toRoto/misc/1", misc1_osc);
    OscWiFi.subscribe(NetworkConfig::osc_from_ap, "/toRoto/misc/2", misc2_osc);
    OscWiFi.subscribe(NetworkConfig::osc_from_ap, "/toRoto/misc/3", misc3_osc);

    OscWiFi.publish("255.255.255.255", NetworkConfig::osc_info, "/info/reading", OBJ_ID, sensor_value_osc)
        ->setFrameRate(10);
  }

  // Timers
  midi_timer = timerBegin(1000);
  timerAttachInterrupt(midi_timer, &midi_ISR);
  timerAlarm(midi_timer, 10, true, 0);
}

void loop() {
  if (connected) {
    OscWiFi.update();
  }

  sensor.read();
  sensor_value = sensor_filter.filter(sensor.ranging_data.range_mm);
  sensor_value_osc = constrain((float)(sensor_value - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE), 0.0f, 1.0f);
  int min_d = (int)floor(MIN_DISTANCE + min_distance_osc * (MAX_DISTANCE - MIN_DISTANCE));   
  int max_d = (int)floor(MIN_DISTANCE + max_distance_osc * (MAX_DISTANCE - MIN_DISTANCE));
  if (max_d <= min_d)
    min_d--;
  sensor_midi = SCALE_TO_MIDI(sensor_value, min_d, max_d);


  Serial.print("Sensor distance: ");
  Serial.print(sensor_value);
  Serial.print(" Stepper speed ");
  Serial.print(stepper_speed);
  Serial.print(" Servo angle ");
  Serial.println(angle);

  misc1_midi = FLOAT_TO_MIDI(misc1_osc);
  misc2_midi = FLOAT_TO_MIDI(misc2_osc);
  misc3_midi = FLOAT_TO_MIDI(misc3_osc);
  
  transport();
}
