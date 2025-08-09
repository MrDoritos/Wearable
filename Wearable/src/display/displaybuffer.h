#pragma once

#include "framebuffer.h"
#include "sh1107.h"
#include "config.h"

namespace wbl {

template<uint8_t WIDTH, uint8_t HEIGHT, uint8_t BPP>
struct FramebufferPageT : public FramebufferT<StaticbufferT<WIDTH, HEIGHT, BPP>> {
    bool rotate = false;

    inline constexpr fb getOffset(const fb &x, const fb &y) const {
        return rotate ?
        ((x / 8) * this->HEIGHT + y): 
        ((y / 8) * this->WIDTH + x);
    }

    inline constexpr fb getBitOffset(const fb &x, const fb &y) const {
        return rotate ?
        (x & 7):
        (y & 7);
    }

    inline constexpr fb getByteMask(const fb &x, const fb &y) const {
        return this->getBitMask() << this->getBitOffset(x, y);
    }

    inline constexpr void putPixel(const fb &x, const fb &y, const pixel &px) {
        const fb offset=this->getOffset(x,y);//(y/8)*this->WIDTH+x;
        const fb shift=this->getBitOffset(x,y);//(y&7);
        this->buffer[offset] &= ~(1<<shift);
        this->buffer[offset] |= (px<<shift);
        //this->buffer[offset] 
        //const fb offset = this->getOffset(x, y);
        //const fb bits = this->getBitOffset(x, y);
        //const fb bitmask = this->getBitMask();
        //const fb bytemask = bitmask << bits;
        //this->buffer[offset] &= ~bytemask;
        //this->buffer[offset] |= (px & bitmask) << bits;
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

    enum PageState : uint8_t {
        PAGE_END = 1,
        PAGE_START = 2,
        PAGE_CONT = 4
    };

    static uint8_t page_pos;
    static uint8_t bytes_rem;
    static uint8_t *page_ptr;
    static uint8_t page_state;
    static bool draw_transaction;
    static uint8_t cmd_buffer[33];

    /*IRAM_ATTR*/ static bool on_master_done(i2c_master_dev_handle_t dev, const i2c_master_event_data_t *data, void *arg) {
        if (!draw_transaction)
            return false;

        const uint8_t size = 16;
        const uint8_t dc = 0x40;
        const uint8_t count = bytes_rem > size ? size : bytes_rem;

        if (page_state & PAGE_START) { // send page pos, set bytes rem
            bytes_rem = Display::BYTES_PER_PAGE;
            cmd_buffer[0] = 0;
            cmd_buffer[1] = SH1107::SET_PAGEADDR + page_pos;
            cmd_buffer[2] = 0x10;
            cmd_buffer[3] = 0;
            page_state = PAGE_CONT;
            i2c_master_transmit(dev, cmd_buffer, 4, 100);
            page_pos++;
            if (page_pos >= Display::PAGES) {
                page_state = PAGE_END;
                draw_transaction = false;
            }
            return false;
        }

        if (page_state & PAGE_CONT) {
            cmd_buffer[0] = dc;
            for (int i = 0; i < count; i++)
                cmd_buffer[i+1] = page_ptr[i];
            i2c_master_transmit(dev, cmd_buffer, 17, 100);

            page_ptr += count;
            bytes_rem -= count;

            if (bytes_rem < 1) {
                page_state = PAGE_START;
            }

            return false;
        }

        return false;
    }

    inline esp_err_t async_flush() {
        if (draw_transaction)
            return ESP_OK;

        page_pos = 0;
        draw_transaction = true;
        page_state = PAGE_START;
        page_ptr = &this->buffer[0]; 

        Display::setPagePosition(0);

        return ESP_OK;
    }

    inline esp_err_t init() {
        I2C::bus.trans_queue_depth = 400;

        ESP_RETURN_ON_ERROR(Display::init(), TAG, "display init failed");
        WBL_D("Display init");

        Frame::clear();
        //ESP_RETURN_ON_ERROR(Display::clearDisplay(), TAG, "clear display failed");
        WBL_D("Display clear");
        
        static i2c_master_event_callbacks_t cbs = {
            .on_trans_done = on_master_done
        };

        draw_transaction = false;

        ESP_RETURN_ON_ERROR(i2c_master_register_event_callbacks(I2C::dev, &cbs, nullptr), TAG, "display set callback failed");

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

//using I2C_SH1107 = I2C<I2C_SH1107_ADDR, I2C_DISPLAY_FREQ>;
//using GME128128 = SH1107::Display<128, 128, I2C_SH1107>;
using GME128128 = SH1107::Display<128, 128, I2C_SH1107_ADDR, I2C_DISPLAY_FREQ>;
using DisplayBuffer = DisplayBufferT<GME128128>;

}