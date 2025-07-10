#pragma once

#include <inttypes.h>

namespace wbl {
namespace SH1107 {

enum : uint8_t {
    SEGREMAP                  = 0xA0,
    RESUME                    = 0xA4,
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