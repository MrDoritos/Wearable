#include "wbl_system.h"

#include "esp_check.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

namespace wbl {

static constexpr const char *TAG = "wbl::wbl_system.cpp";

#define VBAT_ATTEN ADC_ATTEN_DB_12
#define VBAT_CHANNEL ADC_CHANNEL_3
#define VBAT_UNIT ADC_UNIT_2
#define VBAT_GPIO GPIO_NUM_14

wbl_System wbl_system;
adc_oneshot_unit_handle_t adc_handle;
adc_cali_handle_t adc_chars;
bool adc_calib = false;

esp_err_t wbl_System::init() {
    adc_oneshot_unit_init_cfg_t adc_conf = {
        .unit_id = VBAT_UNIT,
    };

    ESP_RETURN_ON_ERROR(adc_oneshot_new_unit(&adc_conf, &adc_handle), TAG, "Failed to create oneshot unit");

    adc_oneshot_chan_cfg_t adc_chan_conf = {
        .atten = VBAT_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    ESP_RETURN_ON_ERROR(adc_oneshot_config_channel(adc_handle, VBAT_CHANNEL, &adc_chan_conf), TAG, "Failed to create adc channel");

    #ifdef ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
        adc_cali_curve_fitting_config_t cal_conf = {
            .unit_id = VBAT_UNIT,
            .chan = VBAT_CHANNEL,
            .atten = VBAT_ATTEN,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ESP_RETURN_ON_ERROR(adc_cali_create_scheme_curve_fitting(&cal_conf, &adc_chars), TAG, "Failed to create curve fitting scheme");
        adc_calib = true;
    #endif

    ESP_RETURN_ON_ERROR(gpio_set_pull_mode(VBAT_GPIO, GPIO_PULLDOWN_ONLY), TAG, "Failed to set VBAT pulldown");

    return ESP_OK;
}

esp_err_t wbl_System::update() {
    return ESP_OK;
}

void wbl_System::setHapticFeedback(const bool &state) {
    use_haptic_feedback = state;
}

void wbl_System::setAudibleFeedback(const bool &state) {
    use_audible_feedback = state;
}

void wbl_System::setHapticFeedbackLevel(const uint8_t &level) {
    haptic_feedback_level = level;
}

void wbl_System::setAudibleFeedbackLevel(const uint8_t &level) {
    audible_feedback_level = level;
}

void wbl_System::beginHapticFeedback(const uint8_t &level, const int32_t &duration) {
    setHapticFeedbackLevel(level);
}

void wbl_System::beginAudibleFeedback(const uint8_t &level, const int32_t &duration) {
    setAudibleFeedbackLevel(level);
}

float wbl_System::getBatteryVoltage() {
    int raw = 0;
    adc_oneshot_read(adc_handle, VBAT_CHANNEL, &raw);

    if (adc_calib) {
        int v = 0;
        adc_cali_raw_to_voltage(adc_chars, raw, &v);
        return v * 0.001;
    }

    return raw * (3100.0/4095.0) * 0.001;
}

float wbl_System::getBatteryLevel() {
    float v = getBatteryVoltage();

    float p = ((v * 2) - 3.6) * (1/0.03) * 5;
    if (p > 100)
        return 100;
    if (p < 0)
        return 0;
    return p;
}

}