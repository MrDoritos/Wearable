#pragma once

#include <stdlib.h>
#include "sizes.h"

namespace wbl {

template<fb _WIDTH, fb _HEIGHT, fb _BPP>
struct StaticbufferT {
    static constexpr fb WIDTH = _WIDTH;
    static constexpr fb HEIGHT = _HEIGHT;
    static constexpr fb BPP = _BPP;
    static constexpr fb PXPERBYTE = 8 / _BPP;
    static constexpr fb SIZE = (_WIDTH * _HEIGHT) / PXPERBYTE;
    
    pixel buffer[SIZE];
};

struct Memorybuffer {
    const fb WIDTH;
    const fb HEIGHT;
    const fb BPP;
    const fb PXPERBYTE = 8 / BPP;
    const fb SIZE = (WIDTH * HEIGHT) / PXPERBYTE;

    pixel *buffer;

    constexpr Memorybuffer(const fb &width, const fb &height, const fb &bpp, pixel *buffer)
        :WIDTH(width),HEIGHT(height),BPP(bpp),buffer(buffer){}
};

template<typename Buffer>
struct FramebufferT : public Buffer {
    using Buffer::Buffer;

    inline constexpr static fb getPxPerByte(fb bpp) {
        return 8 / bpp;
    }

    inline constexpr static fb getSize(fb width, fb height, fb bpp) {
        return (width * height) / getPxPerByte(bpp);
    }

    // Get size of backbuffer in bytes
    inline constexpr fb getSize() const {
        return this->SIZE;
    }

    inline constexpr Length getLength() const {
        return Length(
            this->WIDTH,
            this->HEIGHT
        );
    }

    inline constexpr fb getWidth() const {
        return this->WIDTH;
    }

    inline constexpr fb getHeight() const {
        return this->HEIGHT;
    }

    inline constexpr bool isBound(const fb& x, const fb& y) const {
        return x >= 0 && y >= 0 && x < this->WIDTH && y < this->HEIGHT;
    }

    inline constexpr bool isBound(const Origin &pos) const {
        return isBound(pos.x,pos.y);
    }

    inline constexpr fb getOffset(const fb& x, const fb& y) const {
        return ((y * this->WIDTH) + x) / this->PXPERBYTE;
    }

    inline constexpr fb getOffset(const Origin &pos) const {
        return getOffset(pos.x, pos.y);
    }

    inline constexpr pixel getBitOffset(const fb& x, const fb &y) const {
        return (x % this->PXPERBYTE) * this->BPP;
    }

    inline constexpr pixel getBitOffset(const Origin &pos) const {
        return getBitOffset(pos.x, pos.y);
    }

    inline constexpr pixel getByteMask(const fb& x, const fb &y) const {
        const pixel bits = this->getBitOffset(x, y);
        return (this->getBitMask() << bits);
    }

    inline constexpr pixel getByteMask(const Origin &pos) const {
        return getByteMask(pos.x, pos.y);
    }

    inline constexpr pixel getBitMask() const {
        return (1 << this->BPP) - 1;
    }

    inline void putPixel(const fb& x, const fb& y, const pixel& px) {
        const fb offset = this->getOffset(x, y);
        const fb bits = this->getBitOffset(x, y);
        const fb bytemask = this->getByteMask(x, y);
        const fb bitmask = this->getBitMask();
        this->buffer[offset] &= ~bytemask;
        const pixel pxbyte = (px << bits) & bytemask;
        this->buffer[offset] |= pxbyte;
    }

    inline void putPixel(const Origin &pos, const pixel &px) {
        putPixel(pos.x, pos.y, px);
    }

    inline constexpr pixel getPixel(const fb& x, const fb& y) const {
        const fb offset = this->getOffset(x, y);
        const fb bits = this->getBitOffset(x, y);
        const fb bytemask = this->getByteMask(x, y);
        const fb bitmask = this->getBitMask();
        return ((this->buffer[offset] & bytemask) >> bits) & bitmask;
    }

    inline constexpr pixel getPixel(const Origin &pos) const {
        return getPixel(pos.x, pos.y);
    }

    inline void clear() {
        for (fb i = 0; i < this->SIZE; i++)
            this->buffer[i] = 0;
    }

    inline void flush() {}

    inline constexpr fb getAlphaTest() const {
        return this->BPP - 1;
    }
};

}