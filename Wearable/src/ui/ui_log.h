#pragma once

#include "config.h"
#include "ui.h"
#include "log.h"

namespace wbl {
namespace UI {

template<typename Buffer, typename DataLog = DataLogT<>, typename ElementT = ElementBaseT<Buffer>>
struct ElementLogT : public ElementT, public DataLog {
    using ElementT::ElementT;
    using ElementT::operator<<;

    using point_type = typename DataLog::point_type;
    using value_type = typename DataLog::value_type;
    using time_type = typename DataLog::time_type;
    using storage_type = typename DataLog::storage_type;

    time_type last_data_time = 0;

    constexpr ElementLogT(DataLog &log):DataLog(log){}
    constexpr ElementLogT(Buffer &buffer, DataLog &log):ElementT(buffer),DataLog(log){}
    constexpr ElementLogT(Buffer &buffer, storage_type &log):ElementT(buffer),DataLog(log){}

    inline bool is_stale() const {
        return this->get_data_end_time();
    }

    void on_draw(Event *event) override {
        if (!this->is_stale())
            if (!(event->value & Event::REDRAW))
                return;

        if (this->size() < 2)
            return;

        last_data_time = this->get_data_end_time();

        ElementT::clear();

        uu py;

        const auto value_range = this->range();
        const auto value_min = this->min();

        const time_type time_range = this->get_data_range_time();
        const time_type time_min = this->get_data_start_time();

        if (value_range == 0 || time_range == 0)
            return;

        const float value_scale = 1.0 / value_range;

        const Size window = *this;
        const uu height = window.height - 1;
        const uu width = window.width;
        const uu offsetx = window.x;
        const uu offsety = window.y;

        py = height - (((this->template get(0).value - value_min) * value_scale) * height);

        for (uu x = 1; x < width; x++) {
            const time_type t = time_type((float(x) / width) * time_range) + time_min;

            const float v = this->template interpolate_value<float>(t);
            const float vy = (v - value_min) * value_scale;

            const int y = height - (vy * height);

            this->buffer.line(x + offsetx, py + offsety, x + offsetx, y + offsety, 1);

            py = y;
        }
    }
};

}
}
