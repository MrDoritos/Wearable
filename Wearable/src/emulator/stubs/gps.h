#pragma once

#include "peripheral/gps.h"
#include "wbl_func.h"

#ifndef WBL_GPS_IMPL
#define WBL_GPS_IMPL

namespace wbl {

GPS gps;

esp_err_t GPS::init() { return ESP_OK; }
int64_t GPS::getGPSTime() { return micros(); }
void GPS::setSystemTime() { }
esp_err_t GPS::update() { return ESP_OK; }

GPSPoint GPS::getFix() {
    GPSPoint ret;

    ret.time = getGPSTime();
    ret.altitude = getElevationAboveSeaLevel();
    ret.latitude = getLatitude();
    ret.longitude = getLongitude();

    return ret;
}

double GPS::getGroundSpeed() { return 12; }
double GPS::getGroundSpeedAccuracy() { return 13; }
double GPS::getHeadingMotion() { return 14; }
double GPS::getHeadingVehicle() { return 15; }
double GPS::getHeadingAccuracy() { return 16; }
int32_t GPS::getElevationAccuracy() { return 17; }
double GPS::getElevationAboveSeaLevel() { return 850; }
double GPS::getElevationAboveEllipsoid() { return 900; }
double GPS::getVelocityNorth() { return 1.0; }
double GPS::getVelocityEast() { return 1.5; }
double GPS::getVelocityVertical() { return 0.5; }
int GPS::getSatelliteCount() { return 18; }
double GPS::getLongitude() { return 19; }
double GPS::getLatitude() { return 20; }
double GPS::getAltitude() { return 21; }

}

#endif
