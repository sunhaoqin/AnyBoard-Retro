#ifndef _ST7789_H
#define _ST7789_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "esp_err.h"
#include "driver/spi_master.h"

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/
#define ST7789_HOR_RES          (240)
#define ST7789_VER_RES          (240)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

esp_err_t st7789_init(spi_device_handle_t spi_device);
esp_err_t st7789_deinit(void);

esp_err_t st7789_flush_locating(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
esp_err_t st7789_flash_line(uint16_t *line, int width, int lineCount);
esp_err_t st7789_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color);
esp_err_t st7789_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, void* color_map);

/**********************
 *      MACROS
 **********************/


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*_ST7789_H*/
