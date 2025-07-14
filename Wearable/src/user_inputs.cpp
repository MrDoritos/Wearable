#include "user_inputs.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "soc/soc.h"
#include "soc/gpio_reg.h"
#include "esp_system.h"

const gpio_num_t pins[] = {
    GPIO_NUM_15,
    GPIO_NUM_15,
    GPIO_NUM_16,
    GPIO_NUM_17,
    GPIO_NUM_18
};

static void IRAM_ATTR handle_button(void *arg) {
    for (int i = 0; i < 5; i++)
        wbl::dpad.buttons[i].rising_edge();
    /*
    uint32_t gpio_num = (uint32_t)arg;
    uint8_t index = 0;
    switch (gpio_num) {
        case 7:
            break;
        case 15:
        case 16:
        case 17:
        case 18:
            index = gpio_num - 14;
            break;
        default:
            return;
    }
    wbl::Dpad::Button &button = wbl::dpad.buttons[index];

    if (REG_READ(GPIO_IN_REG) & gpio_num)
        button.rising_edge();
    else
        button.falling_edge();
        */
}

inline void register_button(const gpio_num_t gpio_num) {
    gpio_config_t conf = {};

    conf.intr_type = GPIO_INTR_POSEDGE;
    conf.mode = GPIO_MODE_INPUT;
    conf.pin_bit_mask = 1ULL << gpio_num;
    //conf.pin_bit_mask = (1ULL << GPIO_NUM_7);
    conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    conf.pull_up_en = GPIO_PULLUP_ENABLE;

    ESP_ERROR_CHECK(gpio_config(&conf));
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(gpio_num, handle_button, (void*)gpio_num));
}

void wbl::Dpad::init() {
    //for (int i = 0; i < sizeof(pins)/sizeof(pins[0]); i++)
    //    register_button(pins[i]);
    register_button(pins[0]);
}

wbl::Dpad dpad;