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
        return last_data_time != this->get_data_end_time();
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
        //const time_type time_range = (this->get(0).time) - (this->get(-1).time);
        const time_type time_range = get(0).time - get(-1).time;
        const time_type time_min = get(-1).time;

        if (value_range == 0 || time_range == 0)
            return;

        const float value_scale = 1.0 / value_range;
        const float time_scale = 1.0 / time_range;
        const uu ox = this->getOffsetX(), oy = this->getOffsetY();

        py = ((this->get(0).value - value_min) * value_scale) * (this->getHeight()-1);

        for (uu x = 1; x < this->getWidth(); x++) {
            const time_type t = (float(x) / this->getWidth()) * time_range + time_min;
            float v = this->template interpolate_value<float>(t);
            //if (v == 0)
            //    break;
            
            const float vy = (v - value_min) * value_scale;

            const uu y = vy * (this->getHeight()-1);

            this->buffer.line((uu)(x-1)+ox,py+oy,x+ox,y+oy,1);

            py = y;
        }
    }
};

}
}
