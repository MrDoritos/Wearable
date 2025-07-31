#pragma once

#include "i2c.h"
#include "config.h"
#include "wbl_func.h"

namespace wbl {

using I2C_LSM6DSL = I2C<I2C_LSM6DSL_ADDR, I2C_LSM6DSL_FREQ, 1000, I2C_BUS_1, 40000>;
using I2C_LIS3MDL = I2C<I2C_LIS3MDL_ADDR, I2C_LIS3MDL_FREQ, 1000, I2C_BUS_1, 40000>;
using I2C_BME388 = I2C<I2C_BME388_ADDR, I2C_BME388_FREQ, 1000, I2C_BUS_1, 40000>;

I2C_LSM6DSL lsm6dsl;
I2C_LIS3MDL lis3mdl;
I2C_BME388 bme388;

}