#pragma once

#include "esp_system.h"

namespace wbl {

struct BME688 {
    esp_err_t init();
};

}