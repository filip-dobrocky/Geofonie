/*
 * GeoOta.h
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
#include <FS.h>
#include <LittleFS.h>
#include <MD5Builder.h>
#include <Update.h>
#include <WebServer.h>
#include <painlessMesh.h>

#include <set>

// =====================================================================
//  GeoOta
//
//  Ported from OtaDistributor in the Datel/birb Ecosystem library.
//
//  Any node can distribute a firmware update: join its softAP, POST a
//  firmware.bin to it, and it offers the image to the mesh via the
//  painlessMesh OTA plugin (role-matched, so "roto" firmware can never
//  flash an "acid" node). Receive side is in GeoNode::begin().
//
//  The distributor never hears its own announce, so once the rest of the
//  mesh has fetched the last chunk (or a timeout expires) it flashes
//  ITSELF from the uploaded file and reboots.
//
//  Usage:
//    GeoOta ota(node.mesh(), "roto");
//    setup() { ... node.begin(...); ota.begin(); }
//    loop()  { ... ota.update(); }
//
//  Endpoints on the softAP IP:
//    GET  /        upload form
//    POST /fw      firmware.bin -> starts distribution
//    GET  /status  JSON progress
//    GET  /abort   stop announcing + cancel the pending self-flash
// =====================================================================

class GeoOta {
   public:
    static constexpr size_t PART_SIZE = 1024;  // bytes per mesh OTA chunk
    // Self-flash once every mesh node fetched the last chunk, but no sooner
    // than GRACE (lets late joiners start), and no later than TIMEOUT.
    static constexpr uint32_t SELF_FLASH_GRACE_MS = 60 * 1000UL;
    static constexpr uint32_t SELF_FLASH_TIMEOUT_MS = 25 * 60 * 1000UL;

    GeoOta(painlessMesh &mesh, const char *role, uint16_t port = 80)
        : _mesh(mesh), _role(role), _server(port) {}

    void begin() {
        _server.on("/", HTTP_GET, [this]() { handle_root(); });
        _server.on(
            "/fw", HTTP_POST, [this]() { handle_upload_done(); },
            [this]() { handle_upload_chunk(); });
        _server.on("/status", HTTP_GET, [this]() { handle_status(); });
        _server.on("/abort", HTTP_GET, [this]() { handle_abort(); });
        _server.onNotFound([this]() { _server.send(404, "text/plain", "not found"); });
        _server.begin();
        ESP_LOGI(OTA_TAG, "Firmware upload at http://%s/ (role=%s)",
                 WiFi.softAPIP().toString().c_str(), _role);
    }

    void update() { _server.handleClient(); }

   private:
    static constexpr const char *OTA_TAG = "GeoOta";
    static constexpr const char *FW_PATH = "/fw.bin";

    // Persist the installed-firmware md5 the way the receive plugin does
    // (painlessmesh/ota.hpp), so announces of this image are deduped on boot.
    static void write_ota_state_file(const char *role, const String &md5) {
        painlessmesh::plugin::ota::State st;
        st.role = role;
        st.md5 = md5;
        auto file = LittleFS.open(st.ota_fn, "w");
        if (!file) {
            ESP_LOGE(OTA_TAG, "Cannot write %s (risks OTA re-flash loops)",
                     st.ota_fn.c_str());
            return;
        }
        String msg;
        painlessmesh::protocol::Variant var(&st);
        var.printTo(msg);
        file.print(msg);
        file.close();
    }

    void handle_root() {
        _server.send(200, "text/html",
                     "<html><body><h3>Geofonie OTA distributor</h3>"
                     "<form method='POST' action='/fw' enctype='multipart/form-data'>"
                     "<input type='file' name='fw'> "
                     "<input type='submit' value='Upload &amp; distribute'></form>"
                     "<p><a href='/status'>status</a> | <a href='/abort'>abort</a></p>"
                     "</body></html>");
    }

    void handle_upload_chunk() {
        HTTPUpload &up = _server.upload();

        if (up.status == UPLOAD_FILE_START) {
            // A new upload supersedes any running offer.
            if (_announce) _announce->disable();
            if (_monitor) _monitor->disable();
            _offering = false;
            _upload_ok = false;
            if (_fw) _fw.close();
            LittleFS.remove(FW_PATH);
            _fw = LittleFS.open(FW_PATH, "w");
            if (!_fw)
                ESP_LOGE(OTA_TAG, "Cannot open %s for write", FW_PATH);
            else
                ESP_LOGI(OTA_TAG, "Upload start: %s", up.filename.c_str());
        } else if (up.status == UPLOAD_FILE_WRITE) {
            if (_fw) _fw.write(up.buf, up.currentSize);
        } else if (up.status == UPLOAD_FILE_END) {
            if (!_fw) return;
            _fw.close();
            _fw = LittleFS.open(FW_PATH, "r");
            _fw_size = _fw ? _fw.size() : 0;
            if (_fw_size == 0 || _fw_size != up.totalSize) {
                ESP_LOGE(OTA_TAG, "Upload incomplete: stored %u of %u bytes",
                         (unsigned)_fw_size, (unsigned)up.totalSize);
                return;  // likely LittleFS full -- image too big for the FS partition
            }
            MD5Builder md5;
            md5.begin();
            md5.addStream(_fw, _fw_size);
            md5.calculate();
            _md5 = md5.toString();
            _no_part = (_fw_size + PART_SIZE - 1) / PART_SIZE;
            _upload_ok = true;
            ESP_LOGI(OTA_TAG, "Upload done: %u bytes, md5=%s, %u parts",
                     (unsigned)_fw_size, _md5.c_str(), (unsigned)_no_part);
        } else if (up.status == UPLOAD_FILE_ABORTED) {
            if (_fw) _fw.close();
            _upload_ok = false;
            ESP_LOGW(OTA_TAG, "Upload aborted");
        }
    }

    void handle_upload_done() {
        if (!_upload_ok) {
            _server.send(400, "text/plain",
                         "upload failed (incomplete write -- image too large for "
                         "the FS partition?)\n");
            return;
        }
        start_offer();
        _server.send(200, "text/plain",
                     "distributing: md5=" + _md5 + " parts=" + String(_no_part) +
                         " role=" + _role +
                         "\nthis node self-flashes last; watch /status\n");
    }

    void handle_status() {
        JsonDocument doc;
        doc["offering"] = _offering;
        doc["md5"] = _md5;
        doc["size"] = _fw_size;
        doc["parts"] = _no_part;
        doc["mesh_nodes"] = _mesh.getNodeList().size();
        doc["nodes_done"] = _finished.size();
        doc["elapsed_s"] = _offering ? (millis() - _offer_start) / 1000 : 0;
        doc["running_md5"] = ESP.getSketchMD5();
        String out;
        serializeJson(doc, out);
        _server.send(200, "application/json", out);
    }

    void handle_abort() {
        if (_announce) _announce->disable();
        if (_monitor) _monitor->disable();
        _offering = false;
        _server.send(200, "text/plain", "aborted\n");
        ESP_LOGW(OTA_TAG, "Offer aborted via HTTP");
    }

    void start_offer() {
        if (!_send_init) {
            // Register the chunk server ONCE per boot: the plugin's onPackage
            // APPENDS handlers, so a second init double-sends every chunk.
            // State is read through `this`, so later uploads reuse the handler.
            _mesh.initOTASend(
                [this](painlessmesh::plugin::ota::DataRequest pkg, char *buf) -> size_t {
                    if (!_fw || pkg.md5 != _md5) return 0;  // stale request
                    size_t offset = PART_SIZE * pkg.partNo;
                    if (offset >= _fw_size) return 0;
                    size_t len =
                        _fw_size - offset < PART_SIZE ? _fw_size - offset : PART_SIZE;
                    _fw.seek(offset);
                    _fw.readBytes(buf, len);
                    if (pkg.partNo + 1 == _no_part) _finished.insert(pkg.from);
                    return len;
                },
                PART_SIZE);
            _send_init = true;
        }

        _finished.clear();
        if (_announce) _announce->disable();

        painlessmesh::plugin::ota::State st;  // default hardware string ("ESP32")
        _announce = _mesh.offerOTA(_role, st.hardware, _md5, _no_part, /*forced=*/false);
        _offering = true;
        _offer_start = millis();

        if (_monitor) _monitor->disable();
        _monitor = _mesh.addTask(30 * TASK_SECOND, TASK_FOREVER, [this]() { check_done(); });
        ESP_LOGI(OTA_TAG, "Offering md5=%s to role=%s (%u parts)", _md5.c_str(), _role,
                 (unsigned)_no_part);
    }

    void check_done() {
        if (!_offering) return;
        uint32_t elapsed = millis() - _offer_start;

        bool all = true;
        for (auto id : _mesh.getNodeList()) {
            if (!_finished.count(id)) {
                all = false;
                break;
            }
        }
        ESP_LOGI(OTA_TAG, "OTA progress: %u nodes done, %u in mesh, %us elapsed",
                 (unsigned)_finished.size(), (unsigned)_mesh.getNodeList().size(),
                 (unsigned)(elapsed / 1000));

        if ((all && elapsed >= SELF_FLASH_GRACE_MS) || elapsed >= SELF_FLASH_TIMEOUT_MS)
            self_flash();
    }

    void self_flash() {
        _offering = false;
        if (_announce) _announce->disable();
        if (_monitor) _monitor->disable();

        if (_md5 == ESP.getSketchMD5()) {
            ESP_LOGI(OTA_TAG, "Self-flash skipped: already running md5=%s", _md5.c_str());
            write_ota_state_file(_role, _md5);
            return;
        }

        ESP_LOGI(OTA_TAG, "Self-flashing md5=%s (%u bytes)", _md5.c_str(),
                 (unsigned)_fw_size);
        _fw.seek(0);
        if (!Update.begin(_fw_size)) {
            ESP_LOGE(OTA_TAG, "Update.begin failed");
            Update.printError(Serial);
            return;
        }
        Update.setMD5(_md5.c_str());
        size_t written = Update.writeStream(_fw);
        if (written != _fw_size || !Update.end(true)) {
            ESP_LOGE(OTA_TAG, "Self-flash failed (%u/%u bytes)", (unsigned)written,
                     (unsigned)_fw_size);
            Update.printError(Serial);
            return;
        }

        write_ota_state_file(_role, _md5);
        ESP_LOGI(OTA_TAG, "Self-flash OK, rebooting");
        _mesh.addTask(2 * TASK_SECOND, TASK_ONCE, []() { ESP.restart(); });
    }

    painlessMesh &_mesh;
    const char *_role;
    WebServer _server;

    File _fw;  // uploaded image, kept open for the send callback
    String _md5;
    size_t _fw_size = 0;
    size_t _no_part = 0;
    bool _upload_ok = false;
    bool _send_init = false;

    bool _offering = false;
    uint32_t _offer_start = 0;
    std::shared_ptr<Task> _announce;  // repeating ANNOUNCE broadcast
    std::shared_ptr<Task> _monitor;   // periodic check_done()
    std::set<uint32_t> _finished;     // nodes that requested the last chunk
};
