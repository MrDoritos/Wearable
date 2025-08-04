#pragma once

#include "config.h"
#include "ui.h"
#include "log.h"

namespace wbl {
namespace UI {

template<typename T, typename T_format_type = float>
struct ValueInfo {
    using value_type = T;
    using format_type = T_format_type;

    static constexpr const char *v_prefix;
    static constexpr const char *v_format;
    static constexpr const format_type v_scale;

    static constexpr inline const char *prefix() { return v_prefix; }

    static constexpr inline const char *unit(const value_type &v) { return ""; }

    template<typename RType = value_type, typename IType = value_type>
    static constexpr inline RType scale(const IType &v) {
        return RType(v * v_scale);
    }

    static inline int printf(char *buffer, int maxlen, const value_type &v) {
        const char *v_unit = unit(v);
        format_type scaled = scale<format_type>(v);
        return snprintf(buffer, maxlen, v_format, scaled, v_unit, v_prefix);
    }
};

template<typename T_value, typename T_format_type = float>
struct ValueSI {
    using value_type = T_value;
    using format_type = T_format_type;

    static constexpr const char *v_prefix;
    static constexpr const char *v_format;
    static constexpr const format_type v_scale;

    static constexpr const char *units[] = {
        "", "T", "G", "M", "k", "m", "u", "n", "p"
    };
    
    static constexpr const float scales[] = {
        1, 1e12, 1e9, 1e6, 1e3, 1e-3, 1e-6, 1e-9, 1e-12
    };

    static constexpr inline const char *prefix() { return v_prefix; }

    template<typename T = value_type>
    static constexpr inline int get_n(const T &v) {
        if (v > 1e12) return 1;
        if (v > 1e9) return 2;
        if (v > 1e6) return 3;
        if (v > 1e3) return 4;
        if (v > 0) return 0;
        if (v > 1e-3) return 5;
        if (v > 1e-6) return 6;
        if (v > 1e-9) return 7;
        if (v > 1e-12) return 8;
        return 0;
    }

    template<typename IType = value_type>
    static constexpr inline const char *unit(const IType &v) {
        return units[get_n(v * v_scale)];
    }

    template<typename RType = value_type, typename IType = value_type>
    static constexpr inline RType scale(const IType &v) {
        return RType(v * v_scale) / RType(scales[get_n(v * v_scale)]);
    }

    static inline int printf(char *buffer, int maxlen, const value_type &v) {
        const char *v_unit = unit(v);
        format_type scaled = scale<format_type>(v);
        return snprintf(buffer, maxlen, v_format, scaled, v_unit, v_prefix);
    }
};

/*
using ValueTimeMilli = ValueSI<int64_t, float>;
template<> constexpr const char *ValueTimeMilli::v_format = "%.f%s%s";
template<> constexpr const char *ValueTimeMilli::v_prefix = "s";
template<> constexpr ValueTimeMilli::format_type ValueTimeMilli::v_scale = 0.001f;
using ValueTimeMicro = ValueSI<int64_t, float>;
template<> constexpr const char *ValueTimeMicro::v_format = "%.f%s%s";
template<> constexpr const char *ValueTimeMicro::v_prefix = "s";
template<> constexpr ValueTimeMicro::format_type ValueTimeMicro::v_scale = 0.000001f;
using ValueSIBase = ValueSI<float, float>;
template<> constexpr const char *ValueSIBase::v_format = "%.f%s%s";
template<> constexpr const char *ValueSIBase::v_prefix = "";
template<> constexpr ValueSIBase::format_type ValueSIBase::v_scale = 1.0f;
*/

template<typename DataLog, typename Derived, typename TimeUnit = int, typename ValueUnit = int>
struct LogFieldProvider : public Derived {
    using log_type = DataLog;
    using value_type = typename Derived::value_type;
    using time_type = typename DataLog::time_type;
    using point_type = DataPointT<time_type, value_type>;
    using dl_point_type = typename DataLog::point_type;
    using time_unit = TimeUnit;
    using value_unit = ValueUnit;

    using Derived::get_point_value;

    DataLog *log;

    constexpr LogFieldProvider(DataLog *log):log(log) {}
    constexpr LogFieldProvider(DataLog &log):log(&log) {}
    constexpr LogFieldProvider():LogFieldProvider(nullptr) {}

    inline value_type get_value(const int &index) {
        return Derived::get_point_value(log->template get(index));
    }

    inline point_type get_point(const int &index) {
        const dl_point_type &point = log->template get(index);
        return point_type(point.time, Derived::get_point_value(point));
    }

    constexpr inline const time_type &get_time(const int &index) const {
        const dl_point_type &point = log->template get(index);
        return point.time;
    }

    inline time_type &get_time(const int &index) {
        return log->template get(index).time;
    }
};

template<typename DataPoint, typename ValueT, int N>
struct DataValueAccessor {
    using DataValue = typename DataPoint::value_type;
    //using DVT = typename DataValue::value_type;
    using value_type = ValueT;
    using point_type = DataPoint;

    static constexpr inline const value_type get_point_value(const point_type &point) {
        return point.template get_value().template get<N>();
    }

    static inline value_type get_point_value(point_type &point) {
        return point.template get_value().template get<N>();
    }
};

template<typename Derived>
struct LogField : public Derived {
    using time_type = typename Derived::time_type;
    using value_type = typename Derived::value_type;
    using point_type = typename Derived::point_type;
    using time_unit = typename Derived::time_unit;
    using value_unit = typename Derived::value_unit;

    using Derived::Derived;

    inline int get_binary_index(const time_type &time) {
        return Derived::log->template binary_index(time);
    }

    inline time_type get_time_min() {
        return Derived::log->template get_data_start_time();
    }

    inline time_type get_time_range() {
        return Derived::log->template get_data_range_time();
    }

    inline time_type get_time_max() {
        return Derived::log->template get_data_end_time();
    }

    inline int get_size() const {
        return Derived::log->template size();
    }

    inline bool has_index(const int &index) {
        return Derived::log->template has(index);
    }

    inline bool get_time_pair(const time_type &time, int &first, int &second) {
        const int i = get_binary_index(time);
        const int len = get_size();

        if (i < 1) {
            if (!len)
                return false;
            first = 0;
            second = 1;
            return true;
        }

        if (i >= len - 1) {
            first = len - 2;
            second = len - 1;
            return true;
        }

        first = i;
        second = i + 1;
        return true;
    }

    inline bool get_time_pair_point(const time_type &time, point_type *first, point_type *second) {
        int a, b;

        if (!get_time_pair(time, a, b))
            return false;

        *first = Derived::get_point(a);
        *second = Derived::get_point(b);

        return true;
    }

    inline bool get_time_pair_point(const time_type &time, point_type &first, point_type &second) {
        return get_time_pair_point(time, &first, &second);
    }

    template<typename FType=float>
    inline point_type get_point_interpolate_time(const time_type &time) {
        point_type a, b;

        if (!get_time_pair_point(time, a, b))
            return a;

        FType factor = a.template get_factor<FType>(b, time);

        return a.template interpolate(b, factor);
    }

    template<typename RType = point_type, typename FType = float>
    inline RType get_value_interpolate_time(const time_type &time) {
        //return get_point_interpolate_time(time).value;
        point_type a, b;

        if (!get_time_pair_point(time, a, b))
            return RType(0);

        FType factor = a.template get_factor<FType>(b, time);

        return lerp<decltype(a.value), FType, RType>(a.value, b.value, factor);
    }

    inline point_type get_binary_point(const time_type &time) {
        return Derived::get_point(get_binary_index(time));
    }

    inline value_type get_binary_value(const time_type &time) {
        return Derived::get_value(get_binary_index(time));
    }

    inline value_type get_value_min(const int &index_start, const int &index_end) {
        int len = get_size();
        if (!len) return value_type();

        value_type v = Derived::get_value(index_start);

        for (int i = index_start+1; i < len && i < index_end; i++) {
            const value_type &p = Derived::get_value(i);
            if (p < v)
                v = p;
        }

        return v;
    }

    inline value_type get_value_min() {
        return get_value_min(0, get_size());
    }

    inline value_type get_value_max(const int &index_start, const int &index_end) {
        int len = get_size();
        if (!len) return value_type();

        value_type v = Derived::get_value(index_start);

        for (int i = index_start+1; i < len && i < index_end; i++) {
            const value_type &p = Derived::get_value(i);
            if (p > v)
                v = p;
        }

        return v;
    }

    inline value_type get_value_max() {
        return get_value_max(0, get_size());
    }

    inline value_type get_value_range(const int &index_start, const int &index_end) {
        int len = get_size();
        if (!len) return value_type();

        value_type min, max;
        min = max = Derived::get_value(index_start);

        for (int i = index_start+1; i < len && i < index_end; i++) {
            const value_type &p = Derived::get_value(i);
            if (p > max)
                max = p;
            if (p < min)
                min = p;
        }

        return max - min;
    }

    inline value_type get_value_range() {
        return get_value_range(0, get_size());
    }

    template<typename RType = double>
    inline bool get_value_axis(const int &index_start, const int &index_end, value_type &value_min, value_type &value_max, value_type &value_range, RType &value_sum) {
        int len = get_size();
        if (!len) return false;
        
        value_sum = value_min = value_max = Derived::get_value(index_start);

        for (int i = index_start + 1; i < len && i < index_end; i++) {
            const value_type &p = Derived::get_value(i);
            if (p > value_max)
                value_max = p;
            if (p < value_min)
                value_min = p;
            value_sum += p;
        }

        value_range = value_max - value_min;

        return true;
    }

    template<typename RType = double>
    inline bool get_value_axis(value_type &value_min, value_type &value_max, value_type &value_range, RType &value_sum) {
        return get_value_axis(0, get_size(), value_min, value_max, value_range, value_sum);
    }

    inline bool get_time_axis(const int &index_start, const int &index_end, time_type &time_min, time_type &time_max, time_type &time_range) {
        int len = get_size();
        if (!len) return false;

        time_min = Derived::get_time(index_start);
        time_max = Derived::get_time(index_end);
        time_range = time_max - time_min;

        return true;
    }

    inline bool get_time_axis(time_type &time_min, time_type &time_max, time_type &time_range) {
        return get_time_axis(0, -1, time_min, time_max, time_range);
    }

    template<typename RType = double>
    inline RType get_value_sum() {
        RType s = 0;
        int len = get_size();

        for (int i = 0; i < len; s += Derived::get_value(i), i++);

        return s;
    }

    template<typename RType = value_type>
    inline value_type get_value_average() {
        int len = get_size();
        if (!len) return RType();

        return get_value_sum<RType>() / RType(len);
    }

    template<typename RType = double>
    inline RType get_value_sum(const int &start_index, const int &end_index) {
        RType s = 0;
        int len = get_size();

        for (int i = start_index; i < len && i < end_index; s += Derived::get_value(i), i++);

        return s;
    }

    template<typename RType = double>
    inline RType get_value_average(const int &start_index, const int &end_index) {
        const int range = end_index - start_index;
        int len = get_size();

        if (!range || !len)
            return RType();

        return get_value_sum<RType>(start_index, end_index) / RType(range);
    }

    template<typename RType = double>
    inline RType get_value_average_time(const time_type &start, const time_type &end) {
        return get_value_average<RType>(get_binary_index(start), get_binary_index(end));
    }
};

template<typename Buffer, typename LogField, typename ElementT = ElementBaseT<Buffer>> 
struct ElementPeripheralLogT : public ElementT {
    using ElementT::ElementT;
    using ElementT::operator<<;

    using log_type = LogField;
    using time_type = typename log_type::time_type;
    using value_type = typename log_type::value_type;
    using point_type = typename log_type::point_type;

    log_type *log = nullptr;
    time_type last_data_time = 0;

    constexpr ElementPeripheralLogT(Buffer &buffer, log_type &log):ElementT(buffer),log(&log) {}

    struct PlotContext {
        const Size plot_size;
        const time_type time_min, time_max, time_range;
        const value_type value_min, value_max, value_range;
        const float time_range_inv, value_range_inv;

        constexpr PlotContext(const Size &plot_size,
                              const time_type &time_min,
                              const time_type &time_max,
                              const time_type &time_range,
                              const value_type &value_min,
                              const value_type &value_max,
                              const value_type &value_range,
                              const float &time_range_inv,
                              const float &value_range_inv):
            plot_size(plot_size),
            time_min(time_min),time_max(time_max),time_range(time_range),
            value_min(value_min),value_max(value_max),value_range(value_range),
            time_range_inv(time_range_inv),value_range_inv(value_range_inv) {}
            

        constexpr inline uu get_x(const time_type &time) const {
            const int x = ((time - time_min) * time_range_inv * plot_size.width) + plot_size.x;
            return (uu)x;
        }

        template<typename T = value_type>
        constexpr inline uu get_y(const T &value) const {
            const int y = plot_size.height - ((value - value_min) * value_range_inv * plot_size.height);
            return ((y > plot_size.height) ? plot_size.height : ((y < 0) ? 0 : (uu)y)) + plot_size.y;
        }

        constexpr inline time_type get_time(const uu &x) const {
            return time_type(((float(x) / plot_size.width) * time_range) + time_min);
        }

        inline Origin get_position(const point_type &point) const {
            return Origin(
                get_x(point.time),
                get_y(point.value)
            );
        }
    };

    inline bool is_stale() const {
        return last_data_time != log->template get_time_max();
    }

    constexpr inline PlotContext get_plot_context() const {
        time_type tmin = 0, tmax = 0, trange = 0;
        value_type vmin = 0, vmax = 0, vrange = 0, vsum;

        log->template get_time_axis(tmin, tmax, trange);
        log->template get_value_axis(vmin, vmax, vrange, vsum);

        const Size window = *this;
        const Size plot_size(
            window.x, window.y,
            window.width, window.height - 1
        );

        //WBL_DF("%s window_size [%u %u %u %u] plot_size [%u %u %u %u]\n", this->name, window.x, window.y, window.width, window.height, plot_size.x, plot_size.y, plot_size.width, plot_size.height);

        return PlotContext(
            plot_size,
            tmin, tmax, trange,
            vmin, vmax, vrange,
            (trange > 0) ? 1.0f / float(trange) : 0, (vrange > 0) ? 1.0f / float(vrange) : 0
        );
    }

    void on_draw(Event *event) override {
        if (log == nullptr || log->template get_size() < 2)
            return;

        if (!this->is_stale())
            if (!(event->value & Event::REDRAW))
                return;

        last_data_time = log->template get_time_max();

        this->clear();

        const PlotContext ctx = get_plot_context();

        if (ctx.time_range == 0 || ctx.value_range == 0)
            return;

        uu py = 0, px = 0;
        time_type pt = ctx.time_min;
        const time_type inc = ctx.get_time(1);
        py = ctx.get_y(log->template get_value(0));
        const int len = log->template get_size();

        //printf("%i %i %i %i %lli\n", ctx.plot_size.x, ctx.plot_size.y, ctx.plot_size.width, ctx.plot_size.height, ctx.time_max);
        //printf("%lli %u %.f %.f\n", ctx.time_range, ctx.value_range, ctx.time_range_inv, ctx.value_range_inv);

        for (uu x = 1; x < ctx.plot_size.width; x++) {
            const time_type time = ctx.get_time(x);

            const float v = len < ctx.plot_size.width ? log->template get_value_interpolate_time<float>(time) : log->template get_value_average_time<float>(pt - inc, time + inc);
            
            const uu y = ctx.template get_y<float>(v);

            this->buffer.line(uu(px + ctx.plot_size.x), py, uu(x + ctx.plot_size.x), y, 1);

            //printf("%lli %.f %u %u\n", time, v, x, y);

            py = y;
            px = x;
            pt = time;
        }
    }
};

}
}