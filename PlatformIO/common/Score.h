/*
 * Score.h
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

#define RAND_FLOAT() ((float)random(0, 10001) / 10000.0f)

namespace Score {
    struct Message {
        const char *msg;
        int8_t dest_ID;
        float value;
        uint32_t duration;
    };

    struct State {
        int8_t ID;
        const Message *msgs;
        uint8_t msgs_count;
        const uint8_t *next_states;
        uint8_t next_states_count;
        uint32_t duration = 0;
        bool loop = false;
    };
    
    // Define message arrays separately

    // initial
    const Message state0_msgs[] PROGMEM = {
        {"/toAcid/global/param1", -1, 0.6f, 100},
        {"/toAcid/global/param2", -1, 0.0f, 100},
        {"/toAcid/global/param3", -1, 1.0f, 100},
        {"/toAcid/global/param4", -1, 0.0f, 200},
        {"/toAcid/global/param5", -1, 0.0f, 200},
        {"/toAcid/global/param6", -1, 0.0f, 200},
        {"/toRoto/global/misc/2", -1, 0.9f, 100}, // brightness manual high
        {"/toRoto/global/misc/3", -1, 0.0f, 100}, // engine 0
        {"/toRoto/global/misc/4", -1, 0.164f, 100}, // root
        {"/toRoto/global/misc/5", -1, 0.4f, 100}, // scale
        {"/toRoto/global/misc/6", -1, 0.77f, 100}, // reso
        {"/toRoto/global/rotation/direction", -1, (float)(random(0, 2) * 2 - 1), 5000},
        {"/toRoto/global/rotation/speed", -1, 0.08f, 3000},
    };

    const Message state1_msgs[] PROGMEM = {
        {"/toAcid/global/param4", -1, 0.0f, 200},
        {"/toAcid/global/param5", -1, 0.0f, 200},
        {"/toAcid/global/param6", -1, 0.0f, 200},
        {"/toRoto/global/misc/2", -1, 0.2f, 100}, // brightness auto lower
        {"/toRoto/global/misc/3", -1, 0.0f, 100}, // engine 0
        {"/toRoto/global/misc/4", -1, 0.164f, 100}, // root
        {"/toRoto/global/misc/5", -1, 0.428f, 100}, // scale
        {"/toRoto/global/misc/6", -1, 0.77f, 100}, // reso
        {"/toRoto/rotation/speed", 0, 0.2f, 6000}, // desynchronize speeds gradually
        {"/toRoto/rotation/speed", 1, 0.3f, 4000},
        {"/toRoto/global/misc/6", -1, 0.85f, 100}, // reso
        {"/toRoto/rotation/speed", 2, 0.4f, 3000},
        {"/toAcid/global/param4", -1, 0.5f, 200},
        {"/toAcid/global/param5", -1, 0.2f, 200},
        {"/toAcid/global/param6", -1, 0.3f, 200},
        {"/toRoto/rotation/speed", 3, 0.5f, 1500},
        {"/toRoto/rotation/speed", 4, 0.6f, 1000},
        {"/toRoto/global/misc/6", -1, 1.0f, 100}, // reso
    };

    const Message state2_msgs[] PROGMEM = {
        {"/toRoto/global/misc/3", -1, 0.5f, 10000}, // engine 1 global
        {"/toAcid/global/param4", -1, 0.63f, 200},
        {"/toAcid/global/param5", -1, 0.2f, 200},
        {"/toAcid/global/param6", -1, 0.2f, 200},
        {"/toRoto/global/misc/2", -1, 0.4f, 200}, // brightness auto higher
        {"/toRoto/global/rotation/speed", -1, RAND_FLOAT() * 0.7f + 0.1f, 200},
        {"/toRoto/misc/3", 3, 0.5f, 8000}, // engine 1 for device 3
        {"/toRoto/global/misc/3", -1, 0.5f, 2000}, // engine 1 global 
        {"/toAcid/global/param4", -1, 0.74f, 200},
        {"/toRoto/global/rotation/speed", -1, RAND_FLOAT() * 0.7f + 0.1f, 1000},
        {"/toRoto/global/rotation/speed", -1, RAND_FLOAT() * 0.7f + 0.1f, 1000},
        {"/toRoto/global/rotation/speed", -1, RAND_FLOAT() * 0.7f + 0.1f, 1000},
        {"/toRoto/global/misc/5", -1, 0.0f, 100}, // scale
        {"/toAcid/global/param6", -1, 0.6f, 200},
        {"/toRoto/global/misc/3", -1, 1.0f, 200}, // engine 2
        {"/toRoto/global/direction", -1, -1.0f, 2000}, // engine 1
        {"/toRoto/global/rotation/speed", -1, RAND_FLOAT() * 0.7f + 0.05f, 500},
        {"/toRoto/global/rotation/speed", -1, RAND_FLOAT() * 0.7f + 0.05f, 500},
        {"/toRoto/global/rotation/speed", -1, RAND_FLOAT() * 0.7f + 0.1f, 1000},
        {"/toRoto/global/rotation/speed", -1, RAND_FLOAT() * 0.7f + 0.1f, 1000},
    };

    const Message state3_msgs[] PROGMEM = {
        {"/toAcid/global/param4", -1, RAND_FLOAT(), 200},
        {"/toAcid/global/param5", -1, RAND_FLOAT(), 200},
        {"/toAcid/global/param6", -1, RAND_FLOAT(), 200},
        {"/toRoto/global/misc/4", -1, 0.264f, 100}, // root
        {"/toRoto/global/misc/5", -1, 0.428f, 100}, // scale
        {"/toRoto/global/misc/3", -1, 1.0f, 100}, // engine 2
        {"/toRoto/global/rotation/direction", -1, (float)(random(0, 2) * 2 - 1), 5000},
        {"/toRoto/rotation/speed", (int8_t)random(5), RAND_FLOAT() * 0.2 + 0.6f, 3000},
        {"/toRoto/rotation/direction", (int8_t)random(5), (float)(random(0, 2) * 2 - 1), 5000},
    };

    // Define next_states arrays separately
    const uint8_t state0_next[] PROGMEM = {1, 2};
    const uint8_t state1_next[] PROGMEM = {0, 2};
    const uint8_t state2_next[] PROGMEM = {3, 1};
    const uint8_t state3_next[] PROGMEM = {0};

    // Update msg and state counts when changing above arrays!
    const State score[] PROGMEM = {
        {0, state0_msgs, 13, state0_next, 2, 35000, true},
        {1, state1_msgs, 18, state1_next, 2, 60000, true},
        {2, state2_msgs, 20, state2_next, 2, 80000, true},
        {3, state3_msgs, 9, state3_next, 1, 30000, true},
    };
}