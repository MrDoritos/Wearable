#pragma once

#include "types.h"

namespace wbl {

template<typename TIME_T, typename POINT_T>
struct DataPointT {
    using time_type = TIME_T;
    using value_type = POINT_T;

    TIME_T time;
    POINT_T data;

    constexpr DataPointT(const TIME_T &time, const POINT_T &data):time(time),data(data){}
    constexpr DataPointT():DataPointT(0,0){}

    constexpr inline TIME_T getTime() const {
        return time;
    }

    constexpr inline POINT_T getData() const {
        return data;
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

    constexpr inline T &get(int pos) {
        int i = pos + index;

        if (i < 0)
            i = count - i;

        while (i + 1 > count)
            i -= count;
        
        return data[i];
    }
};

template<typename DataPoint = DataPointT<unsigned short, unsigned int>, typename DataStorage = LoopBufferT<DataPoint>>
struct DataLogT {
    using time_type = typename DataPoint::time_type;
    using value_type = typename DataPoint::value_type;

    DataStorage &log;
    int64_t time_start, time_end;

    constexpr DataLogT(DataStorage &log):log(log),time_start(0),time_end(0){}
    
    constexpr inline void set_start_time(const int64_t &time) {
        time_start = time;
    }

    constexpr inline int size() const { return log.template size(); }

    constexpr inline int capacity() const { return log.template capacity(); }

    constexpr inline void clear() { log.template clear(); }

    constexpr inline void push_back(const DataPoint &point) { log.template push_back(point); }

    constexpr inline DataPoint &get(int pos) { return log.template get(pos); }

};


}