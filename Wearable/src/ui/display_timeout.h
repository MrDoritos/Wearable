#pragma once

#include <inttypes.h>

namespace wbl {

struct DisplayTimeout {
    enum State {
        ACTIVE=1,
        ACCEPTING_INPUT=2,
        ACTIVE_ACCEPTING_INPUT=3,
        OFFLINE=4,
        OFFLINE_ACCEPTING_INPUT=6,
        REJECTING_INPUT=8,
        ACTIVE_REJECTING_INPUT=9
    };

    State state{ACTIVE_ACCEPTING_INPUT};
    int64_t last_input = 0;
    int64_t held_time = 0;

    bool is_display_off();
    bool is_display_locked();
    bool is_rejecting_input();
    bool is_timeout_exceeded();
    void user_sent_input(uint8_t dpad_event);
};
    
}