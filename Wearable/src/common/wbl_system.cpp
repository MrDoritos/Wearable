#include "wbl_system.h"

namespace wbl {

esp_err_t wbl_System::init() {
    return ESP_OK;
}

esp_err_t wbl_System::update() {
    return ESP_OK;
}

void wbl_System::setHapticFeedback(const bool &state) {
    use_haptic_feedback = state;
}

void wbl_System::setAudibleFeedback(const bool &state) {
    use_audible_feedback = state;
}

void wbl_System::setHapticFeedbackLevel(const uint8_t &level) {
    haptic_feedback_level = level;
}

void wbl_System::setAudibleFeedbackLevel(const uint8_t &level) {
    audible_feedback_level = level;
}

void wbl_System::beginHapticFeedback(const uint8_t &level, const int32_t &duration) {
    setHapticFeedbackLevel(level);
}

void wbl_System::beginAudibleFeedback(const uint8_t &level, const int32_t &duration) {
    setAudibleFeedbackLevel(level);
}

}