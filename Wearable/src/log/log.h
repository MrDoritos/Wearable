#pragma once

#include "types.h"
#include <assert.h>

namespace wbl {

template<typename TIME_T, typename POINT_T>
struct DataPointT {
    using time_type = TIME_T;
    using value_type = POINT_T;

    TIME_T time;
    POINT_T data;

    constexpr DataPointT(const TIME_T &time, const POINT_T &data):time(time),data(data){}
    constexpr DataPointT():DataPointT(0,0){}

    constexpr inline TIME_T getTime() const { return time; }

    constexpr inline POINT_T getData() const { return data; }
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

    constexpr inline T &get(int pos) {
        int i = pos + index;

        if (i < 0)
            i = count - i;

        while (i + 1 > count)
            i -= count;
        
        return data[i];
    }
};

template<typename DataPoint = DataPointT<unsigned int, unsigned short>, typename DataStorage = LoopBufferT<DataPoint>>
struct DataLogT {
    using point_type = DataPoint;
    using time_type = typename DataPoint::time_type;
    using value_type = typename DataPoint::value_type;

    DataStorage &log;
    int64_t time_start;

    constexpr DataLogT(DataStorage &log):log(log),time_start(0){}
    constexpr DataLogT(DataStorage &log, const int64_t &time_start):log(log),time_start(time_start){}
    
    constexpr inline void set_start_time(const int64_t &time) { time_start = time; }

    constexpr inline int size() const { return log.template size(); }

    constexpr inline int capacity() const { return log.template capacity(); }

    constexpr inline void clear() { log.template clear(); }

    constexpr inline void push_back(const DataPoint &point) { log.template push_back(point); }

    constexpr inline void push_back(const time_type &time, const value_type &value) {
        push_back(point_type(time - time_start, value));
    }

    constexpr inline DataPoint &get(int pos) { return log.template get(pos); }

    /*
        Returns the previous nearest value or the exact match, never the upper bound
    */
    constexpr DataPoint &binary_search(const time_type &time, const int &start, const int &end, const int &depth=0) {
        assert(depth < 32 && "Too much recursion");

        const int range = end - start;

        if (range < 2)
            return get(start);

        const int mid = start + (range/2);
        const point_type &middle = get(mid);

        if (middle.time <= time)
            return binary_search(time, mid, end, depth+1);
        else
            return binary_search(time, start, mid, depth+1);
    }

    constexpr inline DataPoint &binary_search(const time_type &time) {
        return binary_search(time, 0, size());
    }

};


}