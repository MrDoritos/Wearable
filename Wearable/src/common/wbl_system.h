#include "config.h"
#include "esp_system.h"
#include "wbl_func.h"
#include <inttypes.h>

namespace wbl {

struct wbl_System {
    esp_err_t init();
    esp_err_t update();
    void setHapticFeedback(const bool &state);
    void setHapticFeedbackLevel(const uint8_t &level);
    void beginHapticFeedback(const uint8_t &level, const int32_t &duration);
    void setAudibleFeedback(const bool &state);
    void setAudibleFeedbackLevel(const uint8_t &level);
    void beginAudibleFeedback(const uint8_t &level, const int32_t &duration);
    float getBatteryVoltage();
    float getBatteryVoltageMean();
    float getBatteryLevel();

    bool use_audible_feedback = true;
    bool use_haptic_feedback = true;
    int32_t haptic_feedback_end = 0;
    int32_t audible_feedback_end = 0;
    uint8_t haptic_feedback_level = 255;
    uint8_t audible_feedback_level = 255;
};

extern wbl_System wbl_system;

}