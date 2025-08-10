#include <sys/time.h>
#include "esp_timer.h"

#include "wbl_func.h"

int64_t micros() {
    return esp_timer_get_time();
}

int64_t millis() {
    //struct timeval tv;
    //gettimeofday(&tv, nullptr);
    //return (tv.tv_sec * 1000LL + (tv.tv_usec / 1000LL));
    return micros() / 1000;
}

int64_t seconds() {
    //return time(nullptr);
    return micros() / 1000000;
}

int64_t timestamp_seconds() {
    return time(nullptr);
}

int64_t timestamp_millis() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return (tv.tv_sec * 1000LL + (tv.tv_usec / 1000LL));
}

int64_t timestamp_micros() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return (tv.tv_sec * 1000000LL + (tv.tv_usec));
}