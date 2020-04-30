#include <freertos/FreeRTOS.h>
#include <string.h>
#include "ab_mem.h"
#include "sdkconfig.h"
#include "esp_heap_caps.h"

void* ab_malloc_inner(size_t size)
{
    return heap_caps_malloc(size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
}

void* ab_malloc_extra(size_t size)
{
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);   
}


void* ab_malloc(size_t size)
{
#if CONFIG_SPIRAM_BOOT_INIT
    return ab_malloc_extra(size);
#else
    return ab_malloc_inner(size);
#endif
}


void *ab_realloc_inner(void *ptr, size_t size)
{
    return heap_caps_realloc(ptr, size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
}

void *ab_realloc_extra(void *ptr, size_t size)
{
    return heap_caps_realloc(ptr, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

void *ab_realloc(void *ptr, size_t size)
{
#if CONFIG_SPIRAM_BOOT_INIT
    return ab_realloc_extra(ptr, size);
#else
    return ab_realloc_inner(ptr, size);
#endif
}

void* ab_calloc_inner(size_t nmemb, size_t size)
{
    void *data = heap_caps_malloc(nmemb * size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    if (data) 
        memset(data, 0, nmemb *size);
    return data;
}

void* ab_calloc_extra(size_t nmemb, size_t size)
{
    void *data = heap_caps_malloc(nmemb * size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (data) 
        memset(data, 0, nmemb *size);
    return data;
}


void* ab_calloc(size_t nmemb, size_t size)
{
#if CONFIG_SPIRAM_BOOT_INIT
    return ab_calloc_extra(nmemb, size);
#else
    return ab_calloc_inner(nmemb, size);
#endif    
}


void ab_free(void *ptr)
{
    free(ptr);
}
