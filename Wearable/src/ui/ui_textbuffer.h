#pragma once

#include <stdio.h>

namespace wbl {
namespace UI {

template<int buflen>
struct TextBuffer {
    char buffer[buflen];
    int offset = 0;

    template<typename FORMAT, typename ...Args>
    void print(FORMAT format, const Args&...args) {
        offset += snprintf(buffer + offset, buflen - offset, format, args...);
    }

    void clear() {
        offset = 0;
        buffer[0] = 0;
    }
};

}
}