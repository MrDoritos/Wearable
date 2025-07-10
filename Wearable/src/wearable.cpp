#include <stdio.h>
#include <inttypes.h>
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_chip_info.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" {
void app_main() {
    printf("test\n");

    uint32_t flash_size;
    esp_flash_get_size(0, &flash_size);
    printf("%luMB flash size\n", flash_size / (1024 * 1024));
    
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    fflush(stdout);
    esp_restart();
}
}