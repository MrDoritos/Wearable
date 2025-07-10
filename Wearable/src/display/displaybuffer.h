#pragma once

#include "ui.h"
#include "sh1107.h"

namespace wbl {

template<typename Display, typename Frame = FramebufferT<Display::WIDTH, Display::HEIGHT, 1>>
struct DisplayBufferT : public Frame, public Display {
    static constexpr const char *TAG = "wbl::DisplayBufferT";

    inline esp_err_t init() {
        ESP_RETURN_ON_ERROR(Display::init(), TAG, "display init failed");

        Frame::clear();
        ESP_RETURN_ON_ERROR(Display::clearDisplay(), TAG, "clear display failed");
        
        return ESP_OK;
    }

    inline esp_err_t flush() {
        const uint8_t size = 32;
        const uint8_t dc = 0x40;
        for (uint8_t page = 0; page < Display::PAGES; page++) {
            uint8_t bytes_remaining = Display::BYTES_PER_PAGE;
            ESP_RETURN_ON_ERROR(Display::setPagePosition(page), TAG, "setPagePosition failed");
            while (bytes_remaining > 0) {
                const uint8_t count = bytes_remaining > size ? size : bytes_remaining;
                ESP_RETURN_ON_ERROR(Display::write_payload(Frame::buffer + (page * Display::BYTES_PER_PAGE + count), count, &dc, 1), TAG, "write_payload failed");
                bytes_remaining -= count;
            }
        }

        return ESP_OK;
    }
};

using DisplayBuffer = DisplayBufferT<SH1107::GME128128>;

}