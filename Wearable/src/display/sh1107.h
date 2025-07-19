#pragma once

#include "sh1107_defs.h"
#include "config.h"

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_types.h"
#include "esp_timer.h"

#include <inttypes.h>

namespace wbl {
namespace SH1107 {

template<uint8_t _PORT, gpio_num_t _SDA, gpio_num_t _SCL, i2c_clock_source_t _CLK>
struct I2C_BUS {
    static constexpr const char *TAG = "wbl::SH1107::I2C_BUS";
    static constexpr uint8_t PORT = _PORT;
    static constexpr gpio_num_t SDA = _SDA;
    static constexpr gpio_num_t SCL = _SCL;

    i2c_master_bus_handle_t bus = 0;

    inline esp_err_t probe(uint16_t device_id) {
        ESP_RETURN_ON_ERROR(i2c_master_probe(bus, device_id, 1000 / portTICK_PERIOD_MS), TAG, "failed to probe device %i", device_id);

        return ESP_OK;
    }

    inline esp_err_t init() {
        if (bus != nullptr)
            return ESP_OK;

        const i2c_master_bus_config_t bus_config = {
            .i2c_port = PORT,
            .sda_io_num = SDA,
            .scl_io_num = SCL,
            .clk_source = _CLK,
            .glitch_ignore_cnt = 7,
            .flags = {
                .enable_internal_pullup = true
            },
        };

        ESP_RETURN_ON_ERROR(i2c_new_master_bus(&bus_config, &bus), TAG, "i2c_new_master_bus failed");

        if (bus != nullptr)
            return ESP_OK;
        
        return ESP_ERR_INVALID_STATE;
    }
};

using I2C_BUS_0 = I2C_BUS<I2C_NUM_0, GPIO_NUM_6, GPIO_NUM_5, I2C_CLK_SRC_DEFAULT>;

template<uint16_t _I2C_ADDRESS, uint32_t _I2C_CLOCK, uint16_t _I2C_TIMEOUT=1000, typename BUS=I2C_BUS_0>
struct I2C : public BUS {
    static constexpr const char *TAG = "wbl::SH1107::I2C";
    static constexpr uint16_t I2C_ADDRESS = _I2C_ADDRESS;

    uint16_t I2C_TIMEOUT = _I2C_TIMEOUT;
    uint32_t I2C_CLOCK = _I2C_CLOCK;

    i2c_master_dev_handle_t dev = 0;

    inline esp_err_t write(const uint8_t *c, const uint8_t n) {
        //for (int i = 0; i < n; i++)
        //    printf("%x ", c[i]);
        //printf("\n");
        
        ESP_RETURN_ON_ERROR(i2c_master_transmit(dev, c, n, I2C_TIMEOUT / portTICK_PERIOD_MS), TAG, "i2c_master_transmit failed");

        return ESP_OK;
    }

    template<uint8_t prefix=0x0, typename ...T> 
    inline esp_err_t write_command(const uint8_t &c, const T&... payload) {
        const uint8_t buf[] = {prefix, c, payload...};
        ESP_RETURN_ON_ERROR(write(buf, sizeof(buf)), TAG, "write_command failed");

        return ESP_OK;
    }

    template<typename ...T>
    inline esp_err_t write_command_prefix(const uint8_t &prefix, const uint8_t &c, const T&... payload) {
        const uint8_t buf[] = {prefix, c, payload...};
        ESP_RETURN_ON_ERROR(write(buf, sizeof(buf)), TAG, "write_command_prefix failed");

        return ESP_OK;
    }
    
    inline esp_err_t write_payload(const uint8_t *c, const uint8_t &n, const uint8_t *pfx=nullptr, const uint8_t &pfxn=0) {
        //const uint8_t count = (pfx != nullptr) ? 2 : 1;
        const uint8_t count = (pfx != nullptr) ? n + pfxn : n;
        //i2c_master_transmit_multi_buffer_info_t mbt[2] = {
        //    {.write_buffer = (uint8_t*)c, .buffer_size = n },
        //    {.write_buffer = (uint8_t*)pfx, .buffer_size = pfxn },
        //};

        uint8_t batch[count];
        if (pfx) memcpy(batch, pfx, pfxn);
        if (c) memcpy(batch + pfxn, c, n);

        //ESP_RETURN_ON_ERROR(i2c_master_multi_buffer_transmit(dev, mbt, count, I2C_TIMEOUT / portTICK_PERIOD_MS), TAG, "i2c_master_multi_buffer_transmit failed");
        ESP_RETURN_ON_ERROR(write(batch, count), TAG, "write_payload failed");

        /*
        if (pfx != 0 && pfxn != 0)
            ESP_RETURN_ON_ERROR(write(pfx, pfxn), TAG, "write_payload prefix failed");
        ESP_RETURN_ON_ERROR(write(c, n), TAG, "write_payload payload failed");
        */

        return ESP_OK;
    }
    
    inline esp_err_t write_commands(const uint8_t *c, const uint8_t &n, const uint8_t &prefix=0) {
        //ESP_RETURN_ON_ERROR(write(&prefix, 1), TAG, "write_commands prefix failed");
        //ESP_RETURN_ON_ERROR(write(c, n), TAG, "write_commands commands list failed");
        uint8_t inmem[n+1] = {prefix};
        memcpy(inmem+1, c, n);
        ESP_RETURN_ON_ERROR(write(inmem, n+1), TAG, "write_commands failed");

        return ESP_OK;
    }

    inline esp_err_t probe() {
        ESP_RETURN_ON_ERROR(BUS::probe(I2C_ADDRESS), TAG, "failed to probe device");

        return ESP_OK;
    }

    inline esp_err_t init() {
        ESP_RETURN_ON_ERROR(BUS::init(), TAG, "i2c_bus init failed");

        if (dev != nullptr)
            return ESP_OK;

        ESP_RETURN_ON_ERROR(!this->bus, TAG, "bus invalid");

        i2c_device_config_t dev_config = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = I2C_ADDRESS,
            .scl_speed_hz = I2C_CLOCK,
            .scl_wait_us = 0,
        };

        ESP_RETURN_ON_ERROR(probe(), TAG, "failed to check device during init");

        ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(this->bus, &dev_config, &dev), TAG, "i2c_master_bus_add_device failed");

        if (dev != nullptr)
            return ESP_OK;

        return ESP_ERR_INVALID_STATE;
    }
};

using I2C_SH1107 = I2C<I2C_SH1107_ADDR, I2C_DISPLAY_FREQ>;

template<uint8_t _WIDTH, uint8_t _HEIGHT, typename I2C_DEVICE>
struct Display : public I2C_DEVICE {
    static constexpr const char *TAG = "wbl::SH1107::Display";
    static constexpr uint8_t WIDTH = _WIDTH;
    static constexpr uint8_t HEIGHT = _HEIGHT;
    static constexpr uint8_t PAGES = ((HEIGHT + 7) / 8);
    static constexpr uint8_t BYTES_PER_PAGE = WIDTH;
    
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

    inline esp_err_t init() {
        ESP_RETURN_ON_ERROR(I2C_DEVICE::init(), TAG, "i2c_device init failed");

        ESP_RETURN_ON_ERROR(this->write_commands(SH1107::initcmds, sizeof(SH1107::initcmds)), TAG, "initcmds failed");
        //ESP_RETURN_ON_ERROR(this->write_command(SH1107::ALL_ON), TAG, "ALL_ON failed");

        return ESP_OK;
    }
};

using GME128128 = Display<128, 128, I2C_SH1107>;

}
}