#include "gps.h"
typedef uint8_t byte;
#include "u-blox-m8.h"
#include "i2c.h"
#include "config.h"
#include "wbl_func.h"
#include "driver/i2c_master.h"

namespace wbl {

static constexpr const char *TAG = "wbl::gps.cpp";

using I2C_CAMM8 = I2C<I2C_CAMM8_ADDR, I2C_CAMM8_FREQ, 1000, I2C_BUS_1>;

I2C_CAMM8 cam;
ublox gps;
navpvt8 nav(gps);
cfggnss gc(gps);


esp_err_t init() {
    ESP_RETURN_ON_ERROR(cam.init(), TAG, "gps i2c failed to init");
    delay(100);
    disableNmea();
    delay(100);
    enableNavPvt();
    delay(100);
    return ESP_OK;
}


int64_t getGPSTime() {
    const int buflen = 1;
    uint8_t buffer[buflen];

    while (true) {
        i2c_master_receive(cam.dev, buffer, buflen, 1 / portTICK_PERIOD_MS);
        char *r = (char*)gps.parse(buffer[0]);

        if (strlen(r) > 0) {
            if (strcmp(r, "navpvt8")==0) {
                break;
            }
        }
    }

    double sec = 3600.0 * nav.gethour() + 60.0 * nav.getminute() + 1.0 * nav.getsecond() + nav.getnano() * 1e-9;
    return int64_t(sec*1000*1000);
}

}

void sendByte(uint8_t b) {
    ESP_ERROR_CHECK_WITHOUT_ABORT(wbl::cam.write(&b, 1));
}

void sendPacket(uint8_t *packet, uint8_t length) {
    ESP_ERROR_CHECK_WITHOUT_ABORT(wbl::cam.write(packet, length));
}