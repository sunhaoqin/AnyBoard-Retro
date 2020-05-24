#ifndef _PERIPH_LCD_H__
#define _PERIPH_LCD_H__

#include <stdint.h>
#include "esp_peripherals.h"
#include "esp_err.h"

esp_periph_handle_t periph_lcd_init();

void periph_lcd_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, void* color_map);

esp_err_t set_brightness(int brightness);

#endif