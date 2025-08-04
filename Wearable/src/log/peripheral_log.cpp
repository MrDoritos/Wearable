#include "peripheral_log.h"

namespace wbl {

PeripheralLog log;

template<typename DL, typename DS = typename DL::storage_type>
void make_dl(DL &log, DS *data_storage) {
    log = DL(data_storage);
}

template<typename DL, typename DS = typename DL::storage_type>
esp_err_t make_ext(DL &log) {
   // heap_caps_malloc(sizeof(T), )
   return ESP_OK;
}

esp_err_t PeripheralLog::init() {
    return ESP_OK;
}

esp_err_t PeripheralLog::update() {
    return ESP_OK;
}

int64_t PeripheralLog::getNextPollTime() {
    return 0;
}

}