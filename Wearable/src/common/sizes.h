#pragma once

#include "types.h"

namespace wbl {

enum Unit : ub;

template<typename T>
struct DimensionT;

template<typename T>
struct OriginT;

template<typename T>
struct LengthT;

template<typename T>
struct BoxT;

template<typename T, typename Origin, typename Length>
struct SizeT;

struct Length;

using Dimension = DimensionT<uu>;
using Origin = OriginT<uu>;
using Box = BoxT<Dimension>;
using Size = SizeT<uu, Origin, Length>;

enum Unit : ub {
    PX=0,
    PERC=1,
    NONE=2,
};

template<typename T>
struct DimensionT {
    Unit unit;
    T value;

    constexpr DimensionT():unit(NONE),value(0){}
    constexpr DimensionT(const T &v, const Unit &unit=PX):unit(unit),value(v){}
    
    template<typename RType=T, typename CALC=float>
    constexpr inline RType resolve(const DimensionT &major) const {
        switch (unit) {
            case PX:
                return RType(value);
            case PERC:
                return RType(CALC(major.value) * (CALC(value) * 0.01));
            default:
                return RType();
        }
    }

    template<typename RType=T, typename CALC=float>
    constexpr inline RType resolve(const T &major) const {
        return resolve(DimensionT(major));
    }

    inline constexpr explicit operator T() const {
        return resolve(0);
    }

    friend constexpr inline DimensionT operator+(const DimensionT &a, const DimensionT &b) {
        return DimensionT(
            a.value + b.value
        );
    }

    friend constexpr inline DimensionT operator-(const DimensionT &a, const DimensionT &b) {
        return DimensionT(
            a.value - b.value
        );
    }

    friend constexpr inline bool operator==(const DimensionT &a, const DimensionT &b) {
        return (a.unit == b.unit) && (a.value == b.value);
    }

    friend constexpr inline bool operator!=(const DimensionT &a, const DimensionT &b) {
        return !(a == b);
    }

    friend constexpr inline bool operator>(const DimensionT &a, const DimensionT &b) {
        if (a.unit == b.unit)
            return a.value > b.value;
        return (b.unit == NONE);
    }

    friend constexpr inline bool operator<(const DimensionT &a, const DimensionT &b) {
        return b > a;
    }

    friend constexpr inline bool operator<=(const DimensionT &a, const DimensionT &b) {
        return !(a > b);
    }

    friend constexpr inline bool operator>=(const DimensionT &a, const DimensionT &b) {
        return !(a > b);
    }

    friend constexpr inline bool operator>(const T &a, const DimensionT &b) {
        if (a == 0)
            return false;
        if (b.unit == NONE)
            return true;
        if (b.unit == PX)
            return (a > b.value);
        return true;
        //return a > b.value;
    }

    friend constexpr inline bool operator<(const T &a, const DimensionT &b) {
        return b > a;
    }

    friend constexpr inline bool operator<=(const T &a, const DimensionT &b) {
        return !(a > b);
    }

    friend constexpr inline bool operator>=(const T &a, const DimensionT &b) {
        return !(a > b);
    }
};

static constexpr Dimension NONEDIM = Dimension(0, NONE);

template<typename T>
struct OriginT {
    typedef T value_type;

    T x, y;
    constexpr OriginT():x(0),y(0){}
    constexpr OriginT(const T &x, const T &y):x(x),y(y){}
    constexpr OriginT(const Box &b);

    constexpr inline OriginT getOffset() const { return *this; }
    constexpr inline T getOffsetX() const { return x; }
    constexpr inline T getOffsetY() const { return y; }
    constexpr inline void setOffsetX(const T &offsetX) { x = offsetX; }
    constexpr inline void setOffsetY(const T &offsetY) { y = offsetY; }

    friend constexpr inline OriginT operator+(const OriginT &a, const OriginT &b) {
        return OriginT(
            a.x + b.x,
            a.y + b.y
        );
    }

    friend constexpr inline OriginT &operator+=(OriginT &a, const OriginT &b) {
        return a = a + b;
    }
};

template<typename T>
struct LengthT {
    typedef T value_type;

    T width, height;

    constexpr LengthT(const T &width, const T &height):width(width),height(height){}
    constexpr LengthT(const T &value):width(value),height(value){}
    constexpr LengthT(){}

    constexpr inline LengthT getLength() const { return *this; }
    constexpr inline T getWidth() const { return width; }
    constexpr inline T getHeight() const { return height; }
    constexpr inline void setWidth(const T &width) { this->width = width; }
    constexpr inline void setHeight(const T &height) { this->height = height; }

    /*
    constexpr inline LengthT &operator=(const T &value) {

    }*/

    constexpr inline T getMinLength() const {
        return width > height ? height : width;
    }

    constexpr inline T getMaxLength() const {
        return width > height ? width : height;
    }

    friend constexpr inline LengthT operator+(const LengthT &a, const LengthT &b) {
        return LengthT(
            a.width + b.width,
            a.height + b.height
        );
    }

    friend constexpr inline LengthT &operator+=(LengthT &a, const LengthT &b) {
        return a = a + b;
    }

    friend constexpr inline LengthT operator-(const LengthT &a, const LengthT &b) {
        return LengthT(
            a.width - b.width,
            a.height - b.height
        );
    }

    friend constexpr inline LengthT &operator-=(LengthT &a, const LengthT &b) {
        return a = a - b;
    }

    constexpr inline explicit operator OriginT<T>() {
        return OriginT<T>(width, height);
    }
};

struct Length : public LengthT<uu> {
    using LengthT<uu>::LengthT;

    constexpr Length(const LengthT<uu> &v):LengthT(v){}
    constexpr Length(const uu &width, const uu &height):LengthT(width, height){}
    constexpr Length():Length(0,0){}
    constexpr Length(const Box &b);
};

struct LengthD : public LengthT<Dimension> {
    using LengthT<Dimension>::LengthT;

    constexpr LengthD(const LengthT<Dimension> &v):LengthT<Dimension>(v){}
    constexpr LengthD():LengthT<Dimension>(NONEDIM){}

    constexpr /*explicit*/ inline operator Length() const {
        return Length(
            (uu)width,
            (uu)height
        );
    }

    constexpr inline operator LengthT<uu>() const {
        return LengthT<uu>(
            (uu)width,
            (uu)height
        );
    }
};

template<typename T, typename Origin = OriginT<T>, typename Length = LengthT<T>>
struct SizeT : public Origin, public Length {
    constexpr SizeT(){}
    constexpr SizeT(const T &x, const T &y, const T &w, const T &h):Origin(x,y),Length(w,h){}
    constexpr SizeT(const Origin &o, const Length &l):Origin(o),Length(l){}
    constexpr SizeT(const Box &b);

    constexpr inline T getLeft() const { return this->x; }
    constexpr inline T getTop() const { return this->y; }
    constexpr inline T getRight() const { return this->x + this->width; }
    constexpr inline T getBottom() const { return this->y + this->height; }

    constexpr inline bool isBound(const T &x, const T &y) const {
        return x >= this->getLeft() && y >= this->getTop() && x < this->getRight() && y < this->getBottom();
    }

    constexpr inline bool isBound(const Origin &origin) const {
        return this->isBound(origin.x, origin.y);
    }

    constexpr inline Origin getMidpoint() const {
        return Origin(
            this->x + (this->width * 0.5),
            this->y + (this->height * 0.5)
        );
    }
};

template<typename T>
struct BoxT {
    T top, right, bottom, left;

    constexpr BoxT(){}
    constexpr BoxT(const T &t, const T &r, const T &b, const T &l):top(t),right(r),bottom(b),left(l){}
    constexpr BoxT(const T &v):BoxT(v,v,v,v){}
    constexpr BoxT(const Size &s):top(s.y),right(s.x+s.width),bottom(s.y+s.height),left(s.x){}
    
    constexpr inline T getWidth() const { return right-left; }
    constexpr inline T getHeight() const { return bottom-top; }
    constexpr inline T getTop() const { return top; }
    constexpr inline T getRight() const { return right; }
    constexpr inline T getBottom() const { return bottom; }
    constexpr inline T getLeft() const { return left; }

    constexpr inline BoxT resolve(const BoxT &major) const {
        const BoxT resolved(
            top.resolve(major.top),
            right.resolve(major.right),
            bottom.resolve(major.bottom),
            left.resolve(major.left)
        );
        return resolved;
    }

    constexpr inline BoxT resolve(const LengthD &major) const {
        return BoxT(
            top.resolve(major.height),
            right.resolve(major.width),
            bottom.resolve(major.height),
            left.resolve(major.width)  
        );
    }

    constexpr inline explicit operator Size() const {
        return Size(
            left.value,
            top.value,
            right.value-left.value,
            bottom.value-top.value
        );
    }

    constexpr inline explicit operator LengthD() const {
        return LengthD(
            right-left,
            bottom-top
        );
    }

    constexpr inline explicit operator Origin() const {
        return Origin(
            (uu)left,
            (uu)top
        );
    }

    friend constexpr inline BoxT operator+(const BoxT &a, const BoxT &b) {
        return BoxT(
            a.top.value+b.top.value,
            a.right.value+b.right.value,
            a.bottom.value+b.bottom.value,
            a.left.value+b.left.value
        );
    }

    friend constexpr inline LengthD operator+(const LengthD &a, const BoxT &b) {
        return a + (LengthD)b;
    }

    friend constexpr inline LengthD operator+(const BoxT &a, const LengthD &b) {
        return b + (LengthD)a;
    }
    
    friend constexpr inline BoxT& operator+=(BoxT &a, const BoxT &b) {
        return a = (a + b);
    }
};

template<>
constexpr Origin::OriginT(const Box &b):x(b.left.value),y(b.top.value){}

constexpr Length::Length(const Box &b):Length(b.right.value-b.left.value,b.bottom.value-b.top.value){}

template<>
constexpr Size::SizeT(const Box &b):Origin(b),Length(b){}



}