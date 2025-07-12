#pragma once

#include <inttypes.h>
#include <cassert>
#include <type_traits>
#include <stdio.h>
#include "texture.h"
#include "displaybuffer.h"
#include "types.h"
#include "sizes.h"

namespace wbl {
namespace UI {

enum Display : ub;
enum Align : ub;
enum Position : ub;
enum Overflow : ub;

template<typename T>
struct AxisT;

using OverflowT = AxisT<Overflow>;

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

template<typename T>
struct AxisT {
    typedef T value_type;

    T x, y;

    constexpr AxisT(){}
    constexpr AxisT(const T &x, const T &y):x(x),y(y){}
};

template<typename T>
struct ValueMinMaxT {
    typedef T value_type;

    T value, min, max;

    constexpr ValueMinMaxT(const T &value, const T &min, const T &max):value(value),min(min),max(max){}
};

template<typename RType, typename SRCType, typename BType>
static constexpr inline RType resolve(const SRCType &src, const BType &b) {
    if (std::is_same<BType, Dimension>::value) {
        return b.template resolve<RType>(src);
    }

    return b;
}

/*
template<typename RType, typename SRCType>
static constexpr inline RType resolve<RType, SRCType, Dimension>(const SRCType &src, const Dimension &b) {
    return b.resolve<RType>(src);
}
*/

struct DimensionMinMax : public ValueMinMaxT<Dimension> {
    constexpr DimensionMinMax(const Dimension &value, const Dimension &min, const Dimension &max):ValueMinMaxT(value,min,max){}
    constexpr DimensionMinMax(const Dimension &value):DimensionMinMax(value, NONEDIM, NONEDIM) {}
    constexpr DimensionMinMax(const uu &value, const Unit &unit):DimensionMinMax(Dimension{value,unit}){}
    constexpr DimensionMinMax():DimensionMinMax(NONEDIM){}

    template<typename IType = Dimension, typename RType = Dimension>
    constexpr RType getImplicitValue(const IType &value) const {
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
    constexpr RType getExplicitValue() const {
        return getImplicitValue<RType>(value);
    }

    template<typename IType = Dimension, typename RType = Dimension>
    constexpr RType getComparedValue(const IType &value_candidate) const {
        return getImplicitValue<RType>(value_candidate > value ? value_candidate : value);
    }

    template<typename IType = Dimension, typename RType = DimensionMinMax>
    constexpr RType getCompared(const IType &value_candidate) const {
        return RType(
            getComparedValue(value_candidate),
            min,
            max
        );
    }

    template<typename IType = Dimension, typename RType = DimensionMinMax>
    constexpr RType resolve(const IType &src) const {
        const RType resolved(
            UI::resolve<IType>(src, value),
            UI::resolve<IType>(src, min),
            UI::resolve<IType>(src, max)
        );

        return resolved;
    }
};

struct StyleInfo { 
    Align align{LEFT};
    Align text_align{LEFT};
    Display display{BLOCK};
    Position position{STATIC};
    DimensionMinMax width, height;
    Box margin, padding;
    OverflowT overflow;

    Length content, used;
    LengthD computed, container;
    DimensionMinMax resolved_width, resolved_height;
};

struct Style : public Size, public StyleInfo {
    using StyleInfo::width;
    using StyleInfo::height;

    constexpr Style(){}
    //Style(){}
};

struct Event {
    enum Type : uint8_t {
        TYPE_NONE,
        BUFFER,
        CLEAR,
        LOG,
        USER_INPUT,
        LOAD,
        RESET,
        VISIBILITY,
        LAYOUT,
        FONT,
        DRAW,
        TICK,
        CONTENT_SIZE,
        FRAME,
        DISPLAY,
        FOCUS,
        SCREEN,
    };

    enum Value : uint8_t {
        VALUE_NONE,
        DPAD_LEFT,
        DPAD_RIGHT,
        DPAD_UP,
        DPAD_DOWN,
        DPAD_ENTER,
        DISPLAY_ON,
        DISPLAY_OFF,
        DISPLAY_LOCKED,
        DISPLAY_UNLOCKED,
        VISIBLE,
        HIDDEN,
        FOCUS_LOST,
        FOCUS_GAIN,
        FOCUS_NEXT,
        FOCUS_EMIT,
    };

    enum Direction : uint8_t {
        DIRECTION_NONE=0,
        CHILDREN=1,
        PARENT=2,
        BROADCAST=4,
        DIRECTION_ONLY=7,
        SELF_FIRST=8,
        SKIP_SELF=16,
    };

    enum State : uint8_t {
        NORMAL=0,
        STOP_PROPAGATION=1,
        STOP_IMMEDIATE_PROPAGATION=2,
        STOP_DEFAULT=4,
    };

    Type type;
    Value value;
    Direction direction;
    State state;

    constexpr Event(const Type &type, const Value &value, const Direction &direction, const State &state)
        :type(type),value(value),direction(direction),state(state){}
    constexpr Event(const Type &type, const Value &value)
        :Event(type, value, CHILDREN, NORMAL) {}
    constexpr Event(const Type &type)
        :Event(type, VALUE_NONE) {}

    constexpr inline bool isSelfFirst() const {
        return direction & SELF_FIRST;
    }

    constexpr inline bool isSkipSelf() const {
        return direction & SKIP_SELF;
    }

    constexpr inline bool isStopping() const {
        return state & (STOP_PROPAGATION | STOP_IMMEDIATE_PROPAGATION);
    }

    constexpr inline bool isStopPropagation() const {
        return state & STOP_PROPAGATION;
    }

    constexpr inline bool isStopImmediate() const {
        return state & STOP_IMMEDIATE_PROPAGATION;
    }

    constexpr inline bool isStopDefault() const {
        return state & STOP_DEFAULT;
    }

    constexpr inline void resetPropagation() {
        state = State(state & ~(STOP_PROPAGATION | STOP_IMMEDIATE_PROPAGATION));
    }

    constexpr inline void stopPropagation() {
        state = State(state | STOP_PROPAGATION);
    }

    constexpr inline void stopImmediate() {
        state = State(state | STOP_IMMEDIATE_PROPAGATION);
    }

    constexpr inline void stopDefault() {
        state = State(state | STOP_DEFAULT);
    }
};

struct Element : public Style {
    const char *name;

    Element *parent;
    Element *sibling;
    Element *child;

    TextureT<DisplayBuffer> *buffer;

    virtual void handle_event(Event *event) {
        switch (event->type) {
            case Event::CLEAR:
                /*
                if (buffer)
                    buffer->fill()
                */
                break;
            default:
                break;
        }
    }

    constexpr inline void dispatch_event(Event *event) {
        const bool skipSelf = event->isSkipSelf();

        if (skipSelf)
            event->direction = Event::Direction(event->direction & ~(Event::SKIP_SELF));

        if (event->isStopping())
            return;

        if (event->isSelfFirst() && !skipSelf)
            this->handle_event(event);

        switch (event->direction & Event::DIRECTION_ONLY) {
            case Event::PARENT:
                if (parent)
                    parent->dispatch_event(event);
                break;
            case Event::CHILDREN:
                for (Element *cur = child; cur != nullptr; cur = cur->sibling) {
                    cur->dispatch_event(event);

                    if (event->isStopImmediate())
                        return;

                    event->resetPropagation();
                }
                break;
            case Event::BROADCAST: {
                Event::Direction copy = Event::Direction(Event::SKIP_SELF | (event->direction & Event::SELF_FIRST));
                event->direction = Event::Direction(copy | Event::CHILDREN);
                dispatch_event(event);
                event->direction = Event::Direction(copy | Event::PARENT);
                dispatch_event(event);
                break;
            }
            default:
                break;
        }

        if (!event->isSelfFirst() && !skipSelf)
            this->handle_event(event);
    }

    constexpr Element(const char *name, Element *parent, Element *sibling, Element *child, TextureT<DisplayBuffer> *buffer):name(name),parent(parent),sibling(sibling),child(child),buffer(buffer){}
    constexpr Element(const char *name):Element(name,nullptr,nullptr,nullptr,nullptr){}
    constexpr Element():Element(nullptr){}

    constexpr inline Element &operator<<(const Origin &origin) {
        *((Origin*)this) = origin;
        return *this;
    }

    constexpr inline Element &operator<<(const Length &length) {
        *((Length*)this) = length;
        return *this;
    }

    constexpr inline Element &operator<<(const Style &style) {
        *((Style*)this) = style;
        return *this;
    }

    constexpr inline Element &operator<<(const StyleInfo &style) {
        *((StyleInfo*)this) = style;
        return *this;
    }

    constexpr inline Element *append_sibling(Element *element) {
        assert(element && "Element null");

        if (sibling)
            element->append_sibling(sibling);
            
        sibling = element;
        sibling->parent = parent;

        return element;
    }

    constexpr inline Element &append_sibling(Element &element) {
        return *append_sibling(&element);
    }

    constexpr inline Element *append_child(Element *element) {
        assert(element && "Element null");
        
        element->parent = this;

        if (child)
            child->append_sibling(element);
        else
            child = element;

        return element;
    }

    constexpr inline Element &append_child(Element &element) {
        return *append_child(&element);
    }

    /*
        when units are percent, they are of the container's size with margins

        I'd like computed size to be the container size, otherwise more math is done
    */

    constexpr void resolve_relative_container_sizes() {
        if (!parent) {
            container = {width.getExplicitValue(), height.getExplicitValue()};
            resolved_width = width;
            resolved_height = height;
        } else {
            const LengthD &parent_container = parent->container;

            DimensionMinMax rel_w = width.resolve(parent_container.width);
            DimensionMinMax rel_h = height.resolve(parent_container.height);

            computed = {
                rel_w.value,
                rel_h.value
            };

            container = {
                rel_w.getComparedValue(content.width),
                rel_h.getComparedValue(content.height)
            };

            resolved_width = rel_w.getCompared(content.width);
            resolved_height = rel_h.getCompared(content.height);
        }

        if (child)
            child->resolve_relative_container_sizes();

        if (sibling)
            sibling->resolve_relative_container_sizes();
    }

    struct FlowContext {
        uu inline_width, inline_height, block_width, block_height;

        constexpr FlowContext(const uu &iw, const uu &ih, const uu &bw, const uu &bh):
            inline_width(iw),inline_height(ih),block_width(bw),block_height(bh) {}
        constexpr FlowContext():FlowContext(0,0,0,0){}

        constexpr inline operator Length() const {
            return Length(
                block_width,
                block_height + inline_height
            );
        }

        constexpr void append(const Element &element) {
            if (!element.parent)
                return;

            const LengthD &parent_container = element.parent->container;
            Box resolved = 
                element.margin.resolve(parent_container) +
                element.padding.resolve(parent_container);

            LengthD total = (LengthD)resolved;

            total += element.container;

            Length real = (Length)total;

            switch (element.display) {
                case BLOCK:
                    block_height += inline_height;
                    inline_height = 0;
                    inline_width = 0;
                    block_height += real.height;
                    if (real.width > block_width)
                        block_width = real.width;
                    break;
                case INLINE:
                case INLINE_BLOCK:
                    if (real.height > inline_height)
                        inline_height = real.height;
                    inline_width += real.width;
                    if (block_width < inline_width)
                        block_width = inline_width;
                    break;
                default:
                    break;
            }
        }
    };

    constexpr void resolve_container_growth(FlowContext &parent_context) {
        FlowContext context;

        if (child)
            child->resolve_container_growth(context);
        
        const Length grow = context;

        /*
        if (grow.width > container.width)
            container.width = grow.width;
        if (grow.height > container.height)
            container.height = grow.height;
        */
        container.width = resolved_width.getComparedValue(grow.width);
        container.height = resolved_height.getComparedValue(grow.height);

        parent_context.append(*this);
        
        if (sibling)
            sibling->resolve_container_growth(parent_context);
    }

    constexpr void resolve_container_position() {
        Element *cur = child;

        *this << container;
        const Size original_size = *this;
        const Origin original_origin = *this;
        Length inline_size;
        Origin offset = original_origin;

        while (cur) {
            Box child_margin = cur->margin.resolve(container);
            Box child_padding = cur->padding.resolve(container);
            Box child_box = child_margin + child_padding;
            Length child_use = child_box + cur->container;

            switch (cur->display) {
                case BLOCK:
                    //offset += {0,inline_size.height};
                    //offset.x = original_origin.x;
                    offset.x = original_origin.x;
                    offset.y += inline_size.height;
                    inline_size = 0;
                    break;
                case INLINE:
                case INLINE_BLOCK:
                    break;
                default:
                    break;
            }

            printf("%s -> %s\n", name, cur->name);

            Origin child_offset = (Origin)child_margin + offset;

            printf("x: %i y: %i x: %i y: %i\n", offset.x, offset.y, child_offset.x, child_offset.y);
            *cur << child_offset;
            //*cur << offset;
            //*cur << Origin(1,1);

            switch (cur->display) {
                case BLOCK:
                    //offset += {0,child_use.height};
                    //offset.x = original_origin.x;
                    offset.y += child_use.height;
                    offset.x = original_origin.x;
                    inline_size = 0;
                    break;
                case INLINE:
                case INLINE_BLOCK:
                    inline_size += {child_use.width,0};
                    if (child_use.height > inline_size.height)
                        inline_size.height = child_use.height;
                    break;
                default:
                    break;
            }

            cur = cur->sibling;
        }

        if (child)
            child->resolve_container_position();
        if (sibling)
            sibling->resolve_container_position();
    }
};

}
}
