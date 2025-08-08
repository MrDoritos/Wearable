#pragma once

#include "config.h"
#include "esp_system.h"
#include "wbl_func.h"
#include <inttypes.h>

namespace wbl {

struct MICS6814 {
    esp_err_t init();
    
    float getCOVoltage();
    float getNO2Voltage();
    float getNH3Voltage();

    uint16_t getCOmillivolts();
    uint16_t getNO2millivolts();
    uint16_t getNH3millivolts();
};

extern MICS6814 mics6814;

}