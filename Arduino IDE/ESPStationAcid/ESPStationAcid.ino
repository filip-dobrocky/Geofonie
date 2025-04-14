#include <ArduinoOSCWiFi.h>
#include <MIDI.h>

#include "../NetworkConfig.h"

// Constants

#define OBJ_ID 0

#define RX_PIN D7
#define TX_PIN D8

// Macros

#define SCALE_TO_MIDI(val, min, max) map(constrain(val, min, max), min, max, 0, 127)
#define FLOAT_TO_MIDI(val) constrain(int(val * 127.0), 0, 127)

// Globals

HardwareSerial MidiSerial(1);

MIDI_CREATE_INSTANCE(HardwareSerial, MidiSerial, midi1);


hw_timer_t *midi_timer = NULL;

const IPAddress ip(GET_ACID_IP(OBJ_ID));
const IPAddress gateway(NetworkConfig::gateway);
const IPAddress subnet(NetworkConfig::subnet);

int led_brightness = 0;

float osc_param1;
float osc_param2;
float osc_param3;
float osc_param4;
float osc_param5;
float osc_param6;

//Functions

void IRAM_ATTR midi_ISR() {
  midi1.sendControlChange(1, FLOAT_TO_MIDI(osc_param1), 1);
  midi1.sendControlChange(2, FLOAT_TO_MIDI(osc_param2), 1);
  midi1.sendControlChange(3, FLOAT_TO_MIDI(osc_param3), 1);
  midi1.sendControlChange(4, FLOAT_TO_MIDI(osc_param4), 1);
  midi1.sendControlChange(5, FLOAT_TO_MIDI(osc_param5), 1);
  midi1.sendControlChange(6, FLOAT_TO_MIDI(osc_param6), 1);
}


void setup() {
  MidiSerial.setPins(RX_PIN, TX_PIN);
  midi1.begin(MIDI_CHANNEL_OMNI);
  
  Serial.begin(115200);
  delay(1000);

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

  OscWiFi.subscribe(NetworkConfig::osc_from_ap, "/toAcid/param1", osc_param1);
  OscWiFi.subscribe(NetworkConfig::osc_from_ap, "/toAcid/param2", osc_param2);
  OscWiFi.subscribe(NetworkConfig::osc_from_ap, "/toAcid/param3", osc_param3);
  OscWiFi.subscribe(NetworkConfig::osc_from_ap, "/toAcid/param4", osc_param4);
  OscWiFi.subscribe(NetworkConfig::osc_from_ap, "/toAcid/param5", osc_param5);
  OscWiFi.subscribe(NetworkConfig::osc_from_ap, "/toAcid/param6", osc_param6);

  // Timers
  midi_timer = timerBegin(1000);
  timerAttachInterrupt(midi_timer, &midi_ISR);
  timerAlarm(midi_timer, 10, true, 0);
}

void loop() {
  OscWiFi.update();

  analogWrite(LED_BUILTIN, led_brightness);

  Serial.println(osc_param1);
  Serial.println(osc_param2);
}
