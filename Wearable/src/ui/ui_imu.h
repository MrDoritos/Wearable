#pragma once

#include "config.h"
#include "ui.h"
#include "gps_imu.h"
#include "ui_textbuffer.h"

namespace wbl {
namespace UI {

template<typename Buffer, typename ElementT = ElementBaseT<Buffer>>
struct ElementIMUT : public ElementT {
    using ElementT::ElementT;
    using ElementT::operator<<;

    void on_draw(Event *event) override {
        this->clear();

        TextBuffer<500> buffer;
        AxisData acc = gpsimu.getAccelerometer();
        AxisData gyro = gpsimu.getGyroscope();

        buffer.print("Acc X %lf\n", acc.x);
        buffer.print("Acc Y %lf\n", acc.y);
        buffer.print("Acc Z %lf\n", acc.z);
        buffer.print("Gyro X %lf\n", gyro.x);
        buffer.print("Gyro Y %lf\n", gyro.y);
        buffer.print("Gyro Z %lf\n", gyro.z);

        this->draw_text(buffer.buffer, Sprites::font);
    }
};

}
}