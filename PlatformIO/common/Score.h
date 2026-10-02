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

// Value generator macros
#define RANDF() []() -> float { return (float)random(0, 10001) / 10000.0f; }
#define RANDF_RANGE(a, b) []() -> float { return (float)random(0, 10001) / 10000.0f * ((b) - (a)) + (a); }
#define RANDI(n) []() -> float { return (float)random(0, n); }
#define CONST(f) []() -> float { return (float)(constrain(f, 0.0f, 1.0f)); }
#define RAND_DIR() []() -> float { return (random(0, 2) * 2 - 1); }

#define COUNT_OF(a) (sizeof(a) / sizeof((a)[0]))

// Musical interval macros (root float offset for MIDI mapping 36-84)
#define I_MINOR2ND    (1.0f/48.0f)
#define I_MAJOR2ND    (2.0f/48.0f)
#define I_MINOR3RD    (3.0f/48.0f)
#define I_MAJOR3RD    (4.0f/48.0f)
#define I_PERFECT4TH  (5.0f/48.0f)
#define I_TRITONE     (6.0f/48.0f)
#define I_PERFECT5TH  (7.0f/48.0f)
#define I_MINOR6TH    (8.0f/48.0f)
#define I_MAJOR6TH    (9.0f/48.0f)
#define I_MINOR7TH    (10.0f/48.0f)
#define I_MAJOR7TH    (11.0f/48.0f)
#define I_OCTAVE      (12.0f/48.0f)

namespace Score {
    struct Message {
        const char *msg;
        int8_t dest_ID;
        float (*get_value)();
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
    // misc: 2 brightness, 3 engine (0 granular+reso / 0.5 FM drone / 1 FM pluck),
    //       4 root, 5 scale, 6 reso fb (0 = -fb, 0.5 = clean, 1 = +fb),
    //       7 FM ratio (x8, min 1), 8 FM index
    // Messages loop within a state; states last < 1 min.

    // resonant granular
    const Message state0_msgs[] PROGMEM = {
    {"/toRoto/global/misc/3", -1, CONST(0.0f), 100}, // engine 0
    {"/toRoto/global/misc/6", -1, RANDF_RANGE(0.9f, 1.0f), 100}, // reso strong +fb
    {"/toRoto/global/misc/2", -1, CONST(0.9f), 100}, // brightness manual high
    {"/toRoto/misc/4", 0, CONST(0.164f), 100}, // root
    {"/toRoto/misc/4", 2, CONST(0.164f + I_PERFECT5TH), 100}, // root
    {"/toRoto/misc/4", 4, CONST(0.164f + I_MAJOR7TH), 100}, // root
    {"/toRoto/global/misc/5", -1, CONST(0.4f), 100}, // scale
    {"/toRoto/global/rotation/direction", -1, RAND_DIR(), 100},
    {"/toRoto/global/rotation/speed", -1, RANDF_RANGE(0.03f, 0.1f), 4000},
    {"/toRoto/rotation/speed", 0, RANDF_RANGE(0.05f, 0.2f), 3000}, // desync
    {"/toRoto/rotation/speed", 2, RANDF_RANGE(0.05f, 0.2f), 3000},
    {"/toRoto/global/misc/6", -1, CONST(1.0f), 4000}, // reso max
    {"/toRoto/global/rotation/speed", -1, CONST(0.0f), 4000}, // stop
    {"/toRoto/rotation/speed", 4, RANDF_RANGE(0.1f, 0.3f), 4000},
    };

    // sine drone, mostly pure, slow dephasing
    const Message state1_msgs[] PROGMEM = {
    {"/toRoto/global/misc/3", -1, CONST(0.5f), 100}, // engine 1
    {"/toRoto/global/misc/2", -1, CONST(0.3f), 100}, // brightness auto
    {"/toRoto/global/misc/7", -1, CONST(0.125f), 100}, // FM ratio 1:1
    {"/toRoto/global/misc/8", -1, CONST(0.0f), 100}, // FM index 0 = pure sine
    {"/toRoto/misc/4", 0, CONST(0.164f + I_OCTAVE), 100}, // root
    {"/toRoto/misc/4", 2, CONST(0.164f + I_OCTAVE + I_PERFECT4TH), 100}, // root
    {"/toRoto/misc/4", 4, CONST(0.164f + I_PERFECT5TH), 100}, // root
    {"/toRoto/global/misc/5", -1, CONST(0.428f), 100}, // scale
    {"/toRoto/global/rotation/speed", -1, CONST(0.12f), 12000}, // sync
    {"/toRoto/rotation/speed", 2, CONST(0.124f), 100}, // dephase
    {"/toRoto/rotation/speed", 4, CONST(0.129f), 15000},
    {"/toRoto/global/misc/8", -1, CONST(0.06f), 6000}, // touch of FM
    {"/toRoto/global/misc/8", -1, CONST(0.0f), 100}, // pure again
    {"/toRoto/global/rotation/speed", -1, CONST(0.12f), 15000}, // resync
    };

    // FM pluck, fast
    const Message state2_msgs[] PROGMEM = {
    {"/toRoto/global/misc/3", -1, CONST(1.0f), 100}, // engine 2
    {"/toRoto/global/misc/2", -1, CONST(0.4f), 100}, // brightness auto higher
    {"/toRoto/global/misc/7", -1, CONST(0.1875f), 100}, // FM ratio 1.5:1
    {"/toRoto/global/misc/5", -1, CONST(0.0f), 100}, // scale
    {"/toRoto/global/rotation/speed", -1, RANDF_RANGE(0.3f, 0.8f), 2000},
    {"/toRoto/global/misc/8", -1, RANDF_RANGE(0.05f, 0.25f), 100}, // FM index
    {"/toRoto/global/rotation/speed", -1, RANDF_RANGE(0.1f, 0.8f), 1500},
    {"/toRoto/global/rotation/speed", -1, RANDF_RANGE(0.1f, 0.8f), 1500},
    {"/toRoto/global/misc/7", -1, RANDF_RANGE(0.125f, 0.5f), 100}, // FM ratio random
    {"/toRoto/global/rotation/direction", -1, RAND_DIR(), 1500},
    {"/toRoto/global/rotation/speed", -1, CONST(0.0f), 2500}, // stop
    {"/toRoto/global/rotation/speed", -1, RANDF_RANGE(0.05f, 0.75f), 1000},
    {"/toRoto/global/misc/8", -1, RANDF_RANGE(0.2f, 0.4f), 100}, // FM index up
    {"/toRoto/global/rotation/speed", -1, RANDF_RANGE(0.1f, 0.8f), 1500},
    {"/toRoto/global/rotation/speed", -1, CONST(0.0f), 4000}, // stop
    };

    // negative-feedback resonator, hollow
    const Message state3_msgs[] PROGMEM = {
    {"/toRoto/global/misc/3", -1, CONST(0.0f), 100}, // engine 0
    {"/toRoto/global/misc/6", -1, RANDF_RANGE(0.0f, 0.1f), 100}, // reso strong -fb
    {"/toRoto/global/misc/2", -1, CONST(0.3f), 100}, // brightness auto
    {"/toRoto/misc/4", 0, CONST(0.264f), 100}, // root
    {"/toRoto/misc/4", 2, CONST(0.264f + I_MAJOR3RD), 100}, // root
    {"/toRoto/misc/4", 4, CONST(0.264f + I_MAJOR7TH), 100}, // root
    {"/toRoto/global/misc/5", -1, CONST(0.428f), 100}, // scale
    {"/toRoto/rotation/speed", 0, RANDF_RANGE(0.4f, 0.8f), 1500},
    {"/toRoto/rotation/speed", 2, RANDF_RANGE(0.4f, 0.8f), 1500},
    {"/toRoto/rotation/speed", 4, RANDF_RANGE(0.4f, 0.8f), 1500},
    {"/toRoto/rotation/direction", 0, RAND_DIR(), 2000},
    {"/toRoto/rotation/direction", 2, RAND_DIR(), 2000},
    {"/toRoto/rotation/direction", 4, RAND_DIR(), 2000},
    {"/toRoto/global/rotation/speed", -1, CONST(0.0f), 4000}, // stop
    {"/toRoto/global/misc/6", -1, RANDF_RANGE(0.9f, 1.0f), 4000}, // flip to +fb
    };

    // engine split across devices
    const Message state4_msgs[] PROGMEM = {
    {"/toRoto/misc/3", 0, CONST(0.5f), 100}, // FM drone
    {"/toRoto/misc/3", 2, CONST(1.0f), 100}, // FM pluck
    {"/toRoto/misc/3", 4, CONST(0.0f), 100}, // reso granular
    {"/toRoto/global/misc/6", -1, CONST(0.95f), 100}, // reso strong
    {"/toRoto/global/misc/7", -1, RANDF_RANGE(0.125f, 0.375f), 100}, // FM ratio
    {"/toRoto/global/misc/8", -1, RANDF_RANGE(0.0f, 0.2f), 100}, // FM index
    {"/toRoto/misc/4", 0, CONST(0.164f + I_PERFECT5TH), 100}, // root
    {"/toRoto/misc/4", 2, CONST(0.164f + I_PERFECT5TH + I_MAJOR3RD), 100}, // root
    {"/toRoto/misc/4", 4, CONST(0.164f + I_PERFECT5TH + I_MAJOR7TH), 100}, // root
    {"/toRoto/rotation/speed", 0, RANDF_RANGE(0.05f, 0.3f), 3000},
    {"/toRoto/rotation/speed", 2, RANDF_RANGE(0.3f, 0.8f), 3000},
    {"/toRoto/rotation/speed", 4, RANDF_RANGE(0.05f, 0.2f), 3000},
    {"/toRoto/misc/3", 0, CONST(1.0f), 100}, // rotate engines
    {"/toRoto/misc/3", 2, CONST(0.5f), 4000},
    {"/toRoto/global/rotation/speed", -1, CONST(0.0f), 4000}, // stop
    {"/toRoto/global/misc/8", -1, RANDF_RANGE(0.0f, 0.3f), 3000}, // FM index random
    };

    // Define next_states arrays separately (repeats = weight)
    // priority: granular (0, 3) > sine (1) > FM pluck (2) > long FM (4)
    const uint8_t state0_next[] PROGMEM = {3, 3, 3, 1, 1, 2};
    const uint8_t state1_next[] PROGMEM = {0, 0, 3, 3, 2, 4};
    const uint8_t state2_next[] PROGMEM = {0, 3, 1};
    const uint8_t state3_next[] PROGMEM = {0, 0, 0, 1, 1, 2};
    const uint8_t state4_next[] PROGMEM = {0, 3};

    const State score[] PROGMEM = {
    {0, state0_msgs, COUNT_OF(state0_msgs), state0_next, COUNT_OF(state0_next), 60000, true},
    {1, state1_msgs, COUNT_OF(state1_msgs), state1_next, COUNT_OF(state1_next), 40000, true},
    {2, state2_msgs, COUNT_OF(state2_msgs), state2_next, COUNT_OF(state2_next), 25000, true},
    {3, state3_msgs, COUNT_OF(state3_msgs), state3_next, COUNT_OF(state3_next), 55000, true},
    {4, state4_msgs, COUNT_OF(state4_msgs), state4_next, COUNT_OF(state4_next), 25000, true},
    };
}