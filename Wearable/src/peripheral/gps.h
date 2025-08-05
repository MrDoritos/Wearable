#pragma once

#include  <inttypes.h>

#include "esp_system.h"

namespace wbl {
    struct GPSPoint {
        int64_t time;
        float longitude;
        float latitude;
        float altitude;
    };

    enum GPSState {
        NO_DATA=0,
        COMMUNICATION_ERROR,
        NAVPVT8,
        NAVODO,
    };

    struct GPS {
        esp_err_t init();
        int64_t getGPSTime();
        void setSystemTime();
        GPSState update();
        GPSPoint getFix();
        double getGroundSpeed();
        double getGroundSpeedAccuracy();
        double getHeadingMotion();
        double getHeadingVehicle();
        double getHeadingAccuracy();
        int32_t getElevationAccuracy();
        double getElevationAboveSeaLevel();
        double getElevationAboveEllipsoid();
        double getVelocityNorth();
        double getVelocityEast();
        double getVelocityVertical();
        int getSatelliteCount();
        double getLongitude();
        double getLatitude();
        double getAltitude();
        double getVerticalAccuracy();
        double getHorizontalAccuracy();
        uint32_t getOdometerTotal();
        uint32_t getOdometer();
        uint32_t getOdometerAccuracy();
        uint32_t getTimeAccuracy();
        double getPDOP();
        void resetOdometer();
        void pollOdometer();

        int64_t last_update = 0;
        int64_t last_time_update = 0;
    };

    extern GPS gps;
}