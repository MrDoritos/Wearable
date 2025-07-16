#include "esp_timer.h"

#include "wbl_func.h"

int64_t micros() {
    return esp_timer_get_time();
}