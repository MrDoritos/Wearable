#pragma once

#include "texture.h"

namespace wbl {
namespace Sprites {

using AtlasBuffer = FramebufferT<StaticbufferT<256, 256, 2>>;
using Atlas = AtlasT<AtlasBuffer>;

extern const Atlas atlas asm("_binary_textures_bin_start");

#define TX(name, x, y, w, h) static constexpr Atlas::Sprite name = atlas.getSprite(x, y, w, h);


TX(BATTERY_4x8, 0, 10, 4, 8)
TX(BATTERY_5x10, 0, 18, 5, 10)
TX(BATTERY_13x6, 5, 11, 13, 6)

#define BATTERY BATTERY_5x10

TX(HEART_LARGE, 20, 12, 11, 11)
TX(HEART_SMALL, 32, 13, 9, 8)

#define HEART HEART_SMALL



#undef TX

}
}