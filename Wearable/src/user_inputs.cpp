#include "user_inputs.h"

#define USE_GPIO_ISR_SERVICE 

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "soc/soc.h"
#include "soc/gpio_reg.h"
#include "esp_system.h"
#include "esp_intr_types.h"

#include <stdio.h>

/*static */wbl::Dpad wbl::dpad = wbl::Dpad();

static const char *TAG = "USER_INPUTS";

static constexpr gpio_num_t pins[] = {
    GPIO_NUM_7,
    GPIO_NUM_15,
    GPIO_NUM_16,
    GPIO_NUM_17,
    GPIO_NUM_18
};

IRAM_ATTR static void handle_button(void *arg) {
    //uint32_t gpio_status = REG_READ(GPIO_INT);
    //uint32_t io_num_mask = (uint32_t)arg;

    //REG_WRITE(GPIO_INTR_CLR_REG, gpio_status);

    for (int i = 0; i < 5; i++) {
        /*
        gpio_num_t current_pin = pins[i];
        if (io_num_mask & (1ULL << current_pin)) {
            if (gpio_status & (1ULL << current_pin)) {
                bool level = gpio_get_level(current_pin);

                if (level) {
                    wbl::dpad.buttons[i].falling_edge();
                } else {
                    wbl::dpad.buttons[i].rising_edge();
                }
            }
        }
        */
        
        if (!(REG_READ(GPIO_IN_REG) & (1ULL << pins[i])))
        //if (!gpio_get_level(pins[i]))
            wbl::dpad.buttons[i].rising_edge();
        else
            wbl::dpad.buttons[i].falling_edge();
    }
}

void wbl::Dpad::init() {
    //printf("reference init %p\n", wbl::dpad.buttons);
    printf("register\n");
    fflush(stdout);

    uint32_t pin_bit_mask = 0;

    for (int i = 0; i < 5; i++)
        pin_bit_mask |= (1ULL << pins[i]);

    static gpio_config_t conf = {};

    conf.intr_type = GPIO_INTR_ANYEDGE;
    conf.mode = GPIO_MODE_INPUT;
    conf.pin_bit_mask = pin_bit_mask;
    conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    conf.pull_up_en = GPIO_PULLUP_ENABLE;

    ESP_ERROR_CHECK(gpio_config(&conf));
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_SHARED | ESP_INTR_FLAG_IRAM));
    //ESP_ERROR_CHECK(gpio_install_isr_service(0));
    
    for (int i = 0; i < 5; i++)
        //ESP_ERROR_CHECK(gpio_isr_handler_add(pins[i], handle_button, (void*)pins[i]));
        ESP_ERROR_CHECK(gpio_intr_enable(pins[i]));

    static gpio_isr_handle_t handle = {};
    ESP_ERROR_CHECK(gpio_isr_register(handle_button, (void*)pin_bit_mask, ESP_INTR_FLAG_SHARED | ESP_INTR_FLAG_IRAM, &handle));

    printf("pins registered\n");
    fflush(stdout);
}