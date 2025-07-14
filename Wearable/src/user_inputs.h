#pragma once

#include "types.h"

namespace wbl {

struct Dpad {
    enum States : ub {
        NONE = 0,
        PRESSED = 1,
        HELD = 2,
        RELEASED = 4,
    };

    struct Button {
        States state{NONE};

        constexpr inline void rising_edge() {
            state = States::PRESSED;
        }

        constexpr inline void falling_edge() {
            state = States::RELEASED;
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
};

static Dpad dpad;

}