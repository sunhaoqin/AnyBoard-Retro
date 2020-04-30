#ifndef _AB_LCD_H_
#define _AB_LCD_H_

#include <stdint.h>
#include "esp_err.h"

esp_err_t lcd_init();
esp_err_t lcd_set_brightness(int brightness);
void lcd_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, void* color_map);

#endif