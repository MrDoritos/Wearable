#pragma once

#include "types.h"

#define I2C_DISPLAY_FREQ 1000000
#define I2C_SH1107_ADDR 0x3C
#define DISPLAY_TIMEOUT 10
#define LOG_BUFFER_SIZE 100

#ifdef __linux__
#define INPUT_DEBUG
#define USE_EVENT_DBG
#endif