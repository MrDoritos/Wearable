#pragma once

#include "texture.h"

namespace wbl {
    template<typename SpriteT = Sprite, typename CharT = char>
    struct FontSpriteT : public SpriteT {
        using Char = CharT;
        using Buffer = typename SpriteT::Buffer;

        const fb font_width, font_height;
        const fb advance_x, advance_y;

        constexpr FontSpriteT(const SpriteT &src, const fb &font_width, const fb &font_height, const fb &advance_x, const fb &advance_y)
        :SpriteT(src),font_width(font_width),font_height(font_height),advance_x(advance_x),advance_y(advance_y){}
        constexpr FontSpriteT(const Buffer &src, const fb &x, const fb &y, const fb &font_width, const fb &font_height, const fb &advance_x, const fb &advance_y)
        :SpriteT(&src,x,y,font_width,font_height),font_width(font_width),font_height(font_height),advance_x(advance_x),advance_y(advance_y){}
    };

    template<typename DerivedAtlas, typename CharT = char, typename FontSpriteT = FontSpriteT<typename DerivedAtlas::Sprite, CharT>>
    struct FontProviderT : public DerivedAtlas {
        using FontSprite = FontSpriteT;
        using DerivedAtlas::DerivedAtlas;
        using Atlas = DerivedAtlas;

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
        using FontSprite = typename Provider::FontSprite;
        using Provider::Provider;

        inline constexpr FontSprite *getCharacter(const CharT &character, FontSprite *in) const {
            *in = getCharacter(character);
            return in;
        }

        inline constexpr FontSprite getCharacter(const CharT &character) const {
            return FontSprite(this->getSpriteAligned(character % glyphs_per_row, character / glyphs_per_row, 1, 1, font_width, font_height), font_width, font_height, advance_x, advance_y);
            //FontSprite ret[1];
            //return *this->getCharacter(character, ret);
        }
    };


}