#ifndef _WINDOW_MAIN_H_
#define _WINDOW_MAIN_H_

#include "esp_err.h"

esp_err_t lvgl_init();
void lvgl_lock();
void lvgl_unlock();

#endif
