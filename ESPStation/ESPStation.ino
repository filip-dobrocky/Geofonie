#include <ArduinoOSCWiFi.h>
#include <MIDI.h>
#include <Wire.h>
#include <VL53L4CD.h>
#include <ESP_FlexyStepper.h>
#include <ESP32Servo.h>

#include "../NetworkConfig.h"
#include "Smoothing.h"

// Constants

#define OBJ_ID 0

#define RX_PIN D7
#define TX_PIN D8

#define I2C_SDA D9
#define I2C_SCL D10

#define STEP_PIN D0
#define DIR_PIN D1
#define EN_PIN D2

#define SERVO_PIN D3

// Macros

#define SCALE_TO_MIDI(val, min, max) map(constrain(val, min, max), min, max, 0, 127)
#define FLOAT_TO_MIDI(val) constrain(int(val * 127.0), 0, 127)

// Enums

enum TransportMode {
  LINEAR,
  ALTERNATING
};

// Globals

HardwareSerial MidiSerial(1);

MIDI_CREATE_INSTANCE(HardwareSerial, MidiSerial, midi1);

VL53L4CD sensor;

Smoothing sensor_filter;

hw_timer_t *midi_timer = NULL;

const IPAddress ip(GET_ROTO_IP(OBJ_ID));
const IPAddress gateway(NetworkConfig::gateway);
const IPAddress subnet(NetworkConfig::subnet);

int led_brightness = 0;
int sensor_value = 0;

int stepper_speed = 1;

// Speed settings
const int SPEED_MIN = 10;
const int SPEED_MAX = 10000;
const int ACCELERATION = 500;
const int DECELERATION = 800;

// Transport params
int center_angle = 0;
int angle_dif = 30;
int angle = center_angle - angle_dif;
int servo_direction = 1;
enum TransportMode transport_mode = LINEAR;
float loop_length = 0.1;

// Midi controls
uint8_t start_pos;
uint8_t end_pos;
uint8_t loop_pos = 0;

ESP_FlexyStepper stepper;
int rotation_direction = 1;

Servo servo;

//Functions

int normalize_distance(int distance, int radius, int angle) {
  int difference = radius - radius / cos(DEG_TO_RAD * angle);
  return distance - difference;
}

void stepper_start(int dir) {
  digitalWrite(EN_PIN, LOW);
  stepper.startJogging(dir);
}

void stepper_stop() {
  digitalWrite(EN_PIN, HIGH);
  stepper.stopJogging();
  delay(1000);
  digitalWrite(EN_PIN, HIGH);
}

void servo_tilt(int angle) {
  if (!servo.attached()) {
    servo.attach(SERVO_PIN);
  }

  servo.write(angle);
  
  servo.detach();
}

void transport() {
  // calculate servo speed based on stepper speed
  uint16_t servo_period = SPEED_MAX / stepper_speed;
  const uint16_t min_angle = center_angle - angle_dif;
  const uint16_t max_angle = center_angle + angle_dif;

  // servo control
  static uint32_t servo_elapsed = millis();
  if (millis() - servo_elapsed >= servo_period) {
    servo_tilt(angle);
    angle += servo_direction;

    if (transport_mode == LINEAR) {
      if (servo_direction > 0 && angle >= max_angle) {
        angle = min_angle;
      } else if (servo_direction < 0 && angle <= min_angle) {
        angle = max_angle;
      }
    } else if (transport_mode == ALTERNATING) {
      if (abs(center_angle - angle) >= max_angle) {
        servo_direction = -direction;
      }
    }

    servo_period = millis();
  }

  start_pos = SCALE_TO_MIDI(angle, min_angle, max_angle);
  end_pos = (start_pos + FLOAT_TO_MIDI(loop_length)) % 128;
}


void IRAM_ATTR midi_ISR() {
  // CC 1 - sensor value
  midi1.sendControlChange(1, SCALE_TO_MIDI(sensor_value, 10, 800), 1);
  // CC 2 - loop start position
  midi1.sendControlChange(2, start_pos, 1);
  // CC 3 - loop end position
  midi1.sendControlChange(3, end_pos, 1);
  // CC 4 - loop speed
  midi1.sendControlChange(4, SCALE_TO_MIDI(stepper_speed, SPEED_MIN, SPEED_MAX), 1);
}


void setup() {
  MidiSerial.setPins(RX_PIN, TX_PIN);
  midi1.begin(MIDI_CHANNEL_OMNI);
  
  Serial.begin(115200);
  delay(1000);

  //Sensor init
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000); // use 400 kHz I2C

  sensor.setTimeout(500);
  if (!sensor.init())
  {
    Serial.println("Failed to detect and initialize sensor!");
    while (1);
  }

  sensor.setRangeTiming(20, 0);
  sensor.startContinuous();

  // WiFi init
  WiFi.disconnect(true, true);
  delay(1000);
  WiFi.mode(WIFI_STA);

  WiFi.begin(NetworkConfig::ssid, NetworkConfig::password);
  WiFi.config(ip, gateway, subnet);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    static int count = 0;
    if (count++ > 20) {
      Serial.println("WiFi connection timeout, retry");
      WiFi.begin(NetworkConfig::ssid, NetworkConfig::password);
      count = 0;
    }
  }

  Serial.print("WiFi connected, IP = ");
  Serial.println(WiFi.localIP());

  // Subscribe to OSC messages
  OscWiFi.subscribe(NetworkConfig::osc_from_ap, "/1/brightness", led_brightness);

  // Timers
  midi_timer = timerBegin(1000);
  timerAttachInterrupt(midi_timer, &midi_ISR);
  timerAlarm(midi_timer, 10, true, 0);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);

  stepper.connectToPins(STEP_PIN, DIR_PIN);
  stepper.setSpeedInStepsPerSecond(SPEED_MIN);
  stepper.setAccelerationInStepsPerSecondPerSecond(ACCELERATION);
  stepper.setDecelerationInStepsPerSecondPerSecond(DECELERATION);
  stepper.startAsService(1);

  stepper_start(rotation_direction);

  digitalWrite(DIR_PIN, HIGH);
}

void loop() {
  OscWiFi.update();

  sensor.read();
  sensor_value = normalize_distance(sensor_filter.filter(sensor.ranging_data.range_mm), 250, angle);  
  Serial.print("Sensor distance: ");
  Serial.println(sensor_value);

  digitalWrite(LED_BUILTIN, led_brightness > 127);

  // stepper acceleration test
  static uint32_t speed = SPEED_MIN;
  static uint32_t stepper_elapsed = millis();
  if (millis() - stepper_elapsed >= 2000) {
    rotation_direction *= -1;
    speed += 500;
    if (speed >= SPEED_MAX)
      speed = SPEED_MIN;
    stepper.setSpeedInStepsPerSecond(speed);
    Serial.print("Stepper speed: ");
    Serial.println(speed);

    stepper_elapsed = millis();
  }

  transport();
}
