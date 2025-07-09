#pragma once

typedef unsigned short fb;
typedef unsigned char pixel;

template<fb WIDTH, fb HEIGHT, fb BPP>
struct Framebuffer {
    static constexpr fb PXPERBYTE = 8 / BPP;
    static constexpr fb SIZE = WIDTH * HEIGHT / PXPERBYTE;
    static constexpr fb bpp = BPP;
    static constexpr fb width = WIDTH;
    static constexpr fb height = HEIGHT;

    pixel buffer[SIZE];

    // Get size of backbuffer in bytes
    inline constexpr fb getSize() const {
        return SIZE;
    }

    inline constexpr fb getWidth() const {
        return WIDTH;
    }

    inline constexpr fb getHeight() const {
        return HEIGHT;
    }

    inline constexpr fb isBound(const fb& x, const fb& y) const {
        return x >= 0 && y >= 0 && x < WIDTH && y < HEIGHT;
    }

    inline constexpr fb getOffset(const fb& x, const fb& y) const {
        return ((y * WIDTH) + x) / PXPERBYTE;
    }

    inline constexpr fb getBitOffset(const fb& x) const {
        //return (x % (8 / BPP)) * BPP;
        return (x & 7);
    }

    inline constexpr fb getByteMask(const fb& x) const {
        const fb bits = getBitOffset(x);
        return this->getBitMask() << bits;
    }

    inline constexpr fb getBitMask() const {
        return (1 << BPP) - 1;
    }

    inline void putPixel(const fb& x, const fb& y, const pixel& px) {
        const fb offset = this->getOffset(x, y);
        const fb bits = this->getBitOffset(x);
        const fb bytemask = this->getByteMask(x);
        const fb bitmask = this->getBitMask();
        buffer[offset] &= ~bytemask;
        const pixel pxbyte = (px & bitmask) << bits;
        buffer[offset] |= pxbyte;
    }

    inline constexpr pixel getPixel(const fb& x, const fb& y) const {
        const fb offset = this->getOffset(x, y);
        const fb bits = this->getBitOffset(x);
        const fb bytemask = this->getByteMask(x);
        return (buffer[offset] & bytemask) >> bits;
    }

    inline void clear() {
        for (fb i = 0; i < SIZE; i++)
            buffer[i] = 0;
    }

    inline void flush() {}
};