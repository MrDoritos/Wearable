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
UI::ElementInlineTextT<DisplayTexture, MinifontProvider> TEXT(display, minifont);
UI::ScreenClockT<DisplayTexture> uiclock(display);
UI::ElementRootT<DisplayTexture> uiroot(display);
auto txt = UI::ElementInlineTextT<DisplayTexture, MinifontProvider>(display, minifont);
UI::ElementBatteryT<DisplayTexture> uibattery(display);
UI::ElementDateTimeT<DisplayTexture> uidatetime(display);
UI::ElementBaseT<DisplayTexture> boxtest(display);
UI::ElementBaseT<DisplayTexture> boxtest2(display);

const Atlas::Sprite sprites[] = {
    BATTERY,
    HEART
};
const auto I = font.getCharacter('M');

void demo() {
    /*
    display.clear();
    test.draw_multi({0,0}, BATTERY, "50%");
    spinline.on_draw(nullptr);
    TEXT.on_draw(nullptr);
    uiclock.on_draw(nullptr);
    display.flush();
    */
    uibattery.set_battery_level((millis()%10000)/100);
    uiroot.reset_log();
    //display.clear();
    //uiroot.log_time("BUFCLR");
    //display.fill(uiroot, 1);
    //uiroot.log_time("SET1  ");
    //display.fill(uiroot, 0);
    //uiroot.log_time("SET0  ");
    //uiclock.on_draw(nullptr);
    //uiroot.log_time("CLOCK ");
    //uibattery.on_draw(nullptr);
    //uiroot.log_time("BATT  ");
    //uidatetime.on_draw(nullptr);
    //uidatetime.on_tick(nullptr);
    //uiroot.log_time("DATE  ");

    //uiroot.buffer.border(uiroot, 1);
    ///boxtest.buffer.border(boxtest, 1);
    //boxtest.clear(1);
    //boxtest2.clear();
    //uiroot.flush_log(true,true,{60,80});
    //display.flush();
    uiroot.once();
    //uiroot.overlay_tree_positions(true);

    if (dpad.any(Dpad::PRESSED)) {
        puts("Any pressed");
        display.clear();
        if (dpad.enter.is_pressed())
            display.putTexture(therock, {0,0,128,128}, {0,0});
        if (dpad.up.is_pressed())
            display.putTexture(atlas, {0,0,128,128}, {0,0});
        display.flush();
        delay(1000);
        display.clear();
    }
    dpad.update();
    delay(100);
}

void init() {
    uiroot.setDebug(true);
    using namespace UI;
    using DElem = UI::ElementT<DisplayTexture>;
    static DElem block(display, "block");
    static DElem inner(display, "inner");
    static DElem block2(display, "area");
    static DElem inlineblock(display, "inline");
    static DElem inlineblock2(display, "sprites");
    static DElem inlineblock3(display, "right");
    static DElem inlineblock4(display, "left");
    static DElem block3(display, "after");

    block << UI::StyleInfo { .height{26} };
    inner << StyleInfo {.height {14}};
    block2 << UI::StyleInfo { .width{32}, .height{26} };
    inlineblock << UI::StyleInfo { .display{INLINE}, .width {40}, .height{26} };
    inlineblock2 << UI::StyleInfo { .align{RIGHT}, .display{INLINE}, .width {32}, .height{20}, .margin{2} };
    inlineblock3 << UI::StyleInfo { .align{RIGHT}, .display{INLINE}, .width {20}, .height{10}, .margin{2} };
    inlineblock4 << UI::StyleInfo { .display{INLINE}, .width{20}, .height{30}, .margin{2} };
    block3 << StyleInfo { .width {30}, .height{20} };

    uiroot << UI::StyleInfo { .width{128}, .height{128} };

    block << inner;

    /*
    uiroot << block;
    uiroot << block2;
    uiroot << inlineblock;
    uiroot << inlineblock2;
    uiroot << inlineblock3;
    uiroot << inlineblock4;
    uiroot << block3;

    uiroot.resolve_layout();
    */

    //uiroot << Size { 0, 0, 128, 128 };
    test << Origin { 4, 30 };
    txt << Size { 32, 64, 64, 64 };
    txt << UI::StyleInfo { .wrap{UI::WRAP} };
    uiclock << Size { 16, 16, 97, 97 };
    TEXT << Origin { 12, 16 };
    test.wrap = UI::WrapStyle::WRAP | UI::WrapStyle::TRIM_SPACE;
    test << Size { 32, 30, 64, 64 };
    spinline << HEART << BATTERY << UI::StyleInfo { .wrap{UI::NOWRAP} };
    //uibattery << Size {0,0,32,16};
    //uidatetime << Size {49, 0, 79, 12} << UI::StyleInfo {.wrap{UI::WRAP}};
    uibattery << UI::StyleInfo { .align{LEFT}, .wrap{NOWRAP}, .display{INLINE}, .overflow{AUTO} };
    uidatetime << UI::StyleInfo { .align{RIGHT}, .wrap{NOWRAP}, .display{INLINE}, .overflow{AUTO} };
    boxtest << Size { 8, 13, 5, 5 };
    boxtest2 << Size { 9, 14, 3, 3 };

    TEXT.text = "Hello UI";
    txt.text = "SOME TEST TEXT\n VERY FINE text\n for very small values 0123456789 \% voltage 4.2v !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";

    spinline.on_content_size(nullptr);
    TEXT.on_content_size(nullptr);

    spinline.resolve_layout();
    TEXT.resolve_layout();

    uiroot << uibattery;
    uiroot << uidatetime;

    uiroot.dispatch(EventTypes::CONTENT_SIZE);
    uiroot.resolve_layout();
}

extern "C" {
void app_main() {
    dpad.init();
    init();
    if (display.init() != ESP_OK) {
        printf("Failed to initialize display\n");
    } else {
        printf("Display initialized\n");
        display.clear(0);
        display.flush();
        while (1) {
            demo();
            vPortYield();
        }
    }

    fflush(stdout);
    
    delay(1000);
}
}
