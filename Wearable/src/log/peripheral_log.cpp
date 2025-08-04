#include "peripheral_log.h"

#include "esp_check.h"
#include "esp_heap_caps.h"
#include <malloc.h>

namespace wbl {

static constexpr const char *TAG = "wbl::peripheral_log.cpp";

PeripheralLog log;

template<typename DL, typename DS = typename DL::storage_type>
void make_dl(DL &log, DS *data_storage) {
    log = DL(data_storage);
}

template<typename DL, typename DS = typename DL::storage_type>
esp_err_t make_ext(DL &log) {
    DS *storage = nullptr;
    storage = heap_caps_malloc(sizeof(DS), MALLOC_CAP_SPIRAM);

    if (storage == nullptr)
        return ESP_ERR_NO_MEM;

    make_dl(log, storage);

    return ESP_OK;
}

DLBatteryST::storage_type ds_bat_st;
DLMICS6814ST::storage_type ds_mics_st;
DLCAMM8ST::storage_type ds_camm8_st;

#define LOGALLOC(x) ESP_RETURN_ON_ERROR(make_ext(x), TAG, "Failed to alloc")

esp_err_t PeripheralLog::init() {
    battery_st.set_log(ds_bat_st);
    mics6814_st.set_log(ds_mics_st);
    camm8_st.set_log(ds_camm8_st);

    LOGALLOC(battery_lt);
    LOGALLOC(mics6814_lt);
    LOGALLOC(camm8_lt);
    LOGALLOC(camm8_lt2);

    return ESP_OK;
}

#undef LOGALLOC

esp_err_t PeripheralLog::update() {
    return ESP_OK;
}

int64_t PeripheralLog::getNextPollTime() {
    return 0;
}

}