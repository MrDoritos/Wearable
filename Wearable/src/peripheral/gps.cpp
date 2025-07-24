#include "gps.h"
typedef uint8_t byte;
#include "u-blox-m8.h"
#include "i2c.h"
#include "config.h"
#include "wbl_func.h"
#include "driver/i2c_master.h"

namespace wbl {

static constexpr const char *TAG = "wbl::gps.cpp";

using I2C_CAMM8 = I2C<I2C_CAMM8_ADDR, I2C_CAMM8_FREQ, 1000, I2C_BUS_1, 40000>;

I2C_CAMM8 cam;
ublox gps;
navpvt8 nav(gps);
cfggnss gc(gps);


esp_err_t init() {
    ESP_RETURN_ON_ERROR(cam.init(), TAG, "gps i2c failed to init");
    delay(100);
    restoreDefaults();
    delay(100);
    disableNmea();
    delay(100);
    enableNavPvt();
    delay(100);
    //sendTimePulseParameters(0);
    changeFrequency(1000);
    return ESP_OK;
}


int64_t getGPSTime() {
    const int buflen = 1;
    uint8_t buffer[buflen];
    memset(buffer, 0, buflen);

    double sec;
    esp_err_t err;
    while (true) {
        err = i2c_master_receive(cam.dev, buffer, buflen, 1000 / portTICK_PERIOD_MS);
        if (err == ESP_ERR_INVALID_ARG || err == ESP_ERR_TIMEOUT || err == ESP_ERR_INVALID_STATE)
            goto error;
        //printf("Receive: %i\n", err);
        //if (err < 1)
        //    break;
            
        char *r = (char*)gps.parse(buffer[0]);

        if (strlen(r) > 0) {
            if (strcmp(r, "navpvt8")==0) {
                break;
            }
        }
    }

    sec = 3600.0 * nav.gethour() + 60.0 * nav.getminute() + 1.0 * nav.getsecond() + nav.getnano() * 1e-9;
    return int64_t(sec*1000*1000);

    error:;
    //printf("Error %i\n", err);
    return 0;
}

}

void sendByte(uint8_t b) {
    ESP_ERROR_CHECK(wbl::cam.write(&b, 1));
}

void sendPacket(uint8_t *packet, uint8_t length) {
    ESP_ERROR_CHECK(wbl::cam.write(packet, length));
}