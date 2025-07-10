#include <stdio.h>
#include <inttypes.h>
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_chip_info.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "displaybuffer.h"
#include "texture.h"

extern const char _binary_fixedsys_bin_start[];
extern const char _binary_fixedsys_bin_end[];

using namespace wbl;

using DisplayTexture = TextureT<DisplayBuffer>;

using FontTexture = TextureT<FramebufferT<256, 256, 2>>;

FontTexture font;

DisplayTexture display;

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
    
    display.clear(0);
    /*
    display.flush();
    printf("clear 1\n");
    vTaskDelay(ms / portTICK_PERIOD_MS);
    */
    display.putTexture(font, 0, 0, 128, 128, 0, 0);
    display.flush();
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
    memcpy(font.buffer, &_binary_fixedsys_bin_start[0], _binary_fixedsys_bin_end - _binary_fixedsys_bin_start);

    if (display.init() != ESP_OK) {
        printf("Failed to initialize display\n");
    } else {
        printf("Display initialized\n");
        while (1) {
            demo();
            vPortYield();
        }
    }

    fflush(stdout);


    uint32_t flash_size;
    esp_flash_get_size(0, &flash_size);
    printf("%luMB flash size\n", flash_size / (1024 * 1024));
    
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    fflush(stdout);
    esp_restart();
}
}