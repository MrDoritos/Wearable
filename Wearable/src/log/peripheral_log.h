#pragma once

#include "log.h"
#include "config.h"

#include "esp_system.h"

#include <stdio.h>
#include <inttypes.h>

namespace wbl {

struct DVBAT {
    using v_type = uint16_t;
    using DVT = DataValueTupleT<v_type>;
    using value_type = DVT;

    v_type voltage;

    constexpr DVBAT(const v_type &voltage):voltage(voltage) {}
    constexpr DVBAT(const DVT &value):DVBAT(value.get<0>()) {}

    constexpr inline DVT get_value() const { return DVT(voltage); }
};

struct DVMICS6814 {
    using v_type = uint16_t;
    using DVT = DataValueTupleT<v_type, v_type, v_type>;
    using value_type = DVT;

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
    using value_type = DVT;

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
    using f_type = float;
    using DVT = DataValueTupleT<f_type, f_type, f_type>;
    using value_type = DVT;
    
    float latitude, longitude, altitude;

    constexpr DVCAMM8LT(const f_type &latitude, const f_type &longitude, const f_type &altitude):
        latitude(latitude),longitude(longitude),altitude(altitude) {}
    constexpr DVCAMM8LT(const DVT &v):
        DVCAMM8LT(v.get<0>(), v.get<1>(), v.get<2>()) {}

    constexpr inline DVT get_value() const { return DVT(latitude, longitude, altitude); }
};

struct DVIMU {
    using f_type = float;
    using DVT = DataValueTupleT<f_type, f_type, f_type, f_type, f_type, f_type>;
    using value_type = DVT;

    float accelerometer_x, accelerometer_y, accelerometer_z;
    float gyroscope_x, gyroscope_y, gyroscope_z;

    constexpr DVIMU(const f_type &ax, const f_type &ay, const f_type &az,
                    const f_type &gx, const f_type &gy, const f_type &gz):
        accelerometer_x(ax),accelerometer_y(ay),accelerometer_z(az),
        gyroscope_x(gx),gyroscope_y(gy),gyroscope_z(gz) {}
    constexpr DVIMU(const DVT &v):
        DVIMU(v.get<0>(), v.get<1>(), v.get<2>(), v.get<3>(), v.get<4>(), v.get<5>()) {}

    constexpr inline DVT get_value() const { return DVT(accelerometer_x, accelerometer_y, accelerometer_z, gyroscope_x, gyroscope_y, gyroscope_z); }
};

struct DVPED {
    using v_type = uint16_t;
    using DVT = DataValueTupleT<v_type>;
    using value_type = DVT;

    uint16_t steps;

    constexpr DVPED(const v_type &steps):steps(steps) {}
    constexpr DVPED(const DVT &v):DVPED(v.get<0>()) {}

    constexpr inline DVT get_value() const { return DVT(steps); }
};

struct DVBME688 {
    using f_type = float;
    using DVT = DataValueTupleT<f_type, f_type, f_type, f_type, f_type, f_type>;
    using value_type = DVT;

    float humidity, temperature, pressure, voc, aqi, co2;

    constexpr DVBME688(const f_type &humidity,
                       const f_type &temperature,
                       const f_type &pressure,
                       const f_type &voc,
                       const f_type &aqi,
                       const f_type &co2):
        humidity(humidity),temperature(temperature),pressure(pressure),
        voc(voc),aqi(aqi),co2(co2) {}
    constexpr DVBME688(const DVT &v):
        DVBME688(v.get<0>(), v.get<1>(), v.get<2>(), v.get<3>(), v.get<4>(), v.get<5>()) {}

    constexpr inline DVT get_value() const { return DVT(humidity, temperature, pressure, voc, aqi, co2); }
};

struct DVBME688GAS {
    using f_type = float;
    using DVT = DataValueTupleT<f_type, f_type, f_type>;
    using value_type = DVT;

    float h2s, etoh, co;

    constexpr DVBME688GAS(const f_type &h2s, const f_type &etoh, const f_type &co):
        h2s(h2s),etoh(etoh),co(co) {}
    constexpr DVBME688GAS(const DVT &v):
        DVBME688GAS(v.get<0>(), v.get<1>(), v.get<2>()) {}

    constexpr inline DVT get_value() const { return DVT(h2s, etoh, co); }
};

struct DVBMP388 {
    using f_type = float;
    using DVT = DataValueTupleT<f_type>;
    using value_type = DVT;

    float pressure;

    constexpr DVBMP388(const f_type &pressure):pressure(pressure) {}
    constexpr DVBMP388(const DVT &v):DVBMP388(v.get<0>()) {}

    constexpr inline DVT get_value() const { return DVT(pressure); }
};

struct DVLTR390ST {
    using f_type = float;
    using v_type = uint32_t;
    using DVT = DataValueTupleT<v_type, v_type, f_type, f_type>;
    using value_type = DVT;

    uint32_t als, uvs;
    float lux, uvi;

    constexpr DVLTR390ST(const v_type &als, const v_type &uvs, const f_type &lux, const f_type &uvi):
        als(als),uvs(uvs),lux(lux),uvi(uvi) {}
    constexpr DVLTR390ST(const DVT &v):DVLTR390ST(v.get<0>(), v.get<1>(), v.get<2>(), v.get<3>()) {}

    constexpr inline DVT get_value() const { return DVT(als, uvs, lux, uvi); }
};

struct DVLTR390LT {
    using f_type = float;
    using DVT = DataValueTupleT<f_type, f_type>;
    using value_type = DVT;
    
    float lux, uvi;

    constexpr DVLTR390LT(const f_type &lux, const f_type &uvi):lux(lux),uvi(uvi) {}
    constexpr DVLTR390LT(const DVT &v):DVLTR390LT(v.get<0>(), v.get<1>()) {}

    constexpr inline DVT get_value() const { return DVT(lux, uvi); }
};

struct DVMAX30102 {
    using v_type = uint32_t;
    using DVT = DataValueTupleT<v_type, v_type>;
    using value_type = DVT;

    uint32_t red, ir;

    constexpr DVMAX30102(const v_type &red, const v_type &ir):red(red),ir(ir) {}
    constexpr DVMAX30102(const DVT &v):DVMAX30102(v.get<0>(), v.get<1>()) {}

    constexpr inline DVT get_value() const { return DVT(red, ir); }
};

struct DVMAX30102_EVAL {
    using v_type = uint16_t;
    using DVT = DataValueTupleT<v_type, v_type>;
    using value_type = DVT;

    uint16_t hr, spo2;

    constexpr DVMAX30102_EVAL(const v_type &hr, const v_type &spo2):hr(hr),spo2(spo2) {}
    constexpr DVMAX30102_EVAL(const DVT &v):DVMAX30102_EVAL(v.get<0>(), v.get<1>()) {}

    constexpr inline DVT get_value() const { return DVT(hr, spo2); }
};

template<typename POINT_T = uint16_t>
using DPST = DataPointImplT<DataPointTimeBaseT<int64_t>, POINT_T>;

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
using DLCAMM8LT = DL<DPCAMM8LT, LOG_CAMM8_LT_SIZE>;
using DLCAMM8LT2 = DL<DPCAMM8LT, LOG_CAMM8_LT2_SIZE>;
using DLIMUST = DL<DPIMU, LOG_IMU_ST_SIZE>;
using DLPEDST = DL<DPPED, LOG_IMU_PED_ST_SIZE>;
using DLBME688ST = DL<DPBME688, LOG_BME688_ST_SIZE>;
using DLBME688LT = DL<DPBME688, LOG_BME688_LT_SIZE>;
using DLLTR390ST = DL<DPLTR390ST, LOG_LTR390_ST_SIZE>;
using DLLTR390LT = DL<DPLTR390LT, LOG_LTR390_LT_SIZE>;
using DLMAX30102ST = DL<DPMAX30102, LOG_MAX30102_ST_SIZE>;
using DLMAX30102EVALST = DL<DPMAX30102_EVAL, LOG_MAX30102_EVAL_ST_SIZE>;
using DLMAX30102EVALLT = DL<DPMAX30102_EVAL, LOG_MAX30102_EVAL_LT_SIZE>;
using DLMAX30102EVALLT2 = DL<DPMAX30102_EVAL, LOG_MAX30102_EVAL_LT2_SIZE>;


struct PeripheralLog {
    esp_err_t init();
    esp_err_t update();
    int64_t getNextPollTime();

    DLBatteryST battery_st;
    DLBatteryLT battery_lt;

    DLMICS6814ST mics6814_st;
    DLMICS6814LT mics6814_lt;

    DLCAMM8ST camm8_st;
    DLCAMM8LT camm8_lt;
    DLCAMM8LT2 camm8_lt2;

    DLIMUST imu_st;

    DLPEDST ped_st;

    DLBME688ST bme688_st;
    DLBME688LT bme688_lt;

    DLLTR390ST ltr390_st;
    DLLTR390LT ltr390_lt;

    DLMAX30102ST max30102_st;

    DLMAX30102EVALST max30102_eval_st;
    DLMAX30102EVALLT max30102_eval_lt;
    DLMAX30102EVALLT2 max30102_eval_lt2;

    int64_t last_poll = 0;
};

extern PeripheralLog log;
    
}