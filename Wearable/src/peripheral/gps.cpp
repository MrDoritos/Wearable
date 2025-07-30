#include "gps.h"
typedef uint8_t byte;
#include "u-blox-m8.h"
#include "i2c.h"
#include "config.h"
#include "wbl_func.h"
#include "driver/i2c_master.h"

#include <time.h>
#include <stdio.h>
#include <sys/time.h>

namespace wbl {

static constexpr const char *TAG = "wbl::gps.cpp";

using I2C_CAMM8 = I2C<I2C_CAMM8_ADDR, I2C_CAMM8_FREQ, 1000, I2C_BUS_1, 40000>;

GPS gps;
I2C_CAMM8 cam;
ublox _gps;
navpvt8 nav(_gps);
cfggnss gc(_gps);


esp_err_t GPS::init() {
    ESP_RETURN_ON_ERROR(cam.init(), TAG, "gps i2c failed to init");
    //delay(100);
    //restoreDefaults();
    delay(100);
    disableNmea();
    delay(100);
    enableNavPvt();
    delay(100);
    //sendTimePulseParameters(0);
    changeFrequency(1000);
    return ESP_OK;
}

esp_err_t GPS::update() {
    if (last_update + GPS_UPDATE_INTERVAL < millis())
        return ESP_OK;

    last_update = millis();

    const int buflen = 1;
    uint8_t buffer[buflen];

    esp_err_t ret;

    while (true) {
        ret = i2c_master_receive(cam.dev, buffer, buflen, 100);

        if (ret != ESP_OK)
            return ret;

        for (int i = 0; i < buflen; i++) {
            char *r = (char*)_gps.parse(buffer[i]);

            if (strlen(r) > 0 && strcmp(r, "navpvt8") == 0)
                return ESP_OK;
        }
    }
}

GPSPoint GPS::getFix() {
    GPSPoint ret;

    ret.longitude = nav.getlon();
    ret.latitude = nav.getlat();
    ret.altitude = nav.getheight();
    
    timeval tv;
    tm t;
    
    double sec = 3600.0 * nav.gethour() + 60.0 * nav.getminute() + 1.0 * nav.getsecond() + nav.getnano() * 1e-9;

    t.tm_year = nav.getyear() - 1900;
    t.tm_mday = nav.getday();
    t.tm_hour = nav.gethour();
    t.tm_min = nav.getminute();
    t.tm_sec = nav.getsecond();
    t.tm_mon = nav.getmonth() - 1;

    ret.time = mktime(&t) * 1000000 + (int64_t(nav.getnano() * 1e-3));

    return ret;
}

int64_t GPS::getGPSTime() {
    return getFix().time;
}

void GPS::setSystemTime() {
    GPSPoint point = getFix();
    timeval tv;
    tv.tv_sec = point.time / 1000000;
    tv.tv_usec = point.time % 1000000;

    if (tv.tv_sec < seconds()) {
        printf("No GPS time yet %lli < %lli\n", tv.tv_sec, seconds());
        return;
    }

    printf("Set time %lli -> %lli\n", seconds(), tv.tv_sec);

    settimeofday(&tv,nullptr);
}

double GPS::getGroundSpeed() {
    return nav.getgSpeed();
}

double GPS::getGroundSpeedAccuracy() {
    return nav.gethAcc();
}

double GPS::getHeadingMotion() {
    return nav.getheadMot();
}

double GPS::getHeadingVehicle() {
    return ((_navpvt8*)_gps.getbuffer())->headVeh * en5;
}

double GPS::getHeadingAccuracy() {
    return ((_navpvt8*)_gps.getbuffer())->headAcc * en5;
}

int32_t GPS::getElevationAccuracy() {
    return nav.getvAcc();
}

double GPS::getElevationAboveSeaLevel() {
    return ((_navpvt8*)_gps.getbuffer())->hMSL * mm2m;
}

double GPS::getElevationAboveEllipsoid() {
    return nav.getheight();
}

double GPS::getVelocityNorth() {
    return ((_navpvt8*)_gps.getbuffer())->velN;
}

double GPS::getVelocityEast() {
    return ((_navpvt8*)_gps.getbuffer())->velE;
}

double GPS::getVelocityVertical() {
    return ((_navpvt8*)_gps.getbuffer())->velD;
}

int GPS::getSatelliteCount() {
    return nav.getnumSV();
}

double GPS::getLongitude() {
    return nav.getlon();
}

double GPS::getLatitude() {
    return nav.getlat();
}

double GPS::getAltitude() {
    return getElevationAboveSeaLevel();
}

}

void sendByte(uint8_t b) {
    ESP_ERROR_CHECK(wbl::cam.write(&b, 1));
}

void sendPacket(uint8_t *packet, uint8_t length) {
    ESP_ERROR_CHECK(wbl::cam.write(packet, length));
}