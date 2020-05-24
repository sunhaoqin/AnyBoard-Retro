#ifndef _AB_SDCARD_DEV_H_
#define _AB_SDCARD_DEV_H_

#include "esp_peripherals.h"
#include "periph_sdcard.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief      Create the sdcard peripheral handle for esp_peripherals
 *
 * @note       The handle was created by this function automatically destroy when `esp_periph_destroy` is called
 *
 * @param      sdcard_config  The sdcard configuration
 *
 * @return     The esp peripheral handle
 */
esp_periph_handle_t periph_spi_sdcard_init(periph_sdcard_cfg_t* sdcard_config);

/**
 * @brief      Check the sdcard is mounted or not.
 *
 * @param[in]  periph  The periph
 *
 * @return     SDCARD mounted state
 */
bool periph_spi_sdcard_is_mounted(esp_periph_handle_t periph);

#ifdef __cplusplus
}
#endif

#endif
