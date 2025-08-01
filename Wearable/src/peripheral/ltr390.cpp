#include "ltr390.h"
#include "i2c_dev.h"

#include "Adafruit_LTR390.h"

namespace wbl {

I2C i2c_ltr390(I2C_BUS_1, I2C_LTR390_ADDR, 1000, I2C_LTR390_FREQ, 40000);
LTR390 ltr390;
Adafruit_LTR390 s_ltr390;

static constexpr const char *TAG = "wbl::ltr390.cpp";

esp_err_t LTR390::init() {
    ESP_RETURN_ON_ERROR(i2c_ltr390.init(), TAG, "failed to init ltr390 i2c");

    ESP_RETURN_ON_FALSE(s_ltr390.begin(), 1, TAG, "failed to init ltr390 driver");

    s_ltr390.setMode(LTR390_MODE_UVS);
    uv_mode = true;

    return ESP_OK;
}

uint32_t LTR390::getALS() {
    if (uv_mode) {
        s_ltr390.setMode(LTR390_MODE_ALS);
        uv_mode = false;
    }
    return s_ltr390.readALS();
}

uint32_t LTR390::getUVS() {
    if (!uv_mode) {
        s_ltr390.setMode(LTR390_MODE_UVS);
        uv_mode = true;
    }
    return s_ltr390.readUVS();
}

}