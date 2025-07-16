#include <chrono>

#include "wbl_func.h"

using CLK = std::chrono::high_resolution_clock;
using TP = CLK::time_point;
using DUR = CLK::duration;

TP start = CLK::now();

int64_t micros() {
    TP now = CLK::now();

    const DUR elap = now - start;
    using MDUR = std::chrono::duration<int64_t, std::micro>;
    const MDUR melap = std::chrono::duration_cast<MDUR>(elap);
    return melap.count();
}

void vTaskDelay(TickType_t delay) {
    usleep(delay * 1000);
}

void vPortYield() {
    sched_yield();
}