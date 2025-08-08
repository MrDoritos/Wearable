#pragma once

#include "ui.h"
#include "ui_texturewriter.h"
#include "sprites.h"

namespace wbl {
namespace UI {


template<typename Buffer, typename ElementT = ElementBaseT<Buffer>>
struct ElementFooterT : public ElementT {
    using ElementT::ElementT;
    using ElementT::operator<<;

    const char *footer_text = nullptr;
    bool drawn = false;

    constexpr ElementFooterT(Buffer &buffer, const char *footer_text):ElementT(buffer, "footer"),footer_text(footer_text) {}

    void on_draw(Event *event) override {
        if (!footer_text)
            return;

        if (drawn && !(event->value & Event::REDRAW))
            return;

        drawn = true;

        Length scr = this->buffer.getLength();

        Length len = this->getTextContentSize(footer_text, scr, Sprites::font);

        Size pos(
            (scr.width * 0.5f) - (len.width * 0.5f),
            (scr.height - len.height),
            scr.width,
            scr.height
        );

        TextureWriterT<> writer(this, pos);
        writer.text(Sprites::font, footer_text);
        //this->buffer.line((uu)0, txtlft.y-1, pos.x, txtlft.y-1, 1);
    }
};

template<typename Buffer = Sprites::DisplayTexture, typename ScreenT = ScreenBaseT<>, typename FooterT = ElementFooterT<Buffer>>
struct ScreenFooterT : public ScreenT {
    using ScreenT::ScreenT;
    using ScreenT::operator<<;

    FooterT footer;

    constexpr ScreenFooterT(Buffer &buffer, const char *footer_text):
        ScreenT(footer_text),footer(buffer, footer_text) {}

    void handle_event(Event *event) override {
        ScreenT::handle_event(event);

        footer.handle_event(event);
    }
};

}
}