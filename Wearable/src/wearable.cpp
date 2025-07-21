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
#include "ui_func.h"
#include "ui_log.h"

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
UI::ScreenBaseT<> mainscreen("Main");
UI::ScreenBaseT<> clockscreen("Clock");
UI::ScreenBaseT<> settingscreen("Settings");
LoopBufferT<DataPointT<>, 100> datalog;
UI::ElementLogT<DisplayTexture, DataLogT<DataPointT<>, LoopBufferT<DataPointT<>, 100>>> elementlog(display, datalog);

void demo() {
    uibattery.set_battery_level((millis()%10000)/100);

    if (dpad.enter.is_pressed()) {
        display.putTexture(therock, {0,0,128,128}, {0,0});
        display.flush();
        delay(1000);
        display.clear();
    }

    dispatch_input_events(uiroot, dpad);

    uiroot.once();

    static int cnt = 0;
    if (cnt++ % 4 == 0)
        elementlog.push_back(micros(), (uu)(sinf(float((int(micros()))/(M_PI * 2 * 100000)))*500.0f+1500.0f));

    #ifdef __linux__
    delay(30);
    #endif
}

void init() {
    //uiroot.setDebug(true);
    using namespace UI;
    using DElem = UI::ElementT<DisplayTexture>;
    using FElem = UI::ElementFocusT<DisplayTexture>;
    static DElem block(display, "block");
    static DElem inner(display, "inner");
    static DElem block2(display, "area");
    static DElem inlineblock(display, "inline");
    static DElem inlineblock2(display, "sprites");
    static DElem inlineblock3(display, "right");
    static DElem inlineblock4(display, "left");
    static DElem block3(display, "after");

    static FElem focustest(display, "one");
    static FElem focustest2(display, "two");
    static FElem focustest3(display, "three");

    static UI::StyleInfo styles { .display{INLINE}, .width{{25,PERC}}, .height{25} };

    focustest << styles;
    focustest2 << styles;
    focustest3 << styles;
    settingscreen << focustest;
    settingscreen << focustest2;
    settingscreen << focustest3;
    settingscreen << elementlog;

    block << UI::StyleInfo { .height{26} };
    inner << StyleInfo {.height {14}};
    block2 << UI::StyleInfo { .width{32}, .height{26} };
    inlineblock << UI::StyleInfo { .display{INLINE}, .width {40}, .height{26} };
    inlineblock2 << UI::StyleInfo { .align{RIGHT}, .display{INLINE}, .width {32}, .height{20}, .margin{2} };
    inlineblock3 << UI::StyleInfo { .align{RIGHT}, .display{INLINE}, .width {20}, .height{10}, .margin{2} };
    inlineblock4 << UI::StyleInfo { .display{INLINE}, .width{20}, .height{30}, .margin{2} };
    block3 << StyleInfo { .width {30}, .height{20} };

    elementlog << StyleInfo { .align{RIGHT}, .width {60}, .height{40} };

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
    //uiclock << StyleInfo { .width{96}, .height{96}, .margin{16,4} };
    clockscreen << StyleInfo { .width{{100,PERC}}, .height{{100,PERC}} };
    uiclock << StyleInfo { .align{CENTER}, .width{{85,PERC}}, .height{{85, PERC}} };
    TEXT << Origin { 12, 16 };
    test.wrap = (UI::WrapStyle)(UI::WrapStyle::WRAP | UI::WrapStyle::TRIM_SPACE);
    test << Size { 32, 30, 64, 64 };
    spinline << HEART << BATTERY << UI::StyleInfo { .wrap{UI::NOWRAP} };
    //uibattery << Size {0,0,32,16};
    //uidatetime << Size {49, 0, 79, 12} << UI::StyleInfo {.wrap{UI::WRAP}};
    uibattery << UI::StyleInfo { .align{LEFT}, .wrap{NOWRAP}, .display{INLINE}, .overflow{AUTO} };
    uidatetime << UI::StyleInfo { .align{RIGHT}, .wrap{NOWRAP}, .display{INLINE}, .overflow{AUTO} };
    boxtest << Size { 8, 13, 5, 5 };
    boxtest2 << Size { 9, 14, 3, 3 };
    txt << StyleInfo { .align{CENTER}, .width{{70}}, .margin{4} };
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
    txt.name = "txt";
    header.name = "header";
    uibattery.name = "battery";
    uidatetime.name = "datetime";
    clockscreen.name = "clockscreen";
    mainscreen.name = "mainscreen";
    uiclock.name = "clock";
    uiroot.name = "root";
    elementlog.name = "log";

    clockscreen.show_header = false;

    header << uibattery;
    header << uidatetime;
    //uiroot << header;
    //uiroot << uiclock;
    //uiroot << screenclock;
    //uiroot << screenclocknoheader;
    clockscreen << uiclock;
    mainscreen << txt;
    mainscreen.set_left(clockscreen);
    mainscreen.set_right(settingscreen);

    uiroot.set_header(header);
    //uiroot.set_screen(mainscreen);
    uiroot.set_screen(settingscreen);

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
