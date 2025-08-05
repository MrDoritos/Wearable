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
#include "display_timeout.h"
#include "gps.h"
#include "ui_gps.h"
#include "gps_imu.h"
#include "ui_imu.h"
#include "wbl_system.h"
#include "ltr390.h"
#include "ui_sysinfo.h"
#include "mics6814.h"
#include "sdcard.h"
#include "peripheral_log.h"
#include "ui_peripheral_log.h"
#include "ui_gps_log.h"

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
LoopBuffer sinelog, squarelog, sawlog, voltlog;
UI::ElementLogT<DisplayTexture, DataLog> e_sinelog(display, sinelog), e_squarelog(display, squarelog), e_sawlog(display, sawlog), e_voltlog(display, voltlog);
UI::ElementLockIconT<DisplayTexture> e_lockicon(display);
UI::ScreenBaseT<> gpsscreen("GPSInfo");
UI::ElementGPST<DisplayTexture> uigps(display);
UI::ScreenBaseT<> imuscreen("IMUInfo");
UI::ElementIMUT<DisplayTexture> uiimu(display);
UI::ScreenBaseT<> sysinfoscreen("System");
UI::ElementSysInfoT<DisplayTexture> uisysinfo(display);
LoopBuffer colog, nh3log, no2log;
UI::ElementLogT<DisplayTexture, DataLog> e_colog(display, colog), e_nh3log(display, nh3log), e_no2log(display, no2log);
UI::ScreenBaseT<> gasscreen("Gases");
UI::LogField<UI::LogFieldProvider<DLBatteryLT, UI::DataValueAccessor<DPBattery, uint16_t, 0>>> lfpb(&wbl::log.battery_lt);
UI::LogField<UI::LogFieldProvider<DLBatteryST, UI::DataValueAccessor<DPBattery, uint16_t, 0>>> lfpbst(&wbl::log.battery_st);
UI::ElementPeripheralLogT<DisplayTexture, decltype(lfpb)> e_pbatterylog(display, lfpb);
UI::ElementPeripheralLogT<DisplayTexture, decltype(lfpbst)> e_pbatterylogst(display, lfpbst);
UI::ScreenBaseT<> gpsview("GPS");
UI::UIGPSLogT<DisplayTexture, DLCAMM8ST> uigpsst(display, wbl::log.camm8_st);
UI::UIGPSLogT<DisplayTexture, DLCAMM8LT> uigpslt(display, wbl::log.camm8_lt);

void demo() {
    /*
    if (dpad.enter.is_pressed()) {
        display.putTexture(therock, {0,0,128,128}, {0,0});
        display.flush();
        delay(1000);
        display.clear();
    }
    */

    bool has_input = false;
    if (!displayTimeout.lock_key_state(dpad.enter.is_held()))
        has_input = dispatch_input_events(uiroot, dpad);
    else
        dpad.update();

    displayTimeout.update(has_input);
    
    static bool isDisplayOff = false;

    if (displayTimeout.is_display_off() != isDisplayOff) {
        isDisplayOff = displayTimeout.is_display_off();
        display.setState(!isDisplayOff);
    }

    /*
    int64_t tt = millis() / 200;
    uint8_t c = (tt % 0x10)*16;//((tt/0x10) % 16)*16;
    uint8_t v = ((tt / 8) % 0x10)*4;//(tt % 0x10)*4;
    uint8_t chg = (tt / (8*16));
    
    display.setChargePeriod(chg % 16, (chg / 16) % 16);
    display.setContrast(c);
    display.setVCOM(v);
    */

    uiroot.once();

    static int cnt = 0;
    int64_t t = micros();
    if (cnt++ % 4 == 0) {
        //e_sinelog.push_back(t, (uu)(sinf(float((int(t))/(M_PI * 2 * 100000)))*500.0f+1500.0f));
        //e_squarelog.push_back(t, (cnt & 64));
        if (cnt & 4) {
            e_squarelog.push_back(t, (uu)(ltr390.getUVIhr()*10.0f));
        } else {
            wbl_system.setDisplayBrightness(ltr390.getLux());
            e_sinelog.push_back(t, (uu)ltr390.getLux());
        }
        e_sawlog.push_back(t, (uu)(int(t/5000)%1000));

        float th = (float(((t/1000000) % 1000))/1000.0f)*(M_PI * 2.0f);
        float x = -cos(th) * 10.0f;
        float y = -sin(th) * 10.0f;
        wbl::log.camm8_st.push_back(DPCAMM8ST(t, y, x, 10, 10, 10, 10, 10, 10, 10, 10));
    }

    if (cnt % 32 == 0) {
        //e_voltlog.push_back(t, (uu)(4000 + ((((t ^ 0xDEADBEEF) % 0xC0FFEE) | t) & 31)));
        e_voltlog.push_back(t, wbl_system.getBatteryVoltage() * 1000);
        e_colog.push_back(t, (uu)(mics6814.getCOVoltage() * 1000.0f));
        e_nh3log.push_back(t, (uu)(mics6814.getNH3Voltage() * 1000.0f));
        e_no2log.push_back(t, (uu)(mics6814.getNO2Voltage() * 1000.0f));
        //wbl::log.battery_st.push_back(t, wbl_system.getBatteryVoltage() * 1000);
    }

    wbl::log.update();

    //wbl::log.battery_lt.push_back(t, gpsimu.getAccelerometer().x * 4000 + 16000);

    if (cnt % 200 == 0) {
        uibattery.set_battery_level((uint8_t)wbl_system.getBatteryLevel());
    }

    //gps.update();
    bool time_set = gps.last_time_update > 0;

    gps.setSystemTime();

    if (gps.last_time_update > 0 && !time_set)
        displayTimeout.any_user_input();

    wbl_system.acquirePMLock();
    //wbl_system.releasePMLock();

    #ifdef __linux__
    delay(30);
    #endif
}

void init() {
    //uiroot.setDebug(true);
    WBL_D("UI Init");
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
    //settingscreen << focustest;
    //settingscreen << focustest2;
    //settingscreen << focustest3;
    settingscreen << e_sinelog;
    settingscreen << e_squarelog;
    settingscreen << e_sawlog;
    settingscreen << e_voltlog;
    gpsscreen << uigps;
    imuscreen << uiimu;

    StyleInfo screenremaining { .height{92} };

    block << UI::StyleInfo { .height{26} };
    inner << StyleInfo {.height {14}};
    block2 << UI::StyleInfo { .width{32}, .height{26} };
    inlineblock << UI::StyleInfo { .display{INLINE}, .width {40}, .height{26} };
    inlineblock2 << UI::StyleInfo { .align{RIGHT}, .display{INLINE}, .width {32}, .height{20}, .margin{2} };
    inlineblock3 << UI::StyleInfo { .align{RIGHT}, .display{INLINE}, .width {20}, .height{10}, .margin{2} };
    inlineblock4 << UI::StyleInfo { .display{INLINE}, .width{20}, .height{30}, .margin{2} };
    block3 << StyleInfo { .width {30}, .height{20} };
    e_lockicon << StyleInfo{.display{INLINE}, .overflow{AUTO,AUTO}} << "lock";
    uigps << StyleInfo { .height{92}, .overflow{AUTO,AUTO} } << "GPS";
    uiimu << StyleInfo { .height{92} } << "IMU";
    StyleInfo logstyle = { .display{INLINE}, .width {62}, .height{40}, .margin{1} };
    
    uisysinfo << screenremaining;
    sysinfoscreen << uisysinfo;

    e_sawlog << logstyle << "saw";
    e_voltlog << logstyle << "volts"; 
    e_sinelog << logstyle << "sine";
    e_squarelog << logstyle << "square";

    StyleInfo gaslogstyle = { .height{32}, .margin{1,0,0,0} };
    e_colog << gaslogstyle << "CO";
    e_nh3log << gaslogstyle << "NH3";
    e_no2log << gaslogstyle << "NO2";
    e_pbatterylog << StyleInfo { .display{INLINE}, .width {96}, .height{32}, .margin{1,1,0,0} } << "VBAT";
    e_pbatterylogst << StyleInfo { .display{INLINE}, .width {30}, .height{32}, .margin{1,0,0,0} } << "VBAT";
    gasscreen << e_colog << e_nh3log << e_pbatterylog << e_pbatterylogst;//e_no2log;

    uigpslt << StyleInfo { .display{INLINE}, .width {96}, .height{96} } << "GPSLT";
    uigpsst << StyleInfo { .display{INLINE}, .width {32}, .height{32} } << "GPSST";
    gpsview << uigpslt << uigpsst;

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

    clockscreen.show_header = false;

    header << uibattery;
    header << e_lockicon;
    header << uidatetime;
    //uiroot << header;
    //uiroot << uiclock;
    //uiroot << screenclock;
    //uiroot << screenclocknoheader;
    clockscreen << uiclock;
    mainscreen << txt;
    mainscreen.set_left(clockscreen);
    mainscreen.set_right(settingscreen);
    settingscreen.set_right(gpsscreen);
    gpsscreen.set_right(imuscreen);
    imuscreen.set_right(sysinfoscreen);
    sysinfoscreen.set_right(gasscreen);
    gasscreen.set_right(gpsview);

    uiroot.set_header(header);
    //uiroot.set_screen(mainscreen);
    //uiroot.set_screen(settingscreen);
    //uiroot.set_screen(gpsscreen);
    //uiroot.set_screen(imuscreen);
    //uiroot.set_screen(sysinfoscreen);
    //uiroot.set_screen(gasscreen);
    uiroot.set_screen(gpsview);

    WBL_D("Screen set");

    uiroot.dispatch(EventTypes::CONTENT_SIZE);
    uiroot.resolve_layout();

    WBL_D("First layout");

    for (int i = 0; i < 3; i++)
        wbl_system.getBatteryVoltageMean();
    uibattery.set_battery_level(wbl_system.getBatteryLevel());

    WBL_D("UI endinit");
}

extern "C" {
void app_main() {
    if (wbl_system.init() != ESP_OK) {
        printf("Failed to initialize system\n");
        goto end;
    }
    puts("Initialized system");
    if (mics6814.init() != ESP_OK) {
        printf("Failed to initialize mics6814\n");
        goto end;
    }
    puts("Initialized MiCS-6814");
    wbl_system.beginHapticFeedback(0.5, 500);
    dpad.init();
    puts("Initialized DPAD");
    if (gps.init() != ESP_OK) {
        printf("Failed to initialize gps\n");
        goto end;
    }
    puts("Initialized GPS");
    if (gpsimu.init() != ESP_OK) {
        printf("Failed to initialize imu\n");
        goto end;
    }
    puts("Initialized IMU");
    if (ltr390.init() != ESP_OK) {
        printf("Failed to initialize ltr390\n");
        goto end;
    }
    puts("Initialized LTR390");
    if (sdcard.init() != ESP_OK) {
        printf("Failed to initialize sdcard\n");
        goto end;
    }
    puts("Initialized SDCard");
    if (wbl::log.init() != ESP_OK) {
        printf("Failed to initialize log\n");
        goto end;
    }
    puts("Initialized log");
    init();
    puts("Initialized UI");
    if (display.init() != ESP_OK) {
        printf("Failed to initialize display\n");
        goto end;
    } else {
        printf("Display initialized\n");
        display.clear(0);
        wbl_system.setDisplayRotation(2);
        display.flush();
        gps.update();
        gps.setSystemTime();
        displayTimeout.update(true);
        while (1) {
            demo();
            vPortYield();
        }
    }

    end:;

    fflush(stdout);
    
    delay(1000);

    esp_restart();
}
}
