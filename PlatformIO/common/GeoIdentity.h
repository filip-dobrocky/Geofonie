/*
 * GeoIdentity.h
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
#include <Preferences.h>

// =====================================================================
//  GeoIdentity
//
//  The object id in NVS, so one generic firmware image can serve every
//  node (mesh OTA). Provisioning builds (-DOBJ_ID=n, flashed over USB)
//  seed it with save(); the generic image carries no OBJ_ID and calls
//  load(). NVS lives in its own partition, untouched by an app OTA.
// =====================================================================

namespace GeoIdentity {

inline int load(int fallback = -1) {
    Preferences prefs;
    prefs.begin("geo", /*readOnly=*/true);
    int id = prefs.getInt("obj_id", fallback);
    prefs.end();
    return id;
}

inline void save(int id) {
    Preferences prefs;
    prefs.begin("geo", /*readOnly=*/false);
    prefs.putInt("obj_id", id);
    prefs.end();
}

}  // namespace GeoIdentity
