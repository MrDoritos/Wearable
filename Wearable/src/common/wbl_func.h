#pragma once

#include "freertos/FreeRTOS.h"
#include <inttypes.h>
#include "config.h"

inline void delay(const int64_t &ms) {
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

void delay_sleep(const int64_t &ms);

int64_t micros();

int64_t millis();

int64_t seconds();

int64_t timestamp_micros();
int64_t timestamp_millis();
int64_t timestamp_seconds();