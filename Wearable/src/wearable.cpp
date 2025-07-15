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

void demo() {
    const TickType_t ms=300;
    display.clear();
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
    display.flush();
    vTaskDelay(100 / portTICK_PERIOD_MS);
    TextureGraphicsContext<TextureT<FramebufferT<Memorybuffer>>> graphics(128, 128, 1, display.buffer);
    display.clear(0);
    display.fill({0,0,16,16}, 1);
    graphics.fill(Size{0,24,16,16}, 1);
    display.flush();
    delay(500);
    dpad.print_states();
    if (dpad.any(Dpad::PRESSED)) {
        puts("Any pressed");
        display.clear();
        if (dpad.enter.is_pressed())
            display.putTexture(therock, {0,0,128,128}, {0,0});
        if (dpad.up.is_pressed())
            display.putTexture(atlas, {0,0,128,128}, {0,0});
        display.flush();
        delay(2000);
    }
    dpad.update();
    dpad.print_states();
}

extern "C" {
void app_main() {
    printf("test\n");
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
