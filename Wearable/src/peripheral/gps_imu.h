#pragma once

#include "esp_system.h"

namespace wbl {

template<typename T>
struct AxisDataT {
    T x, y, z;

    constexpr AxisData():x(0),y(0),z(0){}
    constexpr AxisData(const T &x, const T &y, const T &z):x(x),y(y),z(z){}
};

using AxisData = AxisDataT<double>;

struct GPSIMU {
    esp_err_t init();
    AxisData getGyroscope();
    AxisData getAccelerometer();
};

extern GPSIMU gpsimu;

}