#pragma once

#include "log.h"
#include "config.h"

#include <stdio.h>
#include <inttypes.h>

namespace wbl {

struct DVBAT {
    uint16_t voltage;
};

struct DVMICS6814 {
    uint16_t co, nh3, no2;
};

struct DVCAMM8ST {
    float latitude, longitude, altitude, bearing, ground_speed, horizontal_accuracy, vertical_accuracy, odometer, pdop;
    uint8_t satellites;
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