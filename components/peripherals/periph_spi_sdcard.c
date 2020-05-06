/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2018 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "periph_sdcard.h"

#include "esp_log.h"

#include "board.h"
#include "spi_sdcard.h"

static const char *TAG = "PERIPH_SPI_SDCARD";

#define VALIDATE_SDCARD(periph, ret) if (!(periph && esp_periph_get_id(periph) == PERIPH_ID_SDCARD)) { \
    ESP_LOGE(TAG, "Invalid SDCARD periph, at line %d", __LINE__);\
    return ret;\
}

typedef struct {
    char *root;
    int card_detect_pin;
    bool is_mounted;
} periph_spi_sdcard_t;

static esp_err_t periph_sdcard_mount(esp_periph_handle_t periph)
{
    VALIDATE_SDCARD(periph, ESP_FAIL);

    periph_spi_sdcard_t *sdcard = esp_periph_get_data(periph);

    int ret = spi_sdcard_mount(sdcard->root);
    if (ret == ESP_OK) {
        ESP_LOGD(TAG, "Mount SDCARD success");
        sdcard->is_mounted = true;
        return esp_periph_send_event(periph, SDCARD_STATUS_MOUNTED, NULL, 0);
    } else if (ret == ESP_ERR_INVALID_STATE) {
        ESP_LOGD(TAG, "periph sdcard handle already mounted!");
        return ESP_OK;
    } else {
        esp_periph_send_event(periph, SDCARD_STATUS_MOUNT_ERROR, NULL, 0);
        sdcard->is_mounted = false;
        ESP_LOGE(TAG, "mount sdcard error!");
        return ESP_FAIL;
    }
}

static esp_err_t periph_sdcard_unmount(esp_periph_handle_t periph)
{
    VALIDATE_SDCARD(periph, ESP_FAIL);
    periph_spi_sdcard_t *sdcard = esp_periph_get_data(periph);
    int ret = spi_sdcard_unmount(sdcard->root);
    if (ret == ESP_OK) {
        ESP_LOGD(TAG, "UnMount SDCARD success");
        sdcard->is_mounted = false;
        return esp_periph_send_event(periph, SDCARD_STATUS_UNMOUNTED, NULL, 0);
    } else {
        esp_periph_send_event(periph, SDCARD_STATUS_UNMOUNT_ERROR, NULL, 0);
        ESP_LOGE(TAG, "unmount sdcard error!");
        sdcard->is_mounted = false;
        return ESP_FAIL;
    }
    return ESP_OK;
}

static esp_err_t _sdcard_init(esp_periph_handle_t self)
{
    periph_spi_sdcard_t *sdcard = esp_periph_get_data(self);
    esp_err_t ret = spi_sdcard_init(sdcard->card_detect_pin, NULL, self);
    if (spi_sdcard_is_exist()) {
        ret |= periph_sdcard_mount(self);
    } else {
        ESP_LOGE(TAG, "no sdcard detect");
    }
    return ESP_OK;
}

static esp_err_t _sdcard_destroy(esp_periph_handle_t self)
{
    VALIDATE_SDCARD(self, ESP_FAIL);
    esp_err_t ret = ESP_OK;

    periph_spi_sdcard_t *sdcard = esp_periph_get_data(self);

    ret |= spi_sdcard_unmount(sdcard->root);
    ret |= spi_sdcard_destroy();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "stop sdcard error!");
    }
    free(sdcard->root);
    free(sdcard);
    return ret;
}

esp_periph_handle_t periph_spi_sdcard_init(periph_sdcard_cfg_t *sdcard_cfg)
{

    esp_periph_handle_t periph = esp_periph_create(PERIPH_ID_SDCARD, "periph_spi_sdcard");
    AUDIO_MEM_CHECK(TAG, periph, return NULL);

    periph_spi_sdcard_t *sdcard = calloc(1, sizeof(periph_spi_sdcard_t));
    AUDIO_MEM_CHECK(TAG, sdcard, return NULL);
    if (sdcard_cfg->root) {
        sdcard->root = strdup(sdcard_cfg->root);
    } else {
        sdcard->root = strdup("/sdcard");
    }
    AUDIO_MEM_CHECK(TAG, sdcard->root, {
        free(sdcard);
        return NULL;
    });

    sdcard->card_detect_pin = sdcard_cfg->card_detect_pin;
    esp_periph_set_data(periph, sdcard);
    esp_periph_set_function(periph, _sdcard_init, NULL, _sdcard_destroy);
    return periph;
}
