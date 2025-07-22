#pragma once

#include <inttypes.h>

namespace wbl {

struct DisplayTimeout {
    enum State : uint8_t {
        ACTIVE=1,
        ACCEPTING_INPUT=2,
        ACTIVE_ACCEPTING_INPUT=3
    };

    State state{ACTIVE_ACCEPTING_INPUT};
    int64_t last_input = 0;
    int64_t held_time = 0;

    bool is_display_off();
    bool is_display_locked();
    bool is_rejecting_input();
    bool is_timeout_exceeded();
    bool lock_key_state(bool is_held);
    bool any_user_input();
    int64_t time_since_last_input();
    void update(bool any_input);
};

extern DisplayTimeout displayTimeout;
    
}