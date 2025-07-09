#include "texture.h"
#include "console/console.h"
#include <string.h>
#include <bitset>
#include <iostream>

struct ConsoleBuffer : public Framebuffer<128, 128, 1> {
    static constexpr const char* blockMap = " \0▘\0▝\0▀\0▖\0▌\0▞\0▛\0▗\0▚\0▐\0▜\0▄\0▙\0▟\0█\0";
    static constexpr const char* blockMap2w = "  \0▀ \0 ▀\0▀▀\0▄ \0█ \0▄▀\0█▀\0 ▄\0▀▄\0 █\0▀█\0▄▄\0█▄\0▄█\0██\0";
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
    inline void flushBlocks(const BLOCKSRC blockmap, const int &w=1, const int &stride_x=2, const int &stride_y=2);

    inline void flush();
};

template<typename BLOCKSRC>
void ConsoleBuffer::flushBlocks(const BLOCKSRC _blockMap, const int &w, const int &stride_x, const int &stride_y) {
    const int cwidth = console::getConsoleWidth(), cheight = console::getConsoleHeight();
    const int clength = cwidth * cheight;
    const int mapLength = (1 << (stride_x * stride_y));
    const int blength = clength * 4 * w;
    const char *blockMap = (const char*)_blockMap;

    int mapOffsets[mapLength][2];
    for (int i = 0, j = 0, k = 0, l = 0; i < mapLength; 
        l = strlen(blockMap + j), j += l + 1, mapOffsets[i][0] = k, mapOffsets[i][1] = l, k = j, i++);

    char ch_buffer[blength];
    int offset2 = 0, _offset = 0;

    memset(ch_buffer, ' ', blength);

    for (int sy = 0, cy = 0; sy < height && cy < cheight; sy+=stride_y, cy++) {
        for (int sx = 0, cx = 0; sx < width && cx < cwidth; sx+=stride_x, cx+=w) {
            const int num = blockToNum(sx, sy, stride_x, stride_y);
            const char *ch = &blockMap[mapOffsets[num][0]];
            const int l = mapOffsets[num][1];
            const int offset = cy * cwidth + cx;

            for (int i = 0; i < l; i++)
                ch_buffer[i + offset + offset2] = ch[i];

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
    //test();
    texture.clear(0);
    texture.circle(64, 64, 24, 0, false);
    texture.line(0,0,127,127,1);
    texture.line(0,127,127,0,1);
    //texture.clear(0);
    //texture.line_callback(127,0,0,127, [&](const fb &x, const fb &y){ texture.circle(x, y, 1, 1); });
    texture.flush();
    console::readKey();
    return 0;
}