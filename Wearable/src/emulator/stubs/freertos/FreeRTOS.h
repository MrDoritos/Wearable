#pragma once

#include <inttypes.h>
#include <unistd.h>
#include <sched.h>

typedef uint32_t TickType_t;
#define portTICK_PERIOD_MS 1

void vTaskDelay(TickType_t delay);
void vPortYield();