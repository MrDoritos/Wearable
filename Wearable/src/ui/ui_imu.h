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

    template<int N, 
             typename ValueUnit,
             typename _ElementT = ElementBaseT<Buffer>,
             typename PeripheralT = 
                UI::ElementPeripheralLogAutoT<
                    Buffer,
                    DLIMUST,
                    float,
                    N,
                    ValueUnit
                >
            >
    struct ElementIMUInfoT : public _ElementT {
        using _ElementT::_ElementT;
        using _ElementT::operator<<;

        const char *field_name;
        const char *field_sub;
        PeripheralT log;

        constexpr ElementIMUInfoT(Buffer &buffer, const char *field_name, const char *field_sub):
                _ElementT(buffer, field_name),
                field_name(field_name),
                field_sub(field_sub),
                log(buffer, wbl::log.imu_st) {
            StyleInfo style = {
                .display{BLOCK},
                .height{12},
                .margin{0,0,0,0},
                //.margin{0,0,0,42},
            };

            *this << style;

            StyleInfo log_style = {
                .display{INLINE},
                .width{36},
                .height{17},
                .margin{0,0,0,36},
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
            writer.subscript(Sprites::font, field_sub);

            writer.size.x = log.getRight()+1;

            //writer.printf(Sprites::font, "% .3lf", log.get_value(-1));
            writer.template print_value<typename PeripheralT::value_unit>(Sprites::font, log.get_value(-1));
        }

        void on_tick(Event *event) override {
            this->clear();
        }
    };

    ElementIMUInfoT<0, ValueBases::Gs> e_accx;
    ElementIMUInfoT<1, ValueBases::Gs> e_accy;
    ElementIMUInfoT<2, ValueBases::Gs> e_accz;
    ElementIMUInfoT<3, ValueBases::Degs> e_gyrx;
    ElementIMUInfoT<4, ValueBases::Degs> e_gyry;
    ElementIMUInfoT<5, ValueBases::Degs> e_gyrz;
    
    constexpr ElementIMUT(Buffer &buffer):
            ElementT(buffer),
            e_accx(buffer, " Acc", "X"),
            e_accy(buffer, " Acc", "Y"),
            e_accz(buffer, " Acc", "Z"),
            e_gyrx(buffer, "Gyro", "X"),
            e_gyry(buffer, "Gyro", "Y"),
            e_gyrz(buffer, "Gyro", "Z") {
        *this << e_accx << e_accy << e_accz << e_gyrx << e_gyry << e_gyrz;        
    }
};

}
}