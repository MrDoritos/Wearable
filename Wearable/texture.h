#pragma once

#include "ui.h"

template<typename Buffer>
struct SpriteT {
    const Buffer &src;
    const fb x0, y0, x1, y1;
    SpriteT(const Buffer &src, const fb &x0, const fb &y0, const fb &x1, const fb &y1)
        :src(src),x0(x0),y0(y0),x1(x1),y1(y1) {}

    inline constexpr fb getWidth() const { return x1 - x0; }
    inline constexpr fb getHeight() const { return y1 - y0; }
    inline constexpr auto getSize() const { return [getWidth(), getHeight()]; }
};

template<typename Buffer>
struct TextureT : public Buffer {
    using Sprite = SpriteT<Buffer>;

    inline void clear(const pixel &px) {
        const pixel d = 
            (px << (bpp * 4)) |
            (px << (bpp * 3)) |
            (px << (bpp * 2)) |
            px;
        for (fb i = 0; i < getSize(); i++) {
            buffer[i] = d;
        }
    }

    inline void fill(const fb &x0, const fb &y0, const fb &x1, const fb &y1, const pixel &px) {
        for (fb x = x0; x < x1; x++)
            for (fb y = y0; y < y1; y++)
                putPixel(x, y, px);
    }

    inline void circle(const fb &cx, const fb &cy, const fb &r, const pixel &px, const bool fill=true) {
        const fb r2 = r * r;

        for (fb x = cx - r; x < cx + r + 1; x++) {
            for (fb y = cy - r; y < cy + r + 1; y++) {
                const xy2 = (x-cx) * (x-cx) + (y-cy) * (y-cy);
                if (!fill && ((xy2 - r2) > 2 || (xy2 - r2) < -2))
                    continue;
                if (xy2 < r2)
                    putPixel(x, y, px);
            }
        }
    }

    inline void putSprite(const Sprite &sprite, const fb &x, const fb &y, const fb &w = 0, const fb &h = 0) {
        const fb width = w ?? sprite.getWidth();
        const fb height = h ?? sprite.getHeight();

        for (fb i = 0, k = x; i < w; i++, k++) {
            for (fb j = 0, l = y; j < h; j++, l++) {
                putPixel(k, l, sprite.src.getPixel(i + sprite.x0, j + sprite.y0));
            }
        }
    }
};

template<typename Buffer>
struct AtlasT : public TextureT<Buffer> {
    using Sprite = SpriteT<Buffer>;

    inline constexpr Sprite getSprite(const fb &x, const fb &y, const fb &sx, const fb &sy) const {
        return Sprite(*this, x, y, x+sx, y+sy);
    }
};