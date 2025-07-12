#include "ui.h"
#include <string>
#include <vector>
#include <iostream>

using namespace wbl;
using namespace UI;

template<typename T>
std::string stringify(T &obj) {
    return " " + std::to_string(obj);
}

template<>
std::string stringify<Unit>(Unit &u) {
    auto e = [](Unit &u) {
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
std::string stringify<Dimension>(Dimension &v) {
    return " Dim:" + stringify(v.value) + stringify(v.unit);
}

template<>
std::string stringify<LengthT<Dimension>>(LengthT<Dimension> &v) {
    return " Length:\n   w:" + stringify(v.width) + "\n   h:" + stringify(v.height);
}

template<>
std::string stringify<Length>(Length &v) {
    return " Length:\n   w:" + stringify(v.width) + "\n   h:" + stringify(v.height);
}

template<>
std::string stringify<Origin>(Origin &v) {
    return " Origin:\n   x:" + stringify(v.x) + "\n   y:" + stringify(v.y);
}

template<>
std::string stringify<Size>(Size &v) {
    return std::string(" Size:") +
            "\n   x:" + stringify(v.x) +
            "\n   y:" + stringify(v.y) +
            "\n   w:" + stringify(v.width) + 
            "\n   h:" + stringify(v.height);
}

template<>
std::string stringify<Box>(Box &v) {
    return std::string(" Box:") +
            "\n   left:" + stringify(v.left) +
            "\n   top:" + stringify(v.top) +
            "\n   right:" + stringify(v.right) +
            "\n   bottom:" + stringify(v.bottom);
}

template<>
std::string stringify<AxisT<Dimension>>(AxisT<Dimension> &v) {
    return std::string(" Axis:") +
            "\n   x:" + stringify(v.x) +
            "\n   y:" + stringify(v.y);
}

template<>
std::string stringify<AxisT<Overflow>>(AxisT<Overflow> &v) {
    return std::string(" Axis:") +
            "\n   x:" + stringify((int&)v.x) +
            "\n   y:" + stringify((int&)v.y);
}

template<>
std::string stringify<ValueMinMaxT<Dimension>>(ValueMinMaxT<Dimension> &v) {
    return std::string(" ValueMinMax:") +
            "\n  value:" + stringify(v.value) +
            "\n  min:" + stringify(v.min) + 
            "\n  max:" + stringify(v.max);
}

template<>
std::string stringify<DimensionMinMax>(DimensionMinMax &v) {
    return std::string(" DimensionMinMax:") +
            "\n   value:" + stringify(v.value) +
            "\n   min:" + stringify(v.min) + 
            "\n   max:" + stringify(v.max);
}

template<>
std::string stringify<Style>(Style &v) {
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
std::string stringify<Element>(Element &v) {
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

std::string print_tree(Element &src, int levels = 0) {
    std::string ret = std::string(levels, ' ');
    ret += "Element";
    if (src.name)
        ret += std::string(" (") + src.name + ")";
    ret += ":\n";
    if (src.child)
        ret += print_tree(*src.child, levels + 2);
    if (src.sibling)
        ret += print_tree(*src.sibling, levels);
    return ret;
}

Element root("root");
Element screen("screen");
Element header("header");
Element footer("footer");
Element scrollable("scrollable");

int main() {
    root.append_child(header);
    root.append_child(screen);
    screen.append_sibling(footer);
    screen.append_child(scrollable);

    std::cout << stringify(root) << std::endl;    

    std::cout << print_tree(root) << std::endl;

    return 0;
}