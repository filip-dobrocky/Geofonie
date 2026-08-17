/*
 * GeoNode.h
 * Part of Geofonie project
 * Copyright (C) 2025 Filip Dobrocky, Trychtyr collective
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

#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <painlessMesh.h>

#include "NetworkConfig.h"
#include "osc_control.h"

#ifdef USE_FS_LITTLEFS
#include <LittleFS.h>
#endif

// =====================================================================
//  GeoNode
//
//  The mesh + OSC plumbing shared by Roto and Acid.
//
//  Every node runs painlessMesh in WIFI_AP_STA and listens for OSC, so
//  an external controller can attach to ANY node's softAP and still
//  drive the whole installation:
//
//    controller -> node A (UDP)  -> mesh broadcast -> nodes B..N
//    node N telemetry -> mesh broadcast -> every node re-emits to its AP
//
//  Mesh payload is one generic message carrying the OSC address and its
//  float args, re-routed into the normal osc_control callbacks on the
//  far side, so nothing downstream (the OSC_receive_msg registrations,
//  the address suffix tests, the obj id filter) has to change:
//
//    {"t":"osc","a":"/toRoto/rotation/speed","v":[2,0.35]}
//
//  A Roto node relays /toAcid messages it never dispatches itself, and
//  vice versa -- that is why the relay is generic rather than typed.
//
//  Usage:
//    GeoConfig cfg{OBJ_ID, OBJ_ID == 0, "/toRoto", "/fromRoto", "roto"};
//    GeoNode node(cfg);
//    setup() { ...rcv_*.init(cb)...; node.begin(ssid, pass); }
//    loop()  { node.update(); }
// =====================================================================

// osc_control.cpp exports this, but osc_control.h does not declare it.
void osc_control_route_command(OSCMessage &msg, int addressOffset);

struct GeoConfig {
    int obj_id;
    bool is_root;             // painlessMesh topology anchor; exactly one per mesh
    const char *base_address; // OSC commands in,  "/toRoto"   | "/toAcid"
    const char *info_address; // OSC telemetry out,"/fromRoto" | "/fromAcid"
    // When set ("roto" / "acid"), enable mesh OTA receive for firmware
    // announced with this role. Role matching keeps the two firmwares from
    // flashing each other. nullptr disables OTA receive.
    const char *ota_role = nullptr;
};

class GeoNode;
static GeoNode *geo_node_instance = nullptr;  // painlessMesh wants plain fn ptrs

class GeoNode {
   public:
    explicit GeoNode(const GeoConfig &cfg) : _cfg(cfg) { geo_node_instance = this; }

    // Set the NVS-loaded identity. Must be called before begin(); globals are
    // constructed before the id is known, hence the post-hoc setter.
    void setIdentity(int obj_id, bool is_root) {
        _cfg.obj_id = obj_id;
        _cfg.is_root = is_root;
    }

    void begin(const char *ssid, const char *pass) {
        _mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION);

        // Uniform AP_STA on every node so a routing tree can actually form and
        // a controller can attach anywhere. Anything else and the mesh is just
        // a set of nodes hanging off one AP.
        _mesh.init(ssid, pass, &_sched, NetworkConfig::mesh_port, WIFI_AP_STA,
                   NetworkConfig::mesh_channel, 0, NetworkConfig::max_conn);

        _mesh.onReceive(&GeoNode::recvTrampoline);
        _mesh.onNewConnection(&GeoNode::newConnTrampoline);
        _mesh.onChangedConnections(&GeoNode::changedConnTrampoline);

        _udp.begin(NetworkConfig::osc_from_ctl);

        if (_cfg.is_root) {
            _mesh.setRoot(true);
            ESP_LOGI(GEO_TAG, "Mesh root anchor (obj id %d)", _cfg.obj_id);
        } else {
            _mesh.setContainsRoot(true);
        }

#ifdef USE_FS_LITTLEFS
        if (_cfg.ota_role) {
            // The ota plugin's own LittleFS.begin() does not format on fail.
            if (!LittleFS.begin(/*formatOnFail=*/true))
                ESP_LOGE(GEO_TAG, "LittleFS mount failed; OTA dedupe disabled");
            _mesh.initOTAReceive(_cfg.ota_role, [](int part, int total) {
                if (part % 64 == 0 || part == total - 1)
                    ESP_LOGI(GEO_TAG, "OTA recv %d/%d", part + 1, total);
            });
            ESP_LOGI(GEO_TAG, "OTA receive enabled, role=%s", _cfg.ota_role);
        }
#endif

        ESP_LOGI(GEO_TAG, "Node up: meshId=%u obj id=%d root=%d", _mesh.getNodeId(),
                 _cfg.obj_id, _cfg.is_root ? 1 : 0);
    }

    void update() {
        _mesh.update();  // also runs _sched
        pump_udp();
    }

    // Inject a command: out to the mesh and applied here. Used by the Sequencer.
    void send_osc(OSCMessage &m) {
        mesh_send("osc", m);
        dispatch_osc(m);
        m.empty();
    }

    // Emit telemetry: to this node's own AP and over the mesh, so whichever
    // node the controller is attached to delivers it.
    void send_info(OSCMessage &m) {
        mesh_send("info", m);
        emit_info(m);
        m.empty();
    }

    Scheduler &scheduler() { return _sched; }
    painlessMesh &mesh() { return _mesh; }
    int obj_id() const { return _cfg.obj_id; }

   private:
    static constexpr const char *GEO_TAG = "GeoNode";

    // Read every pending OSC datagram, relay it to the mesh and apply it here.
    void pump_udp() {
        int size;
        while ((size = _udp.parsePacket()) > 0) {
            // Keep this INSIDE the loop. osc_control_loop() hoists one OSCMessage
            // out of the drain, so a second queued datagram fills on top of the
            // first and is lost -- the best candidate for the message loss the
            // sequencer used to paper over with a double send. Never confirmed on
            // hardware: if a controller's messages still go missing, start here.
            OSCMessage m;
            while (size--) m.fill(_udp.read());
            if (m.hasError()) {
                ESP_LOGE(GEO_TAG, "OSC parse error %d", m.getError());
                continue;
            }
            const char *addr = m.getAddress();
            // Only commands get relayed; keeps stray traffic off the mesh.
            if (!addr || strncmp(addr, "/to", 3) != 0) continue;
            mesh_send("osc", m);
            dispatch_osc(m);
        }
    }

    void dispatch_osc(OSCMessage &m) {
        m.route(_cfg.base_address, osc_control_route_command);
    }

    void mesh_send(const char *type, OSCMessage &m) {
        JsonDocument doc;
        doc["t"] = type;
        doc["a"] = m.getAddress();
        JsonArray v = doc["v"].to<JsonArray>();
        // getFloat() on a non-float arg returns -1 on ESP, so read by type.
        // ponytail: floats on the wire; add a type tag to v[] if a callback
        // ever needs to tell 1 from 1.0.
        for (int i = 0; i < m.size(); i++)
            v.add(m.isFloat(i) ? m.getFloat(i) : (float)m.getInt(i));

        String out;
        serializeJson(doc, out);
        _mesh.sendBroadcast(out);
        ESP_LOGD(GEO_TAG, "mesh tx %s", out.c_str());
    }

    void emit_info(OSCMessage &m) {
        // softAP-directed broadcast (10.x.y.255) reaches a controller joined to
        // THIS node's AP. 255.255.255.255 only egresses the STA interface on an
        // AP_STA node and never reaches softAP clients.
        IPAddress ap = WiFi.softAPIP();
        IPAddress dest(ap[0], ap[1], ap[2], 255);
        _udp.beginPacket(dest, NetworkConfig::osc_info);
        m.send(_udp);
        _udp.endPacket();
    }

    void handle_receive(uint32_t from, String &raw) {
        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, raw);
        if (err) {
            ESP_LOGE(GEO_TAG, "JSON parse error: %s", err.c_str());
            return;
        }
        const char *type = doc["t"];
        const char *addr = doc["a"];
        if (!type || !addr) {
            ESP_LOGE(GEO_TAG, "Mesh message without type/address");
            return;
        }

        OSCMessage m(addr);
        for (JsonVariant v : doc["v"].as<JsonArray>()) m.add(v.as<float>());

        // Received messages are never re-broadcast, so there is no echo path.
        if (strcmp(type, "osc") == 0)
            dispatch_osc(m);
        else if (strcmp(type, "info") == 0)
            emit_info(m);
        else
            ESP_LOGW(GEO_TAG, "Unknown mesh message type: %s", type);
    }

    // --- trampolines ---
    static void recvTrampoline(uint32_t from, String &msg) {
        if (geo_node_instance) geo_node_instance->handle_receive(from, msg);
    }
    static void newConnTrampoline(uint32_t nodeId) {
        ESP_LOGI(GEO_TAG, "New connection, nodeId=%u", nodeId);
    }
    static void changedConnTrampoline() {
        if (geo_node_instance)
            ESP_LOGI(GEO_TAG, "Changed connections, %d nodes",
                     (int)geo_node_instance->_mesh.getNodeList().size());
    }

    GeoConfig _cfg;
    Scheduler _sched;  // drives painlessMesh
    painlessMesh _mesh;
    WiFiUDP _udp;
};
