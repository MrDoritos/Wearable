#include "display_timeout.h"

#include "config.h"
#include "wbl_func.h"

namespace wbl {

DisplayTimeout displayTimeout;

bool DisplayTimeout::is_display_off() {
    return !(state & ACTIVE);
}

bool DisplayTimeout::is_display_locked() {
    return !(state & ACCEPTING_INPUT);
}

bool DisplayTimeout::is_rejecting_input() {
    return !(state & ACCEPTING_INPUT);
}

int64_t DisplayTimeout::time_since_last_input() {
    return (millis() - last_input);
}

bool DisplayTimeout::is_timeout_exceeded() {
    return time_since_last_input() > DISPLAY_TIMEOUT;
}

bool DisplayTimeout::lock_key_state(bool is_held) {
    if (!is_held) {
        held_time = 0;
        return is_display_locked();
    }

    held_time += time_since_last_input();

    any_user_input();

    if (held_time > HOLD_TIME_TO_LOCK) {
        state = (State)(state ^ ACCEPTING_INPUT);
        held_time = 0;
    }

    return is_display_locked();
}

bool DisplayTimeout::any_user_input() {
    if (time_since_last_input() > 500)
        held_time = 0;

    last_input = millis();

    return is_rejecting_input();
}

void DisplayTimeout::update(bool any_input) {
    if (any_input)
        any_user_input();
    state = (State)(state & ~ACTIVE);
    state = (State)(state | (is_timeout_exceeded() ? 0 : ACTIVE));
}

}