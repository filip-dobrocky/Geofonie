#include <Arduino.h>

#include <painlessMesh.h>
#include <WiFiUdp.h>
#include "osc_control.h"
#include "NetworkConfig.h"
#include <MIDI.h>

// ---- Constants ----


#define OBJ_ID 4
#define RX_PIN D7
#define TX_PIN D8

#define NUM_PARAMS 6

constexpr const char* param_names[NUM_PARAMS] = {
    "/param1", "/param2", "/param3", "/param4", "/param5", "/param6"
};

OSC_receive_msg rcv_param[NUM_PARAMS] = {
    OSC_receive_msg(param_names[0]),
    OSC_receive_msg(param_names[1]),
    OSC_receive_msg(param_names[2]),
    OSC_receive_msg(param_names[3]),
    OSC_receive_msg(param_names[4]),
    OSC_receive_msg(param_names[5])
};

OSC_send_msg snd_param[NUM_PARAMS] = {
    OSC_send_msg(param_names[0]),
    OSC_send_msg(param_names[1]),
    OSC_send_msg(param_names[2]),
    OSC_send_msg(param_names[3]),
    OSC_send_msg(param_names[4]),
    OSC_send_msg(param_names[5])
};

// ---- Macros ----

#define SCALE_TO_MIDI(val, min, max) map(constrain(val, min, max), min, max, 0, 127)
#define FLOAT_TO_MIDI(val) constrain(int(val * 127.0), 0, 127)

// ---- Globals ----

HardwareSerial MidiSerial(1);
MIDI_CREATE_INSTANCE(HardwareSerial, MidiSerial, midi1);


painlessMesh mesh;

float osc_params[NUM_PARAMS] = {0};

WiFiUDP udp;
const char *base_address = "/toAcid";
const char *broadcast_address = nullptr;


// ---- Function declarations ----

void setup();
void loop();
void midi_send();

// OSC callbacks
void param_callback(OSCMessage &msg, int idx);
void generic_param_callback(OSCMessage &msg) {
  const char* addr = msg.getAddress();
  for (int i = 0; i < NUM_PARAMS; ++i) {
    if (strcmp(addr, param_names[i]) == 0) {
      if (msg.size() > 0) {
        osc_params[i] = msg.getFloat(0);
      }
      break;
    }
  }
}

void setup() {
  MidiSerial.setPins(RX_PIN, TX_PIN);
  midi1.begin(MIDI_CHANNEL_OMNI);

  Serial.begin(115200);
  delay(1000);

  // Mesh network init
  mesh.init(NetworkConfig::ssid, NetworkConfig::password, 5555, WIFI_AP_STA, 1, 0, 4); // 5555 is mesh port, 4 is max conn

  udp.begin(NetworkConfig::osc_from_ap);

  // Generate parameter names and create OSC objects
  for (int i = 0; i < NUM_PARAMS; ++i) {
    rcv_param[i].init(generic_param_callback);
    snd_param[i].init(base_address);
  }
}

void loop() {
  mesh.update();
  osc_control_loop(udp, base_address, broadcast_address);
  midi_send();
}

// ---- OSC Callbacks ----
void param_callback(OSCMessage &msg, int idx) {
  if (msg.size() < 1) return;
  osc_params[idx] = msg.getFloat(0);
}

// ---- MIDI send ----

void midi_send() {
  static uint32_t last_time = 0;
  if (millis() - last_time >= 10) {
    for (int i = 0; i < NUM_PARAMS; ++i) {
      midi1.sendControlChange(i + 1, FLOAT_TO_MIDI(osc_params[i]), 1);
    }
    last_time = millis();
  }
}
