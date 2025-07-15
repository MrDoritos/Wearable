#include "user_inputs.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "soc/soc.h"
#include "soc/gpio_reg.h"
#include "esp_intr_types.h"
#include "esp_err.h"
#include "esp_check.h"

#include <stdio.h>

wbl::Dpad wbl::dpad = wbl::Dpad();

static const char *TAG = "USER_INPUTS";

static constexpr gpio_num_t pins[] = {
    GPIO_NUM_7,
    GPIO_NUM_15,
    GPIO_NUM_16,
    GPIO_NUM_17,
    GPIO_NUM_18
};

static constexpr int pin_count = sizeof(pins)/sizeof(pins[0]);

IRAM_ATTR static void handle_button(void *arg) {
    for (int i = 0; i < pin_count; i++) {
        if (!gpio_get_level(pins[i]))
            wbl::dpad.buttons[i].rising_edge();
        else
            wbl::dpad.buttons[i].falling_edge();
    }
}

wbl::Dpad::~Dpad() {}

esp_err_t wbl::Dpad::init() {
    volatile uint32_t pin_bit_mask = 0;

    for (int i = 0; i < pin_count; i++)
        pin_bit_mask |= (1ULL << pins[i]);

    static gpio_config_t conf = {};

    conf.intr_type = GPIO_INTR_ANYEDGE;
    conf.mode = GPIO_MODE_INPUT;
    conf.pin_bit_mask = pin_bit_mask;
    conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    conf.pull_up_en = GPIO_PULLUP_ENABLE;

    ESP_RETURN_ON_ERROR(gpio_config(&conf), TAG, "gpio_config");
    ESP_RETURN_ON_ERROR(gpio_install_isr_service(ESP_INTR_FLAG_SHARED | ESP_INTR_FLAG_IRAM), TAG, "gpio_install_isr_service");
    
    for (int i = 0; i < pin_count; i++)
        ESP_RETURN_ON_ERROR(gpio_intr_enable(pins[i]), TAG, "gpio_intr_enable");

    ESP_RETURN_ON_ERROR(gpio_isr_register(handle_button, (void*)pin_bit_mask, ESP_INTR_FLAG_SHARED | ESP_INTR_FLAG_IRAM, nullptr), TAG, "gpio_isr_register");

    return ESP_OK;
}