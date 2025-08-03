#pragma once

#include "config.h"
#include "esp_system.h"

namespace wbl {
    struct SDCard {
        esp_err_t init();

        uint64_t getTotalSize();
        uint64_t getFreeSize();
    };

    extern SDCard sdcard;
}