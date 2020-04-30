#include "periph_lcd.h"
#include "periph_id.h"
#include "esp_peripherals.h"
#include "lcd.h"

#include "esp_log.h"

static const char* TAG = "PERIPH_LCD";

static esp_err_t _lcd_init(esp_periph_handle_t self)
{
	return lcd_init();
}

static esp_err_t _lcd_destroy(esp_periph_handle_t self)
{
    return ESP_OK;
}

esp_periph_handle_t periph_lcd_init(){
	esp_periph_handle_t lcd = esp_periph_create(PERIPH_ID_LCD, "periph_spi_lcd");
    AUDIO_MEM_CHECK(TAG, lcd, return NULL);

    esp_periph_set_function(lcd, _lcd_init, NULL, _lcd_destroy);

    return lcd;
}

void periph_lcd_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, void* color_map){
	lcd_flush(x1, y1, x2, y2, color_map);
}