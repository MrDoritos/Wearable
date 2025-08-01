#pragma once

#include "config.h"
#include "esp_system.h"
#include "wbl_func.h"
#include <inttypes.h>

namespace wbl {

struct wbl_System {
    esp_err_t init();
    esp_err_t update();
    void setHapticFeedback(const bool &state);
    void setHapticFeedbackLevel(const float &level);
    void beginHapticFeedback(const float &level, const int32_t &duration);
    void setAudibleFeedback(const bool &state);
    void setAudibleFeedbackLevel(const float &level);
    void beginAudibleFeedback(const float &level, const int32_t &duration);
    float getBatteryVoltage();
    float getBatteryVoltageMean();
    float getBatteryLevel();

    bool use_audible_feedback = true;
    bool use_haptic_feedback = true;
    int32_t haptic_feedback_end = 0;
    int32_t audible_feedback_end = 0;
    uint16_t haptic_feedback_level = 0;
    uint16_t audible_feedback_level = 0;
    bool haptic_timer_state = false;
    bool audible_timer_state = false;
};

extern wbl_System wbl_system;

}