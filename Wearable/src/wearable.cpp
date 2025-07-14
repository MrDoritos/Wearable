#include <stdio.h>
#include <inttypes.h>
//#include "esp_flash.h"
#include "esp_system.h"
//#include "esp_chip_info.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//#include "driver/gpio.h"
//#include "freertos/semphr.h"

#include "displaybuffer.h"
#include "texture.h"
#include "font.h"
#include "ui.h"
#include "sprites.h"
#include "user_inputs.h"
#include "driver/gpio.h"

//extern const char _binary_fixedsys_bin_start[];
//extern const char _binary_fixedsys_bin_end[];

using namespace wbl;
using namespace Sprites;


//using DisplayProviderCtx = TextureGraphicsContext<DisplayTexture>;

//DisplayProviderCtx display;

//UI::ElementT<DisplayTexture> test(display);
UI::ElementBaseT<DisplayTexture> test(display);

void delay(uint16_t ms) {
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

void demo_pattern_() {
    display.clear(0);
    for (fb y = 0; y < display.getHeight(); y+=3) {
        for (fb x = (y&1); x < display.getWidth(); x+=3) {
            display.putPixel(x, y, 1);
        }
    }
    display.flush();
    delay(1000);

    for (fb y = display.getHeight()-2; y > 0; y--) {
        for (fb x = display.getWidth()-2; x > 0; x--) {
            if (display.getPixel(x, y))
                display.putPixel(x+1,y+1, 1);
        }
    }
    display.flush();
    delay(1000);
    display.clear();
    for (fb x = 0; x < display.getWidth(); x+=2) {
        for (fb y = 0; y < display.getHeight(); y++) {
            display.putPixel(x, y, 1);
        }
    }

    display.flush();
    delay(1000);
    for (fb x = 1; x < display.getWidth(); x+=2) {
        for (fb y = 0; y < display.getHeight(); y++) {
            if (display.getPixel(x-1, y))
                display.putPixel(x, y, 1);
        }
    }
    display.flush();
    delay(1000);
}

void demo_pattern() {
    FramebufferT<StaticbufferT<16, 16, 2>> buf;
    buf.clear();
    for (fb y = 0; y < buf.getHeight(); y++) {
        for (fb x = 0; x < buf.getWidth(); x++) {
            if (x == y || (x == (y-8))) {
            buf.putPixel(x, y, 3);
            display.putPixel(x + 16, y, 1);
            }
            display.putPixel(x, y+16, buf.getPixel(x, y) > 0 ? 1 : 0);
            display.putPixel(x+16, y+16, buf.getPixel(x, y) > 1 ? 1 : 0);
        }
    }
    for (int x = 32; x < 64; x++) {
        for (int y = 0; y < 32; y++) {
            if (x % 2 ^ y % 2)
                display.putPixel(x, y, 1);
        }
    }
    //display.putTexture(buf, 0, 0, 16, 16, 0, 0);
    display.flush();
}

void write_characters(const char *str, const fb &x, const fb &y) {
    const fb len = strlen(str);
    fb cx = x;
    for (fb i = 0; i < len; i++) {
        const auto sp = font.getCharacter(*(str + i));
        //display.putSprite(sp, cx, y, sp.getWidth(), sp.getHeight());
        display.putSprite(sp, {cx, y});
        cx += sp.getWidth();
    }
    display.flush();
}

void demo() {
    const TickType_t ms=300;
    /*
    display.clearDisplay(0);
    printf("full\n");
    vTaskDelay(ms / portTICK_PERIOD_MS);
    display.clearDisplay(255);
    printf("clear 0\n");
    vTaskDelay(ms / portTICK_PERIOD_MS);
    */
    
    /*
    display.flush();
    printf("clear 1\n");
    vTaskDelay(ms / portTICK_PERIOD_MS);
    */
    display.clear();
    const auto I = font.getCharacter('M');
    test.setWidth(64);
    test.setOffsetX(32);
    test.setHeight(64);
    test.wrap = UI::WrapStyle::WRAP | UI::WrapStyle::TRIM_SPACE;
    //test.draw_text("Hello world!\n On two lines", font);
    const Atlas::Sprite sprites[] = {
        BATTERY,
        HEART
    };
    //test.draw_sprites(sprites, 2);
    test.draw_multi({0,0}, BATTERY, "50%");


    //display.putTexture(font, 0, 0, 128, 128, 0, 0);
    //printf("%i %i %i %i %i %i\n", I.getWidth(), I.getHeight(), I.x0, I.y0, I.x1, I.y1);
    //demo_pattern();
    //display.putSprite(I, 0, 0);
    //write_characters("Hello World!",0 ,0);
    //write_characters("Mnotmworking",0,16);
    //test.on_clear(nullptr);
    //write_characters("M M M 9 9 mm M", 0, 0);
    //write_characters("M M M 9 9 mm M", 0, 24);
    display.flush();
    vTaskDelay(500 / portTICK_PERIOD_MS);
    TextureGraphicsContext<TextureT<FramebufferT<Memorybuffer>>> graphics(128, 128, 1, display.buffer);
    display.clear(0);
    display.fill({0,0,16,16}, 1);
    graphics.fill(Size{0,24,16,16}, 1);
    display.flush();
    //vTaskDelay(1000 / portTICK_PERIOD_MS);
    //display.putTexture(I.src, 0, 0, 128, 64, 0, 0);
    //display.putTexture(I.src, 0, 64, 128, 128, 126, 0);
    //display.putSprite(I, {0,0});
    //display.putSprite(I, {32,32});
    //if (xSemaphoreTake(semaphore, 100 / portTICK_PERIOD_MS)) {
    //if (dpad.enter.state || dpad.up.state || dpad.down.state || dpad.left.state || dpad.right.state) {
        //dpad.enter.update();
    //if (!gpio_get_level(GPIO_NUM_15)) {
    if (dpad.any_state()) {
        dpad.update();
        display.putTexture(therock, {0,0,128,128}, {0,0});
        display.flush();
    }
    /*
    display.circle(64, 64, 24, 1, true);
    printf("circle\n");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    display.line(0,0,127,127,1);
    display.flush();
    printf("line\n");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    display.line(0,127,127,0,1);
    display.flush();
    */
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    /*
    display.clear(0);
    display.flush();
    printf("clear 0\n");
    vTaskDelay(ms / portTICK_PERIOD_MS);
    fflush(stdout);
    */
}

extern "C" {
void app_main() {
    printf("test\n");
    dpad.init();
    //memcpy(font.buffer, &_binary_fixedsys_bin_start[0], _binary_fixedsys_bin_end - _binary_fixedsys_bin_start);
    //memcpy(font.buffer, _binary_dosjpn_bin_start, font.getSize());

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


    //uint32_t flash_size;
    //esp_flash_get_size(0, &flash_size);
    //printf("%luMB flash size\n", flash_size / (1024 * 1024));
    
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    fflush(stdout);
    //esp_restart();
}
}
