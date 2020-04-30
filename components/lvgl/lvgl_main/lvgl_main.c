#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <sys/unistd.h>
#include <sys/stat.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_task.h"
#include "esp_log.h"

#include "lvgl.h"

// #include "screen_touch.h"
// #include "periph_screen_touch.h"
#include "periph_lcd.h"

#include "esp_log.h"
#include "boot_ui.h"

static const char* TAG = "win_main";
static const int PLUTO_GUI_STOP_BIT = BIT0;

// static void lvgl_sleep();
// static void lvgl_wakeup();
// static void register_lvgl_pm_client();
static void lvgl_task(void *pvParameter);
static void lcd_flush_cb(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
static bool touch_pad_read_cb(lv_indev_drv_t * drv, lv_indev_data_t * data);
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
static TaskHandle_t lvgl_task_h = NULL;
static TaskHandle_t win_task_h = NULL;
static lv_indev_t * gululu_kp_indev = NULL;

esp_err_t lvgl_init(){
    if (!s_lvgl_init) {

        s_lv_mutex = xSemaphoreCreateRecursiveMutex();

        s_event_group = xEventGroupCreate();
        if (s_event_group == NULL) {
            return ESP_ERR_NO_MEM;
        } 

        lv_init();

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

        // lv_indev_drv_t indev_drv;
        // lv_indev_drv_init(&indev_drv);
        // indev_drv.type = LV_INDEV_TYPE_POINTER;
        // indev_drv.read_cb = touch_pad_read_cb;
        // lv_indev_t * touch_indev = lv_indev_drv_register(&indev_drv);
        // if (touch_indev == NULL) {
        //     return ESP_ERR_NO_MEM;
        // }

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

        // lv_lodezip_init();

        // register_lvgl_pm_client();

        start_boot_window();

        if (xTaskCreatePinnedToCore(lvgl_task, "lvgl_task", 1024*5, NULL, 4, &lvgl_task_h, 1) != pdPASS) {
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

// static void lvgl_sleep() {
//     ESP_LOGI(TAG, "lvgl_sleep");
//     s_is_notify_task_stop = true;

//     EventBits_t uxBits = xEventGroupWaitBits(
//     s_event_group, 
//     PLUTO_GUI_STOP_BIT, 
//     false, 
//     true, 
//     5 * 1000 / portTICK_PERIOD_MS);

//     if ( !(uxBits & PLUTO_GUI_STOP_BIT) ) {
//         ESP_LOGW(TAG, "wait gui task stop timeout!");
//     }
// }

// static void lvgl_wakeup() {
//     ESP_LOGI(TAG, "lvgl_wakeup");
//     xEventGroupClearBits(s_event_group, PLUTO_GUI_STOP_BIT);
// }

// static void register_lvgl_pm_client(){
//     gll_pm_client_t client = {
//         .type = GLL_PM_CLIENT_TYPE_DISPLAY_APP,
//         .on_wakeup = lvgl_wakeup,
//         .on_sleep = lvgl_sleep,
//         .on_power_off = lvgl_sleep
//     };

//     s_lvgl_pm_client_handle = gll_pm_client_register(&client);
// }

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

// static bool touch_pad_read_cb(lv_indev_drv_t * drv, lv_indev_data_t * data){
//     static periph_screen_touch_raw_t raw_data = {
//         .state = TOUCH_RELEASE,
//     };

//     bool more_to_read = periph_screen_touch_readraw(&raw_data) > 0;

//     data->point.x = raw_data.x;
//     data->point.y = raw_data.y;

//     if(raw_data.state == TOUCH_PRESSED || raw_data.state == TOUCH_CONTACT) {
//         data->state = LV_INDEV_STATE_PR;
//     }
//     else {
//         data->state = LV_INDEV_STATE_REL;
//     }

//     return more_to_read;
    
// }

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