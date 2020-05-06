#ifndef _WINDOW_MAIN_H_
#define _WINDOW_MAIN_H_

#include "esp_err.h"
#include "lvgl.h"

esp_err_t lvgl_init();
void lvgl_lock();
void lvgl_unlock();
lv_group_t * lvgl_get_group();

#endif
