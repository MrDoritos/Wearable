#pragma once

#include "texture.h"

namespace wbl {
    template<typename Buffer, typename CharT, typename Sprite = SpriteT<Buffer>>
    struct FontSpriteT : public Sprite {
        const fb font_width, font_height;
        const fb advance_x, advance_y;

        FontSpriteT(const Sprite &src, const fb &font_width, const fb &font_height, const fb &advance_x, const fb &advance_y)
        :Sprite(src),font_width(font_width),font_height(font_height),advance_x(advance_x),advance_y(advance_y){}

        FontSpriteT(const Buffer &src, const fb &x0, const fb &y0, const fb &x1, const fb &y1, const fb &font_width, const fb &font_height, const fb &advance_x, const fb &advance_y)
        :Sprite(src, x0, y0, x1, y1),font_width(font_width),font_height(font_height),advance_x(advance_x),advance_y(advance_y){}
    };

    template<typename Buffer, typename CharT>
    struct FontProviderT : public Buffer {
        using FontSprite = FontSpriteT<Buffer, CharT>;

        FontSprite getCharacter(const CharT &character);
        FontSprite *getCharacter(const CharT &character, FontSprite *in);
    };

    template<typename Buffer,
             fb font_width = 8,
             fb font_height = 8,
             fb advance_x = 8,
             fb advance_y = 8,
             typename CharT = char,
             fb glyphs_per_row = 8,
             fb glyphs_per_column = 8,
             typename Provider = FontProviderT<AtlasT<Buffer>, CharT>>
    struct MonospaceFontProviderT : public Provider {
        using FontSprite = FontSpriteT<Buffer, CharT>;

        inline constexpr FontSprite *getCharacter(const CharT &character, FontSprite *in) const {
            *in = getCharacter(character);
            return in;
        }

        inline constexpr FontSprite getCharacter(const CharT &character) const {
            return FontSprite(this->getSpriteAligned(character % glyphs_per_row, character / glyphs_per_column, 1, 1, font_width, font_height), font_width, font_height, advance_x, advance_y);
            //FontSprite ret[1];
            //return *this->getCharacter(character, ret);
        }
    };


}