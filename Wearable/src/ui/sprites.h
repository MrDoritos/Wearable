#pragma once

#include "texture.h"

namespace wbl {
namespace Sprites {

using AtlasBuffer = FramebufferT<StaticbufferT<256, 256, 2>>;
using Atlas = AtlasT<AtlasBuffer>;

extern const Atlas atlas asm("_binary_textures_bin_start");

#define TX(name, x, y, w, h) static constexpr Atlas::Sprite name = atlas.getSprite(x, y, w, h);

TX(BATTERY, 0, 18, 5, 10)





#undef TX

}
}