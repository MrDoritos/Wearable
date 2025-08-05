#pragma once

#include "config.h"
#include "ui.h"

namespace wbl {
namespace UI {

template<typename ElementT = ElementBaseT<Sprites::DisplayTexture>>
struct TextureWriterT {
    ElementT &ref;
    bool wrap, clip_x, clip_y;
    Size size;
    const Origin offset;

    constexpr TextureWriterT(ElementT &ref, const Origin &pos):
        ref(ref),
        wrap(ref.wrap & WRAP),
        clip_x(ref.overflow.x & HIDDEN),
        clip_y(ref.overflow.y & HIDDEN),
        size(pos, Length(0)),
        offset(pos) {}

    constexpr TextureWriterT(ElementT &ref):TextureWriterT(ref, ref.getOffset()) {}
    constexpr TextureWriterT(ElementT *ref, const Origin &pos):TextureWriterT(*ref,pos){}
    constexpr TextureWriterT(ElementT *ref):TextureWriterT(*ref) {}

    void add_break() {
        size.y += size.height;
        size.x = offset.x;
        size.height = 0;
    }

    bool add_length(const Length &length, Origin &pos) {
        if (length.width + size.x > ref.getWidth()) {
            if (wrap)
                add_break();
            else
            if (clip_x)
                return false;
        }

        if (clip_y && length.height + size.y > ref.getHeight())
            return false;

        if (size.height < length.height)
            size.height = length.height;
        
        pos = size;

        size.x += length.width;

        return true;
    }

    template<typename FontProvider, typename Format, int buflen=100, typename ...Args>
    void printf(const FontProvider &font, Format format, const Args&...args) {
        char buf[buflen];
        snprintf(buf, buflen, format, args...);
        text(font, buf);
    }

    template<typename Sprite>
    void sprite(const Sprite &sp) {
        Origin pos;
        if (!add_length(sp, pos))
            return;
        ref.buffer.putSprite(sp, pos);
    }

    template<typename Glyph>
    void glyph(const Glyph &gl) {
        Origin pos;
        const Length glyph_size(
            gl.font_width + gl.advance_x,
            gl.font_height + gl.advance_y  
        );
        if (!add_length(glyph_size, pos))
            return;
        ref.buffer.putSprite(gl, pos);
    }

    template<typename Sprite>
    void sprite(const Sprite *sp, const int &num_sp=1) {
        for (int i = 0; i < num_sp; i++)
            sprite(*sp);
    }

    template<typename Sprite>
    void sprites(const Sprite &sp) {
        sprite(sp);
    }

    template<typename Sprite, typename ...Args>
    void sprites(const Sprite &sp, const Args&...args) {
        sprite(sp);
        sprites(args...);
    }

    template<typename FontProvider>
    void text(const FontProvider &font, const char *txt) {
        const int length = strlen(txt);

        for (int i = 0; i < length; i++) {
            const char ch = txt[i];

            if (ch == '\n') {
                add_break();
                continue;
            }

            glyph(font.getCharacter(ch));
        }
    }
};

}
}