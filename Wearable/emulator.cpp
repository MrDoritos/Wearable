#include "texture.h"
#include "console/console.h"
#include <string.h>
#include <bitset>
#include <iostream>

struct ConsoleBuffer : public Framebuffer<128, 128, 1> {
    inline const char* translateBlock(const fb &x, const fb &y) {
        fb dat = 0;
        for (int j = y, k = 0; j < y+2; j++) {
            for (int i = x; i < x+2; i++, k+=bpp) {
                dat |= (getPixel(i, j) & 1) << k;
            }
        }

        dat &= 0xF;

        const char* (blockMap)[] = {
            " ","▘","▝","▀","▖","▌","▞","▛","▗","▚","▐","▜","▄","▙","▟","█",
        };

        //const char *blockMapAscii = " \0`\0'\0^\0,\0[\0/\0<\0.\0\\\0]\0>\0_\0L\0J\0#\0";
        const char *blockMapAscii = "  \0^ \0 ^\0^^\0v \0$ \0v^\0$^\0 v\0^v\0 $\0^$\0vv\0$v\0v$\0$$\0";

        //return blockMap[dat & 0x0F];
        return &blockMapAscii[dat*3];
    }

    inline void flush() {
        const int cwidth = console::getConsoleWidth(), cheight = console::getConsoleHeight();
        const int clength = cwidth * cheight;
        char ch_buffer[clength * 4];
        color_t co_buffer[clength];

        memset(ch_buffer, ' ', clength);
        memset(co_buffer, FWHITE | BBLACK, clength);
        int offset2 = 0;

        /*
        for (int x = 0; x < width && x < cwidth; x++) {
            for (int y = 0; y < height && y < cheight; y++) {
                const int offset = y * cwidth + x;
                co_buffer[offset] = getPixel(x, y) ? BWHITE : BBLACK;
            }
        }
        */

        for (int x = 0; x < width && x < cwidth * 2; x+=2) {
            for (int y = 0; y < height && y < cheight * 2; y+=2) {
                const char *ch = translateBlock(x, y);
                const int l = strlen(ch);
                //const int offset = ((y/2) * cwidth + (x/2));
                const int offset = (y/2) * cwidth + x;
                memcpy(ch_buffer + offset2 + offset, ch, l);
                //offset2 += (l-1);
                //offset2 += 1;
            }
        }

        //console::write(ch_buffer, co_buffer, clength);
        console::write(0, 0, ch_buffer);
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