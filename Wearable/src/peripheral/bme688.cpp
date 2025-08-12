#include "bme688.h"
#include "i2c_dev.h"

#include <stdio.h>

namespace wbl {

I2C i2c_bme688(I2C_BUS_1, I2C_BME688_ADDR, 1000, I2C_BME688_FREQ, 40000);
BME688 bme688;

static constexpr const char *TAG = "wbl::bme688.cpp";

esp_err_t BME688::init() {
    return ESP_OK;
}

}