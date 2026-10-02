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
    // misc: 2 brightness, 3 engine (0 granular / 0.5 sine+FM / 1 reso), 4 root,
    //       5 scale, 6 reso fb (0.5 off), 7 FM ratio (x8, min 1), 8 FM index
    // speed: v*4000 steps/s, 13500 steps/rot -> 0.1 ~ 34 s/rot, 0.2 ~ 17 s/rot

    // granular, synced -> dephasing -> resync
    const Message state0_msgs[] PROGMEM = {
    {"/toRoto/global/misc/3", -1, CONST(0.0f), 100}, // engine 0
    {"/toRoto/global/misc/6", -1, CONST(0.5f), 100}, // reso off
    {"/toRoto/global/misc/8", -1, CONST(0.0f), 100}, // FM index 0
    {"/toRoto/global/misc/2", -1, CONST(0.9f), 100}, // brightness manual high
    {"/toRoto/misc/4", 0, CONST(0.164f), 100}, // root
    {"/toRoto/misc/4", 2, CONST(0.164f + I_OCTAVE), 100}, // root
    {"/toRoto/misc/4", 4, CONST(0.164f), 100}, // root
    {"/toRoto/global/misc/5", -1, CONST(0.4f), 100}, // scale
    {"/toRoto/global/rotation/direction", -1, RAND_DIR(), 2500},
    {"/toRoto/global/rotation/speed", -1, CONST(0.15f), 15000}, // sync
    {"/toRoto/rotation/speed", 2, CONST(0.156f), 100}, // dephase
    {"/toRoto/rotation/speed", 4, CONST(0.162f), 12000},
    {"/toRoto/rotation/speed", 2, CONST(0.162f), 100}, // further apart
    {"/toRoto/rotation/speed", 4, CONST(0.174f), 15000},
    {"/toRoto/rotation/speed", 2, CONST(0.165f), 100},
    {"/toRoto/rotation/speed", 4, CONST(0.18f), 12000},
    {"/toRoto/global/rotation/speed", -1, CONST(0.15f), 15000}, // resync
    };

    // granular + resonator swell tuned to intervals, one device briefly on engine 1
    const Message state1_msgs[] PROGMEM = {
    {"/toRoto/global/misc/3", -1, CONST(0.0f), 100}, // engine 0
    {"/toRoto/global/misc/8", -1, CONST(0.0f), 100}, // FM index 0
    {"/toRoto/global/misc/2", -1, CONST(0.2f), 100}, // brightness auto lower
    {"/toRoto/misc/4", 0, CONST(0.164f), 100}, // root
    {"/toRoto/misc/4", 2, CONST(0.164f + I_PERFECT5TH), 100}, // fifth
    {"/toRoto/misc/4", 4, CONST(0.164f + I_OCTAVE), 100}, // octave
    {"/toRoto/global/misc/5", -1, CONST(0.428f), 100}, // scale
    {"/toRoto/global/misc/6", -1, CONST(0.5f), 100}, // reso off
    {"/toRoto/rotation/speed", 0, CONST(0.12f), 100}, // slow dephase
    {"/toRoto/rotation/speed", 2, CONST(0.123f), 100},
    {"/toRoto/rotation/speed", 4, CONST(0.127f), 6000},
    {"/toRoto/global/misc/6", -1, CONST(0.68f), 8000}, // reso swell
    {"/toRoto/global/misc/6", -1, CONST(0.8f), 8000},
    {"/toRoto/misc/3", 4, CONST(1.0f), 10000}, // engine 1 on device 4
    {"/toRoto/misc/3", 4, CONST(0.0f), 100}, // back to granular
    {"/toRoto/global/misc/6", -1, CONST(0.68f), 6000},
    {"/toRoto/global/misc/6", -1, CONST(0.3f), 4000}, // negative fb dip
    {"/toRoto/global/misc/6", -1, CONST(0.5f), 10000}, // reso off
    };

    // granular, speeds at 2:3:4 so rotations realign; brief dephase, snap back
    const Message state2_msgs[] PROGMEM = {
    {"/toRoto/global/misc/3", -1, CONST(0.0f), 100}, // engine 0
    {"/toRoto/global/misc/8", -1, CONST(0.0f), 100}, // FM index 0
    {"/toRoto/global/misc/6", -1, CONST(0.5f), 100}, // reso off
    {"/toRoto/global/misc/2", -1, CONST(0.4f), 100}, // brightness auto higher
    {"/toRoto/misc/4", 0, CONST(0.264f), 100}, // root
    {"/toRoto/misc/4", 2, CONST(0.264f + I_MAJOR3RD), 100}, // major 3rd
    {"/toRoto/misc/4", 4, CONST(0.264f + I_MAJOR7TH), 100}, // major 7th
    {"/toRoto/global/misc/5", -1, CONST(0.428f), 100}, // scale
    {"/toRoto/rotation/speed", 0, CONST(0.1f), 100}, // 2
    {"/toRoto/rotation/speed", 2, CONST(0.15f), 100}, // 3
    {"/toRoto/rotation/speed", 4, CONST(0.2f), 25000}, // 4
    {"/toRoto/rotation/speed", 2, CONST(0.1545f), 12000}, // +3% dephase
    {"/toRoto/global/misc/6", -1, CONST(0.62f), 6000}, // mild reso
    {"/toRoto/global/misc/6", -1, CONST(0.5f), 100}, // reso off
    {"/toRoto/rotation/speed", 2, CONST(0.15f), 15000}, // snap back to ratio
    };

    // rare short FM episode at the low/high extremes, low end handed to engine 1
    const Message state3_msgs[] PROGMEM = {
    {"/toRoto/global/misc/3", -1, CONST(0.5f), 100}, // engine sine/FM
    {"/toRoto/global/misc/6", -1, CONST(0.5f), 100}, // reso off
    {"/toRoto/global/misc/7", -1, CONST(0.25f), 100}, // FM ratio 2:1
    {"/toRoto/global/misc/8", -1, RANDF_RANGE(0.1f, 0.3f), 100}, // FM index low
    {"/toRoto/misc/4", 0, CONST(0.03f), 100}, // root low
    {"/toRoto/misc/4", 2, CONST(0.85f), 100}, // root high
    {"/toRoto/misc/4", 4, CONST(0.85f + I_PERFECT5TH), 100}, // root high + 5th
    {"/toRoto/global/rotation/speed", -1, CONST(0.2f), 10000}, // sync
    {"/toRoto/misc/3", 0, CONST(1.0f), 10000}, // engine 1 on the low device
    };

    // Define next_states arrays separately (repeats = weight)
    const uint8_t state0_next[] PROGMEM = {1, 2, 2, 1, 3};
    const uint8_t state1_next[] PROGMEM = {0, 2, 0};
    const uint8_t state2_next[] PROGMEM = {0, 1, 1, 3};
    const uint8_t state3_next[] PROGMEM = {0, 2};

    const State score[] PROGMEM = {
    {0, state0_msgs, COUNT_OF(state0_msgs), state0_next, COUNT_OF(state0_next), 70000, true},
    {1, state1_msgs, COUNT_OF(state1_msgs), state1_next, COUNT_OF(state1_next), 55000, true},
    {2, state2_msgs, COUNT_OF(state2_msgs), state2_next, COUNT_OF(state2_next), 60000, true},
    {3, state3_msgs, COUNT_OF(state3_msgs), state3_next, COUNT_OF(state3_next), 20000, true},
    };
}