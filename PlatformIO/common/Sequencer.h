#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <TaskScheduler.h>
#include "osc_control.h"

#include "Score.h"
#include "NetworkConfig.h"

#define CHECK_NEXT(arr, idx) (idx + 1 < (sizeof(arr) / sizeof(arr[0])))

const char* SEQ_TAG = "Sequencer";

class Sequencer {
   public:
    static Sequencer* instance;

    Sequencer(WiFiUDP& udp, uint8_t obj_id = -1) : obj_id(obj_id), udp(udp) {
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

    uint8_t obj_id = -1;
    uint8_t current_state = 0;
    uint8_t current_msg = 0;

    WiFiUDP& udp;

    void send_message(const Score::Message& msg) {
        auto ip = (obj_id == msg.dest_ID) ? WiFi.localIP() : IPAddress(255, 255, 255, 255);
        OSC_send_msg osc(msg.msg);
        osc.init("");
        if (msg.dest_ID != -1)
            osc.m.add((float)msg.dest_ID);
        osc.m.add(msg.value);
        osc.send(udp, ip, NetworkConfig::osc_from_ap);
     };

    static void next_message() {
        auto state = Score::score[instance->current_state];
        auto msg = (CHECK_NEXT(state.msgs, instance->current_msg)) 
                ? state.msgs[++instance->current_msg] 
                : state.msgs[instance->current_msg = 0];

        instance->send_message(msg);

        if (CHECK_NEXT(state.msgs, instance->current_msg) || state.loop) {
            instance->t_next_message.setInterval(msg.duration);
            instance->t_next_message.restartDelayed();
        } else {
            instance->t_next_message.disable();
        }
    }

    void set_state(uint8_t state_id) {
        auto state = Score::score[current_state = state_id];
        current_msg = 0;
        auto msg = state.msgs[current_msg];

        send_message(msg);

        if (CHECK_NEXT(state.msgs, current_msg) || state.loop) {
            t_next_message.setInterval(msg.duration);
            t_next_message.restartDelayed();
        } else {
            t_next_message.disable();
        }

        if (state.duration > 0) {
            t_next_state.setInterval(state.duration);
            t_next_state.restartDelayed();
        } else {
            ESP_LOGW(SEQ_TAG, "State %d has no duration, not scheduling next state", state.ID);
            t_next_state.disable();
        }
    };

    static void next_state() {
        auto state = Score::score[instance->current_state];
        auto next_states_count = sizeof(state.next_states) / sizeof(state.next_states[0]);
        if (next_states_count == 0) {
            ESP_LOGW(SEQ_TAG, "State %d has no next states, stopping sequencer", state.ID);
            instance->t_next_state.disable();
            return;
        }

        instance->current_state = state.next_states[random(0, next_states_count)];
        instance->set_state(instance->current_state);
    };
    

    Task t_next_state { 1000UL, TASK_ONCE, &Sequencer::next_state, &_scheduler, false };
    Task t_next_message { 1000UL, TASK_ONCE, &Sequencer::next_message, &_scheduler, false };
};

Sequencer* Sequencer::instance = nullptr;
