#pragma once

#include <inttypes.h>
#include <cassert>
#include <type_traits>
#include <stdio.h>
#include <vector>
#include <time.h>
#include <string>
#include <iostream>

#include "texture.h"
#include "displaybuffer.h"
#include "types.h"
#include "sizes.h"
#include "sprites.h"
#include "node_iterator.h"
#include "wbl_func.h"
#include "config.h"
#include "display_timeout.h"

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
    BLOCK=1,
    INLINE=2,
    INLINE_BLOCK=4,
    NONE=8, // Do not include in layout
    HIDE=16, // Include in layout (use internally)
};

enum Align : uint8_t {
    LEFT=1,
    TOP=2,
    RIGHT=4,
    BOTTOM=8,
    VCENTER=16,
    HCENTER=32,
    CENTER=48,
    BOTTOM_RIGHT=12,
    BOTTOM_LEFT=9,
    TOP_RIGHT=6,
    TOP_LEFT=3,
    VCENTER_RIGHT=20,
    VCENTER_LEFT=17,
    HCENTER_TOP=34,
    HCENTER_BOTTOM=40,
};

enum Position : uint8_t {
    STATIC=1,
    ABSOLUTE=2,
};

enum Overflow : uint8_t {
    AUTO=1,
    HIDDEN=2,
    SCROLL=4,
};

enum WrapStyle : uint8_t {
    NOWRAP = 1,
    WRAP = 2,
    TRIM_SPACE = 4,
};

template<typename T>
struct AxisT {
    typedef T value_type;

    T x, y;

    constexpr AxisT(){}
    constexpr AxisT(const T &x, const T &y):x(x),y(y){}
    constexpr AxisT(const T &v):x(v),y(v){}
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
    WrapStyle wrap{WRAP};
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
    using StyleInfo::StyleInfo;
    using StyleInfo::width;
    using StyleInfo::height;

    constexpr Style(){}
    constexpr Style(const StyleInfo &style):StyleInfo(style){}
    //Style(){}

    struct ContentSize {
        bool wrap, clip_x, clip_y;
        Length inline_length, block_length, boundary;

        constexpr ContentSize()
            :wrap(false),clip_x(false),clip_y(true) { }
        constexpr ContentSize(const Length &boundary, const bool &wrap=false, const bool &clip_x=false, const bool &clip_y=true)
            :wrap(wrap),clip_x(clip_x),clip_y(clip_y),boundary(boundary) { }

        constexpr inline Length getContentSize() const {
            return Length(
                block_length.width,
                block_length.height + inline_length.height
            );
        }

        constexpr inline bool add_break() {
            if (inline_length.width > block_length.width)
                block_length.width = inline_length.width;

            inline_length.width = 0;

            if (clip_y && block_length.height + inline_length.height > boundary.height)
                return false;

            block_length.height += inline_length.height;
            inline_length.height = 0;

            return true;
        }

        /*
            @brief Add length to content size

            @returns true if added to content size, false if clipped        
        */
        constexpr inline bool add_length(const Length &length) {
            const auto width = length.getWidth();
            const auto height = length.getHeight();

            //fprintf(stderr, "add_length: %iw %ih\n", width, height);

            if (width + inline_length.width > boundary.width) {
                if (wrap) {
                    block_length.height += inline_length.height;
                    inline_length = 0;
                } else
                if (clip_x) {
                    return false;
                }
            }

            if (clip_y && height + block_length.height > boundary.height)
                return false;

            if (height > inline_length.height)
                inline_length.height = height;

            inline_length.width += width;

            if (inline_length.width > block_length.width)
                block_length.width = inline_length.width;

            return true;            
        }

        template<typename Sprite>
        constexpr inline bool add_sprite(const Sprite &sprite) {
            return add_length(sprite);
        }

        template<typename FontSprite>
        constexpr inline bool add_glyph(const FontSprite &sprite) {
            const Length sprite_size(
                sprite.font_width + sprite.advance_x,
                sprite.font_height + sprite.advance_y  
            );
            return add_length(sprite_size);
        }
    };

    constexpr inline ContentSize getContentSizeProvider(const Length &boundary) {
        return ContentSize(
            boundary,
            this->wrap & WrapStyle::WRAP,
            this->overflow.x & Overflow::HIDDEN,
            this->overflow.y & Overflow::HIDDEN
        );
    }

    template<typename FontProvider>
    constexpr inline Length getTextContentSize(const char *text, const Length &boundary, const FontProvider &font = Sprites::font) {
        const bool text_trim = this->wrap & WrapStyle::TRIM_SPACE;

        ContentSize content = this->getContentSizeProvider(boundary);
        const uu text_length = strlen(text);

        for (uu i = 0; i < text_length; i++) {
            const char character = text[i];

            if (text_trim && (content.inline_length.width == 0)) {
                switch (character) {
                    case ' ':
                    case '\t':
                    case '\n':
                        continue;
                    default: break;
                }
            }

            if (character == '\n') {
                content.add_break();
                continue;
            }

            auto sprite = font.getCharacter(character);

            //content.add_sprite(sprite);
            content.add_glyph(sprite);
        }

        return content.getContentSize();
    }

    /*
        @brief Call after layout, it depends on computed layout size    
    */
    template<typename FontProvider>
    constexpr inline Length getTextContentSize(const char *text, const FontProvider &font = Sprites::font) {
        Length boundary = *this;
        return this->getTextContentSize(text, boundary, font);
    }

    template<typename Sprite>
    constexpr inline Length getSpritesContentSize(const Sprite *sprites, const uu &length) {
        Length boundary = *this;
        ContentSize content = this->getContentSizeProvider(boundary);

        for (uu i = 0; i < length; i++) {
            content.add_sprite(sprites[i]);
        }

        return content.getContentSize();
    }
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
        VALUE_NONE = 0,
        DPAD_LEFT = 1,
        DPAD_RIGHT = 2,
        DPAD_UP = 4,
        DPAD_DOWN = 8,
        DPAD_ENTER = 16,
        DISPLAY_ON = 1,
        DISPLAY_OFF = 2,
        DISPLAY_LOCKED = 4,
        DISPLAY_UNLOCKED = 8,
        VISIBLE = 1,
        HIDDEN = 2,
        FOCUS_LOST = 1,
        FOCUS_GAIN = 2,
        FOCUS_NEXT = 4,
        FOCUS_EMIT = 8,
        CHANGE = 32,
        REQUEST = 16,
        PRESSED = 32,
        HELD = 64,
        RELEASED = 128,
        NEXT = 4,
        PREVIOUS = 8,
        REDRAW = 1,
        FORCE = 2,
    };

    enum Direction : uint8_t {
        DIRECTION_NONE=0,
        CHILDREN=1,
        PARENT=2,
        BROADCAST=4,
        DIRECTION_ONLY=7,
        SELF_FIRST=8,
        SKIP_SELF=16,
        RDEPTH=9, // SELF_FIRST | CHILDREN
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

    #pragma region Event Stuff
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

    #ifdef USE_EVENT_DBG

    std::string get_value_str() {
        if (!value)
            return "NONE";

        switch (type) {
            case USER_INPUT: {
                std::string ret;
                switch (value & 31) {
                    case DPAD_DOWN: ret = "DPAD_DOWN"; break;
                    case DPAD_ENTER: ret = "DPAD_ENTER"; break;
                    case DPAD_LEFT: ret = "DPAD_LEFT"; break;
                    case DPAD_RIGHT: ret = "DPAD_RIGHT"; break;
                    case DPAD_UP: ret = "DPAD_UP"; break;
                    default: ret = "UNKOWN"; break;
                }
                if (value & PRESSED)
                    ret += " PRESSED";
                if (value & RELEASED)
                    ret += " RELEASED";
                if (value & HELD)
                    ret += " HELD";
                return ret;
                }
            case DISPLAY:
                {
                    switch (value & 15) {
                        case DISPLAY_ON: return "ON";
                        case DISPLAY_LOCKED: return "LOCKED";
                        case DISPLAY_UNLOCKED: return "UNLOCKED";
                        case DISPLAY_OFF: return "OFF";
                        default: break;
                    }
                }
            case SCREEN:
            case VISIBILITY:
                {
                    switch (value & 3) {
                        case VISIBLE: return "VISIBLE";
                        case HIDDEN: return "HIDDEN";
                        default: break;
                    }
                }
            case DRAW: return "REDRAW";
            case LAYOUT:
            case CONTENT_SIZE:
                {
                    switch (value) {
                        case REQUEST: return "REQUEST";
                        case CHANGE: return "CHANGE";
                        default: break;
                    }
                }
        }

        return std::to_string(value);
    }

    std::string get_type_str() {
        const char *s_types[] = {
            "NONE", "BUFFER", "CLEAR", "LOG", "USER_INPUT", "LOAD", "RESET", "VISIBILITY", "LAYOUT", "FONT",
            "DRAW", "TICK", "CONTENT_SIZE", "FRAME", "DISPLAY", "FOCUS", "SCREEN"
        };

        if (type >= (sizeof(s_types)/sizeof(s_types[0])))
            return "UNKNOWN";
        return std::string(s_types[type]);
    }

    std::string get_direction_str() {
        switch (direction) {
            case 0: return "NONE";
            case 1: return "CHILDREN";
            case 2: return "PARENT";
            case 4: return "BROADCAST";
            case 8: return "SELF_FIRST";
            case 9: return "RDEPTH";
            case 16: return "SKIP_SELF";
            default: return std::to_string(direction);
        }
    }

    std::string get_state_str() {
        switch (state) {
            case 0: return "NORMAL";
            case 1: return "STOP_PROPAGATION";
            case 2: return "STOP_IMMEDIATE";
            case 3: return "STOP_IMMEDIATE|STOP_PROPAGATION";
            case 4: return "STOP_DEFAULT";
            case 5: return "STOP_DEFAULT|STOP_PROPAGATION";
            case 6: return "STOP_DEFAULT|STOP_IMMEDIATE";
            case 7: return "STOP_ALL";
            default: return std::to_string(state);
        }
    }

    std::string to_string() {
        return get_type_str() + " " + get_value_str() + " " + get_direction_str() + " " + get_state_str();
    }

    #endif

    #pragma endregion
};

using EventTypes = Event::Type;
using EventValues = Event::Value;
using EventDirection = Event::Direction;
using EventState = Event::State;

struct IElement : public Style, public NodeMovementOpsT<IElement> {
    const char *name;

    IElement *parent;
    IElement *sibling;
    IElement *child;

    virtual void handle_event(Event *event) { }

    constexpr inline void handle_event_log(Event *event) {
        #ifdef USE_EVENT_DBG
        if (event->type != Event::TICK)
            std::cerr << (name ? name : "null") << ":" << event->to_string() << std::endl;
        #endif
        this->handle_event(event);
    }

    constexpr inline void dispatch_event(Event *event) {
        const bool skipSelf = event->isSkipSelf();

        if (skipSelf)
            event->direction = Event::Direction(event->direction & ~(Event::SKIP_SELF));

        if (event->isStopping())
            return;

        if (event->isSelfFirst() && !skipSelf)
            this->handle_event_log(event);

        switch (event->direction & Event::DIRECTION_ONLY) {
            case Event::PARENT:
                if (parent)
                    parent->dispatch_event(event);
                break;
            case Event::CHILDREN:
                for (IElement *cur = child; cur != nullptr; cur = cur->sibling) {
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
            this->handle_event_log(event);
    }

    constexpr inline void dispatch_event(Event &event) {
        this->dispatch_event(&event);
    }

    constexpr inline void dispatch(const EventTypes &event_type, const EventValues &event_value, const EventDirection &event_direction) {
        Event event(event_type, event_value, event_direction, EventState::NORMAL);
        this->dispatch_event(&event);
    }

    constexpr inline void dispatch(const EventTypes &event_type) {
        this->dispatch(event_type, EventValues::VALUE_NONE, (EventDirection)(EventDirection::BROADCAST|EventDirection::SELF_FIRST|EventDirection::SKIP_SELF));
    }

    constexpr inline void dispatch_parent(const EventTypes &event_type, const EventValues &event_value = EventValues::VALUE_NONE) {
        dispatch(event_type, event_value, (EventDirection)(EventDirection::PARENT|EventDirection::SKIP_SELF));
    }

    constexpr IElement(const char *name, IElement *parent, IElement *sibling, IElement *child):name(name),parent(parent),sibling(sibling),child(child){}
    constexpr IElement(const char *name):IElement(name,nullptr,nullptr,nullptr){}
    constexpr IElement():IElement(""){}
    constexpr IElement(const StyleInfo &styleInfo):Style(styleInfo),name(""),parent(nullptr),sibling(nullptr),child(nullptr){}

    constexpr inline IElement &operator<<(const char *str) {
        name = str ? str : "";
        return *this;
    }

    constexpr inline IElement &operator<<(const Origin &origin) {
        *((Origin*)this) = origin;
        return *this;
    }

    constexpr inline IElement &operator<<(const Length &length) {
        *((Length*)this) = length;
        return *this;
    }

    constexpr inline IElement &operator<<(const Style &style) {
        *((Style*)this) = style;
        return *this;
    }

    constexpr inline IElement &operator<<(const Size &size) {
        *((Size*)this) = size;
        return *this;
    }

    constexpr inline IElement &operator<<(const StyleInfo &style) {
        *((StyleInfo*)this) = style;
        return *this;
    }

    constexpr inline IElement &operator<<(IElement &element) {
        return append_child(element);
    }

    constexpr inline IElement *append_sibling(IElement *element) {
        assert(element && "Element null");

        if (sibling)
            element->append_sibling(sibling);
            
        sibling = element;
        sibling->parent = parent;

        return element;
    }

    constexpr inline IElement &append_sibling(IElement &element) {
        return *append_sibling(&element);
    }

    constexpr inline IElement *remove_child(IElement *element) {
        if (!element) {
            assert("Element is null");
            return nullptr;
        }

        if (!has_child(element)) {
            assert("Node does not have child to remove");
            return nullptr;
        }

        // Reconnect the sibling chain
        auto *prev = element->previous_sibling();

        if (prev) {
            prev->sibling = element->sibling;
            assert(prev != element && "Trying to set same");
        } else {
            child = element->sibling;
        }

        // Remove element from our tree
        element->parent = nullptr;
        element->sibling = nullptr;

        return element;
    }

    constexpr inline IElement *insert_before(IElement *to_insert, IElement *before_child) {
        if (!to_insert) {
            assert("to_insert is null");
            return nullptr;
        }

        if (!before_child)
            return append_child(to_insert);

        if (!has_child(before_child)) {
            assert("before is not a child");
            return nullptr;
        }

        auto *prev = before_child->previous_sibling();

        if (prev) {
            prev->sibling = to_insert;
        } else {
            child = to_insert;
        }

        to_insert->sibling = before_child;
        to_insert->parent = this;

        return to_insert;
    }

    constexpr inline IElement &insert_before(IElement &to_insert, IElement &before_child) {
        return *insert_before(&to_insert, &before_child);
    }

    constexpr inline IElement *append_child(IElement *element) {
        assert(element && "Element null");
        
        element->parent = this;

        if (child)
            this->last_child()->sibling = element;
        else
            child = element;

        return element;
    }

    constexpr inline IElement &append_child(IElement &element) {
        return *append_child(&element);
    }

    constexpr inline void set_content_size(const Length &size) {
        if (this->content != size) {
            this->content = size;
            this->dispatch_parent(Event::CONTENT_SIZE, Event::CHANGE);
        }
    }

    /*
        Does not hide, only propagates the correct event
    */
    constexpr inline void set_hidden(const bool &hidden=true) {
        this->dispatch(EventTypes::VISIBILITY, hidden ? EventValues::HIDDEN : EventValues::VISIBLE, EventDirection::CHILDREN);
    }

    /*
        when units are percent, they are of the container's size with margins

        I'd like computed size to be the container size, otherwise more math is done
    */

    constexpr void resolve_relative_container_sizes() {
        if (!parent) {
            //container = {width.getExplicitValue(), height.getExplicitValue()};
            container = {
                width.getComparedValue(content.width),
                height.getComparedValue(content.height)
            };
            resolved_width = width.getCompared(content.width);
            resolved_height = height.getCompared(content.height);
        } else {
            const LengthD &parent_container = parent->container;

            DimensionMinMax rel_w = width.resolve(parent_container.width);
            DimensionMinMax rel_h = height.resolve(parent_container.height);

            if ((display & BLOCK) && (position & STATIC) && rel_w.value.value == 0)
                rel_w.value = parent_container.width;

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

        //fprintf(stderr, "container_sizes: %s\n", name ? name : "null");
        //fprintf(stderr, "  container: %iw %ih\n", container.width.value, container.height.value);
        //fprintf(stderr, "    content: %iw %ih\n", content.width, content.height);

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

        constexpr void append_break() {
            block_height += inline_height;
            inline_height = 0;
            inline_width = 0;
        }

        constexpr void append_block(const IElement &element, const Length &real) {
            append_break();
            block_height += real.height;
            if (real.width > block_width)
                block_width = real.width;
        }

        constexpr void append_inline(const IElement &element, const Length &real) {
            if (real.height > inline_height)
                inline_height = real.height;
            inline_width += real.width;
            if (block_width < inline_width)
                block_width = inline_width;
        }

        constexpr void append(const IElement &element) {
            if (!element.parent)
                return;

            const LengthD &parent_container = element.parent->container;
            Box resolved = 
                element.margin.resolve(parent_container) +
                element.padding.resolve(parent_container);

            LengthD total = LengthD(resolved.left+resolved.right,resolved.top+resolved.bottom);

            total += element.container;

            Length real = (Length)total;

            switch (element.display) {
                case BLOCK:
                    append_block(element, real);
                    break;
                case INLINE:
                case INLINE_BLOCK:
                    if (parent_container.width.resolve(0) && real.width + inline_width > parent_container.width)
                        append_break();
                    append_inline(element, real);
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
        
        Length grow = context;

        switch (display) {
            case BLOCK:
                if (position == STATIC) {
                    grow.width = (uu)resolved_width.getComparedValue(container.width);
                }
                break;
            default:
                break;
        }

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
        IElement *cur = child;

        *this << container;
        const Size original_size = *this;
        const Origin original_origin = *this;
        Length inline_size;
        //Origin offset = original_origin;
        Size remaining_size = original_size;

        /*
            Floated blocks become inline
        
            Floated blocks offset in accordance with tree order
            
            For simplicity, only inline will float (align) left or right

            Both will float (align) top or bottom

            Floating left or right should be okay when they are after block siblings, it will include the break
        */

        while (cur) {
            if (cur->display & Display::NONE) {
                cur = cur->sibling;
                continue;
            }

            Box child_margin = cur->margin.resolve(container);
            Box child_padding = cur->padding.resolve(container);
            Box child_box = child_margin + child_padding;
            Length child_boxuse((uu)(child_box.left+child_box.right),(uu)(child_box.top+child_box.bottom));

            switch (cur->display) {
                case BLOCK:
                    //offset.x = original_origin.x;
                    //offset.y += inline_size.height;
                    remaining_size.x = original_origin.x;
                    remaining_size.width = original_size.width;
                    remaining_size.y += inline_size.height;
                    remaining_size.height -= inline_size.height;
                    inline_size = 0;
                    if (cur->position == STATIC) {
                        if (cur->container.width == NONEDIM) {
                            cur->container.width = (uu)(container.width - child_box.getWidth());
                        }
                    }

                    break;
                case INLINE:
                case INLINE_BLOCK:
                    break;
                default:
                    break;
            }

            Length child_use = (LengthT<uu>)child_boxuse + (LengthT<uu>)cur->container;

            Origin child_offset;

            if (child_use.width > remaining_size.width) {
                if (inline_size.height > remaining_size.height) 
                    break;
                remaining_size.y += inline_size.height;
                remaining_size.height -= inline_size.height;
                remaining_size.x = original_origin.x;
                remaining_size.width = original_size.width;
                inline_size = 0;
            }

            child_offset = (Origin)child_box + remaining_size.getOffset();

            if (cur->align & LEFT) {
                remaining_size.x += child_use.width;
                remaining_size.width -= child_use.width;
            } else
            if (cur->align & RIGHT) {
                remaining_size.width -= child_use.width;
                child_offset.x = remaining_size.getRight() + (uu)child_box.getLeft();
            } else
            if (cur->align & HCENTER) {
                child_offset.x = remaining_size.getMidpoint().x - (((uu)cur->container.width) * 0.5);
            }

            if (cur->align & TOP) {

            } else
            if (cur->align & BOTTOM) {
                remaining_size.height -= child_use.height;
                child_offset.y = remaining_size.getBottom() + (uu)child_box.getTop();
            } else
            if (cur->align & VCENTER) {
                child_offset.y = remaining_size.getMidpoint().y - (((uu)cur->container.height) * 0.5);
            }

            //Origin child_offset = (Origin)child_margin + offset;

            *cur << child_offset;

            switch (cur->display) {
                case BLOCK:
                    ////offset.y += child_use.height;
                    ////offset.x = original_origin.x;
                    remaining_size.y += child_use.height;
                    remaining_size.height -= child_use.height;
                    //remaining_size.x = original_origin.x;
                    inline_size = 0;
                    remaining_size.x = original_size.x;
                    remaining_size.width = original_size.width;
                    break;
                case INLINE:
                case INLINE_BLOCK:
                    inline_size += {child_use.width,0};
                    if (child_use.height > inline_size.height)
                        inline_size.height = child_use.height;
                    ////offset.x += child_use.width;
                    //remaining_size.x += child_use.width;
                    //remaining_size.width -= child_use.width;
                    break;
                default:
                    break;
            }

            cur = cur->sibling;
        }

        if (child && !(display & Display::NONE))
            child->resolve_container_position();
        if (sibling)
            sibling->resolve_container_position();
    }

    constexpr inline void resolve_layout() {
        #ifdef USE_EVENT_DBG
        fprintf(stderr, "Run layout on %s\n", name ? name : "null");
        #endif
        resolve_relative_container_sizes();
        FlowContext root;
        resolve_container_growth(root);
        resolve_container_position();
    }
};

template<typename Buffer>
struct ElementT : public IElement {
    using IElement::IElement;
    using IElement::operator<<;

    Buffer &buffer;

    constexpr ElementT(const IElement &base, Buffer &buffer):IElement(base),buffer(buffer){}
    constexpr ElementT(Buffer &buffer, const char *name, IElement *parent, IElement *sibling, IElement *child):IElement(name,parent,sibling,child),buffer(buffer){}
    constexpr ElementT(Buffer &buffer, const char *name):ElementT(buffer, name, nullptr, nullptr, nullptr){}
    constexpr ElementT(Buffer &buffer):ElementT(buffer,nullptr){}
    constexpr ElementT(Buffer &buffer, const StyleInfo &styleInfo):IElement(styleInfo),buffer(buffer){}

    constexpr inline ElementT &operator<<(Buffer &buffer) {
        this->buffer = buffer;
        return *this;
    }

    constexpr inline void clear(const pixel &px = 0) {
        this->buffer.fill(*this, px);
    }
};

template<typename Buffer, typename ElementT = ElementT<Buffer>>
struct ElementBaseT : public ElementT {
    using ElementT::ElementT;

    void handle_event(Event *event) override {
        switch (event->type) {
            case EventTypes::CLEAR: this->on_clear(event); return;
            case EventTypes::DRAW: this->on_draw(event); return;
            case EventTypes::FRAME: this->on_frame(event); return;
            case EventTypes::LAYOUT: this->on_layout(event); return;
            case EventTypes::USER_INPUT: this->on_user_input(event); return;
            case EventTypes::RESET: this->on_reset(event); return;
            case EventTypes::LOG: this->on_log(event); return;
            case EventTypes::LOAD: this->on_load(event); return;
            case EventTypes::VISIBILITY: this->on_visibility(event); return;
            case EventTypes::CONTENT_SIZE: this->on_content_size(event); return;
            case EventTypes::SCREEN: this->on_screen(event); return;
            case EventTypes::FOCUS: this->on_focus(event); return;
            case EventTypes::TICK: this->on_tick(event); return;
            default: return;
        }
    }

    virtual void on_clear(Event *event) {
        this->buffer.fill(*this, 0);
    }

    virtual void on_layout(Event *event) { }
    virtual void on_draw(Event *event) { }
    virtual void on_frame(Event *event) { }
    virtual void on_tick(Event *event) { }
    virtual void on_user_input(Event *event) { }
    virtual void on_reset(Event *event) { }
    virtual void on_log(Event *event) { }
    virtual void on_load(Event *event) { }
    virtual void on_visibility(Event *event) { }
    virtual void on_content_size(Event *event) { }
    virtual void on_screen(Event *event) { }
    virtual void on_focus(Event *event) { }

    template<typename FontProvider>
    constexpr inline Length draw_text(const char *text, const FontProvider &font, const Origin &offset_pos = {0,0}, const bool &determine_size = false, const bool &clear_sprite_area = false) {
        const bool text_wrap = this->wrap & WrapStyle::WRAP;
        const bool text_trim = this->wrap & WrapStyle::TRIM_SPACE;

        const uu length = strlen(text);
        const Origin pos = offset_pos + *this;
        Origin cur = pos;
        Length size;

        for (uu i = 0; i < length; i++) {
            const char character = text[i];

            if (text_trim && (cur.x == pos.x || i == 0)) {
                if (character == ' ')
                    continue;
                switch (character) {
                    case ' ':
                    case '\t':
                    case '\n':
                        continue;
                    default:
                        break;                    
                }
            }

            if (character == '\n') {
                cur.x = pos.x;
                cur.y += size.height;
                continue;
            }

            auto sprite = font.getCharacter(character);

            const Length glyph_size(
                sprite.font_width + sprite.advance_x,
                sprite.font_height + sprite.advance_y
            );

            if (cur.x + glyph_size.width > this->getRight()) {
                if (text_wrap) {
                    cur.x = pos.x;
                    cur.y += size.height;
                    size.height = 0;
                }
            }

            if (determine_size && (cur.x - pos.x) + glyph_size.width > size.width)
                size.width = (cur.x - pos.x) + glyph_size.width;

            if (cur.y + glyph_size.height > this->getBottom()) {
                break;
            }

            if (size.height < glyph_size.height)
                size.height = glyph_size.height;

            if (clear_sprite_area)
                this->buffer.fill(cur.x, cur.y, cur.x+glyph_size.width, cur.y+glyph_size.height, 0);

            if (!determine_size)
                this->buffer.putSprite(sprite, cur);

            cur.x += glyph_size.width;
        }

        if (determine_size)
            return Length(
                size.width, 
                (cur.y - pos.y) + size.height
            );
            
        return Length(cur.x - pos.x, cur.y - pos.y);
    }

    template<typename Sprite>
    constexpr inline Length draw_sprites(const Sprite *sprites, const uu &length, const Origin &offset_pos = {0,0}, const bool &determine_size = false, const bool &clear_sprite_area = false) {
        const bool wrap = this->wrap & WrapStyle::WRAP;

        const Origin pos = offset_pos + *this;
        Origin cur = pos;
        Length size;

        for (uu i = 0; i < length; i++) {
            const Sprite &sprite = sprites[i];

            if (cur.x + sprite.getWidth() > this->getRight()) {
                if (wrap) {
                    cur.x = pos.x;
                    cur.y += size.height;
                    size.height = 0;
                }
            }

            if (determine_size && (cur.x - pos.x) + sprite.getWidth() > size.width)
                size.width = (cur.x - pos.x) + sprite.getWidth();

            if (cur.y + sprite.getHeight() > this->getBottom())
                break;

            if (size.height < sprite.getHeight())
                size.height = sprite.getHeight();

            if (clear_sprite_area)
                this->buffer.fill(Size(cur, sprite), 0);

            if (!determine_size)
                this->buffer.putSprite(sprite, cur);

            cur.x += sprite.getWidth();
        }

        if (determine_size)
            return Length(
                size.width,
                (cur.y - pos.y) + size.height
            );
        
        return Length(cur.x - pos.x, cur.y - pos.y);
    }

    template<typename T, typename BASETYPE=std::decay_t<T>>
    constexpr inline Length draw_any(const T v, const Origin &offset_pos) {
        //if constexpr (std::is_same<const char*, T>::value) {
        if constexpr (std::is_same_v<char*, BASETYPE> || std::is_same_v<const char*, BASETYPE>) {
            return this->draw_text(v, Sprites::font, offset_pos);
        } else {
            return this->draw_sprites(&v, 1, offset_pos);
        }
    }

    template<typename T>
    constexpr inline Length draw_multi(const Origin &offset_pos, const T t) {
        return this->draw_any(t, offset_pos);
    }

    template<typename T, typename ...Ts>
    constexpr inline Length draw_multi(const Origin &offset_pos, const T t, const Ts ...args) {
        Origin offset = offset_pos;
        offset += (Origin)this->draw_any(t, offset_pos);
        return this->draw_multi(offset, args...);
    }
};

template<typename Buffer, typename Atlas, typename ElementT = ElementBaseT<Buffer>>
struct ElementInlineSpritesT : public ElementT {
    using Sprite = typename Atlas::Sprite;
    using ElementT::ElementT;
    using ElementT::operator<<;

    std::vector<Sprite> sprites;

    constexpr inline const Sprite &add_sprite(const Sprite &sprite) {
        sprites.push_back(sprite);
        //fprintf(stderr, "add sprite (%i) %p: %iw %ih %p\n", sprites.size(), sprites.data()[0].src, sprite.getWidth(), sprite.getHeight(), sprite.src);
        return sprite;
    }

    constexpr inline ElementInlineSpritesT &operator<<(const Sprite &sprite) {
        add_sprite(sprite);
        return *this;
    }

    void on_content_size(Event *event) override {
        this->content = this->getSpritesContentSize(sprites.data(), sprites.size());
    }

    void on_draw(Event *event) override {
        this->draw_sprites(sprites.data(), sprites.size());
    }
};

template<typename Buffer, typename FontProvider = Sprites::FontProvider, typename ElementT = ElementBaseT<Buffer>>
struct ElementInlineTextT : public ElementT {
    //using ElementT::ElementT;
    using ElementT::operator<<;

    const char *text;
    const FontProvider &font;
    bool text_modified = true;

    constexpr ElementInlineTextT(const char *text, const FontProvider &font):text(text),font(font){}
    constexpr ElementInlineTextT(const char *text):ElementInlineTextT(text, Sprites::font){}
    constexpr ElementInlineTextT():ElementInlineTextT(""){}
    constexpr ElementInlineTextT(Buffer &buffer, const FontProvider &font):ElementT(buffer, StyleInfo{.wrap{NOWRAP}}),text(""),font(font){}

    void on_content_size(Event *event) override {
        this->set_content_size(this->getTextContentSize(text, font));
    }

    void on_draw(Event *event) override {
        if (!event->value && !text_modified)
            return;
        text_modified = false;
        this->draw_text(text, font);
    }
};

template<typename Buffer, typename ElementT = ElementBaseT<Buffer>>
struct ScreenClockT : public ElementT {
    using ElementT::ElementT;
    using ElementT::operator<<;

    int64_t prev_draw_time = -1;
    bool use_milliseconds = false;

    //void on_clear(Event *event) override {}

    void on_draw(Event *event) override {
        using calc = float;

        const int64_t now = use_milliseconds ? millis() : seconds();    
    
        if (now == prev_draw_time)
            return;
    
        this->clear();

        prev_draw_time = now;

        const auto polar_to_rectilinear_coordinates 
            = [](const calc &radians, const calc &radius, const Origin &center = 0) {
                return Origin(
                    radius * cosf(radians) + calc(center.x) - 0.5,
                    radius * sinf(radians) + calc(center.y) - 0.5
                );
        };

        const auto polar_line
            = [this,&polar_to_rectilinear_coordinates](const calc &radians, const calc &radius, const calc &width, const pixel &px, const Origin &center) {
                const Origin end = polar_to_rectilinear_coordinates(radians, radius, center);
                this->buffer.template stroke_line<calc,calc,calc>(center.x, center.y, end.x, end.y, width, px);
        };

        const auto radius_polar_line
            = [this,&polar_to_rectilinear_coordinates](const calc &radians, const calc &radius_minor, const calc &radius_major, const calc &width, const pixel &px, const Origin &center) {
                const Origin start = polar_to_rectilinear_coordinates(radians, radius_minor, center);
                const Origin end = polar_to_rectilinear_coordinates(radians, radius_major, center);
                this->buffer.template stroke_line<calc,short,calc>(start.x, start.y, end.x, end.y, width, px);
                //this->buffer.template line<calc,calc>(start.x, start.y, end.x, end.y, px);
        };

        const auto tick_marks
            = [this,&radius_polar_line](const calc &num_ticks, const calc &radius_minor, const calc &radius_major, const calc &width, const pixel &px, const Origin &center) {
                constexpr const calc D_PI = M_PI * 2.0f;
                const calc tick_step = D_PI * (1/num_ticks);
                for (calc i = 0.001; i < D_PI-0.001; i+=tick_step)
                    radius_polar_line(i, radius_minor, radius_major, width, px, center);
        };

        const time_t milliseconds = use_milliseconds ? 1000 : 1;
        const time_t seconds = milliseconds * 60;
        const time_t minutes_per_hour = seconds * 60;
        const time_t hours_per_day = minutes_per_hour * 12;

        const time_t seconds_abs_now = now % seconds;
        const time_t minutes_abs_now = now % minutes_per_hour;
        const time_t hours_abs_now = now % hours_per_day;

        const calc seconds_rel_now = seconds_abs_now * calc(1.0f/seconds);
        const calc minutes_rel_now = minutes_abs_now * calc(1.0f/minutes_per_hour);
        const calc hours_rel_now = hours_abs_now * calc(1.0f/hours_per_day);

        const calc mult = M_PI * calc(2);
        const calc sub = M_PI * calc(0.5f);
        const calc seconds_rad_now = seconds_rel_now * mult - sub;
        const calc minutes_rad_now = minutes_rel_now * mult - sub;
        const calc hours_rad_now = hours_rel_now * mult - sub;

        const fb min = this->getMinLength();
        const Origin mp = this->getMidpoint();

        const calc rd = min * calc(0.5);

        polar_line(seconds_rad_now, rd * 0.75f, 1.0f, 1, mp);
        polar_line(minutes_rad_now, rd * 0.8f, 1.5f, 1, mp);
        polar_line(hours_rad_now, rd * 0.9f, 1.7f, 1, mp);


        tick_marks(60, rd * 0.91f, rd, 1, 1, mp);
        tick_marks(12, rd * 0.87f, rd, 1.3, 1, mp);

        this->buffer.circle(mp.x, mp.y, 3, 1, true);
        this->buffer.circle(mp.x, mp.y, 1, 0, true);
    }
};

struct IScreen : public IElement {
    using IElement::IElement;
    using IElement::operator<<;

    const char* screen_name;
    IScreen *up, *right, *down, *left;

    constexpr IScreen(const char *screen_name, IScreen *up, IScreen *right, IScreen *down, IScreen *left):screen_name(screen_name),up(up),right(right),down(down),left(left){}
    constexpr IScreen(const char *screen_name):IScreen(screen_name, nullptr, nullptr, nullptr, nullptr){}
    constexpr IScreen():IScreen(nullptr){}

    constexpr inline IScreen *set_left(IScreen *screen) {
        this->left = screen;
        screen->right = this;
        return screen;
    }

    constexpr inline IScreen *set_right(IScreen *screen) {
        this->right = screen;
        screen->left = this;
        return screen;
    }

    constexpr inline IScreen *set_up(IScreen *screen) {
        this->up = screen;
        screen->down = this;
        return screen;
    }

    constexpr inline IScreen *set_down(IScreen *screen) {
        this->down = screen;
        screen->up = this;
        return screen;
    }

    constexpr inline IScreen &set_left(IScreen &screen) { return *set_left(&screen); }
    constexpr inline IScreen &set_right(IScreen &screen) { return *set_right(&screen); }
    constexpr inline IScreen &set_up(IScreen &screen) { return *set_up(&screen); }
    constexpr inline IScreen &set_down(IScreen &screen) { return *set_down(&screen); }
};

template<typename ScreenT = IScreen>
struct ScreenBaseT : public ScreenT {
    using ScreenT::ScreenT;
    using ScreenT::operator<<;

    bool show_header = true;
    bool dirty_layout = true;

    void handle_event(Event *event) override {
        ScreenT::handle_event(event);

        switch (event->type) {
            case EventTypes::SCREEN:
                if (event->value & EventValues::VISIBLE) {
                    this->dispatch(EventTypes::VISIBILITY, EventValues::VISIBLE, EventDirection::CHILDREN);
                    //this->dispatch(EventTypes::LAYOUT, Event::REQUEST, Event::PARENT);
                    //this->dispatch(EventTypes::CONTENT_SIZE, EventValues::REQUEST, EventDirection::RDEPTH);
                }
                if (event->value & EventValues::HIDDEN) {
                    this->dispatch(EventTypes::VISIBILITY, EventValues::HIDDEN, EventDirection::CHILDREN);
                }

                event->stopImmediate();
                break;
            case EventTypes::DRAW:
                if (!show_header)
                    event->stopDefault();
                break;
            case EventTypes::CONTENT_SIZE:
                if (!show_header) {
                    *this << Origin { 0, 0 };
                    //if (this->parent)
                    //    *this << *((Size*)this->parent);
                    //*this << Size { 0, 0, 128, 128 };
                    //this->width.value = {128};
                    if (this->parent) {
                        this->width = this->parent->width;
                        this->height = this->parent->height;
                    }
                    //this->height.value = {128};
                    //if (this->parent)
                    //    this->content = {
                    //        (uu)this->parent->width.getExplicitValue(),
                    //        (uu)this->parent->height.getExplicitValue()
                    //    };
                    //event->stopDefault();
                    if (event->value & EventValues::REQUEST)
                        event->value = EventValues::CHANGE;
                    this->resolve_layout();
                }
                break;
            default: break;
        }
    }
};

template<typename Buffer, typename ElementT = ElementBaseT<Buffer>>
struct ElementRootT : public ElementT {
    using ElementT::ElementT;
    using ElementT::operator<<;

    static constexpr const short debug_log_length = 512;
    bool debug = false;
    ub debug_details=0;
    int64_t utime = 0, ftime = 0;
    short debug_log_offset = 0;
    bool redraw_needed = false;
    char debug_log[debug_log_length];
    IScreen *active_screen = nullptr;
    IElement *header_element = nullptr;
    bool layout_dirty = true;

    template<typename FORMAT, typename ...Args>
    inline int log(FORMAT format, const Args&...args) {
        if (!debug || debug_log_offset > debug_log_length-20)
            return 0;
        
        int count;
        if constexpr (sizeof...(args) < 1) {
            count = snprintf(debug_log+debug_log_offset, debug_log_length-debug_log_offset, "%s", format);
        } else {
            count = snprintf(debug_log+debug_log_offset, debug_log_length-debug_log_offset, format, args...);
        }
        if (count > 0)
            debug_log_offset += count;

        return count;
    }

    template<typename ...Args>
    inline int log_time(const char *name, const Args&...args) {
        if (!debug) return 0;

        int64_t now = micros();
        log("%s:%5llius", name, now-utime);
        utime=now;

        if constexpr (sizeof...(args) > 1) {
            log(std::forward(args)...);
        } else {
            log("%s", "\n");
        }
        return 0;
    }

    inline void reset_log_time() {
        utime = micros();
    }

    inline void reset_log(const bool &reset_time=true) {
        debug_log_offset = 0;
        if (reset_time)
            reset_log_time();
    }

    inline void flush_log(const bool &display_flush = true, const bool &clear_text_boundary=true, const Origin &position={}, const bool &after_last_child=true) {
        if (!debug) return;

        Origin pos = position;

        Length len = this->getTextContentSize(debug_log, Sprites::minifont);

        if (after_last_child) {
            if (this->child && this->last_child()->deepest_child())
                pos = {0,this->last_child()->deepest_child()->getBottom()+2};
            if (pos.y + len.height + 2 > this->getBottom())
                pos.y = this->getBottom()-len.height;
        }

        if (clear_text_boundary) {
            this->buffer.fill({pos,len},0);
        }

        this->draw_text(debug_log, Sprites::minifont, pos);
        
        if (display_flush)
            this->buffer.flush();
    }

    inline void overlay_detailed(IElement *node) {
        const int buflen = 150;
        char buf[buflen];
        int offset=0;

        const auto s_dim = [&](const Dimension &v) {
            const char *ex;
            switch (v.unit) {
                case Unit::PERC: ex = "%"; break;
                default: ex = ""; break;
            }
            switch (v.unit) {
                case Unit::NONE:
                    offset += snprintf(buf+offset, buflen-offset, "N");
                    break;
                default:
                    offset += snprintf(buf+offset, buflen-offset, "%i%s", v.value, ex);
                    break;
            }
        };

        const auto s_s = [&](const char *v) {
            offset += snprintf(buf+offset, buflen-offset, "%s", v);
        };

        const auto s_i = [&](const uu &v) {
            offset += snprintf(buf+offset, buflen-offset, "%i", v);  
        };

        const auto s_dimminmax = [&](const DimensionMinMax &v) {
            s_s("v");
            s_dim(v.value);
            s_s("m");
            s_dim(v.min);
            s_s("M");
            s_dim(v.max);
        };

        const auto s_lengthd = [&](const LengthD &v) {
            s_s("w");
            s_dim(v.width);
            s_s("h");
            s_dim(v.height);
        };

        const auto s_length = [&](const Length &v) {
            s_s("w");
            s_i(v.width);
            s_s("h");
            s_i(v.height);
        };

        const auto s_size = [&](const Size &v) {
            s_i(v.x);
            s_s(",");
            s_i(v.y);
            s_s(" ");  
            s_i(v.width);
            s_s(",");
            s_i(v.height);
        };


        s_s(node->name?node->name:"");
        s_size(*node);

        s_s("\nctnr:");
        s_lengthd(node->container);
        s_s(",comp:");
        s_lengthd(node->computed);
        s_s(",used:");
        s_length(node->used);
        s_s("\nwid:");
        s_dimminmax(node->width);
        s_s(",hei:");
        s_dimminmax(node->height);
        s_s("\nrw:");
        s_dimminmax(node->resolved_width);
        s_s("rh:");
        s_dimminmax(node->resolved_height);
        s_s("\ncont:");
        s_length(node->content);

        this->draw_text(buf, Sprites::minifont, *node, false, true);
    }

    inline void overlay_tree_positions(const bool &detailed=false, const bool &do_not_flush=false) {
        for (auto &child : this->rdepthfirst()) {
            this->buffer.border(child, 1);
            if (detailed) {
                this->overlay_detailed(&child);
                continue;
            }

            const int buflen = 20;
            char buf[buflen];
            snprintf(buf, buflen, "%s\n%i,%i\n%i,%i", child.name?child.name:"", child.x, child.y, child.getWidth(), child.getHeight());
            this->draw_text(buf, Sprites::minifont, child, false, true);
        }
        if (!do_not_flush) {
            //if (debug)
            //    this->flush_log();
            //else
                this->buffer.flush();
        }
    }

    inline void once(const bool &do_not_flush=false) {
        if (displayTimeout.is_display_off())
            return;

        //reset_log(false);
        log_time("ELPSD");
        
        //reset_log_time();
        this->dispatch(Event::TICK);
        log_time("TICK ");

        bool dirty = layout_dirty;
        if (dirty) {
            this->dispatch(Event::CONTENT_SIZE, Event::REQUEST, Event::CHILDREN);
            this->clear();
        }
        log_time("CTSIZ");

        this->handle_deferred_event(Event(Event::DRAW, dirty || redraw_needed ? Event::REDRAW : Event::VALUE_NONE, Event::RDEPTH, Event::NORMAL));
        layout_dirty = false;
        redraw_needed = false;
        
        log_time("DRAW.");
        int64_t log_flush_time = 0;
        if (debug) {
            if (debug_details)
                this->overlay_tree_positions(debug_details==2, true);

            log_time("OVRLY");

            int64_t frametime = micros() - ftime;
            ftime = micros();
            log("TOTAL:%5llius\n", frametime);
            log("FPS  : %.1f\n", 1000000.0f/frametime);
            log_flush_time = micros();
            flush_log(false);
            log_flush_time = micros() - log_flush_time;
        }

        reset_log();

        log("LOGFS:%5llius\n", log_flush_time);
        reset_log_time();

        if (!do_not_flush)
            this->buffer.flush();
        log_time("FLUSH");
    }

    inline void setDebug(const bool &debug_state=true) {
        this->debug = debug_state;
    }

    void handle_deferred_event(const Event &event) {
        Event disp = event;
        if (active_screen)
            active_screen->dispatch_event(&disp);
        if (disp.isStopDefault())
            return;
        if (header_element)
            header_element->dispatch_event(&disp);
            
        this->handle_event(&disp);
    }

    void handle_event(Event *event) override {
        if (event->isStopDefault())
            return;

        ElementT::handle_event(event);    
    }

    void on_clear(Event *event) override {}

    void on_user_input(Event *event) override {
        if (!active_screen)
            return;

        #ifdef USE_LAYOUT_DBG
        if (event->value & EventValues::PRESSED) {
            if (event->value & EventValues::DPAD_UP)
                debug = !debug;
            if (event->value & EventValues::DPAD_DOWN)
                debug_details++;
            debug_details %= 3;
            layout_dirty = true;
        }
        #endif

        if (!(event->value & EventValues::RELEASED))
            return;

        if (event->value & EventValues::DPAD_UP)
            this->set_screen(active_screen->up);

        if (event->value & EventValues::DPAD_DOWN)
            this->set_screen(active_screen->down);

        if (event->value & EventValues::DPAD_LEFT)
            this->set_screen(active_screen->left);

        if (event->value & EventValues::DPAD_RIGHT)
            this->set_screen(active_screen->right);
    }

    void set_header(IElement *header) {
        if (header_element) {
            header_element->set_hidden(true);
            this->remove_child(header_element);
        }

        if (!header)
            return;

        this->insert_before(header, active_screen);

        header_element = header;

        header->set_hidden(false);
    }

    void set_header(IElement &header) {
        this->set_header(&header);
    }

    void set_screen(IScreen *screen) {
        if (!screen)
            return;

        if (active_screen) {
            active_screen->dispatch(EventTypes::SCREEN, EventValues::HIDDEN, EventDirection::RDEPTH);
            this->remove_child(active_screen);
        }

        this->clear();

        this->append_child(screen);

        this->active_screen = screen;

        this->resolve_layout();

        this->active_screen->dispatch(EventTypes::SCREEN, EventValues::VISIBLE, EventDirection::RDEPTH);

        layout_dirty = true;
        //redraw_needed = true;
    }

    void set_screen(IScreen &screen) {
        this->set_screen(&screen);
    }

    void on_layout(Event *event) override {
        if (event->value & EventValues::REQUEST)
            this->layout_dirty = true;
    }

    void on_content_size(Event *event) override {
        if (event->value & EventValues::CHANGE) {
            this->layout_dirty = true;
            return;
        }
        this->layout_dirty = false;
        this->resolve_layout();
    }
};

template<typename Buffer, typename ElementT = ElementBaseT<Buffer>>
struct ElementBatteryT : public ElementT {
    using ElementT::ElementT;
    using ElementT::operator<<;

    static constexpr const int buflen = 10;
    char buf[buflen];

    bool show_percentage = true;
    ub current_level = 50;
    ub last_draw_level = 101;
    Sprites::Atlas::Sprite battery_sprite = Sprites::BATTERY_5x10_PAD;

    void set_battery_level(const ub &level) {
        this->current_level = level;
    }

    inline void update() {
        snprintf(buf, buflen, "%i%% ", current_level);
    }

    void on_content_size(Event *event) override {
        this->update();
        Length size = this->getSpritesContentSize(&battery_sprite, 1) + this->getTextContentSize((const char*)buf, Sprites::font);
        size.height = 12;
        this->set_content_size(size);
    }

    //void on_clear(Event *event) override { }

    void on_draw(Event *event) override {
        if (last_draw_level == current_level && !(event->value & Event::REDRAW))
            return;
        last_draw_level = current_level;

        this->clear();
        this->update();
        Origin pos = *this;
        this->draw_multi({}, battery_sprite, (const char*)buf);
        const ub sheight = battery_sprite.getHeight();
        const ub swidth = battery_sprite.getWidth();
        int nheight = sheight*(current_level/100.0f);
        //this->buffer.fill(pos.x+1,pos.y+nheight+1,pos.x+swidth-1,pos.y+sheight-1,1);
        this->buffer.fill(pos.x+2,pos.y+sheight-nheight,pos.x+swidth-1,pos.y+sheight-1,1);
    }
};

template<typename Buffer, typename ElementT = ElementBaseT<Buffer>>
struct ElementDateTimeT : public ElementT {
    using ElementT::ElementT;
    using ElementT::operator<<;

    static constexpr const char *weekdays[] = {
        "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"
    };

    static constexpr const int bufsize = 6;
    char s_weekday[bufsize], s_month[bufsize], s_day[bufsize], s_hour[bufsize], s_min[bufsize];
    tm last_draw_tm = {.tm_mon=13};
    ub tick = 0;

    inline tm get_date() const {
        time_t now = time(nullptr);
        tm date = *localtime(&now);

        /*
        date.tm_mday = seconds() % 32;
        date.tm_hour = seconds() % 24;
        date.tm_min = seconds() % 60;
        date.tm_wday = seconds() % 7;
        date.tm_mon = seconds() % 12;
        */

        return date;
    }

    inline bool is_stale() const {
        tm now = this->get_date();
        const tm &last = this->last_draw_tm;
        
        return 
            now.tm_min != last.tm_min ||
            now.tm_hour != last.tm_hour ||
            now.tm_mon != last.tm_mon ||
            now.tm_mday != last.tm_mday ||
            now.tm_wday != last.tm_wday;
    }

    inline tm update() {
        tm date = this->get_date();

        snprintf(s_month, bufsize, "%i", date.tm_mon + 1);
        snprintf(s_day, bufsize, "%i ", date.tm_mday);
        snprintf(s_hour, bufsize, "%02i", date.tm_hour);
        snprintf(s_min, bufsize, "%02i", date.tm_min);
        snprintf(s_weekday, bufsize, " %s ", weekdays[date.tm_wday&7]);

        return date;
    }

    inline bool is_interval_tick() {
        //return (this->tick % 5) == 0;
        return (millis() % 500) <= 100;
    }

    inline bool is_major_tick() {
        //return this->tick > 4;
        return (millis() % 1000) >= 500;
    }

    void on_tick(Event *event) override {
        this->tick++;
        this->tick %= 10;
    }

    //void on_clear(Event *event) override {}

    void on_draw(Event *event) override {
        if (!this->is_stale() && !(event->value & Event::REDRAW)) {
            if (!this->is_interval_tick())
                return;
        } else {
            last_draw_tm = this->update();
            this->on_content_size(nullptr);
        }

        this->clear();

        this->draw_multi({}, s_weekday, s_month, Sprites::SLASH, s_day, s_hour, (this->is_major_tick()) ? Sprites::VERT_BAR : Sprites::VERT_BAR_NONE, s_min);
    }

    void on_content_size(Event *event) override {
        this->update();
        const char *texts[] = {
            s_weekday, s_month, s_day, s_hour, s_min
        };
        const Sprites::Atlas::Sprite sprites[] = {
            Sprites::SLASH, Sprites::VERT_BAR
        };

        Length sum;
        for (int i = 0; i < sizeof(texts)/sizeof(texts[0]); i++)
            sum += this->getTextContentSize((const char*)texts[i], Sprites::font);
        sum += this->getSpritesContentSize(&sprites[0], sizeof(sprites)/sizeof(sprites[0]));
        sum.width+=1;
        sum.height=12;
        this->set_content_size(sum);
    }
};

template<typename Buffer, typename ElementT = ElementBaseT<Buffer>>
struct ElementFocusT : public ElementT {
    using ElementT::ElementT;
    using ElementT::operator<<;

    bool focused = false;

    constexpr inline bool isFocused() const {
        return focused;
    }

    constexpr inline void setFocused(const bool &focus_state = true) {
        this->focused = focus_state;
    }

    void on_focus(Event *event) override {
        if (isFocused()) {
            if (event->value & EventValues::FOCUS_NEXT)
                setFocused(false);
        } else {
            if (event->value & EventValues::FOCUS_NEXT) {
                setFocused(true);
                event->stopImmediate();
            }
        }
    }

    void on_draw(Event *event) override {
        ElementT::on_draw(event);
        if (this->isFocused())
            this->buffer.border(*this, 1);
    }
};

template<typename Buffer, typename ElementT = ElementBaseT<Buffer>>
struct ElementLockIconT : public ElementT {
    using ElementT::ElementT;
    using ElementT::operator<<;

    bool prev_state = false;

    bool is_displaying() {
        return displayTimeout.is_display_locked();
    }

    void on_content_size(Event *event) override {
        Length size/*;
        if (is_displaying())
            size*/ = this->getSpritesContentSize(&Sprites::LOCK_ICON, 1);
        this->set_content_size(size);
    }

    void on_draw(Event *event) override {
        if (prev_state == is_displaying() && !(event->value & Event::REDRAW))
            return;

        if (prev_state != is_displaying())
            this->on_content_size(nullptr);

        prev_state = is_displaying();

        if (!is_displaying())
            return;

        this->clear();
        this->draw_sprites(&Sprites::LOCK_ICON, 1);
    }
};

}
}
