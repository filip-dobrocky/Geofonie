#pragma once
#include <Arduino.h>

#define RAND_FLOAT() ((float)random(0, 10001) / 10000.0f)

namespace Score {
    struct Message {
        const char *msg;
        int8_t dest_ID;
        float value;
        uint16_t duration;
    };

    struct State {
        int8_t ID;
        const Message *msgs;
        const uint8_t *next_states;
        uint16_t duration = 0;
        bool loop = false;
    };

    // Define message arrays separately
    const Message state0_msgs[] PROGMEM = {
        {"/toRoto/global/direction", -1, (float)(random(0, 2) * 2 - 1), 2000},
        {"/toRoto/global/speed", -1, RAND_FLOAT(), 3000},
    };

    const Message state1_msgs[] PROGMEM = {
        {"/toRoto/global/direction", -1, 0, 2000},
        {"/toRoto/global/speed", -1, 0.1, 3000},
    };

    const Message state2_msgs[] PROGMEM = {
        {"/toRoto/global/speed", -1, RAND_FLOAT() * 0.5f + 0.1f, 1000},
    };

    // Define next_states arrays separately
    const uint8_t state0_next[] PROGMEM = {2, 3};
    const uint8_t state1_next[] PROGMEM = {1, 3};
    const uint8_t state2_next[] PROGMEM = {1, 2};

    const State score[] PROGMEM = {
        {0, state0_msgs, state0_next, 15000, true},
        {1, state1_msgs, state1_next, 5000, false},
        {2, state2_msgs, state2_next, 10000, false},
    };
}