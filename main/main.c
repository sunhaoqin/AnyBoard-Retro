#include <string.h>

#include "board.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nvs_flash.h"
#include "lvgl_main.h"

#include "esp_log.h"

static const char* TAG = "ad_main";

void app_main(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    ESP_ERROR_CHECK(board_init());
    // init sdcard must befor lcd, because our sdcard use spi
    ESP_ERROR_CHECK(board_sdcard_init());
    ESP_ERROR_CHECK(board_lcd_init());
    ESP_ERROR_CHECK(board_key_init());

	lvgl_init();

    while(true) {        
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}