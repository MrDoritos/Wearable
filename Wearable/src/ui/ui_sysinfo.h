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

        //TextBuffer<500> buffer;

        //buffer.print("BAT %.03lfV %.01lf%%\n", wbl_system.getBatteryVoltage(), wbl_system.getBatteryLevel());
        //buffer.print("UVS %li ALS %li\n", ltr390.getUVS(), ltr390.getALS());

        //this->draw_text(buffer.buffer, Sprites::font);

        TextureWriterT<ElementSysInfoT> writer(this);
        writer.printf(Sprites::font, "BAT %.03lfV %.01lf%%\n", wbl_system.getBatteryVoltage(), wbl_system.getBatteryLevel());
        writer.printf(Sprites::font, "UVS %li ALS %li\n", ltr390.getUVS(), ltr390.getALS());
    }
};

}
}