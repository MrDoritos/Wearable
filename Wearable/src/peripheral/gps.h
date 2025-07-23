#pragma once

#include  <inttypes.h>

#include "esp_system.h"

namespace wbl {
    esp_err_t init();
    int64_t getGPSTime();
}