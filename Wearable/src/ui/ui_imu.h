#pragma once

#include "config.h"
#include "ui.h"
#include "gps_imu.h"
#include "ui_texturewriter.h"
#include "ui_peripheral_log.h"
#include "peripheral_log.h"

namespace wbl {
namespace UI {

template<typename Buffer, typename ElementT = ElementBaseT<Buffer>>
struct ElementIMUT : public ElementT {
    using ElementT::ElementT;
    using ElementT::operator<<;

    template<int N, typename _ElementT = ElementBaseT<Buffer>, typename PeripheralT = UI::ElementPeripheralLogAutoT<Buffer, DLIMUST, float, N>>
    struct ElementIMUInfoT : public _ElementT {
        using _ElementT::_ElementT;
        using _ElementT::operator<<;

        const char *field_name;
        PeripheralT log;

        constexpr ElementIMUInfoT(Buffer &buffer, const char *field_name):
                _ElementT(buffer, field_name),
                field_name(field_name),
                log(buffer, wbl::log.imu_st) {
            StyleInfo style = {
                .display{BLOCK},
                .height{12},
                .margin{1,0,0,0},
                //.margin{0,0,0,42},
            };

            *this << style;

            StyleInfo log_style = {
                .display{INLINE},
                .width{24},
                .height{16},
                .margin{0,0,0,42},
            };

            log << log_style;
            *this << log;
        }

        void on_draw(Event *event) override {
            Size inline_bounds(
                this->getOffset(),
                this->buffer.getLength()
            );

            TextureWriterT<> writer(this, inline_bounds);

            writer.text(Sprites::font, field_name);

            writer.size.x = log.getRight()+1;

            writer.printf(Sprites::font, "% .3lf", log.get_value(-1));
        }

        void on_tick(Event *event) override {
            this->clear();
        }
    };

    ElementIMUInfoT<0> e_accx;
    ElementIMUInfoT<1> e_accy;
    ElementIMUInfoT<2> e_accz;
    ElementIMUInfoT<3> e_gyrx;
    ElementIMUInfoT<4> e_gyry;
    ElementIMUInfoT<5> e_gyrz;
    
    constexpr ElementIMUT(Buffer &buffer):
            ElementT(buffer),
            e_accx(buffer, "Acc  X"),
            e_accy(buffer, "Acc  Y"),
            e_accz(buffer, "Acc  Z"),
            e_gyrx(buffer, "Gyro X"),
            e_gyry(buffer, "Gyro Y"),
            e_gyrz(buffer, "Gyro Z") {
        *this << e_accx << e_accy << e_accz << e_gyrx << e_gyry << e_gyrz;        
    }
};

}
}