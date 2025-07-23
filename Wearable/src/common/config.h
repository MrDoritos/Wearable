#pragma once

#include "types.h"

#define I2C_DISPLAY_FREQ 1000000
#define I2C_SH1107_ADDR 0x3C
#define I2C_CAMM8_FREQ 400000
#define I2C_CAMM8_ADDR 0x42
#define DISPLAY_TIMEOUT 30000
#define HOLD_TIME_TO_LOCK 500
#define LOG_BUFFER_SIZE 100

#ifdef __linux__
#define INPUT_DEBUG
#define USE_EVENT_DBG
#endif

#define USE_LAYOUT_DBG