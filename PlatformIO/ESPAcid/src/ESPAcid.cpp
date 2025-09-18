#include <Arduino.h>

#include <painlessMesh.h>
#include <MIDI.h>
#include "osc_control.h"

#include "NetworkConfig.h"
#include "GEOUtils.h"

// ---- Constants ----

static const char *TAG = "geo_acid";

#ifndef OBJ_ID
#define OBJ_ID 0
#endif

#ifndef STATION_ONLY
#define STATION_ONLY OBJ_ID!=0
#endif

#ifdef BOARD_OLIMEX
  #define RX_PIN 7
  #define TX_PIN 8
#else
  #define RX_PIN D7
  #define TX_PIN D8
#endif

#define NUM_PARAMS 10

constexpr const char* param_names[NUM_PARAMS] = {
    "/param1", "/param2", "/param3", "/param4", "/param5", "/param6", "/param7", "/param8", "/param9", "/param10"
};

constexpr const char* param_names_global[NUM_PARAMS] = {
    "/global/param1", "/global/param2", "/global/param3", "/global/param4", "/global/param5", "/global/param6", "/global/param7", "/global/param8", "/global/param9", "/global/param10"
};

OSC_receive_msg rcv_param[NUM_PARAMS] = {
    OSC_receive_msg(param_names[0]),
    OSC_receive_msg(param_names[1]),
    OSC_receive_msg(param_names[2]),
    OSC_receive_msg(param_names[3]),
    OSC_receive_msg(param_names[4]),
    OSC_receive_msg(param_names[5]),
    OSC_receive_msg(param_names[6]),
    OSC_receive_msg(param_names[7]),
    OSC_receive_msg(param_names[8]),
    OSC_receive_msg(param_names[9])
};

OSC_receive_msg rcv_param_global[NUM_PARAMS] = {
    OSC_receive_msg(param_names_global[0]),
    OSC_receive_msg(param_names_global[1]),
    OSC_receive_msg(param_names_global[2]),
    OSC_receive_msg(param_names_global[3]),
    OSC_receive_msg(param_names_global[4]),
    OSC_receive_msg(param_names_global[5]),
    OSC_receive_msg(param_names_global[6]),
    OSC_receive_msg(param_names_global[7]),
    OSC_receive_msg(param_names_global[8]),
    OSC_receive_msg(param_names_global[9])
};

OSC_send_msg snd_ping("/ping");

// ---- Globals ----

HardwareSerial MidiSerial(1);
MIDI_CREATE_INSTANCE(HardwareSerial, MidiSerial, midi1);


painlessMesh mesh;

float osc_params[NUM_PARAMS] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 1}; // piezo on, master max

WiFiUDP udp;
const char *base_address = "/toAcid";
const char *broadcast_address = "/fromAcid";


// ---- Function prototypes ----

void setup();
void loop();
void midi_send();
void ping();
void generic_param_callback(OSCMessage &msg);

// ---- Function definitions ----

void setup() {
  MidiSerial.setPins(RX_PIN, TX_PIN);
  midi1.begin(MIDI_CHANNEL_OMNI);

  Serial.begin(115200);
  delay(1000);

  ESP_LOGI(TAG, "Acid starting up, obj id %d", OBJ_ID);

  // Mesh network init
  mesh.init(
    NetworkConfig::ssid, NetworkConfig::password, 
    NetworkConfig::mesh_port, 
#if STATION_ONLY 
    WIFI_STA,
#else
    WIFI_AP_STA,
#endif
    NetworkConfig::mesh_channel, 0, NetworkConfig::max_conn
  );

  udp.begin(NetworkConfig::osc_from_ctl);

  // Generate parameter names and create OSC objects
  for (int i = 0; i < NUM_PARAMS; ++i) {
    rcv_param[i].init(generic_param_callback);
    rcv_param_global[i].init(generic_param_callback);
  }

  snd_ping.init(broadcast_address);
}

void loop() {
  mesh.update();
  osc_control_loop(udp, base_address, broadcast_address);
  midi_send();
  ping();
}

// ---- OSC Callbacks ----

void generic_param_callback(OSCMessage &msg) {
  String addr = msg.getAddress();
  bool global = addr.startsWith("/toAcid/global/");
  int val_idx = global ? 0 : 1;

  if (!global) {
    if (msg.size() < 2) {
      ESP_LOGW(TAG, "Param message with insufficient args");
      return;
    }
    if (msg.getFloat(0) != (float)OBJ_ID) {
      return;
    }
  } else {
    if (msg.size() < 1) {
      ESP_LOGW(TAG, "Invalid global param message");
      return;
    }
  }

  ESP_LOGD(TAG, "Received param message %s: %f", addr.c_str(), msg.getFloat(val_idx));

  for (int i = 0; i < NUM_PARAMS; ++i) {
    if (addr.endsWith(param_names[i])) {
      osc_params[i] = msg.getFloat(val_idx);
      ESP_LOGI(TAG, "Param %d set to %f", i + 1, osc_params[i]);
      break;
    }
  }
}

void ping() {
  static uint32_t last_time = 0;
  if (millis() - last_time < 5000) return;
  last_time = millis();
  ESP_LOGD(TAG, "Ping broadcast");

  snd_ping.m.add((float)OBJ_ID);
  snd_ping.send(udp, IPAddress(255, 255, 255, 255), NetworkConfig::osc_info);
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
