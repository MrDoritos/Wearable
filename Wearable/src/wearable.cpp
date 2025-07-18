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
UI::ElementBaseT<DisplayTexture> header(display);
UI::IScreen mainscreen("Main");
UI::IScreen clockscreen("Clock");

static bool tog = false;
void demo() {
    uibattery.set_battery_level((millis()%10000)/100);
    uiroot.reset_log();
    if (dpad.right.is_pressed())
        uiroot.setDebug(!uiroot.debug);


    for (int i = 0; i < dpad.button_count; i++) {
        const Dpad::Button &button = dpad.buttons[i];

        const UI::EventValues values[] = {
            UI::EventValues::DPAD_ENTER,
            UI::EventValues::DPAD_UP,
            UI::EventValues::DPAD_RIGHT,
            UI::EventValues::DPAD_DOWN,
            UI::EventValues::DPAD_LEFT,
        };

        UI::Event input_event(UI::EventTypes::USER_INPUT);

        if (button.is_pressed())
            input_event.value = (UI::EventValues)(input_event.value | UI::EventValues::PRESSED);
        if (button.is_held())
            input_event.value = (UI::EventValues)(input_event.value | UI::EventValues::HELD);
        if (button.is_released())
            input_event.value = (UI::EventValues)(input_event.value | UI::EventValues::RELEASED);

        if (!input_event.value)
            continue;

        input_event.value = (UI::EventValues)(input_event.value | values[i]);

        uiroot.dispatch_event(input_event);
    }

    uiroot.once(uiroot.debug);
    
    if (uiroot.debug) {
        if (dpad.down.is_pressed())
            tog = !tog;
        uiroot.overlay_tree_positions(tog);
        uiroot.clear();
    }

    if (dpad.enter.is_pressed() || dpad.up.is_pressed()) {
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
    //uiroot.setDebug(true);
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
    //txt << Size { 32, 64, 64, 64 };
    //txt << UI::StyleInfo { .wrap{UI::WRAP} };
    //uiclock << Size { 16, 16, 97, 97 };
    uiclock << StyleInfo { .width{96}, .height{96}, .margin{16,4} };
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
    uiclock.use_milliseconds = true;

    TEXT.text = "Hello UI";
    txt.text = "SOME TEST TEXT\n VERY FINE text\n for very small values 0123456789 \% voltage 4.2v !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";

    spinline.on_content_size(nullptr);
    TEXT.on_content_size(nullptr);

    spinline.resolve_layout();
    TEXT.resolve_layout();

    //screenclock.addHeader(header);
    //screenclock << uiclock;
    //screenclocknoheader << txt;

    //uiroot << uibattery;
    //uiroot << uidatetime;
    header.name = "header";
    header << uibattery;
    header << uidatetime;
    //uiroot << header;
    //uiroot << uiclock;
    //uiroot << screenclock;
    //uiroot << screenclocknoheader;
    clockscreen << uiclock;
    mainscreen << txt;
    mainscreen.set_left(clockscreen);

    uiroot.set_header(header);
    uiroot.set_screen(mainscreen);

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
