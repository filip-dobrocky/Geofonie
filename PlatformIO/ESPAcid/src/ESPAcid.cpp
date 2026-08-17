/*
 * ESPAcid.cpp
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

#include "GeoNode.h"
#include "GeoIdentity.h"
#include "GeoOta.h"
#include "GEOUtils.h"

// ---- Constants ----

static const char *TAG = "geo_acid";

#define FW_VERSION 1

// Object id at runtime: read from NVS by the generic OTA image, seeded over USB
// by the per-node provisioning envs (-DOBJ_ID=n). -1 means unprovisioned.
int g_obj_id = -1;

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


float osc_params[NUM_PARAMS] = {1, 0, 0, 0, 0, 0, 0, 0, 0, 1}; // piezo on, master max

const char *base_address = "/toAcid";
const char *broadcast_address = "/fromAcid";

// Acid is never the mesh root anchor -- that is Roto object 0.
GeoConfig geo_cfg{-1, false, base_address, broadcast_address, "acid"};
GeoNode node(geo_cfg);
GeoOta ota(node.mesh(), "acid");


// ---- Function prototypes ----

void setup();
void loop();
void midi_send();
void ping();
void generic_param_callback(OSCMessage &msg);

// ---- Function definitions ----

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  MidiSerial.setPins(RX_PIN, TX_PIN);
  midi1.begin(MIDI_CHANNEL_OMNI);

  Serial.begin(115200);
  delay(1000);

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
  ESP_LOGI(TAG, "Acid starting up, obj id %d, fw v%d", g_obj_id, FW_VERSION);

  node.setIdentity(g_obj_id, /*is_root=*/false);
  node.begin(NetworkConfig::ssid, NetworkConfig::password);
  ota.begin();

  // Generate parameter names and create OSC objects
  for (int i = 0; i < NUM_PARAMS; ++i) {
    rcv_param[i].init(generic_param_callback);
    rcv_param_global[i].init(generic_param_callback);
  }

  snd_ping.init(broadcast_address);
}

void loop() {
  node.update();
  ota.update();
  midi_send();
  ping();

  // Unprovisioned board (generic image, empty NVS): fast blink. It still joins
  // the mesh and still takes OTA, it just ignores every id-addressed message.
  if (g_obj_id < 0) {
    static uint32_t blink = 0;
    if (millis() - blink >= 150) {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      blink = millis();
    }
  }
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
    if (msg.getFloat(0) != (float)g_obj_id) {
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

  snd_ping.m.add((float)g_obj_id);
  snd_ping.m.add((float)FW_VERSION);
  node.send_info(snd_ping.m);
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
