#include <stdlib.h>
#include "string.h"
#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_log.h"
#include "ab_mem.h"
#include "esp_heap_caps.h"


void *ab_malloc(size_t size) 
{
    void *data =  NULL;
#if CONFIG_SPIRAM_BOOT_INIT
    data = heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
#else
    data = malloc(size);
#endif
    return data;
}

void ab_free(void *ptr)
{
    free(ptr);
}

void *ab_calloc(size_t nmemb, size_t size)
{
    void *data =  NULL;
#if CONFIG_SPIRAM_BOOT_INIT
    data = heap_caps_malloc(nmemb * size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (data) {
        memset(data, 0, nmemb * size);
    }
#else
    data = calloc(nmemb, size);
#endif
    return data;
}

void *ab_realloc(void *ptr, size_t size)
{
    void *p = NULL;
#if CONFIG_SPIRAM_BOOT_INIT
    p = heap_caps_realloc(ptr, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
#else
    p = heap_caps_realloc(ptr, size, MALLOC_CAP_8BIT);
#endif
    return p;
}

char *ab_strdup(const char *str)
{
#if CONFIG_SPIRAM_BOOT_INIT
    char *copy = heap_caps_malloc(strlen(str) + 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
#else
    char *copy = malloc(strlen(str) + 1);
#endif
    if (copy) {
        strcpy(copy, str);
    }
    return copy;
}

void *ab_calloc_inner(size_t n, size_t size)
{
    void *data =  NULL;
    data = heap_caps_malloc(n * size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (data) {
        memset(data, 0, n * size);
    }
    return data;
}

void ab_mem_print(const char *tag, int line, const char *func)
{
#ifdef CONFIG_SPIRAM_BOOT_INIT
    ESP_LOGI(tag, "Func:%s, Line:%d, MEM Total:%d Bytes, Inter:%d Bytes, Dram:%d Bytes\r\n", func, line, esp_get_free_heap_size(),
             heap_caps_get_free_size(MALLOC_CAP_INTERNAL), heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
#else
    ESP_LOGI(tag, "Func:%s, Line:%d, MEM Total:%d Bytes\r\n", func, line, esp_get_free_heap_size());
#endif
}