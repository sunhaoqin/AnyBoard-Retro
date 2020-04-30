#ifndef _BOARD_H_
#define _BOARD_H_

#include "board_def.h"
#include "board_pins_config.h"
#include "esp_peripherals.h"
#include "periph_service.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief board handle
 */
struct board_handle {
    esp_periph_set_handle_t 	periph_set;
    esp_periph_set_handle_t 	input_periph_set;
    periph_service_handle_t 	input_service;
};

typedef struct board_handle *board_handle_t;

/**
 * @brief Initialize board
 *
 * @return The board handle
 */
board_handle_t board_init(void);


/**
 * @brief Initialize lcd peripheral
 *
 * @return 
 *     - ESP_OK, success
 *     - Others, fail
 */
esp_err_t board_lcd_init(void);

/**
 * @brief Initialize key peripheral
 *
 * @return
 *     - ESP_OK, success
 *     - Others, fail
 */
esp_err_t board_key_init(void);

/**
 * @brief Initialize sdcard peripheral
 *
 * @return
 *     - ESP_OK, success
 *     - Others, fail
 */
esp_err_t board_sdcard_init(void);

/**
 * @brief Query board_handle
 *
 * @return The board handle
 */
board_handle_t board_get_handle(void);

/**
 * @brief Uninitialize the board
 *
 * @param board The handle of board
 *
 * @return  ESP_OK  success,
 *          others  fail
 */
esp_err_t board_deinit(board_handle_t board);

#ifdef __cplusplus
}
#endif

#endif
