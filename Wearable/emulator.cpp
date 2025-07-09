#include "texture.h"
#include "console/console.h"
#include <string.h>
#include <bitset>
#include <iostream>

struct ConsoleBuffer : public Framebuffer<128, 128, 1> {
    static constexpr const char* (blockMap)[] = {
        " ","▘","▝","▀","▖","▌","▞","▛","▗","▚","▐","▜","▄","▙","▟","█",
    };

    static constexpr const char* (blockMap2w)[] = {
        "  ","▀ "," ▀","▀▀","▄ ","█ ","▄▀","█▀"," ▄","▀▄"," █","▀█","▄▄","█▄","▄█","██",
    };
    static constexpr const char *blockMapAscii = " \0`\0'\0^\0,\0[\0/\0<\0.\0\\\0]\0>\0_\0L\0J\0#\0";
    static constexpr const char *blockMapAscii2w = "  \0^ \0 ^\0^^\0v \0$ \0v^\0$^\0 v\0^v\0 $\0^$\0vv\0$v\0v$\0$$\0";

    inline fb blockToNum(const fb &x, const fb &y, const fb &xn, const fb &yn) {
        fb num = 0;
        const fb mask = (1 << bpp) - 1;
        const fb tot = 1 << (xn * yn);
        for (int j = y, k = 0; j < y + yn; j++)
            for (int i = x; i < x + xn; i++, k+=bpp)
                num |= (getPixel(i, j) & mask) << k;
        return num & (tot - 1);
    }

    template<typename BLOCKSRC>
    inline void flushBlocks(const BLOCKSRC blockmap, int w=1, int stride_x=2, int stride_y=2);

    inline void flush();
};

template<>
void ConsoleBuffer::flushBlocks<char const* const*>(char const* const* blockMap, int w, int stride_x, int stride_y) {
    const int cwidth = console::getConsoleWidth(), cheight = console::getConsoleHeight();
    const int clength = cwidth * cheight;
    const int blength = clength * 4;
    //const int mapLength = sizeof(blockMap)/sizeof(blockMap[0]);
    const int mapLength = (1 << (stride_x * stride_y));
    char ch_buffer[blength];
    color_t co_buffer[blength];
    int offset2 = 0, _offset = 0;

    memset(ch_buffer, 'x', blength);
    memset(co_buffer, FWHITE|BBLACK,blength);

    for (int sy = 0, cy = 0; sy < height && cy < cheight; sy+=stride_y, cy++) {
        for (int sx = 0, cx = 0; sx < width && cx < cwidth; sx+=stride_x, cx+=w) {
            const int num = blockToNum(sx, sy, stride_x, stride_y);
            if (num >= mapLength)
                continue;
            const char *ch = blockMap[num];
            const int l = strlen(ch);
            const int offset = cy * cwidth + cx;

            memcpy(ch_buffer + offset + offset2, ch, l);
            offset2 += (l-w);
            _offset = offset + offset2;
        }
    }

    ch_buffer[_offset+w] = '\0';
    console::clear();
    console::write(0, 0, ch_buffer);
}

template<>
void ConsoleBuffer::flushBlocks<const char*>(const char *blockMap, int w, int stride_x, int stride_y) {
    const int cwidth = console::getConsoleWidth(), cheight = console::getConsoleHeight();
    const int clength = cwidth * cheight;
    const int blength = clength * w;
    const int mapLength = (1 << (stride_x * stride_y));
    char ch_buffer[blength];
    color_t co_buffer[blength];
    int offset2 = 0, _offset = 0;

    memset(ch_buffer, 'x', blength);
    memset(co_buffer, FWHITE|BBLACK,blength);

    for (int sy = 0, cy = 0; sy < height && cy < cheight; sy+=stride_y, cy++) {
        for (int sx = 0, cx = 0; sx < width && cx < cwidth; sx+=stride_x, cx += w) {
            const int num = blockToNum(sx, sy, stride_x, stride_y);
            if (num >= mapLength)
                continue;
            const char *ch = &blockMap[num*(w+1)];
            const int l = strlen(ch);
            const int offset = cy * cwidth + cx;

            memcpy(ch_buffer + offset + offset2, ch, l);
            offset2 += (l-w);
            _offset = offset + offset2;
        }
    }

    ch_buffer[_offset+w] = '\0';
    console::clear();
    console::write(0, 0, ch_buffer);
}

void ConsoleBuffer::flush() {
    flushBlocks(blockMap2w, 2);
}

ConsoleBuffer buffer;

using ConsoleTexture = TextureT<ConsoleBuffer>;

ConsoleTexture texture;

void test() {
    std::cerr << "width: " << texture.width << " height: " << texture.height << " bpp: " << texture.bpp << " SIZE: " << texture.SIZE << " PXPERBYTE: " << texture.PXPERBYTE << std::endl;
    std::cerr << "bitmask: " << std::bitset<16>(texture.getBitMask()) << std::endl;
    for (int i = 0; i < 64; i++) {
        std::cerr << "bytemask (" << i << ") " << std::bitset<16>(texture.getByteMask(i)) << std::endl;
        std::cerr << "bitoffset (" << i << ") " << texture.getBitOffset(i) << std::endl;
        std::cerr << "offset (" << i << ") " << texture.getOffset(i, 0) << std::endl;
    }
}

int main() {
    test();
    texture.clear(1);
    texture.circle(64, 64, 24, 0);
    //texture.flush();
    texture.flushBlocks(texture.blockMap);
    console::readKey();
    texture.flushBlocks(texture.blockMap2w, 2);
    console::readKey();
    texture.flushBlocks(texture.blockMapAscii2w, 2);
    console::readKey();
    return 0;
}