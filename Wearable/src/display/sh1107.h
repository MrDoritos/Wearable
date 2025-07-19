#pragma once

#include "sh1107_defs.h"
#include "config.h"
#include "i2c.h"

#include <inttypes.h>

namespace wbl {
namespace SH1107 {

template<uint8_t _WIDTH, uint8_t _HEIGHT, typename I2C_DEVICE>
struct Display : public I2C_DEVICE {
    static constexpr const char *TAG = "wbl::SH1107::Display";
    static constexpr uint8_t WIDTH = _WIDTH;
    static constexpr uint8_t HEIGHT = _HEIGHT;
    static constexpr uint8_t PAGES = ((HEIGHT + 7) / 8);
    static constexpr uint8_t BYTES_PER_PAGE = WIDTH;
    
    enum {
        FLIP_NONE = 0,
        FLIP_HORIZONTAL = 1,
        FLIP_VERTICAL = 2,
        COLOR_NORMAL = 4,
        COLOR_INVERT = 8,
        DISPLAY_OFF = 16,
        DISPLAY_ON = 32,
    };

    inline esp_err_t setPagePosition(const uint8_t &page, const uint8_t &page_start=0, const uint8_t &page_start_offset=0) {
        const uint8_t cmd[] = {
            SH1107::SET_PAGEADDR + page, 0x10 + ((page_start + page_start_offset) >> 4), (page_start + page_start_offset) & 0xf
        };
        ESP_RETURN_ON_ERROR(this->write_commands(cmd, sizeof(cmd)), TAG, "setPagePosition failed");

        return ESP_OK;
    }
  
    inline esp_err_t clearDisplay(const uint8_t &color=0x0) {
        const uint8_t size = 32;
        uint8_t dc = 0x40;
        uint8_t clearData[size];
        for (uint8_t i = 0; i < size; i++)
            clearData[i] = color;
        for (uint8_t p = 0; p < PAGES; p++) {
            uint8_t bytes_remaining = BYTES_PER_PAGE;
            ESP_RETURN_ON_ERROR(this->setPagePosition(p), TAG, "clearDisplay setPagePosition failed");
            while (bytes_remaining>0) {
                const uint8_t count = bytes_remaining > size ? size : bytes_remaining;
                ESP_RETURN_ON_ERROR(this->write_payload(clearData, count, &dc, 1), TAG, "clearDisplay write_payload failed");
                bytes_remaining -= count;
            }
        }

        return ESP_OK;
    }

    inline esp_err_t setInverted(const bool &inverted_colors = false) {
        return this->write_command(inverted_colors ? SH1107::INVERT : SH1107::NORMAL);
    }

    inline esp_err_t setState(const bool &on = true) {
        return this->write_command(on ? SH1107::ON : SH1107::OFF);
    }

    inline esp_err_t setContrast(const uint8_t &contrast = 0x7f) {
        return this->write_command(SH1107::SET_CONTRAST, contrast);
    }

    inline esp_err_t setOrientation(const uint8_t &flags = 0) {
        uint8_t scan_direction = SH1107::COMSCANINC;
        uint8_t remap = SH1107::SEGREMAP;

        if (flags & FLIP_HORIZONTAL)
            scan_direction = SH1107::COMSCANDEC;
        if (flags & FLIP_VERTICAL)
            remap = SH1107::SEGREMAPINV;

        return this->write_command(remap, scan_direction);
    }

    inline esp_err_t setDisplay(const uint8_t &flags = 0) {
        ESP_RETURN_ON_ERROR(this->setInverted(flags & COLOR_INVERT), TAG, "setInverted failed");
        ESP_RETURN_ON_ERROR(this->setState(!(flags & DISPLAY_OFF)), TAG, "setState failed");
        ESP_RETURN_ON_ERROR(this->setOrientation(flags), TAG, "setOrientation failed");

        return ESP_OK;
    }

    inline esp_err_t reset() {
        ESP_RETURN_ON_ERROR(this->write_commands(SH1107::initcmds, sizeof(SH1107::initcmds)), TAG, "initcmds failed");
        return ESP_OK;
    }

    inline esp_err_t init() {
        ESP_RETURN_ON_ERROR(I2C_DEVICE::init(), TAG, "i2c_device init failed");

        return this->reset();
    }
};

}
}