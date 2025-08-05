#pragma once

#include "ui.h"
#include "ui_texturewriter.h"

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

        Length len = this->getTextContentSize(footer_text, Sprites::font);
        Origin pos(
            this->buffer.getWidth(),
            this->buffer.getHeight()
        );

        Origin txtlft(
            (pos.x * 0.5f) - (len.width * 0.5f),
            (pos.y - len.height)
        );

        TextureWriterT<> writer(this, txtlft);
        writer.text(Sprites::font, footer_text);
        //this->buffer.line((uu)0, txtlft.y-1, pos.x, txtlft.y-1, 1);
    }
};

}
}