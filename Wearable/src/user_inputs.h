#pragma once

#include "types.h"

#include <stdio.h>

namespace wbl {

struct Dpad {
    enum States : ub {
        NONE = 0,
        PRESSED = 1,
        HELD = 2,
        RELEASED = 4,
        NUMSTATES=3,
    };

    struct Button {
        States state{NONE};

        constexpr inline void rising_edge() {
            if (state & ~(States::PRESSED))
                state |= States::PRESSED;
        }

        constexpr inline void falling_edge() {
            if (state & ~(States::RELEASED))
                state |= States::RELEASED;
        }

        constexpr inline void update() {
            if (state & States::PRESSED) 
                state = States::HELD;
            if (state & States::RELEASED)
                state = States::NONE;
        }

        constexpr inline bool is_pressed() const {
            return state & States::PRESSED;
        }

        constexpr inline bool is_released() const {
            return state & States::RELEASED;
        }

        constexpr inline bool is_held() const {
            return state & States::HELD;
        }
    };

    const ub button_count = 5;

    Button enter, up, right, down, left;

    Button *buttons = &enter;

    void init();

    static constexpr const char *names[] = {
        "Enter", "Up", "Right", "Down", "Left"
    };
    static constexpr const char *states[] = {
        "Pressed", "Held", "Released"
    };
    
    constexpr inline void print_states() {
        for (int i = 0; i < States::NUMSTATES; i++) {
            const int buflen = 50;
            char buf[buflen];
            int offset = 0;
            bool v = false;
            offset += snprintf(buf+offset, buflen-offset, "%s: ", states[i]);
            for (int j = 0; j < button_count; j++) {
                if (buttons[j].state & (1<<i)) {
                    offset += snprintf(buf+offset, buflen-offset, "%s%s", (v?", ":""), names[j]);
                    v = true;
                }
            }
            if (v) {
                offset += snprintf(buf+offset, buflen-offset, "\n");
                fwrite(buf, 1, offset, stdout);
            }
        }
        /*
        for (int i = 0; i < button_count; i++) {
            const Button &button = buttons[i];
            const States &state = button.state;
            const int buflen = 50;
            char buf[buflen];
            int offset = snprintf(buf, buflen, "%s: ", names[i]);
            bool v = false;
            for (int j = 0; j < 3; j++) {
                if (state & (1<<j)) {
                    offset += snprintf(buf+offset, buflen-offset, "%s%s", (v?"| ":""),states[j]);
                    v = true;
                }
            }
            offset+=snprintf(buf+offset, buflen-offset, "\n");
            fwrite(buf, 1, offset, stdout);
        }
        */
        fflush(stdout);
    }

    constexpr inline void update() {
        for (int i = 0; i < button_count; i++)
            buttons[i].update();
    }

    constexpr inline bool any_state() {
        for (int i = 0; i < button_count; i++)
            if (buttons[i].state)
                return true;
        return false;
    }

    constexpr inline bool any(const States &state) {
        for (int i = 0; i < button_count; i++)
            if (buttons[i].state & state)
                return true;
        return false;
    }
};

extern Dpad dpad;

}