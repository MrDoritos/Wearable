#pragma once

#include <inttypes.h>

namespace wbl {
namespace SH1107 {

enum : uint8_t {
    SEGREMAP                  = 0xA0,
    SEGREMAPINV               = 0xA1,
    RESUME                    = 0xA4,
    ALL_OFF                   = 0xA4,
    ALL_ON                    = 0xA5,
    NORMAL                    = 0xA6,
    INVERT                    = 0xA7,
    OFF                       = 0xAE,
    ON                        = 0xAF,
    COMSCANINC                = 0xC0,
    COMSCANDEC                = 0xC8,
};

enum : uint8_t {
    // ISEG = a/256 * (IREF = 15.625uA) * (scale factor = 32)
    SET_CONTRAST              = 0x81,
    SET_DCDC                  = 0xAD,
    SET_PRECHARGE             = 0xD9,
    SET_COMDETECT             = 0xDB,
    SET_VCOM                  = 0xDB,
    SET_MUX                   = 0xA8,
    SET_CLOCKDIV              = 0xD5,
    SET_COMPINS               = 0xDA,
};

enum : uint8_t {
    SET_DISPLAYSTARTLINE      = 0xDC,
    SET_STARTLINE             = 0x40,
    SET_OFFSET                = 0xD3,
    SET_PAGEADDR              = 0xB0,
    MEMORYMODE                = 0x20,
    COLUMNADDR                = 0x21,
    PAGEADDR                  = 0x22,
    PAGEADDR_START            = 0x10,
};

/*
    Used to divide oscillator frequency used for internal display clocks (DCLKs)
*/
enum : uint8_t {
    DIVIDE_RATION_POR   = 0,
    DIVIDE_RATION_1     = 0,
    DIVIDE_RATION_2,
    DIVIDE_RATION_3,
    DIVIDE_RATION_4,
    DIVIDE_RATION_5,
    DIVIDE_RATION_6,
    DIVIDE_RATION_7,
    DIVIDE_RATION_8,
    DIVIDE_RATION_9,
    DIVIDE_RATION_10,
    DIVIDE_RATION_11,
    DIVIDE_RATION_12,
    DIVIDE_RATION_13,
    DIVIDE_RATION_14,
    DIVIDE_RATION_15,
    DIVIDE_RATION_16,
};

/*
    Oscillator frequency from -25% to +50%

    Frame frequency is determined by divide ratio, number of
        display clocks per row, and oscillator frequency
*/
enum : uint8_t {
    OSCILLATOR_POR  = 5,
    OSCILLATOR_N25  = 0,
    OSCILLATOR_N20,
    OSCILLATOR_N15,
    OSCILLATOR_N10,
    OSCILLATOR_N5,
    OSCILLATOR_0,
    OSCILLATOR_P5,
    OSCILLATOR_P10,
    OSCILLATOR_P15,
    OSCILLATOR_P20,
    OSCILLATOR_P25,
    OSCILLATOR_P30,
    OSCILLATOR_P35,
    OSCILLATOR_P40,
    OSCILLATOR_P45,
    OSCILLATOR_P50,
};

/*
    Internal or external DC-DC converter

    Display must be off to modify the DCDC register
*/
enum : uint8_t {
    DCDC_EXTERNAL,
    DCDC_BUILTIN,
};

/*
    Switching speed of the internal DC-DC converter from 0.6x (-25%) to 1.3x (+35%)

    Display must be off to modify the DCDC register
*/
enum : uint8_t {
    DCDC_SWITCH_POR     = 0,
    DCDC_SWITCH_0SF6    = 0,
    DCDC_SWITCH_0SF7,
    DCDC_SWITCH_0SF8,
    DCDC_SWITCH_0SF9,
    DCDC_SWITCH_1SF0,
    DCDC_SWITCH_1SF1,
    DCDC_SWITCH_1SF2,
    DCDC_SWITCH_1SF3,
};

/*
    Precharge period in DCLKs. 
    When PRECHARGE_0, period for display will decrease by 2 DCLKs
*/
enum : uint8_t {
    PRECHARGE_0,
    PRECHARGE_1,
    PRECHARGE_2     = 2,
    PRECHARGE_POR   = 2,
    PRECHARGE_3,
    PRECHARGE_4,
    PRECHARGE_5,
    PRECHARGE_6,
    PRECHARGE_7,
    PRECHARGE_8,
    PRECHARGE_9,
    PRECHARGE_10,
    PRECHARGE_11,
    PRECHARGE_12,
    PRECHARGE_13,
    PRECHARGE_14,
    PRECHARGE_15,
};

/*
    Discharge period in DCLKs
*/
enum : uint8_t {
    DISCHARGE_1     = 1,
    DISCHARGE_2     = 2,
    DISCHARGE_POR   = 2,
    DISCHARGE_3,
    DISCHARGE_4,
    DISCHARGE_5,
    DISCHARGE_6,
    DISCHARGE_7,
    DISCHARGE_8,
    DISCHARGE_9,
    DISCHARGE_10,
    DISCHARGE_11,
    DISCHARGE_12,
    DISCHARGE_13,
    DISCHARGE_14,
    DISCHARGE_15,
};

/*
    VCOM Deselect Level

    B1 = VCOM * 0.006415 + 0.430
    Vcomh = B1 * Vref

    0x3F (1) seems to be the max? Or the datasheet truncated table rows
*/
enum : uint8_t {
    VCOM_0      = 0x00, // 0.430
    VCOM_POR    = 0x35, // 0.770
    VCOM_63     = 0x3F, // 0.834
    VCOM_64     = 0x40, // 1
    VCOM_255    = 0x40, // 1
};

const uint8_t initcmds[] = {
    OFF, 
    SET_CLOCKDIV, 0x50,
    MEMORYMODE,
    SET_CONTRAST, 0x7f,
    SET_DCDC, 0x8a,
    SEGREMAP,
    COMSCANINC,
    SET_DISPLAYSTARTLINE, 0x00,
    SET_OFFSET, 0x00,
    SET_PRECHARGE, 0x11,//0x22,
    SET_COMDETECT, 0x35,
    SET_MUX, 0x7f,
    RESUME,
    NORMAL,
    ON,   
};

}
}