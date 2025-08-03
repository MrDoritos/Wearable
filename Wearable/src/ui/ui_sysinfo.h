#pragma once 

#include "config.h"
#include "ui.h"
#include "ui_texturewriter.h"

#include "ltr390.h"
#include "wbl_system.h"
#include "sh1107.h"
#include "sdcard.h"

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
        uint32_t uvs = ltr390.getUVS();
        writer.printf(Sprites::font, "UVS %li %.01fUVI\n", uvs, ltr390.getUVIhr(uvs));
        uint32_t als = ltr390.getALS();
        writer.printf(Sprites::font, "ALS %li %.01flx\n", als, ltr390.getLux(als));
        writer.printf(Sprites::font, "OLED %.3fV %.3fmA\n", Sprites::display.getVoltageCOM(), Sprites::display.getDisplayCurrentDraw()*1000.0f);
        writer.printf(Sprites::font, "OLED %.1f%% %.1fHz\n", float(Sprites::display.display_contrast)/255.0f*100.0f, Sprites::display.getDisplayRefreshFrequency());
        writer.printf(Sprites::minifont, "OLED pre %i dis %i\n", Sprites::display.precharge_period, Sprites::display.discharge_period);
        writer.printf(Sprites::font, "Free Heap %lu\n", esp_get_free_heap_size());
        writer.printf(Sprites::minifont, "SD Free %llu\n", sdcard.getFreeSize());
        writer.printf(Sprites::minifont, "SD Size %llu\n", sdcard.getTotalSize());
    }
};

}
}