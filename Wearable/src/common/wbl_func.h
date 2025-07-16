#pragma once

#include "freertos/FreeRTOS.h"
#include <inttypes.h>

inline void delay(TickType_t ms) {
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

int64_t micros();