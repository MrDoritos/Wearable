#include "wbl_system.h"

#include "esp_check.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "driver/ledc.h"
#include "esp_timer.h"
#include "sprites.h"

#include <stdio.h>

namespace wbl {

static constexpr const char *TAG = "wbl::wbl_system.cpp";

#define VBAT_ATTEN ADC_ATTEN_DB_6
#define VBAT_CHANNEL ADC_CHANNEL_3
#define VBAT_UNIT ADC_UNIT_2
#define VBAT_GPIO GPIO_NUM_14

#define HAPTIC_GPIO GPIO_NUM_38
#define HAPTIC_CHANNEL LEDC_CHANNEL_1
#define HAPTIC_TIMER LEDC_TIMER_1
#define HAPTIC_MODE LEDC_LOW_SPEED_MODE
#define HAPTIC_BITS LEDC_TIMER_13_BIT
#define PIEZO_GPIO GPIO_NUM_38
#define PIEZO_CHANNEL LEDC_CHANNEL_2
#define PIEZO_TIMER LEDC_TIMER_1
#define PIEZO_MODE LEDC_LOW_SPEED_MODE
#define PIEZO_BITS LEDC_TIMER_13_BIT
#define TIMER_DURATION 10

wbl_System wbl_system;
adc_oneshot_unit_handle_t adc_handle;
adc_cali_handle_t adc_chars;
bool adc_calib = false;
esp_timer_handle_t timer_handle;

static void pwm_timer_callback(void *arg) {
    int32_t &dur = wbl_system.haptic_feedback_end;
    uint16_t &level = wbl_system.haptic_feedback_level;
    bool &tstate = wbl_system.haptic_timer_state;

    dur -= TIMER_DURATION;
    const bool state = dur > 0;

    if (!state) {
        dur = 0;
        tstate = false;
    }

    if (!tstate) {
        esp_timer_stop(timer_handle);
    }

    ledc_set_duty(HAPTIC_MODE, HAPTIC_CHANNEL, state ? level : 0);
    ledc_update_duty(HAPTIC_MODE, HAPTIC_CHANNEL);
}

esp_err_t init_pwm() {
    ledc_timer_config_t haptic_cfg = {
        .speed_mode = HAPTIC_MODE,
        .duty_resolution = HAPTIC_BITS,
        .timer_num = HAPTIC_TIMER,
        .freq_hz = 4000,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ledc_timer_config_t piezo_cfg = {
        .speed_mode = PIEZO_MODE,
        .duty_resolution = PIEZO_BITS,
        .timer_num = PIEZO_TIMER,
        .freq_hz = 4000,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ESP_RETURN_ON_ERROR(ledc_timer_config(&haptic_cfg), TAG, "Failed to init haptic");
    ESP_RETURN_ON_ERROR(ledc_timer_config(&piezo_cfg), TAG, "Failed to init piezo");

    ledc_channel_config_t channel_cfg = {
        .gpio_num = PIEZO_GPIO,
        .speed_mode = PIEZO_MODE,
        .channel = PIEZO_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = PIEZO_TIMER,
        .duty = 0,
        .hpoint = 0,
    };

    ESP_RETURN_ON_ERROR(ledc_channel_config(&channel_cfg), TAG, "Failed to init channel");

    esp_timer_create_args_t timer_cfg = {
        .callback = &pwm_timer_callback,
        .arg = (void*)timer_handle,
        .name = "pwm_timer",
    };

    ESP_RETURN_ON_ERROR(esp_timer_create(&timer_cfg, &timer_handle), TAG, "Failed to init pwm timer");

    return ESP_OK;
}

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

    ESP_RETURN_ON_ERROR(init_pwm(), TAG, "Failed to init pwm");

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

void wbl_System::setHapticFeedbackLevel(const float &level) {
    haptic_feedback_level = level * ((1 << HAPTIC_BITS)-1);
}

void wbl_System::setAudibleFeedbackLevel(const float &level) {
    audible_feedback_level = level * ((1 << PIEZO_BITS)-1);
}

void wbl_System::beginHapticFeedback(const float &level, const int32_t &duration) {
    setHapticFeedbackLevel(level);
    haptic_feedback_end = duration;
    printf("Level: %u Duration %li\n", haptic_feedback_level, haptic_feedback_end);

    if (!haptic_timer_state) {
        haptic_timer_state = true;
        ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handle, TIMER_DURATION));
    }
}

void wbl_System::beginAudibleFeedback(const float &level, const int32_t &duration) {
    setAudibleFeedbackLevel(level);
}

float wbl_System::getBatteryVoltage() {
    int raw = 0;
    adc_oneshot_read(adc_handle, VBAT_CHANNEL, &raw);
    const float vscale = 2.619789893;//(4.23/1.3);
    const float voff = 0.83;

    if (adc_calib) {
        int v = 0;
        adc_cali_raw_to_voltage(adc_chars, raw, &v);
        return v * 0.001 * vscale + voff;
    }

    return raw * (1600.0/3333.0) * 0.001 * vscale + voff;
    return 0;
}

static constexpr const int voltage_count = 3;
float vbats[voltage_count];
int voltage_offset = 0;

float wbl_System::getBatteryVoltageMean() {
    vbats[voltage_offset] = getBatteryVoltage();

    if (++voltage_offset >= voltage_count) voltage_offset = 0;

    float sum = 0;
    for (int i = 0; i < voltage_count; i++)
        sum += vbats[i];

    return sum / voltage_count;
}

float wbl_System::getBatteryLevel() {
    float v = getBatteryVoltageMean();

    const float pscale = 117.0f;
    const float poff = 3.2f;

    float p = (v - poff) * pscale;
    if (p > 100)
        return 100;
    if (p < 0)
        return 0;
    return p;
}

void wbl_System::setDisplayBrightness(const float &lux) {
    // 0lx - no light
    // 3.4lx - civil twilight
    // 20-50 - public areas with dark surroundings
    // 50-100 - living room
    // 100 - very dark overcast day
    // 320-500 - office lighting
    // 400 - sunrise/sunset
    // 1000 - overcast day
    // 10000-25000 indirect sunlight on a clear day
    // 32000-100000 direct sunlight

    float per = log10f(lux) * 0.25f;
    if (per < 0.1f) per = 0.1f;
    if (per > 1.0f) per = 1.0f;
    Sprites::display.setContrast((uint8_t)(per * 255));
}

}