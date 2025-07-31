#pragma once

#include "config.h"
#include "ui.h"
#include "gps.h"
#include "ui_textbuffer.h"

namespace wbl {
namespace UI {

template<typename Buffer, typename ElementT = ElementBaseT<Buffer>>
struct ElementGPST : public ElementT {
    using ElementT::ElementT;
    using ElementT::operator<<;

    constexpr ElementGPST(Buffer &buffer):ElementT(buffer){}

    void on_draw(Event *event) override {
        this->clear();

        TextBuffer<500> buffer;

        buffer.print("Lon %lf\n", gps.getLongitude());
        buffer.print("Lat %lf\n", gps.getLatitude());
        buffer.print("Alt %lf\n", gps.getAltitude());
        buffer.print("Speed %.2lfm/s\n", gps.getGroundSpeed()*0.001);
        buffer.print("Bearing %.2lf\n", gps.getHeadingVehicle());

        time_t now = time(nullptr);
        tm t = *gmtime(&now);
        buffer.print("UTC %02i:%02i:%02i:%03i\n", t.tm_hour, t.tm_min, t.tm_sec, int(millis() % 1000));
        
        Length offset = this->draw_text(buffer.buffer, Sprites::font);


        buffer.clear();

        buffer.print("Satellites: %i ", gps.getSatelliteCount());
        buffer.print("PDOP %.2lf\n", gps.getPDOP());
        buffer.print("HAcc %.2lfmm ", gps.getHorizontalAccuracy());
        buffer.print("VAcc %.2lfmm\n", gps.getVerticalAccuracy());
        buffer.print("TAcc %uns\n", gps.getTimeAccuracy());

        this->draw_text(buffer.buffer, Sprites::minifont, {0, offset.height});
    }

};

}
}