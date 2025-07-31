#pragma once

#include "config.h"
#include "ui.h"
#include "gps.h"

namespace wbl {
namespace UI {

template<typename Buffer, typename ElementT = ElementBaseT<Buffer>>
struct ElementGPST : public ElementT {
    using ElementT::ElementT;
    using ElementT::operator<<;

    int64_t last_update = 0;

    constexpr ElementGPST(Buffer &buffer):ElementT(buffer){}

    inline bool is_stale() const {
        return last_update != gps.last_update;
    }

    template<int buflen>
    struct TextBuffer {
        char buffer[buflen];
        int offset = 0;

        template<typename FORMAT, typename ...Args>
        void print(FORMAT format, const Args&...args) {
            offset += snprintf(buffer + offset, buflen - offset, format, args...);
        }
    };

    void on_draw(Event *event) override {
        //if (!this->is_stale())
        //    if (!(event->value & Event::REDRAW))
        //        return;
        
        //last_update = gps.last_update;

        ElementT::clear();

        TextBuffer<500> buffer;

        buffer.print("Lon %lf\n", gps.getLongitude());
        buffer.print("Lat %lf\n", gps.getLatitude());
        buffer.print("Alt %lf\n", gps.getAltitude());
        time_t now = time(nullptr);
        tm t = *gmtime(&now);
        buffer.print("UTC %02i:%02i:%02i:%03i\n", t.tm_hour, t.tm_min, t.tm_sec, int(millis() % 1000));
        buffer.print("Satellites: %i\n", gps.getSatelliteCount());
        buffer.print("HAcc %.2lfmm\n", gps.getHorizontalAccuracy());
        buffer.print("VAcc %.2lfmm\n", gps.getVerticalAccuracy());

        this->draw_text(buffer.buffer, Sprites::font);
    }

};

}
}