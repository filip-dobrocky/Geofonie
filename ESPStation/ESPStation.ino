#include <ArduinoOSCWiFi.h>
#include <MIDI.h>
#include <Wire.h>
#include <VL53L4CD.h>

#include "../NetworkConfig.h"

#define OBJ_ID 0

#define RX_PIN D4
#define TX_PIN D5

#define I2C_SDA D3
#define I2C_SCL D2

HardwareSerial MidiSerial(1);

MIDI_CREATE_INSTANCE(HardwareSerial, MidiSerial, midi1);

VL53L4CD sensor;

hw_timer_t *midi_timer = NULL;

const IPAddress ip(GET_OBJ_IP(OBJ_ID));
const IPAddress gateway(NetworkConfig::gateway);
const IPAddress subnet(NetworkConfig::subnet);

int led_brightness = 0;
int sensor_value = 0;

void IRAM_ATTR midi_ISR() {
  // int cc_value = map(led_brightness, 0, 255, 0, 127);
  // Serial.print("MIDI: ");
  // Serial.println(cc_value);
  midi1.sendControlChange(3, sensor_value, 1);
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

  sensor.setRangeTiming(10, 0);
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
  pinMode(D7, OUTPUT);
}

void loop() {
  OscWiFi.update();

  sensor.read();
  sensor_value = constrain(sensor.ranging_data.range_mm, 0, 800);
  sensor_value = map(sensor_value, 0, 800, 0, 127);
  // analogWrite(D7, map(sensor_value, 0, 1000, 0, 255));
  
  Serial.println(sensor_value);

  analogWrite(LED_BUILTIN, led_brightness);
}
