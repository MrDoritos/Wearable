#pragma once

#include  <inttypes.h>

#include "esp_system.h"

namespace wbl {
    struct GPSPoint {
        int64_t time;
        float longitude;
        float latitude;
        float altitude;
    };

    struct GPS {
        esp_err_t init();
        int64_t getGPSTime();
        esp_err_t update();
        GPSPoint getFix();
    };

    extern GPS gps;
    int64_t last_update = 0;
}