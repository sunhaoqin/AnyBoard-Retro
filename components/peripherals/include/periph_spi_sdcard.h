#ifndef _AB_SDCARD_DEV_H_
#define _AB_SDCARD_DEV_H_

#include "sys/queue.h"
#include "audio_error.h"
#include "audio_common.h"
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

#ifdef __cplusplus
}
#endif

#endif
