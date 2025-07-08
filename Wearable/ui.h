#pragma once

typedef short fb;
typedef char pixel;

template<fb Width, fb Height, fb BPP>
struct Framebuffer {
    // Total width, total height
    // BPP determines buffer size
    const fb width = Width, height = Height, bpp = BPP;

    pixel buffer[getSize()];

    // Get size of backbuffer in bytes
    inline constexpr fb getSize() {
        return (Width * Height * BPP) / 8;
    }

    inline constexpr fb getWidth() {
        return Width;
    }

    inline constexpr fb getHeight() {
        return Height;
    }

    inline constexpr fb isBound(const fb& x, const fb& y) {
        return x >= 0 && y >= 0 && x < Width && y < Height;
    }

    inline constexpr fb getOffset(const fb& x, const fb& y) {
        return (y * Width) + x;
    }

    inline constexpr fb getBitOffset(const fb& x) {
        return (x % (8 / BPP)) * BPP;
    }

    inline constexpr fb getByteMask(const fb& x) {
        const fb bits = getBitOffset(x);
        return getBitMask() << bits;
    }

    inline constexpr fb getBitMask() {
        return (1 << BPP) - 1;
    }

    inline void putPixel(const fb& x, const fb& y, const pixel& px) {
        const fb offset = getOffset(x, y);
        const fb bits = getBitOffset(x);
        const fb bytemask = getByteMask(x);
        buffer[offset] &= ~bytemask;
        const pixel pxbyte = px << bits;
        buffer[offset] |= pxbyte;
    }

    inline pixel getPixel(const fb& x, const fb& y) {
        const fb offset = getOffset(x, y);
        const fb bits = getBitOffset(x);
        const fb bytemask = getByteMask(x);
        return (buffer[offset] & bytemask) >> bits;
    }

    inline void clear() {
        for (fb i = 0; i < getSize(); i++)
            buffer[i] = 0;
    }

    inline void flush() {}
};