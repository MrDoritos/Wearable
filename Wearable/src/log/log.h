#pragma once

#include "types.h"
#include "config.h"
#include <assert.h>
#include <tuple>
#include <string>

namespace wbl {

template<typename IType, typename FType = float, typename RType = IType>
constexpr inline RType lerp(const IType &v1, const IType &v2, const FType &factor) {
    return RType(RType(v1) * (FType(1) - factor) + RType(v2) * factor);
}

template<typename TIME_T = int64_t>
struct DataPointTimeBaseT {
    using time_type = TIME_T;

    time_type time;

    constexpr DataPointTimeBaseT(const time_type &time):time(time) {}
    constexpr DataPointTimeBaseT():DataPointTimeBaseT(time_type()) {}

    constexpr inline time_type get_time() const { return time; }

    template<typename RType=float>
    constexpr inline RType get_factor(const DataPointTimeBaseT &other, const time_type &time) const {
        return RType(time - this->time) / RType(other.time - this->time);
    }
};

template<typename TIME_T = int64_t, typename POINT_T = unsigned short, typename DataPointBase = DataPointTimeBaseT<TIME_T>>
struct DataPointT : public DataPointBase {
    using time_type = typename DataPointBase::time_type;
    using value_type = POINT_T;

    value_type value;

    constexpr DataPointT(const time_type &time, const value_type &value):DataPointBase(time),value(value) {}
    constexpr DataPointT():DataPointT(time_type(),value_type()) {}

    constexpr inline POINT_T get_value() const { return value; }

    template<typename FType = float>
    constexpr inline DataPointT interpolate(const DataPointT &other, const FType &factor) {
        return DataPointT(
            lerp<time_type, FType, time_type>(this->time, other.time, factor),
            lerp<value_type, FType, value_type>(value, other.value, factor)
        );
    }
};

template<typename ...Args>
struct DataValueTupleT {
    using DPTup = DataValueTupleT<Args...>;

    std::tuple<Args...> members;

    constexpr DataValueTupleT(Args ...args):members(std::make_tuple(args...)) {}
    constexpr DataValueTupleT():members() {}

    template<typename Op, size_t ...Is>
    static inline constexpr DPTup apply_op(const DPTup &lhs, const DPTup &rhs, Op op, std::index_sequence<Is...>) {
        return DPTup(op(std::get<Is>(lhs.members), std::get<Is>(rhs.members))...);
    }

    template<typename RHSType, typename Op, size_t ...Is>
    static inline constexpr DPTup apply_op(const DPTup &lhs, const RHSType &rhs, Op op, std::index_sequence<Is...>) {
        return DPTup(op(std::get<Is>(lhs.members), rhs)...);
    }

    template<typename OtherT = DPTup>
    inline constexpr DPTup operator*(const OtherT &other) const {
        return apply_op(*this, other, [](auto a, auto b){return a * b;}, std::index_sequence_for<Args...>{});
    }

    template<typename OtherT = DPTup>
    inline constexpr DPTup operator+(const OtherT &other) const {
        return apply_op(*this, other, [](auto a, auto b){return a + b;}, std::index_sequence_for<Args...>{});
    }

    template<typename OtherT = DPTup>
    inline constexpr DPTup operator/(const OtherT &other) const {
        return apply_op(*this, other, [](auto a, auto b){return a / b;}, std::index_sequence_for<Args...>{});
    }

    template<typename OtherT = DPTup>
    inline constexpr DPTup &operator+=(const OtherT &other) {
        *this = (*this + other);
        return *this;
    }

    template<size_t N>
    inline constexpr const auto &get() const {
        return std::get<N>(members);
    }

    template<size_t N>
    inline constexpr auto &get() {
        return std::get<N>(members);
    }
};

template<typename TimeBase, typename Derived>
struct DataPointImplT : public TimeBase, public Derived {
    using time_type = typename TimeBase::time_type;
    using value_type = typename Derived::value_type;
    using dp_type = DataPointImplT<TimeBase, Derived>;

    using Derived::Derived;

    template<typename ...Args>
    constexpr DataPointImplT(const time_type &time, const Args& ...args):
        TimeBase(time),Derived(args...) {}
    constexpr DataPointImplT(const time_type &time, const value_type &value):
        TimeBase(time),Derived(value) {}
    constexpr DataPointImplT():
        DataPointImplT(time_type(),value_type()) {}

    template<typename FType=float>
    constexpr inline dp_type interpolate(const dp_type &other, const FType &factor) {
        return dp_type(
            lerp(this->time, other.time, factor),
            lerp(this->get_value(), other.get_value(), factor)
        );
    }
};

template<typename T, int LOOP_SIZE = LOG_BUFFER_SIZE>
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

    constexpr inline int to_rel(const int &pos) const {
        int i = pos + index;

        if (i < 0) i = count - i;

        while (i + 1 > count) i -= count;

        return i;
    }

    constexpr inline T &get(const int &pos) {
        return data[to_rel(pos)];
    }

    constexpr inline bool has(const int &pos) const {
        return count && to_rel(pos) >= 0;
    }
};

template<typename DataPoint = DataPointT<int, unsigned short>, typename DataStorage = LoopBufferT<DataPoint>>
struct DataLogT {
    using point_type = DataPoint;
    using time_type = typename DataPoint::time_type;
    using value_type = typename DataPoint::value_type;
    using storage_type = DataStorage;

    DataStorage *log;
    int64_t time_start;

    constexpr DataLogT(DataLogT &data_log):log(data_log.log),time_start(data_log.time_start){}
    constexpr DataLogT(DataStorage &log):log(&log),time_start(0){}
    constexpr DataLogT(DataStorage &log, const int64_t &time_start):log(&log),time_start(time_start){}
    constexpr DataLogT(DataStorage *log):log(log),time_start(0) {}
    constexpr DataLogT():log(nullptr),time_start(0) {}
    
    constexpr inline void set_start_time(const int64_t &time) { time_start = time; }

    constexpr inline void set_log(DataStorage &log) { this->log = &log; }

    constexpr inline void set_log(DataStorage *log) { this->log = log; }

    constexpr inline int64_t get_start_time() const { return time_start; }

    constexpr inline int64_t get_data_time(const time_type &time) const { return time_start + time; }

    constexpr inline int64_t get_data_time(const point_type &point) const { return time_start + point.time; }

    constexpr inline int64_t get_data_index_time(const int &index) const { return get_data_time(has(index) ? get(index).time : 0); }

    constexpr inline int64_t get_data_start_time() const { return get_data_index_time(0); }

    constexpr inline int64_t get_data_end_time() const { return get_data_index_time(-1); }

    constexpr inline time_type get_data_range_time() const { return time_type(get_data_end_time() - get_data_start_time()); }

    constexpr inline int size() const { return log->template size(); }

    constexpr inline int capacity() const { return log->template capacity(); }

    constexpr inline void clear() { log->template clear(); }

    constexpr inline void push_back(const DataPoint &point) { log->template push_back(point); }

    constexpr inline void push_back(const time_type &time, const value_type &value) {
        push_back(point_type(time - time_start, value));
    }

    constexpr inline bool has(const int &pos) const { return log->template has(pos); }

    constexpr inline DataPoint &get(const int &pos) { return log->template get(pos); }

    constexpr inline const DataPoint &get(const int &pos) const { return log->template get(pos); }

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
        
        *first = get(a);
        *second = get(b);

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
            return RType();

        float factor = v1.get_factor(v2, time);

        return lerp<value_type, float, RType>(v1.get_value(), v2.get_value(), factor);
    }

    constexpr inline value_type min() const {
        if (!size())
            return 0;

        value_type v = get(0).get_value();

        for (int i = 1; i < size(); i++) {
            const value_type &p = get(i).get_value();
            if (p < v)
                v = p;
        }

        return v;
    }

    constexpr inline value_type max() const {
        if (!size())
            return 0;

        value_type v = get(0).get_value();

        for (int i = 1; i < size(); i++) {
            const value_type &p = get(i).get_value();
            if (p > v)
                v = p;
        }

        return v;
    }

    constexpr inline value_type range() const {
        return max() - min();
    }

    template<typename RType = int64_t>
    constexpr inline int64_t sum() const {
        RType s = RType();

        for (int i = 0; i < size(); s += get(i).get_value(), i++);
        
        return s;
    }

    template<typename RType = value_type>
    constexpr inline RType avg() const {
        if (!size())
            return RType();

        return
            this->sum<RType>() / RType(size());
    }

    template<typename RType = int64_t>
    constexpr inline RType sum_range(const int &start_index, const int &end_index) const {
        RType s = RType();

        for (int i = start_index; i < size() && i < end_index; s += get(i).get_value(), i++);

        return s;
    }

    template<typename RType = int64_t>
    constexpr inline int64_t sum_range_time(const time_type &start, const time_type &end) const {
        return sum_range<RType>(binary_index(start), binary_index(end));
    }

    template<typename RType = value_type>
    constexpr inline RType avg_range(int start_index, int end_index) const {
        const int range = end_index - start_index;
        if (!range || !size())
            return RType();

        return
            this->sum_range<RType>(start_index, end_index) / range;
    }

    template<typename RType = value_type>
    constexpr inline RType avg_range_time(const time_type &start, const time_type &end) const {
        return avg_range<RType>(binary_index(start), binary_index(end));
    }
};

using DataPoint = DataPointT<>;
using LoopBuffer = LoopBufferT<DataPoint>;
using DataLog = DataLogT<DataPoint, LoopBuffer>;

}