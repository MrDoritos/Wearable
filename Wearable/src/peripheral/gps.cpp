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

//using I2C_CAMM8 = I2C<I2C_CAMM8_ADDR, I2C_CAMM8_FREQ, 1000, I2C_BUS_1, 40000>;

GPS gps;
I2C cam(I2C_BUS_1, I2C_CAMM8_ADDR, 1000, I2C_CAMM8_FREQ, 40000);
ublox _gps;
navpvt8 nav(_gps);
cfggnss gc(_gps);
_navodo currentOdometer;


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
    configureOdometer(1);
    return ESP_OK;
}

esp_err_t bytes_available(uint16_t &available) {
    uint8_t r = 0xfd;
    uint8_t buf[2] = {0,0};
    available = 0;
    ESP_RETURN_ON_ERROR(cam.write_read(&r, 1, buf, 2), TAG, "Failed to get bytes available");
    available = (uint16_t(buf[0])<<8)|buf[1];
    return ESP_OK;
}

GPSState GPS::update() {
    //if (micros() - GPS_UPDATE_INTERVAL * 1000 < last_update)
    //    return ;

    //printf("Updating GPS data %lli -> %lli\n", last_update, micros());

    esp_err_t ret = ESP_OK;

    uint16_t to_read;
    ret = bytes_available(to_read);

    if (ret != ESP_OK)
        return COMMUNICATION_ERROR;

    if (!to_read)
        return NO_DATA;

    int available = to_read;
    last_update = micros();

    const int buflen = 32;
    uint8_t buffer[buflen];

    WBL_DF("GPS bytes available: %u\n", to_read);

    while (available > 0) {
        const int bytes_to_read = available < buflen ? available : buflen;

        ret = i2c_master_receive(cam.dev, buffer, bytes_to_read, 100);

        if (ret != ESP_OK)
            return COMMUNICATION_ERROR;

        for (int i = 0; i < bytes_to_read; i++) {
            char *r = (char*)_gps.parse(buffer[i]);

            if (strlen(r) > 0) {
                if (strcmp(r, "navpvt8") == 0) {
                    return NAVPVT8;
                }
                if (strcmp(r, "navodo") == 0) {
                    memcpy(&currentOdometer, _gps.buffer, sizeof(currentOdometer));
                    return NAVODO;
                }
            }
        }

        available -= bytes_to_read;
    }

    return NO_DATA;
}

GPSPoint GPS::getFix() {
    GPSPoint ret;

    ret.longitude = nav.getlon();
    ret.latitude = nav.getlat();
    ret.altitude = nav.getheight();
    
    tm t;
    
    t.tm_year = nav.getyear() - 1900;
    t.tm_mday = nav.getday();
    t.tm_hour = nav.gethour();
    t.tm_min = nav.getminute();
    t.tm_sec = nav.getsecond();
    t.tm_mon = nav.getmonth() - 1;

    ret.time = mktime(&t) * 1000000 + (int64_t(nav.getnano() * 1e-3));

    return ret;
}

/*
    @brief Microseconds
*/
int64_t GPS::getGPSTime() {
    return getFix().time;
}

void GPS::setSystemTime() {
    if (getSatelliteCount() < 1)
        return;

    GPSPoint point = getFix();

    if (point.time == last_time_update)
        return;

    timeval tv;
    tv.tv_sec = point.time / 1000000;
    tv.tv_usec = point.time % 1000000;

    if (tv.tv_sec < seconds()) {
        //printf("No GPS time yet %lli < %lli\n", tv.tv_sec, seconds());
        return;
    }


    //printf("Set time %lli -> %lli\n", seconds(), tv.tv_sec);

    settimeofday(&tv,nullptr);

    last_time_update = point.time;
}

double GPS::getGroundSpeed() {
    return nav.getgSpeed();
}

uint32_t GPS::getTimeAccuracy() {
    return nav.gettacc();
}

double GPS::getPDOP() {
    return nav.getpDOP();
}

double GPS::getGroundSpeedAccuracy() {
    return nav.gethAcc();
}

double GPS::getHorizontalAccuracy() {
    return nav.gethAcc();
}

double GPS::getVerticalAccuracy() {
    return nav.getvAcc() * mm2m;
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

uint32_t GPS::getOdometer() {
    return currentOdometer.block.distance;
}

uint32_t GPS::getOdometerTotal() {
    return currentOdometer.block.totalDistance;
}

uint32_t GPS::getOdometerAccuracy() {
    return currentOdometer.block.distanceStd;
}

void GPS::resetOdometer() {
    struct {
        uint8_t cmd[2] = { 0xB5, 0x62 };
        uint8_t pk[2] = { 0x01, 0x10 };
        uint8_t ck[2];
    } packet;

    _gps.calculatechecksum(packet.ck, packet.pk, 2);
    sendPacket((uint8_t*)&packet, sizeof(packet));
}

void GPS::pollOdometer() {
    struct {
        uint8_t cmd[2] = { 0xB5, 0x62 };
        uint8_t pk[2] = { 0x01, 0x09 };
        uint8_t ck[2];
    } packet;

    _gps.calculatechecksum(packet.ck, packet.pk, 2);
    sendPacket((uint8_t*)&packet, sizeof(packet));
}

void GPS::configureOdometer(uint8_t odoflags, uint8_t odofilter, uint8_t odomaxspeed, uint8_t odomaxaccuracy, uint8_t odovelocitylowpass, uint8_t odolowpass) {
    struct {
        uint8_t cmd[2] = { 0xB5, 0x62 };
        struct {
            uint8_t version = 0;
            uint8_t res1[3] = {0,0,0};
            uint8_t flags;
            uint8_t filter;
            uint8_t res2[6] = {0,0,0,0,0,0};
            uint8_t maxspeed;
            uint8_t maxacc;
            uint8_t res3[2] = {0,0};
            uint8_t vellowpass;
            uint8_t lowpass;
            uint8_t res4[2] = {0,0};
        } payload;
        uint8_t ck[2];
    } packet;

    packet.payload = {
        .flags = odoflags,
        .filter = odofilter,
        .maxspeed = odomaxspeed,
        .maxacc = odomaxaccuracy,
        .vellowpass = odovelocitylowpass,
        .lowpass = odolowpass,
    };

    _gps.calculatechecksum(packet.ck, (uint8_t*)&packet.payload, sizeof(packet.payload));
    sendPacket((uint8_t*)&packet, sizeof(packet));
}

}

void sendByte(uint8_t b) {
    ESP_ERROR_CHECK(wbl::cam.write(&b, 1));
}

void sendPacket(uint8_t *packet, uint8_t length) {
    ESP_ERROR_CHECK(wbl::cam.write(packet, length));
}