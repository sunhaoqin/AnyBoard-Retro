#include "boot_ui.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "lvgl.h"

LV_IMG_DECLARE(boot_res)

static lv_obj_t* boot_page = NULL;
static TaskHandle_t boot_ui_task_h = NULL;
static const char* TAG = "boot_ui";

static void shutdown_boot_window(void)
{
   if (boot_page != NULL)
   {
       lv_obj_del(boot_page);
       boot_page = NULL;
       ESP_LOGI(TAG,"real shutdown_boot_window");
   }
   ESP_LOGI(TAG,"shutdown_boot_window");
}

void start_boot_window(void)
{
    if (boot_page != NULL)
        return;

    static lv_style_t bg;
    lv_style_copy(&bg, &lv_style_plain);
    bg.body.main_color = LV_COLOR_BLACK;
    bg.body.grad_color = LV_COLOR_BLACK;
    bg.body.border.width = 0;
    bg.body.radius = 0;
    bg.body.shadow.width = 0;

    boot_page = lv_page_create(lv_layer_top(), NULL);
    lv_obj_set_size(boot_page, LV_HOR_RES, LV_VER_RES);
    lv_page_set_style(boot_page, LV_PAGE_STYLE_BG, &bg);
    lv_page_set_style(boot_page, LV_PAGE_STYLE_SCRL, &bg);
    lv_page_set_sb_mode(boot_page, LV_SB_MODE_OFF);

    lv_obj_t* img = lv_img_create(boot_page, NULL);
    lv_img_set_src(img, &boot_res);
    lv_obj_set_pos(img, 0, 0);

    ESP_LOGI(TAG, "show boot window");
}



