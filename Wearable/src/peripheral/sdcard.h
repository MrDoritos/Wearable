#pragma once

#include "config.h"
#include "esp_system.h"

namespace wbl {
    struct SDCard {
        esp_err_t init();
    };

    extern SDCard sdcard;
}