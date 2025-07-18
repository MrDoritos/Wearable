#include "ui.h"
#include <string>
#include <vector>
#include <iostream>

using namespace wbl;
using namespace UI;

using Buf = TextureT<StaticbufferT<1,1,1>>;
Buf disbuf;
using Element = ElementT<Buf>;

template<typename T>
std::string stringify(const T &obj) {
    return " " + std::to_string(obj);
}

template<>
std::string stringify<Unit>(const Unit &u) {
    auto e = [](const Unit &u) {
        switch (u) {
            case PX: return "PX";
            case PERC: return "%";
            case NONE: return "None";
            default: return "Undefined";
        }
    };
    return " Unit: " + std::string(e(u)) + " ";
}

template<>
std::string stringify<Dimension>(const Dimension &v) {
    return " Dim:" + stringify(v.value) + stringify(v.unit);
}

template<>
std::string stringify<LengthD>(const LengthD &v) {
    return " Length:\n   w:" + stringify(v.width) + "\n   h:" + stringify(v.height);
}

template<>
std::string stringify<Length>(const Length &v) {
    return " Length:\n   w:" + stringify(v.width) + "\n   h:" + stringify(v.height);
}

template<>
std::string stringify<Origin>(const Origin &v) {
    return " Origin:\n   x:" + stringify(v.x) + "\n   y:" + stringify(v.y);
}

template<>
std::string stringify<Size>(const Size &v) {
    return std::string(" Size:") +
            "\n   x:" + stringify(v.x) +
            "\n   y:" + stringify(v.y) +
            "\n   w:" + stringify(v.width) + 
            "\n   h:" + stringify(v.height);
}

template<>
std::string stringify<Box>(const Box &v) {
    return std::string(" Box:") +
            "\n   left:" + stringify(v.left) +
            "\n   top:" + stringify(v.top) +
            "\n   right:" + stringify(v.right) +
            "\n   bottom:" + stringify(v.bottom);
}

template<>
std::string stringify<AxisT<Dimension>>(const AxisT<Dimension> &v) {
    return std::string(" Axis:") +
            "\n   x:" + stringify(v.x) +
            "\n   y:" + stringify(v.y);
}

template<>
std::string stringify<AxisT<Overflow>>(const AxisT<Overflow> &v) {
    return std::string(" Axis:") +
            "\n   x:" + stringify((int&)v.x) +
            "\n   y:" + stringify((int&)v.y);
}

template<>
std::string stringify<ValueMinMaxT<Dimension>>(const ValueMinMaxT<Dimension> &v) {
    return std::string(" ValueMinMax:") +
            "\n  value:" + stringify(v.value) +
            "\n  min:" + stringify(v.min) + 
            "\n  max:" + stringify(v.max);
}

template<>
std::string stringify<DimensionMinMax>(const DimensionMinMax &v) {
    return std::string(" DimensionMinMax:") +
            "\n   value:" + stringify(v.value) +
            "\n   min:" + stringify(v.min) + 
            "\n   max:" + stringify(v.max);
}

template<>
std::string stringify<Style>(const Style &v) {
    return std::string("Style:") +
        "\n  width:" + stringify(v.width) +
        "\n  height:" + stringify(v.height) + 
        "\n  content:" + stringify(v.content) +
        "\n  used:" + stringify(v.used) +
        "\n  computed:" + stringify(v.computed) +
        "\n  margin:" + stringify(v.margin) +
        "\n  padding:" + stringify(v.padding) +
        "\n  overflow:" + stringify(v.overflow);
}

template<>
std::string stringify<IElement>(const IElement &v) {
    std::string ret = std::string("\nElement");

    if (v.name)
        ret += std::string(" (") + v.name + ")";
    ret += ":\n";

    ret += stringify((Style&)(v));

    if (v.child)
        ret += stringify(*v.child);
    
    if (v.sibling)
        ret += stringify(*v.sibling);

    return ret;
}

template<>
std::string stringify<Element>(const Element &e) {
    return stringify<IElement>(*(const IElement*)(&e));
}

std::string element_name(const Element &src, int levels = 0) {
    std::string ret = std::string(levels, ' ');
    ret += "Element";
    if (src.name)
        ret += std::string(" (") + src.name + ")";
    ret += ":";
    return ret;
}

std::string print_tree(const Element &src, int levels = 0) {
    std::string ret = element_name(src, levels) + "\n";
    if (src.child)
        ret += print_tree(*(Element*)src.child, levels + 2);
    if (src.sibling)
        ret += print_tree(*(Element*)src.sibling, levels);
    return ret;
}

enum Stage {
    INITIAL,
    CONTAINER_SIZES,
    CONTAINER_GROWTH,
    POSITION,
};

std::string print_layout_state(const Element &src, Stage stage, int levels=0) {
    std::string ret = "\n\n"+ element_name(src, levels);

    std::string app(levels, ' ');
    app = "\n" + app;

    switch (stage) {
        case CONTAINER_SIZES:
            ret += app + "CONTAINER_SIZES";
            ret += app + "container:" + stringify(src.container);
            ret += app + "content:" + stringify(src.content);
            ret += app + "computed:" + stringify(src.computed);
            break;
        case CONTAINER_GROWTH:
            ret += app + "CONTAINER_GROWTH";
            ret += app + "container:" + stringify(src.container);
            ret += app + "computed:" + stringify(src.computed);
            ret += app + "margin:" + stringify(src.margin);
            break;
        case POSITION:
            ret += app + "POSITION";
            ret += app + "container:" + stringify(src.container);
            ret += app + "to px:" + stringify<Length>(src.container);
            ret += app + "add:" + stringify<LengthD>(Box() + src.container);
            ret += app + stringify<Origin>(src);
            ret += app + stringify<Size>(src);
            break;
    }

    if (src.child)
        ret += print_layout_state(*(Element*)src.child, stage, levels + 2);
    if (src.sibling)
        ret += print_layout_state(*(Element*)src.sibling, stage, levels);

    return ret;
}

Element root(disbuf, "root");
Element screen(disbuf, "screen");
Element header(disbuf, "header");
Element footer(disbuf, "footer");
Element scrollable(disbuf, "scrollable");

int main() {
    root.append_child(header);
    root.append_child(screen);
    screen.append_sibling(footer);
    screen.append_child(scrollable);
/*
    root << StyleInfo {
        .width = { 20 },
        .height = { 20 },
    };

    header << StyleInfo {
        .width = { 5 },
        .height = { 5 },
    };

    screen << StyleInfo {
        .width = { 50, PERC },
        .height = { 50, PERC }
    };

    scrollable << StyleInfo {
        .width = { 30 }
    };

    footer << StyleInfo {
        .width = { 10 },
        .height = { 10 },
    };*/

    std::cout << stringify(root) << std::endl;    

    std::cout << print_tree(root) << std::endl;

    root.resolve_relative_container_sizes();

    std::cout << print_layout_state(root, CONTAINER_SIZES) << std::endl;

    Element::FlowContext c;
    root.resolve_container_growth(c);

    std::cout << print_layout_state(root, CONTAINER_GROWTH) << std::endl;

    root.resolve_container_position();

    std::cout << print_layout_state(root, POSITION) << std::endl;

    //auto r = screen.width.resolve(Dimension{20});

    //std::cout << stringify(r) << std::endl;
    //std::cout << stringify(r.getComparedValue(0)) << std::endl;
    //std::cout << stringify(r.getComparedValue(9)) << std::endl;
    //std::cout << stringify(r.getImplicitValue(0)) << std::endl;

    return 0;
}
