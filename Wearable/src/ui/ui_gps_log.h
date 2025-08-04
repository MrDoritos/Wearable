#pragma once

#include "config.h"
#include "ui.h"
#include "log.h"
#include "peripheral_log.h"
#include "ui_peripheral_log.h"

namespace wbl {
namespace UI {

template<typename Buffer, typename DataLog, typename ElementT = ElementBaseT<Buffer>>
struct UIGPSLogT : public ElementT {
    using ElementT::ElementT;
    using ElementT::operator<<;

    using log_type = DataLog;
    using time_type = typename log_type::time_type;
    using value_type = typename log_type::value_type;
    using point_type = typename log_type::point_type;
    using axis_type = float;

    using x_provider_type = LogField<LogFieldProvider<log_type, DataValueAccessor<point_type, float, 0>>>;
    using y_provider_type = LogField<LogFieldProvider<log_type, DataValueAccessor<point_type, float, 1>>>;

    log_type *log = nullptr;
    x_provider_type x_provider;
    y_provider_type y_provider;
    time_type last_data_time = 0;

    constexpr UIGPSLogT(Buffer &buffer, log_type &log)
        :ElementT(buffer),log(&log),
        x_provider(&log), y_provider(&log) {}

    struct PlotContext2D {
        Size plot_size;
        axis_type x_min, x_max, x_range;
        axis_type y_min, y_max, y_range;
        float x_range_inv, y_range_inv;

        constexpr inline uu get_x(const axis_type &x) const {
            const int _x = ((x - x_min) * x_range_inv * plot_size.width) + plot_size.x;
            return (uu)_x;
        }

        constexpr inline uu get_y(const axis_type &y) const {
            const int _y = plot_size.height - ((y - y_min) * y_range_inv * plot_size.height);
            return ((_y > plot_size.height) ? plot_size.height : ((y < 0) ? 0 : (uu)_y)) + plot_size.y;
        }

        inline Origin get_position(const axis_type &x, const axis_type &y) const {
            return Origin(
                get_x(x),
                get_y(y)  
            );
        }

        inline Origin get_position(const point_type &point) const {
            return get_position(point.longitude, point.latitude);
        }
    };

    inline bool is_stale() const {
        return last_data_time != log->template get_data_end_time();
    }

    inline PlotContext2D get_plot_context() {
        axis_type xmin = 0, xmax = 0, xrange = 0, xsum;
        axis_type ymin = 0, ymax = 0, yrange = 0, ysum;

        x_provider.template get_value_axis(xmin, xmax, xrange, xsum);
        y_provider.template get_value_axis(ymin, ymax, yrange, ysum);

        Size window = *this;
        Size plot_size(
            window.x, window.y, window.width, window.height - 1
        );

        PlotContext2D ctx;
        ctx.x_min = xmin;
        ctx.x_max = xmax;
        ctx.x_range = xrange;
        ctx.y_min = ymin;
        ctx.y_max = ymax;
        ctx.y_range = yrange;
        ctx.plot_size = plot_size;
        ctx.x_range_inv = xrange > 0 ? 1.0f / xrange : 0;
        ctx.y_range_inv = yrange > 0 ? 1.0f / yrange : 0;

        return ctx;
    }

    void on_draw(Event *event) override {
        if (log == nullptr || log->template size() < 2)
            return;

        if (!this->is_stale())
            if (!(event->value & Event::REDRAW))
                return;
        
        last_data_time = log->template get_data_end_time();

        this->clear();

        PlotContext2D ctx = get_plot_context();

        if (ctx.x_range == 0 || ctx.y_range == 0)
            return;

        Origin prev = ctx.get_position(log->template get(0));
        const int len = log->template size();

        for (int i = 1; i < len; i++) {
            Origin n = ctx.get_position(log->template get(i));

            this->buffer.line(prev.x, prev.y, n.x, n.y, 1);

            prev = n;
        }        
    }
};

}
}