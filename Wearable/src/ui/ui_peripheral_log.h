#pragma once

#include "config.h"
#include "ui.h"
#include "log.h"

namespace wbl {
namespace UI {

namespace ValueBases {

template<typename Base, typename T, typename T_format_type = float, int64_t base_divT = 1>
struct ValueInfo {
    using value_type = T;
    using format_type = T_format_type;

    static constexpr inline const char *prefix(const value_type &v) { return ""; }

    static constexpr inline const char *unit() { return ""; }

    static constexpr inline const char *format() { return "%f%s%s"; }

    static constexpr inline float prescale() { return 1.0 / float(base_divT); }

    template<typename RType = value_type, typename IType = value_type>
    static constexpr inline RType scale(const IType &v) {
        return RType(v * Base::prescale());
    }

    static inline int printf(char *buffer, int maxlen, const value_type &v) {
        format_type scaled = Base::template scale<format_type>(v);
        return snprintf(buffer, maxlen, Base::format(), scaled, Base::prefix(v), Base::unit());
    }
};

template<typename Base, typename T_value, typename T_format_type = float>
struct ValueSI {
    using value_type = T_value;
    using format_type = T_format_type;

    static constexpr const char *units[] = {
        "", "T", "G", "M", "k", "m", "u", "n", "p"
    };
    
    static constexpr const float scales[] = {
        1, 1e12, 1e9, 1e6, 1e3, 1e-3, 1e-6, 1e-9, 1e-12
    };

    template<typename T = value_type>
    static constexpr inline int get_n(const T &v) {
        for (int i = 0; i < sizeof(scales)/sizeof(scales[0]); i++)
            if (v > scales[i])
                return i;
        return 0;
    }

    static constexpr inline const char *unit() { return ""; }

    static constexpr inline const char *format() { return "%f%s%s"; }

    template<typename IType = value_type>
    static constexpr inline const char *prefix(const IType &v) {
        return units[get_n<IType>(v)];
    }

    static constexpr inline float prescale() { return 1.0; }

    template<typename RType = value_type, typename IType = value_type>
    static constexpr inline RType scale(const IType &value) {
        return RType(value * (1.0/scales[get_n<IType>(value)]));
    }

    template<typename IType = value_type>
    static inline int printf(char *buffer, int maxlen, const IType &v) {
        const format_type prescaled = Base::prescale() * v;
        const format_type scaled = Base::template scale<format_type, format_type>(prescaled);
        return snprintf(buffer, maxlen, Base::format(), scaled, Base::prefix(prescaled), Base::unit());
    }

    template<typename IType = value_type>
    static inline int print_value(char *buffer, const int &maxlen, const IType &v) {
        const format_type scaled = Base::template scale<format_type, format_type>(Base::prescale() * v);
        return snprintf(buffer, maxlen, Base::format(), scaled, "", "");
    }

    template<typename IType = value_type>
    static inline int print_unit(char *buffer, const int &maxlen, const IType &v) {
        const format_type prescaled = Base::prescale() * v;
        return snprintf(buffer, maxlen, "%s%s", Base::prefix(prescaled), Base::unit());
    }
};
    
template<int64_t base_scale = 1, typename valueT = int64_t, typename formatT = float>
struct TimeScale : public ValueSI<TimeScale<base_scale, valueT, formatT>, valueT, formatT> {
    using value_type = valueT;
    using format_type = formatT;

    static constexpr inline const char *unit() { return "s"; }
    static constexpr inline float prescale() { return 1.0f / float(base_scale); }
};

using TimeMilli = TimeScale<1000>;
using TimeMicro = TimeScale<1000000>;

template<char const *s_unitT, char const *s_formatT, int64_t base_divT = 1, typename valueT = float, typename formatT = float>
struct SIBase : public ValueSI<SIBase<s_unitT, s_formatT, base_divT, valueT, formatT>, valueT, formatT> {
    using value_type = valueT;
    using format_type = formatT;

    static constexpr inline const char *unit() { return s_unitT; }
    static constexpr inline const char *format() { return s_formatT; }
    static constexpr inline float prescale() { return 1.0f / float(base_divT); }
};

char unit_none[] = "";
char format_float[] = "%.1f%s%s";
char unit_voltage[] = "V";
char unit_Gs[] = "g";
char unit_Rads[] = "rads";
char unit_Degs[] = "d/s";

using ValueBase = SIBase<unit_none, format_float>;
using Voltage = SIBase<unit_voltage, format_float, 1000>;
using Gs = SIBase<unit_Gs, format_float>;
using Rads = SIBase<unit_Rads, format_float>;
using Degs = SIBase<unit_Degs, format_float>;

}

template<typename DataLog, typename Derived, typename TimeUnit = ValueBases::TimeMicro, typename ValueUnit = ValueBases::ValueBase>
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

    template<typename VType = value_type, typename SumType = double>
    inline bool get_value_axis(const int &index_start, const int &index_end, VType &value_min, VType &value_max, VType &value_range, SumType &value_sum) {
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

    template<typename VType = value_type, typename SumType = double>
    inline bool get_value_axis(VType &value_min, VType &value_max, VType &value_range, SumType &value_sum) {
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

namespace LogOpts {

enum Interpolation : uint8_t {
    INTERPOLATION_AUTO=0,
    ALWAYS=1,
    NEVER=2,
};

enum Range : uint8_t {
    RANGE_AUTO=0,
    MIRROR=1,
    REFERENCE_VALUE=2,
    REFERENCE_MIDPOINT=4,
};

enum Display : uint16_t {
    DISPLAY_NONE=0,
    LABEL_X=1,
    LABEL_Y=2,
    REFERENCE_X=4,
    REFERENCE_Y=8,
    MIN_X=16,
    MIN_Y=32,
    MAX_X=64,
    MAX_Y=128,
    SAMPLE_COUNT=256,
    RANGE_X=512,
    RANGE_Y=1024,
    LABEL_OUTSIDE=2048,
};

template<
    LogOpts::Interpolation ValueInterpolation,
    LogOpts::Range ValueRange,
    LogOpts::Display PlotDisplay
>
struct Opts {
    static constexpr LogOpts::Interpolation value_interpolation = ValueInterpolation;
    static constexpr bool auto_interp = value_interpolation == 0;
    static constexpr bool always_interp = value_interpolation & LogOpts::ALWAYS;
    static constexpr bool use_interp = always_interp || auto_interp;
    static constexpr LogOpts::Range value_range = ValueRange;
    static constexpr bool auto_range = value_range == 0;
    static constexpr bool mirror_range = value_range & LogOpts::MIRROR;
    static constexpr bool use_midpoint = value_range & LogOpts::REFERENCE_MIDPOINT;
    static constexpr bool use_reference = value_range & LogOpts::REFERENCE_VALUE;
    static constexpr float reference_value = 0.0f;
    static constexpr LogOpts::Display plot_display = PlotDisplay;
    static constexpr bool draw_reference_x = plot_display & LogOpts::REFERENCE_X;
    static constexpr bool draw_reference_y = plot_display & LogOpts::REFERENCE_Y;
    static constexpr bool draw_label_x = plot_display & LogOpts::LABEL_X;
    static constexpr bool draw_label_y = plot_display & LogOpts::LABEL_Y;
    static constexpr bool draw_label_min_x = plot_display & LogOpts::MIN_X;
    static constexpr bool draw_label_min_y = plot_display & LogOpts::MIN_Y;
    static constexpr bool draw_label_max_x = plot_display & LogOpts::MAX_X;
    static constexpr bool draw_label_max_y = plot_display & LogOpts::MAX_Y;
    static constexpr bool draw_sample_count = plot_display & LogOpts::SAMPLE_COUNT;
    static constexpr bool draw_label_outside = plot_display & LogOpts::LABEL_OUTSIDE;
    static constexpr bool draw_range_x = plot_display & LogOpts::RANGE_X;
    static constexpr bool draw_range_y = plot_display & LogOpts::RANGE_Y;
};

using Basic = Opts<INTERPOLATION_AUTO, RANGE_AUTO, REFERENCE_Y | RANGE_Y | LABEL_OUTSIDE>;
using Small = Opts<ALWAYS, RANGE_AUTO, REFERENCE_Y | RANGE_Y | LABEL_OUTSIDE>;

}

template<typename Buffer, typename LogField, typename LogOptsT = LogOpts::Basic, typename ElementT = ElementBaseT<Buffer>> 
struct ElementPeripheralLogT : public ElementT {
    using ElementT::ElementT;
    using ElementT::operator<<;

    using log_type = LogField;
    using time_type = typename log_type::time_type;
    using value_type = typename log_type::value_type;
    using point_type = typename log_type::point_type;
    using log_opts = LogOptsT;

    log_type *log = nullptr;
    time_type last_data_time = 0;

    constexpr ElementPeripheralLogT(Buffer &buffer, log_type &log):ElementT(buffer),log(&log) {}
    constexpr ElementPeripheralLogT(Buffer &buffer, log_type *log):ElementT(buffer),log(log) {}

    struct PlotContext {
        const Size plot_size;
        const time_type time_min, time_max, time_range;
        const value_type value_min, value_max, value_range;
        const float time_range_inv, value_range_inv;
        TextureWriterT<> writer;

        constexpr PlotContext(const Size &plot_size,
                                const time_type &time_min,
                                const time_type &time_max,
                                const time_type &time_range,
                                const value_type &value_min,
                                const value_type &value_max,
                                const value_type &value_range,
                                const float &time_range_inv,
                                const float &value_range_inv,
                                TextureWriterT<> &&writer):
            plot_size(plot_size),
            time_min(time_min),time_max(time_max),time_range(time_range),
            value_min(value_min),value_max(value_max),value_range(value_range),
            time_range_inv(time_range_inv),value_range_inv(value_range_inv),
            writer(writer) {}

        constexpr inline uu get_offset_x(const time_type &time) const {
            return (uu)((time - time_min) * time_range_inv * plot_size.width);
        }

        constexpr inline uu get_x(const time_type &time) const {
            return get_offset_x(time) + plot_size.x;
        }

        template<typename T = value_type>
        constexpr inline uu get_offset_y(const T &value) const {
            const int y = plot_size.height - ((value - value_min) * value_range_inv * plot_size.height);
            return (y > plot_size.height) ? plot_size.height : (y < 0 ? 0 : (uu)y);
        }

        constexpr inline uu get_text_top_offset() const {
            return 0;
        }

        constexpr inline uu get_text_bottom_offset() const {
            if (log_opts::draw_label_outside)
                return plot_size.height + 5;
            if (plot_size.height > 4)
                return plot_size.height - 5; 
            return 0;               
        }

        template<typename T = value_type>
        constexpr inline uu get_y(const T &value) const {
            return get_offset_y<T>(value) + plot_size.y;
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

    inline PlotContext get_plot_context() {
        time_type tmin = 0, tmax = 0, trange = 0;
        value_type vmin = 0, vmax = 0, vrange = 0, vsum;

        log->template get_time_axis(tmin, tmax, trange);
        log->template get_value_axis(vmin, vmax, vrange, vsum);

        Size window = *this;

        if (log_opts::draw_label_outside) {
            if (window.height > 10) {
                window.height = window.height - 10;
                window.y += 5;
            }
        }

        const Size plot_size(
            window.x, window.y,
            window.width, window.height - 1
        );

        if (log_opts::mirror_range) {
            value_type dmin = vmin - log_opts::reference_value;
            value_type dmax = vmax - log_opts::reference_value;
            dmin = dmin < 0 ? -dmin : dmin;
            dmax = dmax < 0 ? -dmax : dmax;

            if (dmin > dmax)
                dmax = dmin;
            else
                dmin = dmax;

            vmin = log_opts::reference_value - dmin;
            vmax = log_opts::reference_value + dmax;
            vrange = vmax - vmin;
        }

        //WBL_DF("%s window_size [%u %u %u %u] plot_size [%u %u %u %u]\n", this->name, window.x, window.y, window.width, window.height, plot_size.x, plot_size.y, plot_size.width, plot_size.height);

        return PlotContext(
            plot_size,
            tmin, tmax, trange,
            vmin, vmax, vrange,
            (trange > 0) ? 1.0f / float(trange) : 0,
            (vrange > 0) ? 1.0f / float(vrange) : 0,
            TextureWriterT<>(this)
        );
    }

    void draw_reference_y(PlotContext &ctx) {
        value_type yref = ctx.value_range * 0.5 + ctx.value_min;
        if (log_opts::use_reference)
            yref = log_opts::reference_value;
        const uu ypos = ctx.get_offset_y(yref);

        for (int x = 0; x < ctx.plot_size.width; x+=2) {
            this->buffer.putPixel(x + ctx.plot_size.x, ypos + ctx.plot_size.y, 1);
        }

        if (log_opts::draw_label_y) {
            const uu ytx = ypos < 5 ? 0 : ypos - 4;
            ctx.writer.setOffsetY(ytx);
            ctx.writer.template print_value<typename log_type::value_unit>(Sprites::minifont, yref);
        }
    }

    void draw_reference(PlotContext &ctx) {
        if (log_opts::draw_reference_y)
            draw_reference_y(ctx);
        if (log_opts::draw_range_y) {
            ctx.writer.setOffsetY(ctx.get_text_bottom_offset());
            ctx.writer.template print_value<typename log_type::value_unit>(Sprites::minifont, ctx.value_range);
        }
    }

    void on_draw(Event *event) override {
        if (log == nullptr || log->template get_size() < 2)
            return;

        if (!this->is_stale())
            if (!(event->value & Event::REDRAW))
                return;

        last_data_time = log->template get_time_max();

        this->clear();

        PlotContext ctx = get_plot_context();

        if (ctx.time_range == 0 || ctx.value_range == 0)
            return;

        draw_reference(ctx);

        uu py = 0, px = 0;
        time_type pt = ctx.time_min;
        const time_type inc = (float(1.0)/ctx.plot_size.width) * ctx.time_range;//ctx.get_time(1);
        const int len = log->template get_size();

        const bool intrp = ((len < ctx.plot_size.width) && log_opts::use_interp) || log_opts::always_interp;

        py = ctx.get_y(intrp ? log->template get_value(0) : log->template get_value_average_time<float>(ctx.time_min, ctx.time_min + inc));

        //printf("%i %i %i %i %lli\n", ctx.plot_size.x, ctx.plot_size.y, ctx.plot_size.width, ctx.plot_size.height, ctx.time_max);
        //printf("%lli %u %.f %.f\n", ctx.time_range, ctx.value_range, ctx.time_range_inv, ctx.value_range_inv);

        for (uu x = 1; x < ctx.plot_size.width; x++) {
            const time_type time = ctx.get_time(x);

            const float v = intrp ? log->template get_value_interpolate_time<float>(time) : log->template get_value_average_time<float>(pt - inc, time + inc);
            
            const uu y = ctx.template get_y<float>(v);

            this->buffer.line(uu(px + ctx.plot_size.x), py, uu(x + ctx.plot_size.x), y, 1);

            //printf("%lli %.f %u %u\n", time, v, x, y);

            py = y;
            px = x;
            pt = time;
        }
    }
};
    
template<
    typename Buffer, 
    typename DataLog,
    typename ValueT,
    int N,
    typename ValueUnitT = ValueBases::ValueBase,
    typename TimeUnitT = ValueBases::TimeMicro,
    typename LogOptsT = LogOpts::Basic,
    typename PointT = typename DataLog::point_type,
    typename LogFieldT = LogField<
        LogFieldProvider<
            DataLog,
            DataValueAccessor<
                PointT,
                ValueT,
                N
            >,
            TimeUnitT,
            ValueUnitT
        >
    >,
    typename ElementT = ElementPeripheralLogT<Buffer, LogFieldT, LogOptsT>
>
struct ElementPeripheralLogAutoT : public LogFieldT, public ElementT {
    using ElementT::ElementT;
    using ElementT::operator<<;

    constexpr ElementPeripheralLogAutoT(Buffer &buffer, DataLog &log)
        :LogFieldT(&log),ElementT(buffer, (LogFieldT*)this) {}
};

}
}