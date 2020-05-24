#include <stdio.h>
#include <dirent.h>
#include <string.h>

#include "board.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nvs_flash.h"
#include "lvgl_main.h"

#include "esp_log.h"

#include "ab_mem.h"

#include "esp_ota_ops.h"

#include "lvgl.h"

static const char* TAG = "ad_main";

static void flash_rom(char *filename) {
    ESP_LOGE(TAG, "flash_rom!");

    char *root_path = "/sdcard/";
    size_t root_path_len = strlen(root_path);
    size_t filename_len = strlen(filename);

    char *file_path = (char *)ab_calloc(root_path_len + filename_len + 1, sizeof(char));
    strcpy(file_path, root_path);
    strncat(file_path, filename, filename_len);

    FILE* file = fopen(file_path, "rb");
    ab_free(file_path);
    if (file == NULL) {
        ESP_LOGE(TAG, "open file failed!");
        return;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size =  ftell(file);
    fseek(file, 0, SEEK_SET);

    // esp_partition_t *running_partition =  esp_ota_get_running_partition();
    // esp_partition_t *next_update_partition = esp_ota_get_next_update_partition(running_partition);
    esp_partition_t *any_app_partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_2, "any_app");
    ESP_LOGI(TAG, "update_partition : %s", any_app_partition->label);
    esp_ota_handle_t ota_handle = NULL;
    esp_ota_begin(any_app_partition, file_size, &ota_handle);



    int buffer_size = 20 * 1024;
    uint8_t *buffer = ab_malloc(sizeof(uint8_t) * buffer_size);

    size_t read_count = 0;

    while(file_size > buffer_size) {
        size_t read_count = fread(buffer, sizeof(uint8_t), buffer_size, file);
        if (read_count != buffer_size) {
            ESP_LOGE(TAG, "read file failed!");
        }

        esp_ota_write(ota_handle, buffer, sizeof(uint8_t) * read_count);

        file_size -= read_count;
    }

    if (file_size > 0) {
        size_t read_count = fread(buffer, sizeof(uint8_t), buffer_size, file);

        if (read_count > 0) {
            esp_ota_write(ota_handle, buffer, sizeof(uint8_t) * read_count);
        }
    }

    ab_free(buffer);
    esp_ota_end(ota_handle);

    esp_app_desc_t app_desc = {0};
    esp_ota_get_partition_description(any_app_partition, &app_desc);
    ESP_LOGI(TAG, "project_name = %s!", app_desc.project_name);
    

    esp_ota_set_boot_partition(any_app_partition);

    ESP_LOGI(TAG, "Prepare to restart system!");

    vTaskDelay(3000 / portTICK_RATE_MS);

    esp_restart();
}

static esp_err_t read_dir_filename(const char* path, const char* extension, char* arr_filename[], size_t arr_size) {

    if (extension == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    DIR *dir = opendir(path);
    if (dir == NULL) {
        ESP_LOGE(TAG, "opendir failed!");
        return ESP_FAIL;
    }

    size_t ext_len = strlen(extension);

    esp_err_t ret = ESP_OK;
    struct dirent *entry;
    char file_ext[10];
    for (int i = 0; i < arr_size; ) {
        entry = readdir(dir);
        if (entry == NULL) {
            break;
        }

        if (entry->d_name[0] == '.') {
            continue;
        }

        if (ext_len != 0) {
            size_t file_name_len = strlen(entry->d_name);
            strncpy(file_ext, entry->d_name + file_name_len - ext_len, ext_len + 1);
            if (strcmp(extension, file_ext) != 0) {
                continue;
            }
        }

        char *filename = (char*)ab_strdup(entry->d_name);
        if (filename == NULL) {
            ret = ESP_ERR_NO_MEM;
            break;
        }

        arr_filename[i] = filename;
        i++;
    }

    closedir(dir);
    return ret;
 }

static void on_btn_click(lv_obj_t *btn, lv_event_t event) {
    if(event == LV_EVENT_CLICKED) {
        lv_obj_t * text = lv_obj_get_child(btn, NULL);
        char *filename = lv_label_get_text(text);
        flash_rom(filename);
    }
}

void app_main(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    board_init();
    ESP_ERROR_CHECK(board_lcd_init());
    ESP_ERROR_CHECK(board_key_init());
    ESP_ERROR_CHECK(board_sdcard_init());

    ESP_ERROR_CHECK(lvgl_init());
    lvgl_lock();

    lv_obj_clean(lv_layer_top());
    lv_obj_t *list_view = lv_list_create(lv_scr_act(), NULL);
    lv_obj_set_size(list_view, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_group_add_obj(lvgl_get_group(), list_view);
    lvgl_unlock();

    char *arr_filename[20] = {0};
    esp_err_t ret = read_dir_filename("/sdcard", "bin", arr_filename, 20);

    if (ret == ESP_OK) {
        for (int i = 0; i < 20; i++) {
            if (arr_filename[i] != NULL) {
                ESP_LOGI(TAG, "filename[%d] = %s", i, arr_filename[i]);
                lvgl_lock();
                lv_obj_t * btn = lv_list_add_btn(list_view, NULL, arr_filename[i]);
                lv_obj_set_event_cb(btn, on_btn_click);
                lv_obj_t * text = lv_obj_get_child(btn, NULL);
                lv_label_set_align(text, LV_LABEL_ALIGN_CENTER);
                lvgl_unlock();
                ab_free(arr_filename[i]);
            }
        }
    }else {
        ESP_LOGE(TAG, "read_dir_filename error! %s", esp_err_to_name(ret));
    }

    while(true) {        
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}