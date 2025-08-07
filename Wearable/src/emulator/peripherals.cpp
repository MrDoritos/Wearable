#include "gps_imu.h"
#include "sdcard.h"
#include "mics6814.h"
#include "ltr390.h"
#include "peripheral_log.h"
#include "wbl_system.h"

namespace wbl {

GPSIMU gpsimu;

esp_err_t GPSIMU::init() { return ESP_OK; }
AxisData GPSIMU::getAccelerometer() {
    return AxisData(0.4, 0.2, 0.3);
}

AxisData GPSIMU::getGyroscope() {
    return AxisData(0.44, 0.22, 0.33);
}

esp_err_t GPSIMU::enablePedometer() { return ESP_OK; }
esp_err_t GPSIMU::disablePedometer() { return ESP_OK; }
esp_err_t GPSIMU::setPedometerState(uint8_t state) { return ESP_OK; }
esp_err_t GPSIMU::resetPedometer() { return ESP_OK; }
uint16_t GPSIMU::getPedometer() { return 155; }
uint8_t GPSIMU::getPedometerState() { return 1; }

SDCard sdcard;

esp_err_t SDCard::init() { return ESP_OK; }
uint64_t SDCard::getTotalSize() { return 10000000; }
uint64_t SDCard::getFreeSize() { return 1000000; }

MICS6814 mics6814;

esp_err_t MICS6814::init() { return ESP_OK; }
float MICS6814::getCOVoltage() { return 0.4; }
float MICS6814::getNO2Voltage() { return 0.9; }
float MICS6814::getNH3Voltage() { return 0.69; }

LTR390 ltr390;

esp_err_t LTR390::init() { return ESP_OK; }
uint32_t LTR390::getUVS() { return 40; }
uint32_t LTR390::getALS() { return 100; }
float LTR390::getLux() { return 55; }
float LTR390::getLux(const uint32_t &als) { return als; }
float LTR390::getUVI() { return 99; }
float LTR390::getUVI(const uint32_t &uvs) { return uvs; }
float LTR390::getUVIhr() { return 12; }
float LTR390::getUVIhr(const uint32_t &uvs) { return uvs; }
void LTR390::setState(const bool &state) {}
bool LTR390::isActive() { return 1; }
void LTR390::setMeasurementRate(const uint32_t &ms) {}
void LTR390::setGain(const uint8_t &gain) {}
void LTR390::setResolution(const uint8_t &resolution) {}
void LTR390::setMode(const bool &uv_mode) {}
uint32_t LTR390::getIntegrationTime() { return 11; }
int LTR390::getGainMultiplier() { return 3; }

PeripheralLog log;

esp_err_t PeripheralLog::init() { return ESP_OK; }
esp_err_t PeripheralLog::update() { return ESP_OK; }
int64_t PeripheralLog::getNextPollTime() { return 0; }
TimeState PeripheralLog::getTimeState() { return DONE; }
void PeripheralLog::pushOdometer() {}
void PeripheralLog::pushPedometer() {}

wbl_System wbl_system;

}