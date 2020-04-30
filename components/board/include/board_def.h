/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2019 <ESPRESSIF SYSTEMS (SHANGHAI) CO., LTD>
 *
 * Permission is hereby granted for use on all ESPRESSIF SYSTEMS products, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef _AUDIO_BOARD_DEFINITION_H_
#define _AUDIO_BOARD_DEFINITION_H_

#define SPI_CLOCK_RATE           40000000
#define SPI_PIN_NUM_MOSI         GPIO_NUM_23
#define SPI_PIN_NUM_MISO         GPIO_NUM_19
#define SPI_PIN_NUM_CLK          GPIO_NUM_18
#define SPI_PIN_NUM_CS           GPIO_NUM_5

#define LCD_PIN_NUM_DC           GPIO_NUM_21
#define LCD_PIN_NUM_BCKL         GPIO_NUM_14

#define SDCARD_OPEN_FILE_NUM_MAX    5
#define SDCARD_PIN_NUM_MOSI         SPI_PIN_NUM_MOSI
#define SDCARD_PIN_NUM_MISO         SPI_PIN_NUM_MISO
#define SDCARD_PIN_NUM_CLK          SPI_PIN_NUM_CLK
#define SDCARD_PIN_NUM_CS           GPIO_NUM_22
#define SDCARD_INTR_GPIO            -1

#define BUTTON_X_ID ADC1_CHANNEL_6
#define BUTTON_Y_ID ADC1_CHANNEL_7
#define BUTTON_SELECT_ID GPIO_NUM_27
#define BUTTON_START_ID  GPIO_NUM_39
#define BUTTON_A_ID GPIO_NUM_32
#define BUTTON_B_ID GPIO_NUM_33
#define BUTTON_MENU_ID GPIO_NUM_13
#define BUTTON_VOLUME_ID GPIO_NUM_0

#define BUTTON_REC_ID             GPIO_NUM_36
#define BUTTON_MODE_ID            GPIO_NUM_39
#define BUTTON_SET_ID             TOUCH_PAD_NUM9
#define BUTTON_PLAY_ID            TOUCH_PAD_NUM8
#define BUTTON_VOLUP_ID           TOUCH_PAD_NUM7
#define BUTTON_VOLDOWN_ID         TOUCH_PAD_NUM4

#define AUXIN_DETECT_GPIO         GPIO_NUM_12
#define HEADPHONE_DETECT          GPIO_NUM_19
#define PA_ENABLE_GPIO            GPIO_NUM_21

#define GREEN_LED_GPIO            GPIO_NUM_22

extern audio_hal_func_t AUDIO_CODEC_ES8388_DEFAULT_HANDLE;

#define AUDIO_CODEC_DEFAULT_CONFIG(){                   \
        .adc_input  = AUDIO_HAL_ADC_INPUT_LINE1,        \
        .dac_output = AUDIO_HAL_DAC_OUTPUT_ALL,         \
        .codec_mode = AUDIO_HAL_CODEC_MODE_BOTH,        \
        .i2s_iface = {                                  \
            .mode = AUDIO_HAL_MODE_SLAVE,               \
            .fmt = AUDIO_HAL_I2S_NORMAL,                \
            .samples = AUDIO_HAL_48K_SAMPLES,           \
            .bits = AUDIO_HAL_BIT_LENGTH_16BITS,        \
        },                                              \
};

#define INPUT_KEY_NUM     8

#define INPUT_KEY_DEFAULT_INFO() {                       \
    {                                                    \
        .type = PERIPH_ID_ADC_BTN,                       \
        .user_id = 1,                \
        .act_id = 0,                           \
    },                                                   \
    {                                                    \
        .type = PERIPH_ID_ADC_BTN,                       \
        .user_id = 2,               \
        .act_id = 1,                           \
    },                                                   \
    {                                                    \
        .type = PERIPH_ID_BUTTON,                        \
        .user_id = 3,                \
        .act_id = BUTTON_SELECT_ID,                      \
    },                                                   \
    {                                                    \
        .type = PERIPH_ID_BUTTON,                        \
        .user_id = 4,               \
        .act_id = BUTTON_START_ID,                       \
    },                                                   \
    {                                                    \
        .type = PERIPH_ID_BUTTON,                        \
        .user_id = 5,              \
        .act_id = BUTTON_A_ID,                           \
    },                                                   \
    {                                                    \
        .type = PERIPH_ID_BUTTON,                        \
        .user_id = 6,            \
        .act_id = BUTTON_B_ID,                           \
    },                                                   \
    {                                                    \
        .type = PERIPH_ID_BUTTON,                        \
        .user_id = 7,            \
        .act_id = BUTTON_MENU_ID,                        \
    },                                                   \
    {                                                    \
        .type = PERIPH_ID_BUTTON,                        \
        .user_id = 8,            \
        .act_id = BUTTON_VOLUME_ID,                        \
    },                                                   \
}

#endif
