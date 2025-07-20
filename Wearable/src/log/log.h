#pragma once

#include "types.h"
#include <assert.h>

namespace wbl {

template<typename IType, typename FType = float, typename RType = IType>
constexpr inline RType lerp(const IType &v1, const IType &v2, const FType &factor) {
    return RType(v1) * (FType(1) - factor) + RType(v2) * factor;
}

template<typename TIME_T, typename POINT_T>
struct DataPointT {
    using time_type = TIME_T;
    using value_type = POINT_T;

    TIME_T time;
    POINT_T value;

    constexpr DataPointT(const TIME_T &time, const POINT_T &value):time(time),value(value){}
    constexpr DataPointT():DataPointT(0,0){}

    constexpr inline TIME_T get_time() const { return time; }

    constexpr inline POINT_T get_value() const { return value; }

    constexpr inline DataPointT interpolate(const DataPointT &other, float factor) {
        return DataPointT(
            lerp(time, other.time, factor),
            lerp(value, other.value, factor)
        );
    }

    constexpr inline float get_factor(const DataPointT &other, const time_type &time) {
        return float(time - this->time) / float(other.time - this->time);
    }
};

template<typename T, int LOOP_SIZE = 1000>
struct LoopBufferT {
    static constexpr const int _size = LOOP_SIZE;

    int index, count;
    T data[_size];

    constexpr LoopBufferT():index(0),count(0){}

    constexpr inline int size() const { return count; }

    constexpr inline int capacity() const { return _size; }

    constexpr inline void clear() { count = 0; index = 0; }

    constexpr inline void push_back(const T &value) {
        if (index >= _size)
            index = 0;
        if (count < _size)
            count++;
        
        data[index++] = value;
    }

    constexpr inline int to_rel(const int &pos) {
        int i = pos + index;

        if (i < 0) i = count - i;

        while (i + 1 > count) i -= count;

        return i;
    }

    constexpr inline T &get(const int &pos) {
        return data[to_rel(pos)];
    }

    constexpr inline bool has(const int &pos) const {
        return to_rel(pos) >= 0;
    }
};

template<typename DataPoint = DataPointT<unsigned int, unsigned short>, typename DataStorage = LoopBufferT<DataPoint>>
struct DataLogT {
    using point_type = DataPoint;
    using time_type = typename DataPoint::time_type;
    using value_type = typename DataPoint::value_type;

    DataStorage &log;
    int64_t time_start;

    constexpr DataLogT(DataLogT &data_log):log(data_log.log),time_start(data_log.time_start){}
    constexpr DataLogT(DataStorage &log):log(log),time_start(0){}
    constexpr DataLogT(DataStorage &log, const int64_t &time_start):log(log),time_start(time_start){}
    
    constexpr inline void set_start_time(const int64_t &time) { time_start = time; }

    constexpr inline void set_log(DataStorage &log) { this->log = log; }

    constexpr inline void set_log(DataStorage *log) { set_log(*log); }

    constexpr inline int64_t get_start_time() const { return time_start; }

    constexpr inline int64_t get_data_time(const time_type &time) const { return time_start + time; }

    constexpr inline int64_t get_data_time(const point_type &point) const { return time_start + point.time; }

    constexpr inline int64_t get_data_index_time(const int &index) const { return get_data_time(has(index) ? get(index).time : 0); }

    constexpr inline int64_t get_data_start_time() const { return get_data_index_time(0); }

    constexpr inline int64_t get_data_end_time() const { return get_data_index_time(-1); }

    constexpr inline time_type get_data_range_time() const { return time_type(get_data_end_time() - get_data_start_time()); }

    constexpr inline int size() const { return log.template size(); }

    constexpr inline int capacity() const { return log.template capacity(); }

    constexpr inline void clear() { log.template clear(); }

    constexpr inline void push_back(const DataPoint &point) { log.template push_back(point); }

    constexpr inline void push_back(const time_type &time, const value_type &value) {
        push_back(point_type(time - time_start, value));
    }

    constexpr inline bool has(const int &pos) const { return log.template has(pos); }

    constexpr inline DataPoint &get(const int &pos) { return log.template get(pos); }

    constexpr inline const DataPoint &get(const int &pos) const { return log.template get(pos); }

    /*
        Returns the previous nearest value or the exact match, never the upper bound
    */
    constexpr int binary_index(const time_type &time, const int &start, const int &end, const int &depth=0) const {
        assert(depth < 32 && "Too much recursion");

        const int range = end - start;

        if (range < 2)
            return start;

        const int mid = start + (range/2);
        const point_type &middle = get(mid);

        if (middle.time <= time)
            return binary_index(time, mid, end, depth+1);
        else
            return binary_index(time, start, mid, depth+1);
    }

    constexpr inline int binary_index(const time_type &time) const {
        return binary_index(time, 0, size());
    }

    constexpr inline point_type &binary_search(const time_type &time) {
        return get(binary_index(time));
    }

    constexpr inline bool time_pair(const time_type &time, int &first, int &second) const {
        int i = binary_index(time);

        if (i < 1) {
            if (!size())
                return false;
            first = 0;
            second = 1;
            return true;
        }

        if (i >= size() - 1) {
            first = size()-2;
            second = size()-1;
            return true;
        }

        first = i;
        second = i+1;
        return true;
    }

    constexpr inline bool time_pair(const time_type &time, point_type *first, point_type *second) {
        int a, b;

        if (!time_pair(time, a, b))
            return false;
        
        first = &get(a);
        second = &get(b);

        return true;
    }

    constexpr inline bool time_pair(const time_type &time, point_type &first, point_type &second) {
        return time_pair(time, &first, &second);
    }

    constexpr inline point_type interpolate_point(const time_type &time) {
        point_type v1, v2;

        if (!time_pair(time, v1, v2))
            return v1;

        float factor = float(time - v1.time) / float(v2.time - v1.time);

        return v1.interpolate(v2, factor);
    }

    template<typename RType = value_type>
    constexpr inline RType interpolate_value(const time_type &time) {
        point_type v1, v2;

        if (!time_pair(time, v1, v2))
            return 0;

        float factor = v1.get_factor(v2, time);

        return lerp<value_type, float, RType>(v1.value, v2.value, factor);
    }
};


}