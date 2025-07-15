#pragma once

#include "font.h"
#include "texture.h"

#define THEROCK

namespace wbl {
namespace Sprites {

using FontBuffer = FramebufferT<StaticbufferT<256, 256, 2>>;
using FontTexture = TextureT<FontBuffer>;
using FontProvider = MonospaceFontProviderT<FontTexture, 6, 12, 0, 0, char, 42, 14>;
using FontSprite = FontProvider::FontSprite;
extern const FontProvider font asm("_binary_dosjpn_bin_start");

using AtlasBuffer = FramebufferT<StaticbufferT<256, 256, 2>>;
using Atlas = AtlasT<AtlasBuffer>;
using AtlasFontSprite = FontSpriteT<Atlas::Sprite, char>;

extern const Atlas atlas asm("_binary_textures_bin_start");

#ifdef THEROCK

using RockTexture = TextureT<FramebufferT<StaticbufferT<128, 128, 2>>>;
extern const RockTexture therock asm("_binary_therock_bin_start");

#endif

#define TX(name, x, y, w, h) static constexpr Atlas::Sprite name = atlas.getSprite(x, y, w, h);
#define FT(x,y,w,h) AtlasFontSprite(atlas, x, y, w, h, 1, 1)
#define FT35(x,y) FT(x,y,3,5)


TX(BATTERY_4x8, 0, 10, 4, 8)
TX(BATTERY_5x10, 0, 18, 5, 10)
TX(BATTERY_13x6, 5, 11, 13, 6)

#define BATTERY BATTERY_5x10

TX(HEART_LARGE, 20, 12, 11, 11)
TX(HEART_SMALL, 32, 13, 9, 8)

#define HEART HEART_SMALL

static constexpr const AtlasFontSprite GLYPHS_NUM_3x5[] = {
    FT35(0,0),
    FT35(3,0),
    FT35(6,0),
    FT35(9,0),
    FT35(12,0),
    FT35(15,0),
    FT35(18,0),
    FT35(21,0),
    FT35(24,0),
    FT35(27,0),
};

static constexpr const AtlasFontSprite GLYPHS_ALPHA_3x5[] = {
    FT35(71,0),
    FT35(75,0),
    FT35(79,0),
    FT35(83,0),
    FT35(87,0),
    FT35(91,0),
    FT35(95,0),
    FT35(99,0),
    FT35(103,0),
    FT35(107,0),
    FT35(111,0),
    FT35(115,0),
    FT(175,0,5,5),
    FT(181,0,4,5),
    FT35(127,0),
    FT35(131,0),
    FT35(135,0),
    FT35(139,0),
    FT35(143,0),
    FT35(147,0),
    FT35(151,0),
    FT35(155,0),
    FT35(163,0),
    FT35(167,0),
    FT35(171,0),
    FT(186,0,5,5),
};

static constexpr const AtlasFontSprite GLYPHS_SPECIAL_3x5[] = {
    FT35(0,5), // %
    FT35(191,0), // SPACE
};

#undef TX
#undef FT
#undef FT35

/*
template<typename Derived, typename Fallback, typename AtlasSourceT = typename Derived::Atlas, typename FontSpriteT = typename Derived::FontSprite, typename FontSpriteFallbackT = typename Fallback::FontSprite, typename CharT = char>
struct GlyphFontProviderT : public Derived, public Fallback {
    using Char = CharT;

    inline constexpr FontSprite getCharacter(const CharT &character) const {
        const FontSprite *sprite = Derived::getCharacter(character);
        if (sprite)
            return sprite;
        
        return Fallback::getCharacter(character);
    }
};

struct MinifontProvider : public GlyphFontProviderT<MinifontProvider, FontProvider, FontTexture, AtlasFontSprite> {
    using GlyphBase = GlyphFontProviderT<MinifontProvider, FontProvider>;
    using FontSprite = typename FontProvider::FontSprite;
    using Char = GlyphBase::Char;

    inline constexpr FontSprite *getCharacter(const Char &character) const {
        if (character >= '0' && character <= '9')
            return &GLYPHS_NUM_3x5[character-'0'];
    }

}
*/

struct MinifontProvider : public TextureT<FramebufferT<Memorybuffer>> {
    constexpr MinifontProvider():TextureT(256, 256, 2, &atlas.buffer[0]){}

    using Memory = FramebufferT<Memorybuffer>;
    using MemTex = TextureT<Memory>;
    using MemAtl = AtlasT<Memory, MemTex>;
    using Sprite = typename MemAtl::Sprite;
    using Font = FontSpriteT<Sprite, char>;

    template<typename T>
    inline constexpr Font convSrc(const T &sprite) const {
        //return Font(*((const MemTex*)sprite.src), sprite.font_width, sprite.font_height, sprite.advance_x, sprite.advance_y);
        Font font = (Font&)sprite;
        const_cast<const MemTex*&>(font.src) = this;
        return font;
    }

    inline constexpr Font getCharacter(const char &character) const {
        if (character >= '0' && character <= '9')
            return convSrc(GLYPHS_NUM_3x5[character-'0']);
        if (character >= 'a' && character <= 'z')
            return convSrc(GLYPHS_ALPHA_3x5[character-'a']);
        if (character >= 'A' && character <= 'Z')
            return convSrc(GLYPHS_ALPHA_3x5[character-'A']);
        if (character == '%')
            return convSrc(GLYPHS_SPECIAL_3x5[character-'%']);
        if (character == ' ')
            return convSrc(GLYPHS_SPECIAL_3x5[1]);
        return convSrc(font.getCharacter(character));
    }
};

MinifontProvider minifont;

}
}