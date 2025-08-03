#include "sdcard.h"

#include "esp_check.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <stdio.h>

namespace wbl {

static constexpr const char *TAG = "wbl::sdcard.cpp";

struct SPI {
    spi_host_device_t host;
    gpio_num_t mosi, miso, clk;
    int max_transfer_size;
    spi_dma_chan_t dma;

    constexpr SPI(const spi_host_device_t &host,
                  const gpio_num_t &mosi,
                  const gpio_num_t &miso,
                  const gpio_num_t &clk,
                  const int &max_transfer_size=4000,
                  const spi_dma_chan_t &dma=SPI_DMA_CH_AUTO):
        host(host),
        mosi(mosi),
        miso(miso),
        clk(clk),
        max_transfer_size(max_transfer_size),
        dma(dma) {}

    esp_err_t init() {
        spi_bus_config_t bus_cfg = {
            .mosi_io_num = mosi,
            .miso_io_num = miso,
            .sclk_io_num = clk,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = max_transfer_size,
        };

        ESP_RETURN_ON_ERROR(spi_bus_initialize(host, &bus_cfg, dma), TAG, "Failed to create SPI bus");

        return ESP_OK;
    }
};

struct SDCardDev {
    SPI &spi;

    const char *mount_point;
    gpio_num_t cs_pin;
    const int max_open_files;

    sdmmc_card_t *card = nullptr;

    uint64_t total_size;

    constexpr SDCardDev(SPI &spi,
                        const char *mount_point,
                        const gpio_num_t &cs_pin = GPIO_NUM_NC,
                        const int &max_open_files = 10):
        spi(spi),
        mount_point(mount_point),
        cs_pin(cs_pin),
        max_open_files(max_open_files) {}

    esp_err_t init() {
        esp_vfs_fat_mount_config_t mount_cfg = {
            .format_if_mount_failed = false,
            .max_files = max_open_files,
            .allocation_unit_size = 0,
        };

        sdspi_device_config_t sdspi_cfg = SDSPI_DEVICE_CONFIG_DEFAULT();
        sdmmc_host_t host = SDSPI_HOST_DEFAULT();

        sdspi_cfg.gpio_cs = cs_pin;
        sdspi_cfg.host_id = spi.host;

        ESP_RETURN_ON_ERROR(esp_vfs_fat_sdspi_mount(mount_point, &host, &sdspi_cfg, &mount_cfg, &card), TAG, "Failed to mount sd card");

        uint64_t dummy;
        ESP_RETURN_ON_ERROR(esp_vfs_fat_info(mount_point, &total_size, &dummy), TAG, "Could not get SD card info");

        return ESP_OK;
    }

    uint64_t getFreeSize() {
        uint64_t dummy, ret=0;
        esp_vfs_fat_info(mount_point, &dummy, &ret);
        return ret;
    }

    uint64_t getTotalSize() {
        return total_size;
    }
};

SPI sdbus(SDCARD_SPI_HOST, SDCARD_SPI_MOSI, SDCARD_SPI_MISO, SDCARD_SPI_CLK);
SDCardDev sddev(sdbus, "/sdcard", SDCARD_SPI_CS, 5);
SDCard sdcard;

esp_err_t SDCard::init() {
    ESP_RETURN_ON_ERROR(sdbus.init(), TAG, "Failed to init sdcard bus");
    ESP_RETURN_ON_ERROR(sddev.init(), TAG, "Failed to init sdcard");

    return ESP_OK;
}

uint64_t SDCard::getFreeSize() {
    return sddev.getFreeSize();
}

uint64_t SDCard::getTotalSize() {
    return sddev.getTotalSize();
}

}