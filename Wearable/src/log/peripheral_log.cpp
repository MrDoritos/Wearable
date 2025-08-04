#include "peripheral_log.h"
#include "wbl_func.h"
#include "wbl_system.h"
#include "gps.h"

#include "esp_check.h"
#include "esp_heap_caps.h"
#include <malloc.h>

namespace wbl {

static constexpr const char *TAG = "wbl::peripheral_log.cpp";

PeripheralLog log;

template<typename DL, typename DS = typename DL::storage_type>
void make_dl(DL &log, DS *data_storage) {
    new (data_storage) DS();
    new (&log) DL(data_storage);
}

template<typename DL, typename DS = typename DL::storage_type>
esp_err_t make_ext(DL &log) {
    DS *storage = nullptr;
    storage = (DS*)heap_caps_malloc(sizeof(DS), MALLOC_CAP_SPIRAM);

    if (storage == nullptr)
        return ESP_ERR_NO_MEM;

    make_dl(log, storage);

    return ESP_OK;
}

DLBatteryST::storage_type ds_bat_st;
DLMICS6814ST::storage_type ds_mics_st;
DLCAMM8ST::storage_type ds_camm8_st;

#define LOGALLOC(x) ESP_RETURN_ON_ERROR(make_ext(x), TAG, "Failed to alloc")

esp_err_t PeripheralLog::init() {
    battery_st.set_log(ds_bat_st);
    mics6814_st.set_log(ds_mics_st);
    camm8_st.set_log(ds_camm8_st);

    LOGALLOC(battery_lt);
    LOGALLOC(mics6814_lt);
    LOGALLOC(camm8_lt);
    LOGALLOC(camm8_lt2);

    return ESP_OK;
}

#undef LOGALLOC

bool get_camm8_st(DPCAMM8ST &camm8) {
    camm8.satellites = gps.getSatelliteCount();

    if (camm8.satellites < 1)
        return false;

    camm8.latitude = gps.getLatitude();
    camm8.longitude = gps.getLongitude();
    camm8.altitude = gps.getElevationAboveSeaLevel();
    camm8.bearing = gps.getHeadingVehicle();
    camm8.ground_speed = gps.getGroundSpeed();
    camm8.horizontal_accuracy = gps.getHorizontalAccuracy();
    camm8.vertical_accuracy = gps.getVerticalAccuracy();
    camm8.odometer = 0;
    camm8.pdop = gps.getPDOP();
    camm8.satellites = gps.getSatelliteCount();
    camm8.time = gps.getGPSTime();

    return true;
}

bool get_camm8_lt(DPCAMM8LT &camm8, const int64_t &rate_ms) {
    int64_t last = log.camm8_st.get_data_end_time();
    const int64_t rate = rate_ms * 1000;
    DVCAMM8ST v = log.camm8_st.avg_range_time(last - rate, last);
    camm8.altitude = v.altitude;
    camm8.latitude = v.latitude;
    camm8.longitude = v.longitude;
    camm8.time = last - (rate / 2);

    return true;
}

bool update_camm8() {
    int64_t t = micros();

    gps.update();

    if (LOG_CAMM8_ST_RATE * 1000 + log.camm8_st.get_data_end_time() < t) {
        DPCAMM8ST v;
        if (get_camm8_st(v))
            log.camm8_st.push_back(v);
    }

    if (log.camm8_st.size() < 2)
        return true;

    if (LOG_CAMM8_LT_RATE * 1000 + log.camm8_lt.get_data_end_time() < t) {
        DPCAMM8LT v;
        if (get_camm8_lt(v, LOG_CAMM8_LT_RATE))
            log.camm8_lt.push_back(v);
    }

    if (LOG_CAMM8_LT2_RATE * 1000 + log.camm8_lt2.get_data_end_time() < t) {
        DPCAMM8LT v;
        if (get_camm8_lt(v, LOG_CAMM8_LT2_RATE))
            log.camm8_lt2.push_back(v);
    }

    return true;
}

bool update_vbat() {
    int64_t t = millis();
    uint16_t bv = wbl_system.getBatteryMillivolts();
    
    if (LOG_BATTERY_ST_RATE * 1000 + log.battery_st.get_data_end_time() < t)
        log.battery_st.push_back(micros(), bv);
    
    if (log.battery_st.size() < 2)
        return true;

    const int64_t rate = LOG_BATTERY_LT_RATE * 1000;
    if (rate + log.battery_lt.get_data_end_time() < t) {
        int64_t last = log.battery_st.get_data_end_time();
        DVBAT v = log.battery_st.avg_range_time(last - rate, last);
        DPBattery p(last - (rate / 2), v.voltage);
        log.battery_lt.push_back(p);
    }

    return true;
}

esp_err_t PeripheralLog::update() {
    update_vbat();
    update_camm8();

    return ESP_OK;
}

int64_t PeripheralLog::getNextPollTime() {
    return 0;
}

}