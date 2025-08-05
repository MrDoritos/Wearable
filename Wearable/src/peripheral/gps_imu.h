#pragma once

#include "esp_system.h"

namespace wbl {

template<typename T>
struct AxisDataT {
    T x, y, z;

    constexpr AxisDataT():x(0),y(0),z(0){}
    constexpr AxisDataT(const T &x, const T &y, const T &z):x(x),y(y),z(z){}
};

using AxisData = AxisDataT<double>;

struct GPSIMU {
    esp_err_t init();
    AxisData getGyroscope();
    AxisData getAccelerometer();
    esp_err_t enablePedometer();
    esp_err_t disablePedometer();
    esp_err_t setPedometerState(uint8_t state);
    esp_err_t resetPedometer();
    uint16_t getPedometer();
    uint8_t getPedometerState();
};

extern GPSIMU gpsimu;

}