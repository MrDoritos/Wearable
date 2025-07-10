#pragma once

#include "ui.h"
#include "sh1107.h"

namespace wbl {

template<typename Display, typename Frame = FramebufferT<Display::WIDTH, Display::HEIGHT, 1>>
struct DisplayBufferT : public Frame, public Display {
    inline bool init() {
        if (!Display::init())
            return 1;

        Frame::clear();
        Display::clearDisplay();
        return 0;
    }

    inline void flush() {
        const uint8_t size = 32;
        const uint8_t dc = 0x40;
        for (uint8_t page = 0; page < Display::PAGES; page++) {
            uint8_t bytes_remaining = Display::BYTES_PER_PAGE;
            Display::setPagePosition(page);
            while (bytes_remaining > 0) {
                const uint8_t count = bytes_remaining > size ? size : bytes_remaining;
                Display::write_payload(Frame::buffer + (page * Display::BYTES_PER_PAGE + bytes_remaining), count, &dc, 1);
                bytes_remaining -= count;
            }
        }
    }
};

using DisplayBuffer = DisplayBufferT<SH1107::GME128128>;

}