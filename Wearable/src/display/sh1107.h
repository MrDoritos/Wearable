#pragma once

#include "sh1107.h"

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"

#include <inttypes.h>

namespace wbl {
namespace SH1107 {

template<typename _PORT, typename _SDA, typename _SCL, typename _CLK>
struct I2C_BUS {
    static constexpr typename PORT = _PORT;
    static constexpr typename SDA = _SDA;
    static constexpr typename SCL = _SCL;
    static constexpr typename CLK = _CLK;

    static i2c_master_bus_handle_t *bus = 0;

    inline bool init() {
        if (bus > 0)
            return 0;

        const i2c_master_bus_config_t bus_config = {
            .i2c_port = PORT,
            .sda_io_num = SDA,
            .scl_io_num = SCL,
            .clk_source = CLK,
            .glitch_ignore_cnt = 7,
            .flags.enable_internal_pullup = true,
        };

        i2c_new_master_bus(&bus_config, bus);

        return (bus > 0);
    }
};

using I2C_BUS_0 = I2C_BUS<I2C_NUM_0, CONFIG_I2C_MASTER_SDA, CONFIG_I2C_MASTER_SCL, CONFIG_I2C_MASTER_FREQUENCY>;

template<uint8_t _I2C_ADDRESS, uint32_t _I2C_CLOCK, uint16_t _I2C_TIMEOUT=1000, typename BUS=I2C_BUS_0>
struct I2C : public BUS {
    static constexpr uint8_t I2C_ADDRESS = _I2C_ADDRESS;

    static uint16_t I2C_TIMEOUT = _I2C_TIMEOUT;
    static uint32_t I2C_CLOCK = _I2C_CLOCK;

    static i2c_master_dev_handle_t *dev = 0;

    inline void write(const uint8_t *c, const uint8_t n) {
        i2c_master_transmit(dev, c, n, I2C_TIMEOUT / portTick_PERIOD_MS);
    }

    template<uint8_t prefix=0x0, typename ...T> 
    inline void write_command(const uint8_t &c, const T&... payload) {
        const uint8_t buf[] = {prefix, c, payload...};
        write(buf, sizeof(buf));
    }
    
    inline void write_payload(const uint8_t *c, const uint8_t &n, const uint8_t *pfx=0, const uint8_t &pfxn=0) {
        if (pfx != 0 && pfxn != 0)
            write(pfx, pfxn);
        write(c, n);
    }
    
    inline void write_commands(const uint8_t *c, const uint8_t &n, const uint8_t &prefix=0) {
        write(&prefix, 1);
        write(c, n);
    }

    inline bool init() {
        if (!BUS::init())
            return 1;

        if (dev > 0)
            return 0;

        i2c_device_config_t dev_config = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = I2C_ADDRESS,
            .scl_speed_hz = I2C_CLOCK,
        };

        i2c_master_bus_add_device(bus, &dev_config, dev);

        return (dev > 0);
    }
};

using I2C_SH1107 = I2C<0x3C, 400000>;

template<uint8_t _WIDTH, uint8_t _HEIGHT, typename I2C_DEVICE>
struct Display : public I2C_DEVICE {
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

    inline bool init() {
        if (!I2C::init())
            return 1;

        write_commands(initcmds, sizeof(initcmds));
        
        return 0;
    }
};

using GME128128 = Display<128, 128, I2C_SH1107>;

}
}