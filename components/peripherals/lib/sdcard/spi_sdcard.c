#include "spi_sdcard.h"

#include "esp_log.h"

#include "board.h"
#include "esp_vfs_fat.h"
#include "driver/sdspi_host.h"
#include "driver/sdmmc_types.h"
#include "driver/sdmmc_defs.h"
#include "driver/gpio.h"

static const char *TAG = "SPI_SDCARD";

static int s_gpio = -1;

static sdmmc_card_t *s_sdcard;

static void sdmmc_card_print_info(const sdmmc_card_t *card)
{
    ESP_LOGD(TAG, "Name: %s\n", card->cid.name);
    ESP_LOGD(TAG, "Type: %s\n", (card->ocr & SD_OCR_SDHC_CAP) ? "SDHC/SDXC" : "SDSC");
    ESP_LOGD(TAG, "Speed: %s\n", (card->csd.tr_speed > 25000000) ? "high speed" : "default speed");
    ESP_LOGD(TAG, "Size: %lluMB\n", ((uint64_t) card->csd.capacity) * card->csd.sector_size / (1024 * 1024));
    ESP_LOGD(TAG, "CSD: ver=%d, sector_size=%d, capacity=%d read_bl_len=%d\n",
             card->csd.csd_ver,
             card->csd.sector_size, card->csd.capacity, card->csd.read_block_len);
    ESP_LOGD(TAG, "SCR: sd_spec=%d, bus_width=%d\n", card->scr.sd_spec, card->scr.bus_width);
}

esp_err_t spi_sdcard_mount(const char *base_path)
{
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = (gpio_num_t)SDCARD_PIN_NUM_CS;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = get_sdcard_open_file_num_max(),
    };

    ESP_LOGI(TAG, "Trying to mount with base path=%s", base_path);
    esp_err_t ret = esp_vfs_fat_sdspi_mount(base_path, &host, &slot_config, &mount_config, &s_sdcard);

    switch (ret) {
        case ESP_OK:
            // Card has been initialized, print its properties
            sdmmc_card_print_info(s_sdcard);
            ESP_LOGI(TAG, "CID name %s!\n", s_sdcard->cid.name);
            break;

        case ESP_ERR_INVALID_STATE:
            ESP_LOGE(TAG, "File system already mounted");
            break;

        case ESP_FAIL:
            ESP_LOGE(TAG, "Failed to mount filesystem. If you want the card to be formatted, set format_if_mount_failed = true.");
            break;

        default:
            ESP_LOGE(TAG, "Failed to initialize the card (%d). Make sure SD card lines have pull-up resistors in place.", ret);
            break;
    }

    return ret;
}


esp_err_t spi_sdcard_unmount(const char* base_path)
{
    esp_err_t ret = esp_vfs_fat_sdcard_unmount(base_path, &s_sdcard);

    if (ret == ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "File system not mounted");
    }
    return ret;
}

bool spi_sdcard_is_exist()
{
    if (s_gpio >= 0) {
        return (gpio_get_level(s_gpio) == 0x00);
    } else {
        return true;
    }
    return false;
}

int IRAM_ATTR spi_sdcard_read_detect_pin(void)
{
    if (s_gpio >= 0) {
        return gpio_get_level(s_gpio);
    } else {
        return -1;
    }
    return 0;
}

esp_err_t spi_sdcard_destroy()
{
    if (s_gpio >= 0) {
        return gpio_isr_handler_remove(s_gpio);
    }
    return ESP_OK;
}

esp_err_t spi_sdcard_init(int card_detect_pin, void (*detect_intr_handler)(void *), void *isr_context)
{
    esp_err_t ret = ESP_OK;
    if (card_detect_pin >= 0) {
        gpio_set_direction(card_detect_pin, GPIO_MODE_INPUT);
        if (detect_intr_handler) {
            gpio_set_intr_type(card_detect_pin, GPIO_INTR_ANYEDGE);
            gpio_isr_handler_add(card_detect_pin, detect_intr_handler, isr_context);
            gpio_intr_enable(card_detect_pin);
        }
        gpio_pullup_en(card_detect_pin);
    }
    s_gpio = card_detect_pin;
    return ret;
}