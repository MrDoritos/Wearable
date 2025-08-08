#include "peripheral_log.h"
#include "wbl_func.h"
#include "wbl_system.h"
#include "gps.h"
#include "display_timeout.h"
#include "gps_imu.h"
#include "ltr390.h"
#include "mics6814.h"

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
DLCAMM8ODOST::storage_type ds_camm8_odo_st;
DLIMUST::storage_type ds_imu_st;
DLPEDST::storage_type ds_ped_st;
DLLTR390ST::storage_type ds_ltr390_st;

#define LOGALLOC(x) ESP_RETURN_ON_ERROR(make_ext(x), TAG, "Failed to alloc")

esp_err_t PeripheralLog::init() {
    battery_st.set_log(ds_bat_st);
    mics6814_st.set_log(ds_mics_st);
    camm8_st.set_log(ds_camm8_st);
    camm8_odo_st.set_log(ds_camm8_odo_st);
    imu_st.set_log(ds_imu_st);
    ped_st.set_log(ds_ped_st);
    ltr390_st.set_log(ds_ltr390_st);

    LOGALLOC(battery_lt);
    LOGALLOC(mics6814_lt);
    LOGALLOC(camm8_lt);
    LOGALLOC(camm8_lt2);
    LOGALLOC(camm8_odo_lt);
    LOGALLOC(ped_lt);
    LOGALLOC(ltr390_lt);

    return ESP_OK;
}

#undef LOGALLOC

void PeripheralLog::pushOdometer() {
    DPCAMM8ODO odo;

    if (camm8_odo_st.size() < 1)
        odo = DPCAMM8ODO(timestamp_micros(), 0, 0, 0);
    else
        odo = camm8_odo_st.get(-1);
    
    camm8_odo_lt.push_back(odo);
}

void PeripheralLog::pushPedometer() {
    DPPED p;

    if (ped_st.size() < 1)
        p = DPPED(timestamp_micros(), 0);
    else
        p = ped_st.get(-1);

    ped_lt.push_back(p);
}

bool update_camm8_odo() {
    DPCAMM8ODO odo;

    odo.time = timestamp_micros();
    odo.distance = gps.getOdometer();
    odo.total_distance = gps.getOdometerTotal();
    odo.distance_accuracy = gps.getOdometerAccuracy();

    WBL_DF("Time %lli odometer %lu total %lu accuracy %lu\n", odo.time, odo.distance, odo.total_distance, odo.distance_accuracy);

    log.camm8_odo_st.push_back(odo);

    return true;
}

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

TimeState time_state{0};
int64_t last_odometer_poll = 0;

bool update_camm8() {
    GPSState ret = gps.update();

    if (ret == NAVODO)
        return update_camm8_odo();

    int64_t last_odo = LOG_CAMM8_ODO_ST_RATE * 1000 + log.camm8_odo_st.get_data_end_time();

    if (last_odo != last_odometer_poll && last_odo < timestamp_micros()) {
        WBL_DF("Poll odometer %llu\n", last_odo);
        gps.pollOdometer();
        last_odometer_poll = last_odo;
    }

    if (ret != NAVPVT8)
        return false;

    gps.setSystemTime();

    //if (gps.last_time_update > 0) {
        if (time_state == FIRST_SET)
            time_state = DONE;
        if (time_state == NOT_SET)
            time_state = FIRST_SET;

        if (time_state == FIRST_SET && !displayTimeout.is_display_off())
            displayTimeout.any_user_input();
    //}

    int64_t t = timestamp_micros();

    /*if (LOG_CAMM8_ST_RATE * 1000 + log.camm8_st.get_data_end_time() < t)*/ {
        DPCAMM8ST v;
        if (get_camm8_st(v)) {
            WBL_DF("Push GPS ST %lli -> %lli -> %lli\n", log.camm8_st.get_data_end_time(), t, v.time);
            log.camm8_st.push_back(v);
        }
    }

    if (log.camm8_st.size() < 2)
        return true;

    if (LOG_CAMM8_LT_RATE * 1000 + log.camm8_lt.get_data_end_time() < t) {
        DPCAMM8LT v;
        if (log.camm8_lt.get_data_end_time() != log.camm8_st.get_data_end_time() && get_camm8_lt(v, LOG_CAMM8_LT_RATE)) {
            WBL_DF("Push GPS LT %lli -> %lli -> %lli\n", log.camm8_lt.get_data_end_time(), t, v.time);
            log.camm8_lt.push_back(v);
        }
    }

    if (LOG_CAMM8_LT2_RATE * 1000 + log.camm8_lt2.get_data_end_time() < t) {
        DPCAMM8LT v;
        if (log.camm8_lt2.get_data_end_time() != log.camm8_st.get_data_end_time() && get_camm8_lt(v, LOG_CAMM8_LT2_RATE))
            log.camm8_lt2.push_back(v);
    }

    return true;
}

TimeState PeripheralLog::getTimeState() {
    return time_state;
}

bool update_gpsimu() {
    if (!gpsimu.getPedometerState())
        return false;

    int64_t t = timestamp_micros();

    if (LOG_IMU_PED_ST_RATE * 1000 + log.ped_st.get_data_end_time() < t) {
        DPPED p(t, gpsimu.getPedometer());
        WBL_DF("Push pedometer ST %lli -> %lli (%u)\n", log.ped_st.get_data_end_time(), p.time, p.steps);
        log.ped_st.push_back(p);
        //return true;
    }

    t = timestamp_micros();

    if (LOG_IMU_ST_RATE * 1000 + log.imu_st.get_data_end_time() < t) {
        AxisData acc = gpsimu.getAccelerometer(), gyro = gpsimu.getGyroscope();
        DPIMU p(t, acc.x, acc.y, acc.z, gyro.x, gyro.y, gyro.z);
        WBL_D("Push IMU ST");
        log.imu_st.push_back(p);
        //return true;
    }

    return true;
}

bool update_vbat() {
    int64_t t = micros();
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
        //log.battery_lt.push_back(micros(), bv);
    }

    return true;
}

bool update_ltr390() {
    int64_t t = timestamp_micros();
    const int64_t half_rate = LOG_LTR390_ST_RATE * 500;
    const int64_t last = log.ltr390_st.get_data_end_time();
    static DVLTR390ST st_p(0,0,0,0);

    if (half_rate * 2 + last < t) {
        WBL_D("Push LTR390 ST");
        log.ltr390_st.push_back(DPLTR390ST(t, st_p));
    }

    if (half_rate + last < t) {
        st_p.als = ltr390.getALS();
        st_p.lux = ltr390.getLux(st_p.als);
        //WBL_DF("ALS LTR390 %lu\n", st_p.als);
    } else {
        st_p.uvs = ltr390.getUVS();
        st_p.uvi = ltr390.getUVIhr(st_p.uvs);
        //WBL_DF("UVS LTR390 %lu\n", st_p.uvs);
    }

    if (log.ltr390_st.size() < 2)
        return true;

    const int64_t lt_rate = LOG_LTR390_LT_RATE * 1000;
    if (LOG_LTR390_LT_RATE * 1000 + log.ltr390_lt.get_data_end_time() < t) {
        WBL_D("Push LTR390 LT");
        const int64_t prev = log.ltr390_st.get_data_end_time();
        DVLTR390ST v = log.ltr390_st.avg_range_time(prev - lt_rate, prev);
        DPLTR390LT p(prev - (lt_rate / 2), v.lux, v.als);
        log.ltr390_lt.push_back(p);
    }

    return true;
}

bool update_mics6814() {
    int64_t t = timestamp_micros();

    const int64_t st_rate = LOG_MICS6814_ST_RATE * 1000;
    
    if (st_rate + log.mics6814_st.get_data_end_time() < t) {
        DPMICS6814 p(
            t, 
            mics6814.getCOmillivolts(), 
            mics6814.getNH3millivolts(), 
            mics6814.getNO2millivolts()
        );

        log.mics6814_st.push_back(p);

        WBL_D("Push MICS6814 ST");
    }

    if (log.mics6814_st.size() < 2)
        return true;

    const int64_t lt_rate = LOG_MICS6814_LT_RATE * 1000;

    if (lt_rate + log.mics6814_lt.get_data_end_time() < t) {
        int64_t st_last = log.mics6814_st.get_data_end_time();
        DVMICS6814 dv = log.mics6814_st.avg_range_time(st_last - lt_rate, st_last);
        log.mics6814_lt.push_back(DPMICS6814(t, dv));

        WBL_D("Push MICS6814 LT");
    }

    return true;
}

esp_err_t PeripheralLog::update() {
    update_vbat();
    update_camm8();
    update_gpsimu();
    update_ltr390();
    update_mics6814();

    return ESP_OK;
}

int64_t PeripheralLog::getNextPollTime() {
    return 0;
}

}