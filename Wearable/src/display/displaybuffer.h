#pragma once

#include "ui.h"
#include "sh1107.h"

namespace wbl {

template<uint8_t WIDTH, uint8_t HEIGHT, uint8_t BPP>
struct FramebufferPageT : public FramebufferT<WIDTH, HEIGHT, BPP> {
    inline constexpr fb getOffset(const fb &x, const fb &y) const {
        //return (x / 2) + (y * WIDTH / 2);
        return (y / 8) * WIDTH + x;
    }

    inline constexpr fb getBitOffset(const fb &x, const fb &y) const {
        return y & 7;
        //return 0;
    }

    inline constexpr fb getByteMask(const fb &x, const fb &y) const {
        return this->getBitMask() << this->getBitOffset(x, y);
    }

    inline void putPixel(const fb &x, const fb &y, const pixel &px) {
        const fb offset = this->getOffset(x, y);
        const fb bits = this->getBitOffset(x, y);
        const fb bitmask = this->getBitMask();
        const fb bytemask = bitmask << bits;
        this->buffer[offset] &= ~bytemask;
        this->buffer[offset] |= (px & bitmask) << bits;
    }

    inline constexpr pixel getPixel(const fb &x, const fb &y) const {
        const fb offset = this->getOffset(x, y);
        const fb bits = this->getBitOffset(x, y);
        const fb bitmask = this->getBitMask();
        const fb bytemask = bitmask << bits;
        return ((this->buffer[offset] & bytemask) >> bits) & bitmask;
    }
};

template<typename Display, typename Frame = FramebufferPageT<Display::WIDTH, Display::HEIGHT, 1>>
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
        uint8_t *ptr = &this->buffer[0];
        for (uint8_t page = 0; page < Display::PAGES; page++) {
            uint8_t bytes_remaining = Display::BYTES_PER_PAGE;
            ESP_RETURN_ON_ERROR(Display::setPagePosition(page), TAG, "setPagePosition failed");
            while (bytes_remaining > 0) {
                const uint8_t count = bytes_remaining > size ? size : bytes_remaining;
                ESP_RETURN_ON_ERROR(Display::write_payload(ptr, count, &dc, 1), TAG, "write_payload failed");
                ptr += count;
                bytes_remaining -= count;
            }
        }

        return ESP_OK;
    }
};

using DisplayBuffer = DisplayBufferT<SH1107::GME128128>;

}