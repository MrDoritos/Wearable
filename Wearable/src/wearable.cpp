#include <stdio.h>
#include <inttypes.h>
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_chip_info.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "displaybuffer.h"
#include "texture.h"

using namespace wbl;

using DisplayTexture = TextureT<DisplayBuffer>;

DisplayTexture display;

void demo() {
    const TickType_t ms=300;
    display.clearDisplay(0);
    printf("full\n");
    vTaskDelay(ms / portTICK_PERIOD_MS);
    display.clearDisplay(255);
    printf("clear 0\n");
    vTaskDelay(ms / portTICK_PERIOD_MS);
    display.clear(0);
    display.flush();
    printf("clear 1\n");
    vTaskDelay(ms / portTICK_PERIOD_MS);
    display.circle(64, 64, 24, 1, false);
    display.flush();
    printf("circle\n");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    display.line(0,0,127,127,1);
    display.flush();
    printf("line\n");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    display.clear(0);
    display.flush();
    printf("clear 0\n");
    vTaskDelay(ms / portTICK_PERIOD_MS);
    fflush(stdout);
}

extern "C" {
void app_main() {
    printf("test\n");

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