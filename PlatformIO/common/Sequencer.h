/*
 * Sequencer.cpp
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

#include "GeoNode.h"  // must precede TaskScheduler.h; see the note in ESPRoto.cpp
#include "Score.h"

#define CHECK_NEXT(state, idx) (idx + 1 < state.msgs_count)

const char* SEQ_TAG = "Sequencer";

class Sequencer {
   public:
    static Sequencer* instance;

    Sequencer(GeoNode& node) : node(node) {
        _scheduler.startNow();

        Sequencer::instance = this;
    };

    void start() {
        set_state(0);
    };

    void stop() {
        t_next_state.disable();
        t_next_message.disable();
    };

    void update() {
        _scheduler.execute();
    };

   private:
    Scheduler _scheduler;

    uint8_t current_state = 0;
    uint8_t current_msg = 0;

    GeoNode& node;

    void send_message(const Score::Message& msg) {
        OSCMessage osc(msg.msg);
        if (msg.dest_ID != -1)
            osc.add((float)msg.dest_ID);
        float value = msg.get_value();
        osc.add(value);
        node.send_osc(osc);
        ESP_LOGD(SEQ_TAG, "Sending %s %d %f", msg.msg, msg.dest_ID, value);
     };

    static void next_message() {
        auto state = Score::score[instance->current_state];
        auto msg = (CHECK_NEXT(state, instance->current_msg)) 
                ? state.msgs[++instance->current_msg] 
                : state.msgs[instance->current_msg = 0];

        instance->send_message(msg);

        if (CHECK_NEXT(state, instance->current_msg) || state.loop) {
            ESP_LOGD(SEQ_TAG, "Scheduling next message in %d ms", msg.duration);
            instance->t_next_message.setInterval(msg.duration);
            instance->t_next_message.restartDelayed();
        } else {
            instance->t_next_message.disable();
        }
    }

    void set_state(uint8_t state_id) {
        auto state = Score::score[current_state = state_id];
        
        ESP_LOGD(SEQ_TAG, "Entering state %d", state.ID);
        
        current_msg = 0;
        auto msg = state.msgs[current_msg];
        send_message(msg);

        if (CHECK_NEXT(state, current_msg) || state.loop) {
            ESP_LOGD(SEQ_TAG, "Scheduling next message in %d ms", msg.duration);
            t_next_message.setInterval(msg.duration);
            t_next_message.restartDelayed();
        } else {
            t_next_message.disable();
        }

        if (state.duration > 0) {
            ESP_LOGD(SEQ_TAG, "Scheduling next state in %d ms", state.duration);
            t_next_state.setInterval(state.duration);
            t_next_state.restartDelayed();
        } else {
            ESP_LOGW(SEQ_TAG, "State %d has no duration, not scheduling next state", state.ID);
            t_next_state.disable();
        }
    };

    static void next_state() {
        auto state = Score::score[instance->current_state];
        if (state.next_states_count == 0) {
            ESP_LOGW(SEQ_TAG, "State %d has no next states, stopping sequencer", state.ID);
            instance->t_next_state.disable();
            return;
        }

        instance->current_state = state.next_states[random(0, state.next_states_count)];
        instance->set_state(instance->current_state);
    };
    

    Task t_next_state { 1000UL, TASK_ONCE, &Sequencer::next_state, &_scheduler, false };
    Task t_next_message { 1000UL, TASK_ONCE, &Sequencer::next_message, &_scheduler, false };
};

Sequencer* Sequencer::instance = nullptr;
