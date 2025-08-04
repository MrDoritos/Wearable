#pragma once

#include "log.h"
#include "config.h"

#include <stdio.h>
#include <inttypes.h>

namespace wbl {

struct DVBAT {
    using v_type = uint16_t;
    using DVT = DataValueTupleT<v_type>;

    v_type voltage;

    constexpr DVBAT(const v_type &voltage):voltage(voltage) {}
    constexpr DVBAT(const DVT &value):DVBAT(value.get<0>()) {}

    constexpr inline DVT get_value() const { return DVT(voltage); }
};

struct DVMICS6814 {
    using v_type = uint16_t;
    using DVT = DataValueTupleT<v_type, v_type, v_type>;

    uint16_t co, nh3, no2;

    constexpr DVMICS6814(const v_type &co, const v_type &nh3, const v_type &no2):
        co(co),nh3(nh3),no2(no2) {}
    constexpr DVMICS6814(const DVT &value):
        DVMICS6814(value.get<0>(), value.get<1>(), value.get<2>()) {}

    constexpr inline DVT get_value() const { return DVT(co, nh3, no2); }

};

struct DVCAMM8ST {
    using f_type = float;
    using v_type = uint8_t;
    using DVT = DataValueTupleT<f_type, f_type, f_type, f_type, f_type, f_type, f_type, f_type, f_type, v_type>;

    float latitude, longitude, altitude, bearing, ground_speed, horizontal_accuracy, vertical_accuracy, odometer, pdop;
    uint8_t satellites;

    constexpr DVCAMM8ST(const f_type &latitude,
                        const f_type &longitude,
                        const f_type &altitude,
                        const f_type &bearing,
                        const f_type &ground_speed,
                        const f_type &horizontal_accuracy,
                        const f_type &vertical_accuracy,
                        const f_type &odometer,
                        const f_type &pdop,
                        const v_type &satellites):
        latitude(latitude),longitude(longitude),altitude(altitude),
        bearing(bearing),ground_speed(ground_speed),horizontal_accuracy(horizontal_accuracy),
        vertical_accuracy(vertical_accuracy),odometer(odometer),pdop(pdop),
        satellites(satellites) {}
    constexpr DVCAMM8ST(const DVT &v):
        DVCAMM8ST(v.get<0>(), v.get<1>(), v.get<2>(),
                  v.get<3>(), v.get<4>(), v.get<5>(),
                  v.get<6>(), v.get<7>(), v.get<8>(),
                  v.get<9>()) {}

    constexpr inline DVT get_value() const { return DVT(latitude, longitude, altitude, bearing, ground_speed, horizontal_accuracy, vertical_accuracy, odometer, pdop, satellites); }
};

struct DVCAMM8LT {
    float latitude, longitude, altitude;
};

struct DVIMU {
    float accelerometer_x, accelerometer_y, accelerometer_z;
    float gyroscope_x, gyroscope_y, gyroscope_z;
};

struct DVPED {
    uint16_t steps;
};

struct DVBME688 {
    float humidity, temperature, pressure, voc, aqi, co2;
};

struct DVBME688GAS {
    float h2s, etoh, co;
};

struct DVBMP388 {
    float pressure;
};

struct DVLTR390ST {
    uint32_t als, uvs;
    float lux, uvi;
};

struct DVLTR390LT {
    float lux, uvi;
};

struct DVMAX30102 {
    uint32_t red, ir;
};

struct DVMAX30102_EVAL {
    uint16_t hr, spo2;
    //float
};

template<typename POINT_T = uint16_t>
using DPST = DataPointT<int64_t, POINT_T>;

using DPBattery = DPST<DVBAT>;
using DPMICS6814 = DPST<DVMICS6814>;
using DPCAMM8ST = DPST<DVCAMM8ST>;
using DPCAMM8LT = DPST<DVCAMM8LT>;
using DPIMU = DPST<DVIMU>;
using DPPED = DPST<DVPED>;
using DPBME688 = DPST<DVBME688>;
using DPBME688GAS = DPST<DVBME688GAS>;
using DPBMP388 = DPST<DVBMP388>;
using DPLTR390ST = DPST<DVLTR390ST>;
using DPLTR390LT = DPST<DVLTR390LT>;
using DPMAX30102 = DPST<DVMAX30102>;
using DPMAX30102_EVAL = DPST<DVMAX30102_EVAL>;

template<typename DP, int sz=100>
using DL = DataLogT<DP, LoopBufferT<DP, sz>>;

using DLBatteryST = DL<DPBattery, LOG_BATTERY_ST_SIZE>;
using DLBatteryLT = DL<DPBattery, LOG_BATTERY_LT_SIZE>;
using DLMICS6814ST = DL<DPMICS6814, LOG_MICS6814_ST_SIZE>;
using DLMICS6814LT = DL<DPMICS6814, LOG_MICS6814_LT_SIZE>;
using DLCAMM8ST = DL<DPCAMM8ST, LOG_CAMM8_ST_SIZE>;


struct PeripheralLog {

};

extern PeripheralLog log;
    
}