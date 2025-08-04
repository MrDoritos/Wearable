#pragma once

#include "config.h"
#include "ui.h"
#include "log.h"

namespace wbl {
namespace UI {

template<typename T, const char *T_prefix = "", const char *T_format = "%.f%s", typename T_format_type = float>
struct ValueInfo {
    using value_type = T;

    static constexpr inline const char *v_prefix = T_prefix;
    static constexpr inline const char *v_format = T_format;

    static constexpr inline const char *prefix() { return v_prefix; }

    static constexpr inline const char *unit(const value_type &v) { return ""; }

    template<typename RType = value_type>
    static constexpr inline RType scale(const value_type &v) {
        return RType(v);
    }

    static inline int printf(char *buffer, int maxlen, const value_type &v) {
        const char *v_unit = unit(v);
        T_format_type scaled = scale<T_format_type>(v);
        return snprintf(buffer, maxlen, v_format, scaled, v_unit);
    }
};

template<typename T, const char *T_prefix = "", const char *T_format = "%.f%s", typename T_format_type = float>
struct ValueSI {
    using value_type = T;

    static constexpr inline const char *v_prefix = T_prefix;
    static constexpr inline const char *v_format = T_format;

    static constexpr const char *units[] = {
        "", "T", "G", "M", "k", "m", "u", "n", "p"
    };
    
    static constexpr const float scales[] = {
        1, 1e12, 1e9, 1e6, 1e3, 1e-3, 1e-6, 1e-9, 1e-12
    };

    static constexpr inline const char *prefix() { return v_prefix; }

    static constexpr inline int get_n(const value_type &v) {
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

    static constexpr inline const char *unit(const value_type &v) {
        return units[get_n(v)];
    }

    template<typename RType = value_type>
    static constexpr inline RType scale(const value_type &v) {
        return RType(v) / RType(scales[get_n(v)]);
    }

    static inline int printf(char *buffer, int maxlen, const value_type &v) {
        const char *v_unit = unit(v);
        T_format_type scaled = scale<T_format_type>(v);
        return snprintf(buffer, maxlen, v_format, scaled, v_unit);
    }
};

template<typename DataLog, typename Derived>
struct LogFieldProvider : public Derived {
    using log_type = DataLog;
    using value_type = typename Derived::value_type;
    using time_type = typename DataLog::time_type;
    using point_type = DataPointT<time_type, value_type>;
    using dl_point_type = typename DataLog::point_type;

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
};

template<typename DataPoint, typename ValueT, int N>
struct DataValueAccessor {
    using DataValue = typename DataPoint::value_type;
    using DVT = typename DataValue::DVT;
    using value_type = ValueT;
    using point_type = DataPoint;

    static constexpr inline const value_type &get_point_value(const point_type &point) {
        return point.template get_value().get<N>();
    }

    static inline value_type &get_point_value(point_type &point) {
        return point.template get_value().get<N>();
    }
};

template<typename Derived>
struct LogField : public Derived {
    using time_type = typename Derived::time_type;
    using value_type = typename Derived::value_type;
    using point_type = typename Derived::point_type;

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

    template<typename FType=float>
    inline value_type get_value_interpolate_time(const time_type &time) {
        return Derived::get_point_value(get_point_interpolate_time(time));
    }

    inline point_type get_binary_point(const time_type &time) {
        return Derived::get_point(get_binary_index(time));
    }

    inline value_type get_binary_value(const time_type &time) {
        return Derived::get_value(get_binary_index(time));
    }

    inline value_type get_value_min() {
        int len = get_size();
        if (!len) return value_type();

        value_type v = Derived::get_value(0);

        for (int i = 1; i < len; i++) {
            const value_type &p = Derived::get_value(i);
            if (p < v)
                v = p;
        }

        return v;
    }

    inline value_type get_value_max() {
        int len = get_size();
        if (!len) return value_type();

        value_type v = Derived::get_value(0);

        for (int i = 1; i < len; i++) {
            const value_type &p = Derived::get_value(i);
            if (p > v)
                v = p;
        }

        return v;
    }

    inline value_type get_value_range() {
        int len = get_size();
        if (!len) return value_type();

        value_type min, max;
        min = max = Derived::get_value(0);

        for (int i = 1; i < len; i++) {
            const value_type &p = Derived::get_value(i);
            if (p > max)
                max = p;
            if (p < min)
                min = p;
        }

        return max - min;
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
    inline RType get_value_range_sum(const int &start_index, const int &end_index) {
        RType s = 0;
        int len = get_size();

        for (int i = start_index; i < len && i < end_index; s += Derived::get_value(i), i++);

        return s;
    }

    template<typename RType = double>
    inline RType get_value_range_average(const int &start_index, const int &end_index) {
        const int range = end_index - start_index;
        int len = get_size();

        if (!range || !len)
            return RType();

        return this->get_value_range_sum<RType>(start_index, end_index) / RType(range);
    }

    template<typename RType = double>
    inline RType get_value_range_average_time(const time_type &start, const time_type &end) {
        return get_value_range_average<RType>(get_binary_index(start), get_binary_index(end));
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
};

}
}