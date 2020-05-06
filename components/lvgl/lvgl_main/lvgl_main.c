#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <sys/unistd.h>
#include <sys/stat.h>

#include "lvgl_main.h"

#include "board.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_task.h"
#include "esp_log.h"

// #include "screen_touch.h"
// #include "periph_screen_touch.h"
#include "periph_lcd.h"
#include "periph_service.h"
#include "input_key_service.h"

#include "esp_log.h"
#include "boot_ui.h"

static const char* TAG = "win_main";
static const int PLUTO_GUI_STOP_BIT = BIT0;

static void lvgl_task(void *pvParameter);
static void lcd_flush_cb(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
static esp_err_t input_key_service_cb(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx);
static bool keypad_read_cb(lv_indev_drv_t * drv, lv_indev_data_t * data);
static void lv_log_print(lv_log_level_t level, const char * file_path, uint32_t line_number, const char * description);
static void lcd_flash_monitor_cb(struct _disp_drv_t * disp_drv, uint32_t time, uint32_t px);
static lv_fs_res_t pcfs_open(lv_fs_drv_t *fs_drv, FILE **file_p, const char *path, lv_fs_mode_t mode);
static lv_fs_res_t pcfs_close(lv_fs_drv_t *fs_drv, FILE **file_p);
static lv_fs_res_t pcfs_read(lv_fs_drv_t *fs_drv, FILE **file_p, void *buf, uint32_t btr, uint32_t *br);
static lv_fs_res_t pcfs_seek(lv_fs_drv_t *fs_drv, FILE **file_p, uint32_t pos);
static lv_fs_res_t pcfs_tell(lv_fs_drv_t *fs_drv, FILE **file_p, uint32_t *pos_p);
static lv_fs_res_t pcfs_size(lv_fs_drv_t *fs_drv, FILE **file_p, uint32_t *size_p);

static bool s_lvgl_init = false;
static SemaphoreHandle_t s_lv_mutex;

static bool s_is_notify_task_stop = false;
static EventGroupHandle_t s_event_group;
static TaskHandle_t s_lvgl_task;
static lv_group_t *s_lv_group;

esp_err_t lvgl_init(){
    if (!s_lvgl_init) {

        s_lv_mutex = xSemaphoreCreateRecursiveMutex();

        s_event_group = xEventGroupCreate();
        if (s_event_group == NULL) {
            return ESP_ERR_NO_MEM;
        } 

        lv_init();

        lv_theme_set_current(lv_theme_mono_init(100, NULL));

        lv_log_register_print_cb(lv_log_print);

        /*Create a display buffer*/
        static lv_disp_buf_t disp_buf;
        static lv_color_t buf_1[LV_HOR_RES_MAX * 24];
        lv_disp_buf_init(&disp_buf, buf_1, NULL, LV_HOR_RES_MAX * 24);

        /*Create a display*/
        lv_disp_drv_t disp_drv;
        lv_disp_drv_init(&disp_drv);
        disp_drv.buffer = &disp_buf;
        disp_drv.flush_cb = lcd_flush_cb;
        // disp_drv.monitor_cb = lcd_flash_monitor_cb;

        lv_disp_t * disp = lv_disp_drv_register(&disp_drv);
        if (disp == NULL) {
            return ESP_ERR_NO_MEM;
        }

        lv_indev_drv_t indev_drv;
        lv_indev_drv_init(&indev_drv);
        indev_drv.type = LV_INDEV_TYPE_KEYPAD;
        indev_drv.read_cb = keypad_read_cb;
        lv_indev_t *keypad_indev = lv_indev_drv_register(&indev_drv);
        if (keypad_indev == NULL) {
            return ESP_ERR_NO_MEM;
        }
        s_lv_group = lv_group_create();
        if (s_lv_group == NULL) {
            return ESP_ERR_NO_MEM;
        }
        lv_group_set_wrap(s_lv_group, true);
        lv_indev_set_group(keypad_indev, s_lv_group);
        periph_service_set_callback(board_get_handle()->input_service, input_key_service_cb, NULL);

        lv_fs_drv_t pcfs_drv;
        lv_fs_drv_init(&pcfs_drv);
        pcfs_drv.letter = 'E';
        pcfs_drv.file_size = sizeof(FILE*);
        pcfs_drv.open_cb = pcfs_open;
        pcfs_drv.close_cb = pcfs_close;
        pcfs_drv.read_cb = pcfs_read;
        pcfs_drv.seek_cb = pcfs_seek;
        pcfs_drv.tell_cb = pcfs_tell;
        pcfs_drv.size_cb = pcfs_size;
        lv_fs_drv_register(&pcfs_drv);

        // start_boot_window();

        if (xTaskCreatePinnedToCore(lvgl_task, "lvgl_task", 1024*5, NULL, 4, &s_lvgl_task, 1) != pdPASS) {
            ESP_LOGE(TAG, "create lvgl task error!");
            return ESP_FAIL;
        }

        s_lvgl_init = true;
    }

    return ESP_OK;
}

void lvgl_lock() {
    xSemaphoreTakeRecursive(s_lv_mutex, portMAX_DELAY);
}

void lvgl_unlock() {
    xSemaphoreGiveRecursive(s_lv_mutex);
}

lv_group_t * lvgl_get_group() {
    return s_lv_group;
}

static void lvgl_task(void *pvParameter) {
    while(true) {

        if (s_is_notify_task_stop) {
            s_is_notify_task_stop = false;
            xEventGroupSetBits(s_event_group, PLUTO_GUI_STOP_BIT);
            vTaskDelay(100/ portTICK_PERIOD_MS);
            continue;
        }

        if (!(xEventGroupGetBits(s_event_group) & PLUTO_GUI_STOP_BIT)) {
            lvgl_lock();
            lv_task_handler();
            lvgl_unlock();
        }

        vTaskDelay(10/ portTICK_PERIOD_MS);
    }
}

static void lcd_flush_cb(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p) {
    periph_lcd_flush(area->x1, area->y1, area->x2, area->y2, (void *)color_p);
    lv_disp_flush_ready(disp_drv);
}

static lv_key_t last_key = 0;
static lv_indev_state_t last_key_state = LV_INDEV_STATE_REL;
static esp_err_t input_key_service_cb(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx) {

    input_key_user_id_t key_id = (input_key_user_id_t)evt->data;

    switch(key_id) {
        case INPUT_KEY_USER_ID_XY_1:
            switch(evt->len) {
                case BUTTON_X_ID:
                    last_key = LV_KEY_LEFT;
                    break;
                case BUTTON_Y_ID:
                    last_key = LV_KEY_UP;
                    break;
            }
            break;
        case INPUT_KEY_USER_ID_XY_2:
            switch(evt->len) {
                case BUTTON_X_ID:
                    last_key = LV_KEY_RIGHT;
                    break;
                case BUTTON_Y_ID:
                    last_key = LV_KEY_DOWN;
                    break;
            }
            break;
        case INPUT_KEY_USER_ID_SELECT:
            if (evt->type == INPUT_KEY_SERVICE_ACTION_CLICK_RELEASE) {
                ESP_LOGI(TAG, "input key select!");
            }
            break;
        case INPUT_KEY_USER_ID_START:
            if (evt->type == INPUT_KEY_SERVICE_ACTION_CLICK_RELEASE) {
                ESP_LOGI(TAG, "input key start!");
            }
            break;
        case INPUT_KEY_USER_ID_A:
            last_key = LV_KEY_ENTER;
            break;
        case INPUT_KEY_USER_ID_B:
            last_key = LV_KEY_ESC;
            break;
        case INPUT_KEY_USER_ID_MENU:
            if (evt->type == INPUT_KEY_SERVICE_ACTION_CLICK_RELEASE) {
                ESP_LOGI(TAG, "input key menu!");
            }
            break;
        case INPUT_KEY_USER_ID_VOLUME:
            if (evt->type == INPUT_KEY_SERVICE_ACTION_CLICK_RELEASE) {
                ESP_LOGI(TAG, "input key volume!");
            }
            break;
        default:
            return ESP_OK;
    }

    switch(evt->type) {
        case INPUT_KEY_SERVICE_ACTION_CLICK:
        case INPUT_KEY_SERVICE_ACTION_PRESS:
            last_key_state = LV_INDEV_STATE_PR;
            break;
        case INPUT_KEY_SERVICE_ACTION_CLICK_RELEASE:
        case INPUT_KEY_SERVICE_ACTION_PRESS_RELEASE:
            last_key_state = LV_INDEV_STATE_REL;
            break;
    }

    return ESP_OK;
}

static bool keypad_read_cb(lv_indev_drv_t * drv, lv_indev_data_t * data) {
    data->key = last_key;
    data->state = last_key_state;
    return false;
}

static void lv_log_print(lv_log_level_t level, const char * file_path, uint32_t line_number, const char * description) {
    switch(level) {
        case LV_LOG_LEVEL_TRACE:
            ESP_LOGD("LVGL", "%s", description);
            break;
        case LV_LOG_LEVEL_INFO:
            ESP_LOGI("LVGL", "%s", description);
            break;
        case LV_LOG_LEVEL_WARN:
            ESP_LOGW("LVGL", "%s", description);
            break;
        case LV_LOG_LEVEL_ERROR:
            ESP_LOGE("LVGL", "%s", description);
            break;
    }
}

static void lcd_flash_monitor_cb(struct _disp_drv_t * disp_drv, uint32_t time, uint32_t px) {
    ESP_LOGW("LVGL", "flash --> tiem = %d px = %d", time, px);
}

static lv_fs_res_t pcfs_open(lv_fs_drv_t *fs_drv, FILE **file_p, const char *path, lv_fs_mode_t mode) {
    const char * flags = "";

    if(mode == LV_FS_MODE_WR) flags = "wb";
    else if(mode == LV_FS_MODE_RD) flags = "rb";
    else if(mode == (LV_FS_MODE_WR | LV_FS_MODE_RD)) flags = "a+";
    else return LV_FS_RES_INV_PARAM;

    char *real_path = path - 1;
    FILE* fd = fopen(real_path, flags);
    if((long int)fd <= 0) {
        return LV_FS_RES_UNKNOWN;
    }

    fseek(fd, 0, SEEK_SET);

    *file_p = fd;

    return LV_FS_RES_OK;
}

static lv_fs_res_t pcfs_close(lv_fs_drv_t *fs_drv, FILE **file_p) {
    fclose(*file_p);
    return LV_FS_RES_OK;
}

static lv_fs_res_t pcfs_read(lv_fs_drv_t *fs_drv, FILE **file_p, void * buf, uint32_t btr, uint32_t *br) {
    FILE *file = *file_p;
    *br = read(fileno(file), buf, btr);
    return LV_FS_RES_OK;
}

static lv_fs_res_t pcfs_seek(lv_fs_drv_t *fs_drv, FILE **file_p, uint32_t pos) {
    fseek(*file_p, pos, SEEK_SET);
    return LV_FS_RES_OK;
}

static lv_fs_res_t pcfs_tell(lv_fs_drv_t *fs_drv, FILE **file_p, uint32_t * pos_p) {
    *pos_p = ftell(*file_p);
    return LV_FS_RES_OK;
}

static lv_fs_res_t pcfs_size(lv_fs_drv_t *fs_drv, FILE **file_p, uint32_t * size_p) {
    FILE *file = *file_p;
    if(fseek(file, 0, SEEK_END) != 0) {
        return LV_FS_RES_UNKNOWN;
    }
    uint32_t size = ftell(file);
    /* It may give LONG_MAX as directory size, this is invalid for us. */
    if(size == LONG_MAX) size = -1;
    *size_p = size;
    fseek(file, 0, SEEK_SET);
    return LV_FS_RES_OK;
}