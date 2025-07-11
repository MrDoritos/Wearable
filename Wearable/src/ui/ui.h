#pragma once

#include <inttypes.h>

namespace wbl {
namespace UI {

typedef uint16_t uu;

enum Unit : uint8_t;
struct Dimension;
struct Origin;
struct Length;
struct Box;
struct Size;

enum Unit : uint8_t {
    PX=0,
    PERC=1,
    NONE=2,
};

enum Display : uint8_t {
    BLOCK=0,
    INLINE=1,
    INLINE_BLOCK=2,
};

enum Align : uint8_t {
    LEFT=0,
    TOP=1,
    RIGHT=2,
    BOTTOM=4,
    VCENTER=8,
    HCENTER=16,
};

enum Position : uint8_t {
    STATIC=0,
    ABSOLUTE=1,
};

enum Overflow : uint8_t {
    AUTO=0,
    HIDDEN=1,
    SCROLL=2,
};

struct Dimension {
    Unit unit;
    uu value;

    constexpr Dimension():unit(NONE),value(0){}
    constexpr Dimension(const uu &v, const Unit &unit=PX):unit(unit),value(v){}
    
    template<typename RET=uu, typename CALC=float>
    constexpr RET resolve(const Dimension &major) {
        switch (unit) {
            case PX:
                return RET(value);
            case PERC:
                return RET(CALC(major.value) * (CALC(value) * 0.01));
            default:
                return RET(0);
        }
    }

    friend constexpr inline bool operator==(const Dimension &a, const Dimension &b) {
        return (a.unit == b.unit) && (a.value == b.value);
    }

    friend constexpr inline bool operator!=(const Dimension &a, const Dimension &b) {
        return !(a == b);
    }

    friend constexpr inline bool operator>(const Dimension &a, const Dimension &b) {
        if (a.unit == b.unit)
            return a.value > b.value;
        return (b.unit == NONE);
    }

    friend constexpr inline bool operator<(const Dimension &a, const Dimension &b) {
        return b > a;
    }

    friend constexpr inline bool operator<=(const Dimension &a, const Dimension &b) {
        return !(a > b);
    }

    friend constexpr inline bool operator>=(const Dimension &a, const Dimension &b) {
        return !(a > b);
    }
};

static constexpr Dimension NONEDIM = Dimension(0, NONE);

struct Origin {
    uu x, y;
    constexpr Origin():x(0),y(0){}
    constexpr Origin(const uu &x, const uu &y):x(x),y(y){}
    constexpr Origin(const Box &b);

    constexpr uu getOffsetX() { return x; }
    constexpr uu getOffsetY() { return y; }
};

template<typename T>
struct LengthT {
    typedef T value_type;

    T width, height;

    constexpr LengthT(const T &width, const T &height):width(width),height(height){}
    constexpr LengthT(){}

    constexpr T getWidth() { return width; }
    constexpr T getHeight() { return height; }
};

struct Length : public LengthT<uu> {
    constexpr Length(const uu &width, const uu &height):LengthT(width, height){}
    constexpr Length():Length(0,0){}
    constexpr Length(const Box &b);
};

struct Size : public Origin, public Length {
    constexpr Size(){}
    constexpr Size(const uu &x, const uu &y, const uu &w, const uu &h):Origin(x,y),Length(w,h){}
    constexpr Size(const Origin &o, const Length &l):Origin(o),Length(l){}
    constexpr Size(const Box &b);

    constexpr uu getLeft() { return x; }
    constexpr uu getTop() { return y; }
    constexpr uu getRight() { return x + width; }
    constexpr uu getBottom() { return y + height; }
};

struct Box {
    Dimension top, right, bottom, left;

    constexpr Box(){}
    constexpr Box(const uu &v, const Unit &u):top(v,u),right(v,u),bottom(v,u),left(v,u){}
    constexpr Box(const Dimension &t, const Dimension &r, const Dimension &b, const Dimension &l):top(t),right(r),bottom(b),left(l){}
    constexpr Box(const uu &t, const uu &r, const uu &b, const uu &l):top(t),right(r),bottom(b),left(l){}
    constexpr Box(const Size &s):top(s.y),right(s.x+s.width),bottom(s.y+s.height),left(s.x){}
    
    constexpr Box resolve(const Box &major) {
        const Box resolved(
            top.resolve(major.top),
            right.resolve(major.right),
            bottom.resolve(major.bottom),
            left.resolve(major.left)
        );
        return resolved;
    }

    friend constexpr inline Box operator+(const Box &a, const Box &b) {
        return Box(
            a.top.value+b.top.value,
            a.right.value+b.right.value,
            a.bottom.value+b.bottom.value,
            a.left.value+b.left.value
        );
    }
    
    friend constexpr inline Box& operator+=(Box &a, const Box &b) {
        return a = (a + b);
    }
};

constexpr Origin::Origin(const Box &b):x(b.left.value),y(b.top.value){}

constexpr Length::Length(const Box &b):Length(b.right.value-b.left.value,b.bottom.value-b.top.value){}

constexpr Size::Size(const Box &b):Origin(b),Length(b){}

template<typename T>
struct AxisT {
    typedef T value_type;

    T x, y;

    constexpr AxisT(){}
    constexpr AxisT(const T &x, const T &y):x(x),y(y){}
};

using OverflowT = AxisT<Overflow>;

template<typename T>
struct ValueMinMaxT {
    typedef T value_type;

    T value, min, max;

    constexpr ValueMinMaxT(const T &value, const T &min, const T &max):value(value),min(min),max(max){}
};

template<typename RType, typename SRCType, typename BType>
static constexpr inline RType resolve(const SRCType &src, const BType &b) {
    return b;
}

template<typename RType, typename SRCType>
static constexpr inline RType resolve(const SRCType &src, const Dimension &b) {
    return b.resolve<RType>(src);
}

struct DimensionMinMax : public ValueMinMaxT<Dimension> {
    constexpr DimensionMinMax(const Dimension &value, const Dimension &min, const Dimension &max):ValueMinMaxT(value,min,max){}
    constexpr DimensionMinMax(const Dimension &value):DimensionMinMax(value, NONEDIM, NONEDIM) {}
    constexpr DimensionMinMax():DimensionMinMax(NONEDIM){}

    template<typename IType = Dimension, typename RType = Dimension>
    constexpr RType getImplicitValue(const IType &value) {
        if (max > value) {
            if (min > max)
                return min;
            return max;
        }

        if (min > value)
            return min;

        return value;
    }

    template<typename RType = Dimension>
    constexpr RType getExplicitValue() {
        return getImplicitValue<RType>(value);
    }

    template<typename IType = Dimension, typename RType = Dimension>
    constexpr RType getComparedValue(const IType &value_candidate) {
        return getImplicitValue<RType>(value_candidate > value ? value_candidate : value);
    }

    template<typename IType = Dimension, typename RType = DimensionMinMax>
    constexpr RType resolve(const IType &src) {
        const RType resolved(
            UI::resolve(src, value),
            UI::resolve(src, min),
            UI::resolve(src, max)
        );

        return resolved;
    }
};

struct Style : public Size {
    Align align{LEFT}, text_align{LEFT};
    Display display{BLOCK};
    Position position{STATIC};
    DimensionMinMax width, height;
    Box margin, padding;
    OverflowT overflow;

    Length content, used;
    LengthT<Dimension> computed;

    constexpr Style(){}
};

struct Element : public Style {
    Element *parent;
    Element *sibling;
    Element *child;

    constexpr Element():parent(nullptr),sibling(nullptr),child(nullptr){}

    constexpr void compute_layout() {
        if (!parent)
            computed = {width.getExplicitValue(), height.getExplicitValue()};
    }
};

};
};