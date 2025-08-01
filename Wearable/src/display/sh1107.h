#pragma once

#include "sh1107_defs.h"
#include "config.h"
#include "i2c.h"

#include <inttypes.h>

namespace wbl {
namespace SH1107 {

template<uint8_t _WIDTH, uint8_t _HEIGHT, uint16_t _i2c_addr, uint32_t _i2c_freq>
struct Display : public I2C {
    constexpr Display():I2C(I2C_BUS_0, _i2c_addr, 1000, _i2c_freq, 0){}

    static constexpr const char *TAG = "wbl::SH1107::Display";
    static constexpr uint8_t WIDTH = _WIDTH;
    static constexpr uint8_t HEIGHT = _HEIGHT;
    static constexpr uint8_t PAGES = ((HEIGHT + 7) / 8);
    static constexpr uint8_t BYTES_PER_PAGE = WIDTH;

    uint8_t vcom;
    uint8_t discharge_period;
    uint8_t precharge_period;
    uint8_t dcdc_switching_scale;
    uint8_t dcdc_source;
    uint8_t oscillator_divide;
    uint8_t oscillator_scale;
    uint8_t scanning_mode;
    uint8_t remap_mode;
    uint8_t address_mode;
    uint8_t display_contrast;
    uint8_t display_state;
    uint8_t display_mode;
    
    enum {
        FLIP_NONE = 0,
        FLIP_HORIZONTAL = 1,
        FLIP_VERTICAL = 2,
        COLOR_NORMAL = 4,
        COLOR_INVERT = 8,
        DISPLAY_OFF = 16,
        DISPLAY_ON = 32,
    };

    inline esp_err_t setPagePosition(const uint8_t &page, const uint8_t &page_start=0, const uint8_t &page_start_offset=0) {
        const uint8_t cmd[] = {
            SH1107::SET_PAGEADDR + page, 0x10 + ((page_start + page_start_offset) >> 4), (page_start + page_start_offset) & 0xf
        };
        ESP_RETURN_ON_ERROR(this->write_commands(cmd, sizeof(cmd)), TAG, "setPagePosition failed");

        return ESP_OK;
    }
  
    inline esp_err_t clearDisplay(const uint8_t &color=0x0) {
        const uint8_t size = 32;
        uint8_t dc = 0x40;
        uint8_t clearData[size];
        for (uint8_t i = 0; i < size; i++)
            clearData[i] = color;
        for (uint8_t p = 0; p < PAGES; p++) {
            uint8_t bytes_remaining = BYTES_PER_PAGE;
            ESP_RETURN_ON_ERROR(this->setPagePosition(p), TAG, "clearDisplay setPagePosition failed");
            while (bytes_remaining>0) {
                const uint8_t count = bytes_remaining > size ? size : bytes_remaining;
                ESP_RETURN_ON_ERROR(this->write_payload(clearData, count, &dc, 1), TAG, "clearDisplay write_payload failed");
                bytes_remaining -= count;
            }
        }

        return ESP_OK;
    }

    inline esp_err_t setDisplayMode(const uint8_t &mode) {
        if (display_mode == mode)
            return ESP_OK;

        display_mode = mode;

        return this->write_command(mode);
    }

    inline esp_err_t setInverted(const bool &inverted_colors = false) {
        return this->setDisplayMode(inverted_colors ? SH1107::INVERT : SH1107::NORMAL);
    }

    inline esp_err_t setDisplayState(const uint8_t &state) {
        if (display_state == state)
            return ESP_OK;

        display_state = state;

        return this->write_command(state);
    }

    inline esp_err_t setState(const bool &on) {
        return setDisplayState(on ? SH1107::ON : SH1107::OFF);
    }

    inline bool isDisplayOn() {
        return display_state != SH1107::OFF;
    }

    inline esp_err_t setContrast(const uint8_t &contrast = 0x7f) {
        if (display_contrast == contrast) return ESP_OK;
        
        display_contrast = contrast;
        return this->write_command(SH1107::SET_CONTRAST, contrast);
    }

    inline esp_err_t setOrientation(const uint8_t &flags = 0) {
        uint8_t scan_direction = SH1107::COMSCANINC;
        uint8_t remap = SH1107::SEGREMAP;

        if (flags & FLIP_HORIZONTAL)
            scan_direction = SH1107::COMSCANDEC;
        if (flags & FLIP_VERTICAL)
            remap = SH1107::SEGREMAPINV;

        ESP_RETURN_ON_ERROR(setRemappingMode(remap), TAG, "setRemappingMode failed");
        ESP_RETURN_ON_ERROR(setScanningMode(scan_direction), TAG, "setScanningMode failed");

        return ESP_OK;
    }

    inline esp_err_t setDisplay(const uint8_t &flags = 0) {
        ESP_RETURN_ON_ERROR(this->setInverted(flags & COLOR_INVERT), TAG, "setInverted failed");
        ESP_RETURN_ON_ERROR(this->setState(!(flags & DISPLAY_OFF)), TAG, "setState failed");
        ESP_RETURN_ON_ERROR(this->setOrientation(flags), TAG, "setOrientation failed");

        return ESP_OK;
    }

    inline esp_err_t setAddressingMode(const uint8_t &mode) {
        if (mode == address_mode)
            return ESP_OK;

        address_mode = mode;

        return this->write_command(mode);
    }

    inline esp_err_t setScanningMode(const uint8_t &mode) {
        if (mode == scanning_mode)
            return ESP_OK;

        scanning_mode = mode;

        return this->write_command(mode);
    }

    inline esp_err_t setRemappingMode(const uint8_t &mode) {
        if (mode == remap_mode)
            return ESP_OK;

        remap_mode = mode;

        return this->write_command(mode);
    }

    inline esp_err_t setChargePeriod(const uint8_t &discharge, const uint8_t &precharge) {
        if (precharge == precharge_period && discharge == discharge_period)
            return ESP_OK;

        precharge_period = precharge;
        discharge_period = discharge;
        
        uint8_t reg = (precharge & 0x0F) | (discharge << 4);

        return this->write_command(SH1107::SET_PRECHARGE, reg);
    }

    inline esp_err_t setOscillator(const uint8_t &oscillator_frequency, const uint8_t &divide_ration) {
        if (divide_ration == oscillator_divide && oscillator_frequency == oscillator_scale)
            return ESP_OK;

        oscillator_divide = divide_ration;
        oscillator_scale = oscillator_frequency;

        uint8_t reg = (divide_ration & 0x0F) | (oscillator_frequency << 4);

        return this->write_command(SH1107::SET_CLOCKDIV, reg);
    }

    inline esp_err_t setVCOM(const uint8_t &vcom) {
        if (this->vcom == vcom)
            return ESP_OK;

        bool modify_state = isDisplayOn();
        uint8_t prev_state = display_state;

        this->vcom = vcom;

        if (modify_state) setState(false);
        this->write_command(SH1107::SET_VCOM, vcom);
        if (modify_state) setDisplayState(prev_state);

        return ESP_OK;
    }

    inline esp_err_t setDCDC(const uint8_t dcdc_source, const uint8_t &dcdc_switching_scale) {
        if (this->dcdc_switching_scale == dcdc_switching_scale && this->dcdc_source == dcdc_source)
            return ESP_OK;

        this->dcdc_switching_scale = dcdc_switching_scale;
        this->dcdc_source = dcdc_source;

        uint8_t reg = (dcdc_source & 1) | ((dcdc_switching_scale & 7) << 1) | 0x80;

        bool modify_state = display_state != SH1107::OFF;
        uint8_t prev_state = display_state;

        if (modify_state)
            ESP_RETURN_ON_ERROR(this->setState(false), TAG, "Failed to turn off display in setDCDC");

        ESP_RETURN_ON_ERROR(this->write_command(SH1107::SET_DCDC, reg), TAG, "Failed to write dcdc in setDCDC");

        if (modify_state)
            ESP_RETURN_ON_ERROR(this->setDisplayState(prev_state), TAG, "Failed to turn on display in setDCDC");

        return ESP_OK;
    }

    inline float getDisplayCurrentDraw() {
        if (!isDisplayOn())
            return 0.00000001; // 0.01uA

        const float power = float(display_contrast) / 256.0f;
        const float sf = 32.0f;
        const float iref = 0.000015625f * sf; // Iref * 32 = 15.625uA * 32

        const float idd2 = 0.000190; // Idd2 190uA
        const float ipp = 0.001;     // Ipp  1mA

        return power * iref + idd2 + ipp;
    }

    inline float getDisplayOscillatorFrequency() {
        const float base = 720000.0f;

        const float mult = 1.0f+((int(oscillator_scale * 5) - 25) * 0.01f);
        const float div = oscillator_divide + 1;

        return (base * mult) / div;
    }

    inline float getDisplayRefreshFrequency() {
        const int dclks = (precharge_period == 0 ? 2 : precharge_period) + discharge_period;

        return (getDisplayOscillatorFrequency() / float(HEIGHT)) / float(dclks);
    }

    inline float getVoltageCOM() {
        return float(vcom) * 0.006415f + 0.430f;
    }

    inline esp_err_t reset() {
        //ESP_RETURN_ON_ERROR(this->write_commands(SH1107::initcmds, sizeof(SH1107::initcmds)), TAG, "initcmds failed");

        using namespace SH1107;

        setDisplayState(OFF);
        setOscillator(OSCILLATOR_0, DIVIDE_RATION_POR);
        setAddressingMode(MEMORYMODE);
        setContrast(0x7f);
        setDCDC(DCDC_BUILTIN, DCDC_SWITCH_POR);
        setRemappingMode(SEGREMAP);
        setScanningMode(COMSCANINC);
        setVCOM(VCOM_POR);
        this->write_command(SET_DISPLAYSTARTLINE, 0, SET_OFFSET, 0, SET_MUX, 0x7f, ALL_OFF);
        setDisplayMode(NORMAL);
        setDisplayState(ON);

        return ESP_OK;
    }

    inline esp_err_t init() {
        ESP_RETURN_ON_ERROR(I2C::init(), TAG, "i2c_device init failed");

        return this->reset();
    }
};

}
}