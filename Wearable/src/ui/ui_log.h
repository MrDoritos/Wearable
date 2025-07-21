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

    template<typename IType>
    constexpr inline const char* time_unit(const IType &f_time) const {
        if (f_time > 1e6) return "s";
        if (f_time > 1e3) return "ms";
        return "us";
    }

    template<typename IType>
    constexpr inline const char* value_unit(const IType &f_v) const {
        if (f_v > 1e12) return "T";
        if (f_v > 1e9) return "G";
        if (f_v > 1e6) return "M";
        if (f_v > 1e3) return "k";
        if (f_v > 0) return "";
        if (f_v > 1e-3) return "m";
        if (f_v > 1e-6) return "u";
        if (f_v > 1e-9) return "n";
        if (f_v > 1e-12) return "p";
        return "";
    }

    template<typename RType = float, typename IType = time_type>
    constexpr inline RType time_scale(const IType &time) const {
        if (time > 1e6) return time/RType(1e6f);
        if (time > 1e3) return time/RType(1e3f);
        return RType(time);
    }

    template<typename IType>
    inline int time_snprintf(char *dest, const int &len, const IType &time) const {
        const char *unit = time_unit(time);
        const float ts = time_scale<float>(time);
        return snprintf(dest, len, "%.f%s", ts, unit);
    }

    inline int samples_per_time_snprintf(char *dest, const int &len, const time_type &time_range, const int &sample_count) const {
        const char *time_unit = this->time_unit(time_range);
        const float ts = time_scale<float>(time_range);
        const float ratio = sample_count / ts;
        const char *ratio_unit = value_unit<float>(ratio);
        return snprintf(dest, len, "%.f%s VALs/%s", ratio, ratio_unit, time_unit);
    }

    void draw_range_time(const Origin &pos=Origin()) {
        const time_type time_range = this->get_data_range_time();

        const int bufsize = 10;
        char buf[bufsize];

        time_snprintf(buf, bufsize, time_range);

        this->draw_text(buf, Sprites::minifont, pos, false, true);
    }

    void draw_samples_per_second(const Origin &pos=Origin()) {
        const int bufsize = 20;
        char buf[bufsize];
        const int sample_count = this->size();
        const time_type time_range = this->get_data_range_time();

        samples_per_time_snprintf(buf, bufsize, time_range, sample_count);

        this->draw_text(buf, Sprites::minifont, pos, false, true);
    }

    constexpr inline Origin get_point_position(const point_type &point, const Size &plot_size, const float &time_range_inv, const time_type &time_min, const float &value_range_inv, const value_type &value_min) {
        return Origin(
           ((point.time - time_min) * time_range_inv * plot_size.width) + plot_size.x,
           plot_size.height - ((point.value - value_min) * value_range_inv * plot_size.height) + plot_size.y
        );
    }

    constexpr inline Origin get_point_position(const point_type &point, const Size &plot_size) const {
        return get_point_position(point, plot_size, 1.0f / this->get_data_range_time(), this->get_data_start_time(), 1.0f / this->range(), this->min());
    }

    void draw_reference(const Size &plot_size) {
        const auto median = this->range()/2+this->min();
        const Origin pos = {0, plot_size.height/2};

        for (int x = 0; x < plot_size.width; x+=2) {
            this->buffer.putPixel(x + plot_size.x, pos.y + plot_size.y, 1);
        }

        const int bufsize = 10;
        char buf[bufsize];

        snprintf(buf, bufsize, "%i", median);

        this->draw_text(buf, Sprites::minifont, {0,pos.y});
    }

    void draw_min_max_reference(const Size &plot_size) {
        const auto min = this->min(), max = this->max();

        const int bufsize_min = 10, bufsize_max = 10;
        char buf_min[bufsize_min], buf_max[bufsize_max];

        snprintf(buf_min, bufsize_min, "%i", min);
        snprintf(buf_max, bufsize_max, "%i", max);

        this->draw_text(buf_max, Sprites::minifont);
        this->draw_text(buf_min, Sprites::minifont, {0, plot_size.height-5});
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
        const uu extra_height = 6;
        const uu height = window.height - 1 - extra_height;
        const uu width = window.width;
        const uu offsetx = window.x;
        const uu offsety = window.y;
        //const uu refy = height / 2;//((this->template avg<float>() - value_min) * value_scale) * height;

        const Size plot_size = {offsetx, offsety, width, height};
        draw_reference(plot_size);
        draw_min_max_reference(plot_size);

        py = height - (((this->template get(0).value - value_min) * value_scale) * height);

        for (uu x = 1; x < width; x++) {
            const time_type t = time_type((float(x) / width) * time_range) + time_min;

            const float v = this->template interpolate_value<float>(t);
            const float vy = (v - value_min) * value_scale;

            const int y = height - (vy * height);

            this->buffer.line(x + offsetx, py + offsety, x + offsetx, y + offsety, 1);

            py = y;
        }

        Origin text_pos(0, height+1);
        int bufsize = 30;
        char buf[bufsize];
        int offset = time_snprintf(buf, bufsize, time_range);
        buf[offset++] = ' ';
        offset += samples_per_time_snprintf(buf+offset, bufsize-offset, time_range, this->size());
        this->draw_text(buf, Sprites::minifont, text_pos);
        //this->draw_range_time(text_pos);
        //text_pos.y += 6;
        //this->draw_samples_per_second(text_pos);
    }
};

}
}
