#include <sys/time.h>
#include "esp_timer.h"
#include "esp_sleep.h"
#include "esp_check.h"
#include "driver/uart.h"
#include <sys/stdio.h>

#include "wbl_system.h"
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

void delay_sleep(const int64_t &ms) {
    //if (wbl::wbl_system.isPMLocked()) {
    //    return;
    //}
    wbl::wbl_system.releasePMLock();
    //fflush(stdout);
    //ESP_ERROR_CHECK_WITHOUT_ABORT(uart_wait_tx_idle_polling(CONFIG_ESP_CONSOLE_UART_NUM));
    //int64_t sleep_start = micros();
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_sleep_enable_timer_wakeup(ms * 1000));
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_light_sleep_start());
    //int64_t sleep_after = micros();
    wbl::wbl_system.acquirePMLock();
}