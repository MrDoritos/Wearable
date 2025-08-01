#pragma once

#include "config.h"
#include "ui.h"

namespace wbl {
namespace UI {

template<typename ElementT>
struct TextureWriterT {
    ElementT &ref;
    Origin pos;
    Origin origin;
    Length inline_length;
    Length total_length;

    constexpr TextureWriterT(ElementT &ref, const Origin &origin=Origin(0)):
        ref(ref),
        pos(origin),
        origin(origin),
        inline_length(0),
        total_length(0) {}
    constexpr TextureWriterT(ElementT *ref):TextureWriterT(*ref){}

    void add_break() {
        pos.y += inline_length.height;

        if (pos.y - origin.y > total_length.height)
            total_length.height = pos.y - origin.y;

        inline_length = 0;
        pos.x = origin.x;
    }

    void add_length(const Length &length) {
        if (length.height > inline_length.height)
            inline_length.height = length.height;

        pos.x += length.width;

        if (pos.x - origin.x > total_length.width)
            total_length.width = pos.x - origin.x;

        if (pos.x > ref.getRight())
            add_break();
    }

    template<typename FontProvider, typename Format, int buflen=100, typename ...Args>
    void printf(const FontProvider &font, Format format, const Args&...args) {
        char buf[buflen];
        snprintf(buf, buflen, format, args...);
        text(font, buf);
        add_break();
    }

    template<typename Sprite>
    void sprite(const Sprite &sp) {
        const Length use = ref.template draw_sprites(&sp, 1, pos);
        add_length(use);
    }

    template<typename Sprite>
    void sprite(const Sprite *sp, const int &num_sp=1) {
        for (int i = 0; i < num_sp; i++)
            sprite(*sp);
    }

    template<typename FontProvider>
    void text(const FontProvider &font, const char *txt) {
        const Length use = ref.template draw_text(txt, font, pos);
        add_length(use);
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

};

}
}