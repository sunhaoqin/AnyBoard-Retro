#include "board.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "ab_mem.h"

#include "esp_peripherals.h"
#include "periph_spi_sdcard.h"
#include "periph_button.h"
#include "periph_adc_button.h"
#include "periph_lcd.h"
#include "periph_service.h"
#include "input_key_service.h"

static const char *TAG = "BOARD";

static board_handle_t s_board = NULL;

static esp_err_t input_key_service_cb(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx) {
    ESP_LOGI(TAG, "[ * ] input key id is %d", (int)evt->data);
    return ESP_OK;
}

board_handle_t board_init(void) {
    if (s_board) {
        ESP_LOGW(TAG, "The board has already been initialized!");
        return s_board;
    }
    board_handle_t board = (board_handle_t) ab_calloc(1, sizeof(struct board_handle));
    AB_NULL_CHECK(TAG, board, return NULL);
    s_board = board;

    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    esp_periph_set_handle_t periph_set = esp_periph_set_init(&periph_cfg);
    AB_NULL_CHECK(TAG, periph_set, goto failed);

    s_board->periph_set = periph_set;
    return s_board;

failed:
    board_deinit(s_board);
    return NULL;
}

esp_err_t board_lcd_init() {
    AB_NULL_CHECK(TAG, s_board, return ESP_FAIL);

    esp_periph_handle_t lcd_handle = periph_lcd_init();
    AB_NULL_CHECK(TAG, lcd_handle, return ESP_ERR_NO_MEM);

    return esp_periph_start(s_board->periph_set, lcd_handle);
}

esp_err_t board_key_init() {
    AB_NULL_CHECK(TAG, s_board, return ESP_FAIL);

    esp_err_t ret = ESP_OK;

    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    periph_cfg.task_prio = 20;
    esp_periph_set_handle_t input_periph_set = esp_periph_set_init(&periph_cfg);
    AB_NULL_CHECK(TAG, input_periph_set, return ESP_ERR_NO_MEM);

    input_key_service_info_t input_key_info[] = INPUT_KEY_DEFAULT_INFO();
    periph_service_handle_t input_service = input_key_service_create(input_periph_set);
    AB_NULL_CHECK(TAG, input_service, goto failed);
    ret = input_key_service_add_key(input_service, input_key_info, INPUT_KEY_NUM);
    AB_CHECK(TAG, ret == ESP_OK, goto failed, "input key service add key failed");
    periph_service_set_callback(input_service, input_key_service_cb, NULL);

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
    AB_NULL_CHECK(TAG, button_handle, {ret = ESP_ERR_NO_MEM; goto failed;});
    ret = esp_periph_start(input_periph_set, button_handle);
    AB_CHECK(TAG, ret == ESP_OK, goto failed, "button start failed");


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
    AB_NULL_CHECK(TAG, adc_btn_handle, {ret = ESP_ERR_NO_MEM; goto failed;});
    ret = esp_periph_start(input_periph_set, adc_btn_handle);
    AB_CHECK(TAG, ret == ESP_OK, goto failed, "adc button start failed");

    s_board->input_periph_set = input_periph_set;
    s_board->input_service = input_service;
    return ret;

failed:
    if (input_service != NULL) {
        periph_service_destroy(input_service);
    }
    esp_periph_set_destroy(input_periph_set);
    return ret;
}

esp_err_t board_sdcard_init() {
    AB_NULL_CHECK(TAG, s_board, return ESP_FAIL);

    periph_sdcard_cfg_t sdcard_cfg = {
        .root = "/sdcard",
        .card_detect_pin = get_sdcard_intr_gpio(),
    };
    esp_periph_handle_t sdcard_handle = periph_spi_sdcard_init(&sdcard_cfg);
    AB_NULL_CHECK(TAG, sdcard_handle, return ESP_ERR_NO_MEM);

    esp_err_t ret = esp_periph_start(s_board->periph_set, sdcard_handle);
    while (!periph_sdcard_is_mounted(sdcard_handle)) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    return ret;
}

board_handle_t board_get_handle(void) {
    return s_board;
}

esp_err_t board_deinit(board_handle_t board) {
    if (s_board->periph_set != NULL) {
        esp_periph_set_destroy(s_board->periph_set);
    }

    if (s_board->input_periph_set != NULL) {
        esp_periph_set_destroy(s_board->input_periph_set);
    }

    if (s_board->input_service != NULL) {
        periph_service_destroy(s_board->input_service);
    }

    ab_free(board);

    s_board = NULL;
    return ESP_OK;
}
