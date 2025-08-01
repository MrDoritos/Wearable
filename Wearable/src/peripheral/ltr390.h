#pragma once

#include "esp_system.h"

namespace wbl {

struct LTR390 {
    bool uv_mode = false;

    esp_err_t init();
    uint32_t getUVS();
    uint32_t getALS();
    void setState(const bool &state);
    bool isActive();
};

extern LTR390 ltr390;
    
}