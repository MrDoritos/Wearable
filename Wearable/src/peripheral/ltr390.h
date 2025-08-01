#pragma once

#include "esp_system.h"

namespace wbl {

struct LTR390 {
    bool state = false;
    bool uv_mode = false;
    uint32_t measurement_rate;
    uint8_t measurement_rate_flag;
    uint8_t gain;
    uint8_t resolution;

    esp_err_t init();
    uint32_t getUVS();
    uint32_t getALS();
    float getLux();
    float getLux(const uint32_t &als);
    float getUVI();
    float getUVI(const uint32_t &uvs);
    void setState(const bool &state);
    bool isActive();
    void setMeasurementRate(const uint32_t &ms);
    void setGain(const uint8_t &gain);
    void setResolution(const uint8_t &resolution);
    void setMode(const bool &uv_mode);
    uint32_t getIntegrationTime();
    int getGainMultiplier();
};

extern LTR390 ltr390;
    
}