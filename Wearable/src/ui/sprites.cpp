#include "sprites.h"
#include "esp_system.h"

namespace wbl {
namespace Sprites {

DRAM_ATTR DisplayTexture display = DisplayTexture();

template<>
bool DisplayBuffer::draw_transaction = false;
template<>
uint8_t DisplayBuffer::page_state = 0;
template<>
uint8_t DisplayBuffer::page_pos = 0;
template<>
uint8_t DisplayBuffer::bytes_rem = 0;
template<>
uint8_t *DisplayBuffer::page_ptr = 0;
template<>
uint8_t DisplayBuffer::cmd_buffer[33] = {};

}
}