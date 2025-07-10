#include <stdio.h>
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_chip_info.h"

void app_main() {
    printf("test\n");

    uint32_t flash_size;
    esp_flash_get_size(0, &flash_size);
    printf("%iMB flash size\n", flash_size / (1024 * 1024));
    
    fflush(stdout);
}