#pragma once 

#include "config.h"
#include "ui.h"
#include "ui_textbuffer.h"

#include "ltr390.h"
#include "wbl_system.h"

namespace wbl {
namespace UI {

template<template<typename> typename ElementT, typename Buffer>
struct TextureWriterT {
    using Element = ElementT<Buffer>;

    Element &ref;
    Origin pos;
    Origin origin;
    Length inline_length;

    constexpr TextureWriterT(Element &ref, const Origin &origin=Origin{0}):ref(ref),pos(0),origin(origin){}

    void add_break() {
        pos.y += inline_length.height;
        inline_length = 0;
        pos.x = 0;
    }

    template<typename FontProvider, typename Format, int buflen=100, typename ...Args>
    void print(const FontProvider &font, Format format, const Args&...args) {
        char buf[buflen];
        snprintf(buf, buflen, format, args...);
        Length use = ref.template draw_text(buf, font, pos);
    }

    template<typename Sprite>
    void sprite(const Sprite &sprite) {
        if (pos.x + sprite.getWidth() > ref.getRight()) {
            pos.y += inline_length.height;
            inline_length = 0;
            pos.x = 0;
        }
    }


};

template<typename Buffer, typename ElementT = ElementBaseT<Buffer>>
struct ElementSysInfoT : public ElementT {
    using ElementT::ElementT;
    using ElementT::operator<<;

    void on_draw(Event *event) override {
        this->clear();

        TextBuffer<500> buffer;

        buffer.print("BAT %.03lfV %.01lf%%\n", wbl_system.getBatteryVoltage(), wbl_system.getBatteryLevel());
        buffer.print("UVS %li ALS %li\n", ltr390.getUVS(), ltr390.getALS());

        this->draw_text(buffer.buffer, Sprites::font);
    }
};

}
}