#pragma once

#include "sh1107.h"
#include <inttypes.h>

namespace wbl {
namespace SH1107 {

template<uint8_t _I2C_ADDRESS, uint32_t _I2C_CLOCK>
struct I2C {
    static constexpr uint8_t I2C_ADDRESS = _I2C_ADDRESS;
    uint32_t I2C_CLOCK = _I2C_CLOCK;

    template<typename ...T> inline void write_command(const uint8_t &c, const T&... payload) {
        const uint8_t buf[] = {0x00, c, payload...};
        Wire.beginTransmission(I2C_ADDRESS);
        Wire.write(buf, sizeof(buf));
        Wire.endTransmission();
    }
    
    inline void write_payload(const uint8_t *c, const uint8_t &n, const uint8_t *pfx=0, const uint8_t &pfxn=0) {
        Wire.beginTransmission(I2C_ADDRESS);
        if (pfx != 0 && pfxn != 0)
        Wire.write(pfx, pfxn);
        Wire.write(c, n);
        Wire.endTransmission();
    }
    
    inline void write_commands(const uint8_t *c, const uint8_t &n) {
        Wire.beginTransmission(I2C_ADDRESS);
        Wire.write(0);
        Wire.write(c, n);
        Wire.endTransmission();
    }

    inline void init() {
        //set clock
    }
};

template<uint8_t _WIDTH, uint8_t _HEIGHT, uint8_t _I2C_ADDRESS, uint32_t _I2C_CLOCK>
struct Display : public I2C<_I2C_ADDRESS> {
    static constexpr uint8_t WIDTH = _WIDTH;
    static constexpr uint8_t HEIGHT = _HEIGHT;
    static constexpr uint32_t I2C_CLOCK = _I2C_CLOCK;
    static constexpr uint8_t PAGES = ((HEIGHT + 7) / 8);
    static constexpr uint8_t BYTES_PER_PAGE = WIDTH;
    
    inline void setPagePosition(const uint8_t &page, const uint8_t &page_start=0, const uint8_t &page_start_offset=0) {
        const uint8_t cmd[] = {
            SET_PAGEADDR + page, 0x10 + ((page_start + page_start_offset) >> 4), (page_start + page_start_offset) & 0xf
        };
        write_commands(cmd, sizeof(cmd));
    }
  
    inline void clearDisplay(const uint8_t &color=0x0) {
        const uint8_t size = 32;
        const uint8_t dc = 0x40;
        uint8_t clearData[size];
        for (uint8_t i = 0; i < size; i++)
        clearData[i] = color;
        for (uint8_t p = 0; p < PAGES; p++) {
        uint8_t bytes_remaining = BYTES_PER_PAGE;
        setPagePosition(p);
        while (bytes_remaining>0) {
            const uint8_t count = bytes_remaining > size ? size : bytes_remaining;
            write_payload(clearData, count, &dc, 1);
            bytes_remaining -= count;
        }
        }
    }

    inline void init() {
        I2C::init();
        write_commands(initcmds, sizeof(initcmds));
    }
};

using GME128128 = Display<128, 128, 0x3C, 400000>;

}
}