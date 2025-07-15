#pragma once

#include "freertos/FreeRTOS.h"

inline void delay(TickType_t ms) {
    vTaskDelay(ms / portTICK_PERIOD_MS);
}