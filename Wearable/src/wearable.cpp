#include <stdio.h>
#include <inttypes.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "displaybuffer.h"
#include "texture.h"
#include "font.h"
#include "ui.h"
#include "sprites.h"
#include "user_inputs.h"
#include "wbl_func.h"

using namespace wbl;
using namespace Sprites;

UI::ElementBaseT<DisplayTexture> test(display);
UI::ElementInlineSpritesT<DisplayTexture, Atlas> spinline(display);
UI::ElementInlineTextT<DisplayTexture> TEXT(display, Sprites::font);

void demo() {
    const TickType_t ms=300;
    display.clear();
    test << display;
    test << Origin { 4, 30 };
    const auto I = font.getCharacter('M');
    test.setWidth(64);
    test.setOffsetX(32);
    test.setHeight(64);
    test.wrap = UI::WrapStyle::WRAP | UI::WrapStyle::TRIM_SPACE;
    const Atlas::Sprite sprites[] = {
        BATTERY,
        HEART
    };
    test.draw_multi({0,0}, BATTERY, "50%");
    spinline.on_content_size(nullptr);
    TEXT << Origin { 12, 16 };
    TEXT.on_content_size(nullptr);
    spinline.resolve_layout();
    TEXT.resolve_layout();
    spinline.on_draw(nullptr);
    TEXT.on_draw(nullptr);
    display.flush();
    vTaskDelay(800 / portTICK_PERIOD_MS);
    TextureGraphicsContext<TextureT<FramebufferT<Memorybuffer>>> graphics(128, 128, 1, display.buffer);
    display.clear(0);
    display.fill({0,0,16,16}, 1);
    graphics.fill(Size{0,24,16,16}, 1);
    display.flush();
    dpad.print_states();
    if (dpad.any(Dpad::PRESSED)) {
        puts("Any pressed");
        display.clear();
        if (dpad.enter.is_pressed())
            display.putTexture(therock, {0,0,128,128}, {0,0});
        if (dpad.up.is_pressed())
            display.putTexture(atlas, {0,0,128,128}, {0,0});
        display.flush();
        delay(1200);
    }
    dpad.update();
    dpad.print_states();
    delay(500);
}

extern "C" {
void app_main() {
    printf("test\n");
    spinline << HEART << BATTERY << UI::StyleInfo { .wrap{UI::NOWRAP} };
    TEXT.text = "Hello UI";
    dpad.init();
    if (display.init() != ESP_OK) {
        printf("Failed to initialize display\n");
    } else {
        printf("Display initialized\n");
        display.clear(1);
        display.flush();
        vTaskDelay(100 / portTICK_PERIOD_MS);
        while (1) {
            demo();
            vPortYield();
        }
    }

    fflush(stdout);
    
    delay(1000);
}
}
