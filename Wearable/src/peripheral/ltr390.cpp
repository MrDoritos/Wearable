#include "ltr390.h"
#include "i2c_dev.h"

#include "Adafruit_LTR390.h"

#include <stdio.h>

namespace wbl {

I2C i2c_ltr390(I2C_BUS_1, I2C_LTR390_ADDR, 1000, I2C_LTR390_FREQ, 40000);
LTR390 ltr390;
Adafruit_LTR390 s_ltr390;

static constexpr const char *TAG = "wbl::ltr390.cpp";

esp_err_t LTR390::init() {
    ESP_RETURN_ON_ERROR(i2c_ltr390.init(), TAG, "failed to init ltr390 i2c");

    ESP_RETURN_ON_FALSE(s_ltr390.begin(), 1, TAG, "failed to init ltr390 driver");

    setMode(true);
    setGain(LTR390_GAIN_3);
    setResolution(LTR390_RESOLUTION_16BIT);
    setMeasurementRate(200);
    setState(true);
    
    printf("gain: %i, mode: %i, resolution: %i, active: %i\n", s_ltr390.getGain(), s_ltr390.getMode(), s_ltr390.getResolution(), s_ltr390.enabled());
    printf("status: %i\n", i2c_ltr390.read_register(LTR390_MAIN_STATUS));
    printf("ctrl: %i\n", i2c_ltr390.read_register(LTR390_MAIN_CTRL));
    printf("als_uvs_meas_rate: %i\n", i2c_ltr390.read_register(LTR390_MEAS_RATE));
    printf("als_uvs_gain: %i\n", i2c_ltr390.read_register(LTR390_GAIN));
    
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

float LTR390::getLux(const uint32_t &als) {
    float lux;

    lux = (als * 0.523636f) + 7.9449;

    return lux;
}

float LTR390::getLux() {
    return getLux(getALS());
}

void LTR390::setState(const bool &state) {
    s_ltr390.enable(state);
    this->state = isActive();
}

bool LTR390::isActive() {
    return s_ltr390.enabled();
}

void LTR390::setMode(const bool &uv_mode) {
    s_ltr390.setMode(uv_mode ? LTR390_MODE_UVS : LTR390_MODE_ALS);
    this->uv_mode = uv_mode;
}

void LTR390::setMeasurementRate(const uint32_t &ms) {
    uint8_t flag = 0;

    if (ms > 25) flag = 0b001;
    if (ms > 50) flag = 0b010;
    if (ms > 100) flag = 0b011;
    if (ms > 200) flag = 0b100;
    if (ms > 500) flag = 0b101;
    if (ms > 1000) flag = 0b111;

    uint8_t reg = i2c_ltr390.read_register(LTR390_MEAS_RATE);
    reg &= ~0b111;
    reg |= flag;
    i2c_ltr390.write_command_prefix(LTR390_MEAS_RATE, reg);

    measurement_rate_flag = i2c_ltr390.read_register(LTR390_MEAS_RATE)&0b111;

    const uint32_t rates[] = {
        25, 50, 100, 200, 500, 1000, 2000, 2000
    };

    measurement_rate = rates[measurement_rate_flag];
}

void LTR390::setGain(const uint8_t &gain) {
    s_ltr390.setGain((ltr390_gain_t)gain);
    this->gain = (uint8_t)s_ltr390.getGain();
}

void LTR390::setResolution(const uint8_t &resolution) {
    s_ltr390.setResolution((ltr390_resolution_t)resolution);
    this->resolution = (uint8_t)s_ltr390.getResolution();
}

}