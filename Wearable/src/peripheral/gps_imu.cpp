#include "gps_imu.h"
#include "imu.h"

#include "LSM6DSLSensor.h"

namespace wbl {

I2C lsm6dsl(I2C_BUS_1, I2C_LSM6DSL_ADDR, 1000, I2C_LSM6DSL_FREQ, 40000);
I2C lis3mdl(I2C_BUS_1, I2C_LIS3MDL_ADDR, 1000, I2C_LIS3MDL_FREQ, 40000);
I2C bme388(I2C_BUS_1, I2C_BME388_ADDR, 1000, I2C_BME388_FREQ, 40000);
GPSIMU gpsimu;
LSM6DSLSensor s_lsm6dsl((TwoWire*)1, I2C_LSM6DSL_ADDR);

static constexpr const char *TAG = "wbl::gps_imu.cpp";

esp_err_t GPSIMU::init() {
    ESP_RETURN_ON_ERROR(lsm6dsl.init(), TAG, "lsm6dsl init failed");
    ESP_RETURN_ON_ERROR(lis3mdl.init(), TAG, "lis3mdl init failed");
    ESP_RETURN_ON_ERROR(bme388.init(), TAG, "bme388 init failed");

    if (s_lsm6dsl.begin()) {
        return 1;
    }

    if (s_lsm6dsl.Enable_X()) return 1;
    if (s_lsm6dsl.Enable_G()) return 1;

    return ESP_OK;
}

AxisData GPSIMU::getGyroscope() {
    int32_t axis[3];
    s_lsm6dsl.Get_G_Axes(axis);
    return AxisData(axis[0], axis[1], axis[2]);
}

AxisData GPSIMU::getAccelerometer() {
    int32_t axis[3];
    s_lsm6dsl.Get_X_Axes(axis);
    return AxisData(axis[0], axis[1], axis[2]);
}

}