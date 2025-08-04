#include "i2c.h"
#include "config.h"

namespace wbl {

I2C_BUS I2C_BUS_0(I2C_NUM_0, I2C_BUS_0_SDA, I2C_BUS_0_SCL, I2C_CLK_SRC_DEFAULT);
I2C_BUS I2C_BUS_1(I2C_NUM_1, I2C_BUS_1_SDA, I2C_BUS_1_SCL, I2C_CLK_SRC_DEFAULT, false);

}