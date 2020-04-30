#include "board.h"

#include "esp_log.h"
#include "audio_mem.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "periph_spi_sdcard.h"
#include "periph_button.h"
#include "periph_adc_button.h"
#include "input_key_service.h"
#include "periph_lcd.h"
#include "lvgl_main.h"

static const char *TAG = "ANY_BOARD";

static board_handle_t board_handle = NULL;

board_handle_t board_init(void)
{
    if (board_handle) {
        ESP_LOGW(TAG, "The board has already been initialized!");
        return board_handle;
    }
    board_handle = (board_handle_t) audio_calloc(1, sizeof(struct board_handle));
    AUDIO_MEM_CHECK(TAG, board_handle, return NULL);

    return board_handle;
}

esp_err_t board_lcd_init(esp_periph_set_handle_t set)
{
    esp_periph_handle_t lcd_handle = periph_lcd_init();
    AUDIO_NULL_CHECK(TAG, lcd_handle, return ESP_ERR_ADF_MEMORY_LACK);

    return esp_periph_start(set, lcd_handle);
}

esp_err_t board_key_init(esp_periph_set_handle_t set)
{

    periph_button_cfg_t btn_cfg = {
        .gpio_mask = (1ULL << BUTTON_SELECT_ID)
            | (1ULL << BUTTON_START_ID)
            | (1ULL << BUTTON_A_ID)
            | (1ULL << BUTTON_B_ID)
            | (1ULL << BUTTON_MENU_ID)
            | (1ULL << BUTTON_VOLUME_ID),
        .long_press_time_ms = 2000
    };
    esp_periph_handle_t button_handle = periph_button_init(&btn_cfg);
    AUDIO_NULL_CHECK(TAG, button_handle, return ESP_ERR_ADF_MEMORY_LACK);

    esp_err_t ret = ESP_OK;
    ret = esp_periph_start(set, button_handle);
    if (ret != ESP_OK) {
        return ret;
    }

    int adc_level_step[] = {300, 2000, 4000};

    adc_arr_t adc_arr[] = {
        {
            .adc_ch = BUTTON_X_ID,
            .adc_level_step = adc_level_step,
            .total_steps = 2,
            .press_judge_time = 2000
        },
        {
            .adc_ch = BUTTON_Y_ID,
            .adc_level_step = adc_level_step,
            .total_steps = 2,
            .press_judge_time = 2000
        }
    };
    periph_adc_button_cfg_t adc_btn_cfg = {
        .arr = adc_arr,
        .arr_size = 2
    };
    esp_periph_handle_t adc_btn_handle = periph_adc_button_init(&adc_btn_cfg);
    AUDIO_NULL_CHECK(TAG, adc_btn_handle, return ESP_ERR_ADF_MEMORY_LACK);

    ret = esp_periph_start(set, adc_btn_handle);

    return ret;
}

esp_err_t board_sdcard_init(esp_periph_set_handle_t set)
{
    periph_sdcard_cfg_t sdcard_cfg = {
        .root = "/sdcard",
        .card_detect_pin = get_sdcard_intr_gpio(),
    };
    esp_periph_handle_t sdcard_handle = periph_spi_sdcard_init(&sdcard_cfg);
    esp_err_t ret = esp_periph_start(set, sdcard_handle);
    while (!periph_sdcard_is_mounted(sdcard_handle)) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    return ret;
}

board_handle_t board_get_handle(void)
{
    return board_handle;
}

esp_err_t board_deinit(board_handle_t audio_board)
{
    esp_err_t ret = ESP_OK;

    if (audio_board->audio_hal != NULL) {
        ret = audio_hal_deinit(audio_board->audio_hal);
    }
    free(audio_board);
    board_handle = NULL;
    return ret;
}
