#include "texture.h"
#include "console/console.h"
#include <string.h>
#include <bitset>
#include <iostream>

struct ConsoleBuffer : public Framebuffer<128, 128, 1> {
    inline void flush() {
        const int cwidth = console::getConsoleWidth(), cheight = console::getConsoleHeight();
        const int clength = cwidth * cheight;
        char ch_buffer[clength];
        color_t co_buffer[clength];

        memset(ch_buffer, ' ', clength);

        for (int x = 0; x < width && x < cwidth; x++) {
            for (int y = 0; y < height && y < cheight; y++) {
                const int offset = y * cwidth + x;
                co_buffer[offset] = getPixel(x, y) ? BWHITE : BBLACK;
            }
        }

        console::write(ch_buffer, co_buffer, clength);
    }
};

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
    texture.flush();
    console::readKey();
}