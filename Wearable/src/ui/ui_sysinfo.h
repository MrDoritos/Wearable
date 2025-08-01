#pragma once 

#include "config.h"
#include "ui.h"
#include "ui_texturewriter.h"

#include "ltr390.h"
#include "wbl_system.h"

namespace wbl {
namespace UI {

template<typename Buffer, typename ElementT = ElementBaseT<Buffer>>
struct ElementSysInfoT : public ElementT {
    using ElementT::ElementT;
    using ElementT::operator<<;

    void on_draw(Event *event) override {
        this->clear();

        TextureWriterT<ElementSysInfoT> writer(this);
        
        writer.printf(Sprites::font, "BAT %.03fV %.01f%%\n", wbl_system.getBatteryVoltage(), wbl_system.getBatteryLevel());
        writer.printf(Sprites::font, "UVS %li\n", /*ltr390.getUVS()*/0);
        uint32_t als = ltr390.getALS();
        writer.printf(Sprites::font, "ALS %li %.01flx\n", als, ltr390.getLux(als));
    }
};

}
}