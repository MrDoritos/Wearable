#include "mics6814.h"

#include "esp_check.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include <stdio.h>

namespace wbl {

static constexpr const char *TAG = "wbl::mics6814.cpp";

struct ADCUnit {
    adc_unit_t unit;

    adc_oneshot_unit_handle_t handle = nullptr;

    constexpr ADCUnit(const adc_unit_t &unit):
        unit(unit) {}

    esp_err_t init() {
        adc_oneshot_unit_init_cfg_t adc_conf = {
            .unit_id = unit,
        };

        ESP_RETURN_ON_ERROR(adc_oneshot_new_unit(&adc_conf, &handle), TAG, "Failed to create adc unit");

        return ESP_OK;
    }
};

struct ADCChannel {
    ADCUnit &unit;
    adc_channel_t channel;
    gpio_num_t gpio;
    adc_atten_t atten;
    adc_bitwidth_t bitwidth;

    adc_cali_handle_t chars = nullptr;
    bool calib = false;

    constexpr ADCChannel(ADCUnit &unit,
                         const adc_channel_t &channel,
                         const gpio_num_t &gpio,
                         const adc_atten_t& atten = ADC_ATTEN_DB_6,
                         const adc_bitwidth_t &bitwidth = ADC_BITWIDTH_DEFAULT):
        unit(unit),
        channel(channel),
        gpio(gpio),
        atten(atten),
        bitwidth(bitwidth) {}

    esp_err_t init() {
        adc_oneshot_chan_cfg_t adc_chan_conf = {
            .atten = atten,
            .bitwidth = bitwidth,
        };

        ESP_RETURN_ON_ERROR(adc_oneshot_config_channel(unit.handle, channel, &adc_chan_conf), TAG, "Failed to create adc channel");

        #ifdef ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
            adc_cali_curve_fitting_config_t cal_conf = {
                .unit_id = unit.unit,
                .chan = channel,
                .atten = atten,
                .bitwidth = bitwidth,
            };

            ESP_RETURN_ON_ERROR(adc_cali_create_scheme_curve_fitting(&cal_conf, &chars), TAG, "Failed to create curve fitting scheme");
            calib = true;
        #endif

        return ESP_OK;
    }

    int getmV() {
        int raw = 0;
        adc_oneshot_read(unit.handle, channel, &raw);

        if (calib) {
            int v = 0;
            adc_cali_raw_to_voltage(chars, raw, &v);
            return v;
        }

        return raw;
    }

    float getVoltage() {
        return float(getmV()) * 0.001f;
    }
};

ADCUnit unit(ADC_UNIT_1);
ADCChannel v_co(unit, ADC_CHANNEL_1, GPIO_NUM_2);
ADCChannel v_nh3(unit, ADC_CHANNEL_0, GPIO_NUM_1);
ADCChannel v_no2(unit, ADC_CHANNEL_2, GPIO_NUM_3);

MICS6814 mics6814;

esp_err_t MICS6814::init() {
    ESP_RETURN_ON_ERROR(unit.init(), TAG, "Failed to init unit");

    ESP_RETURN_ON_ERROR(v_co.init(), TAG, "Failed to init v_co");
    ESP_RETURN_ON_ERROR(v_nh3.init(), TAG, "Failed to init v_nh3");
    ESP_RETURN_ON_ERROR(v_no2.init(), TAG, "Failed to init v_no2");

    return ESP_OK;
}

float MICS6814::getCOVoltage() {
    return v_co.getVoltage();
}

float MICS6814::getNH3Voltage() {
    return v_nh3.getVoltage();
}

float MICS6814::getNO2Voltage() {
    return v_no2.getVoltage();
}

uint16_t MICS6814::getCOmillivolts() {
    return (uint16_t)v_co.getmV();
}

uint16_t MICS6814::getNH3millivolts() {
    return (uint16_t)v_nh3.getmV();
}

uint16_t MICS6814::getNO2millivolts() {
    return (uint16_t)v_no2.getmV();
}

}