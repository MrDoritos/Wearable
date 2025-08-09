#pragma once

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_types.h"
#include "esp_timer.h"

namespace wbl {

struct I2C_BUS {
    static constexpr const char *TAG = "wbl::I2C_BUS";
    uint8_t PORT;
    gpio_num_t SDA;
    gpio_num_t SCL;
    i2c_clock_source_t CLK;
    bool INTERNAL_PULLUP;
    size_t trans_queue_depth;
    

    i2c_master_bus_handle_t bus;

    constexpr I2C_BUS(const uint8_t &port, const gpio_num_t &sda, const gpio_num_t &scl, const i2c_clock_source_t &clk, const bool &internal_pullup=true)
    :PORT(port),SDA(sda),SCL(scl),CLK(clk),INTERNAL_PULLUP(internal_pullup),trans_queue_depth(0){}

    inline esp_err_t probe(uint16_t device_id) {
        if (trans_queue_depth)
            return ESP_OK;

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
            .clk_source = CLK,
            .glitch_ignore_cnt = 7,
            .trans_queue_depth = trans_queue_depth,
            .flags = {
                .enable_internal_pullup = INTERNAL_PULLUP
            },
        };

        ESP_RETURN_ON_ERROR(i2c_new_master_bus(&bus_config, &bus), TAG, "i2c_new_master_bus failed");

        if (bus != nullptr)
            return ESP_OK;
        
        return ESP_ERR_INVALID_STATE;
    }
};

extern I2C_BUS I2C_BUS_0, I2C_BUS_1;

struct I2C {
    static constexpr const char *TAG = "wbl::I2C";

    I2C_BUS& bus;

    uint16_t I2C_ADDRESS;
    uint16_t I2C_TIMEOUT;
    uint32_t I2C_CLOCK;
    uint16_t SCL_WAIT;
    uint8_t disable_ack_check;

    i2c_master_dev_handle_t dev = 0;

    constexpr I2C(I2C_BUS &bus, const uint16_t &address, const uint16_t &timeout, const uint32_t &clock, const uint16_t &stretch)
    :bus(bus),I2C_ADDRESS(address),I2C_TIMEOUT(timeout),I2C_CLOCK(clock),SCL_WAIT(stretch),disable_ack_check(1){}

    inline esp_err_t write(const uint8_t *c, const uint8_t n) {
        ESP_RETURN_ON_ERROR(i2c_master_transmit(dev, c, n, I2C_TIMEOUT / portTICK_PERIOD_MS), TAG, "i2c_master_transmit failed");

        return ESP_OK;
    }

    inline esp_err_t write(const uint8_t &v) {
        return write(&v, 1);
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
        const uint8_t count = (pfx != nullptr) ? n + pfxn : n;

        uint8_t batch[count];
        if (pfx) memcpy(batch, pfx, pfxn);
        if (c) memcpy(batch + pfxn, c, n);

        ESP_RETURN_ON_ERROR(write(batch, count), TAG, "write_payload failed");

        return ESP_OK;
    }
    
    inline esp_err_t write_commands(const uint8_t *c, const uint8_t &n, const uint8_t &prefix=0) {
        uint8_t inmem[n+1] = {prefix};
        memcpy(inmem+1, c, n);
        ESP_RETURN_ON_ERROR(write(inmem, n+1), TAG, "write_commands failed");

        return ESP_OK;
    }

    inline esp_err_t read(uint8_t *buffer, const uint16_t &bytes) {
        ESP_RETURN_ON_ERROR(i2c_master_receive(dev, buffer, bytes, I2C_TIMEOUT / portTICK_PERIOD_MS), TAG, "read failed");

        return ESP_OK;
    }

    inline uint8_t read_register(const uint8_t &reg) {
        uint8_t ret = 0;
        ESP_ERROR_CHECK(write_read(&reg, 1, &ret, 1));
        return ret;
    }

    inline esp_err_t write_read(const uint8_t *c, const uint8_t &n, uint8_t *recv, const uint16_t &recv_len) {
        ESP_RETURN_ON_ERROR(i2c_master_transmit_receive(dev, c, n, recv, recv_len, I2C_TIMEOUT / portTICK_PERIOD_MS), TAG, "write_read failed");

        return ESP_OK;
    }

    inline esp_err_t probe() {
        ESP_RETURN_ON_ERROR(bus.probe(I2C_ADDRESS), TAG, "failed to probe device");

        return ESP_OK;
    }

    inline esp_err_t init() {
        ESP_RETURN_ON_ERROR(bus.init(), TAG, "i2c_bus init failed");

        if (dev != nullptr)
            return ESP_OK;

        ESP_RETURN_ON_ERROR(!bus.bus, TAG, "bus invalid");

        i2c_device_config_t dev_config = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = I2C_ADDRESS,
            .scl_speed_hz = I2C_CLOCK,
            .scl_wait_us = SCL_WAIT,
            .flags = {
                .disable_ack_check = disable_ack_check,
            },
        };

        ESP_RETURN_ON_ERROR(probe(), TAG, "failed to check device during init");

        ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(bus.bus, &dev_config, &dev), TAG, "i2c_master_bus_add_device failed");

        if (dev != nullptr)
            return ESP_OK;

        return ESP_ERR_INVALID_STATE;
    }
};

}