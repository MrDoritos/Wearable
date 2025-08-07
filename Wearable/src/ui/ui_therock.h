#pragma once

#include "ui.h"
#include "sprites.h"

namespace wbl {
namespace UI {

template<typename Buffer, typename ElementT = ElementBaseT<Buffer>>
struct ElementTheRockT : public ElementT {
    using ElementT::ElementT;
    using ElementT::operator<<;

    constexpr ElementTheRockT(Buffer &buffer):ElementT(buffer) {}

    bool drawn = false;

    void on_draw(Event *event) {
        if (!(event->value & EventValues::REDRAW))
            return;

        drawn = true;

        this->buffer.putTexture(Sprites::therock, {0,0,128,128}, {0,0});
    }
};

}
}