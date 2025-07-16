#include <chrono>

#include "wbl_func.h"

using CLK = std::chrono::high_resolution_clock;
using TP = CLK::time_point;
using DUR = CLK::duration;

TP start = CLK::now();

template<typename RATIO>
int64_t get_duration() {
    TP now = CLK::now();

    return std::chrono::duration_cast
        <std::chrono::duration<int64_t, RATIO>>
            (now - start)
                .count();
}

int64_t micros() {
    return get_duration<std::micro>();
}

int64_t millis() {
    return get_duration<std::milli>();
}

int64_t seconds() {
    return get_duration<std::ratio<1>>();    
}

void vTaskDelay(TickType_t delay) {
    usleep(delay * 1000);
}

void vPortYield() {
    sched_yield();
}