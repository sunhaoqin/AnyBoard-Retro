#include <string.h>

#include "board.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_peripherals.h"
#include "periph_service.h"
#include "input_key_service.h"
#include "nvs_flash.h"
#include "lvgl_main.h"

#include "esp_log.h"

#include "st7789.h"

static const char* TAG = "ad_main";

static esp_periph_set_handle_t s_periph_set = NULL;
static periph_service_handle_t s_input_service = NULL;

static esp_err_t input_key_service_cb(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx) {
    ESP_LOGI(TAG, "[ * ] input key id is %d", (int)evt->data);
    return ESP_OK;
}

void app_main(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    board_init();
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    s_periph_set = esp_periph_set_init(&periph_cfg);
    // board_sdcard_init(s_periph_set);
    board_lcd_init(s_periph_set);
	board_key_init(s_periph_set);
    input_key_service_info_t input_key_info[] = INPUT_KEY_DEFAULT_INFO();
    s_input_service = input_key_service_create(s_periph_set);
    input_key_service_add_key(s_input_service, input_key_info, INPUT_KEY_NUM);
    periph_service_set_callback(s_input_service, input_key_service_cb, NULL);

	lvgl_init();

    vTaskDelay(2000 / portTICK_RATE_MS);

    uint16_t line[320];

    memset(line, 0xff, sizeof(uint16_t) * 320);

    st7789_flush_locating(0, 0, 320 - 1, 240 - 1);

    for (int i = 0; i < 240; ++i)
    {
       st7789_flash_line(line, 320, 1);
       vTaskDelay(100 / portTICK_RATE_MS);
    }

    while(true) {        
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}