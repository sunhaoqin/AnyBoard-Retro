#ifndef _AB_MEM_H_
#define _AB_MEM_H_

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AB_MEM_SHOW(x)  ab_mem_print(x, __LINE__, __func__)

#define AB_CHECK(TAG, a, action, msg) if (!(a)) {                                 	\
        ESP_LOGE(TAG,"%s:%d (%s): %s", __FILENAME__, __LINE__, __FUNCTION__, msg);  \
        action;                                                                   	\
        }

#define AB_MEM_CHECK(TAG, a, action)  AB_CHECK(TAG, a, action, "Memory exhausted")

#define AB_NULL_CHECK(TAG, a, action) AB_CHECK(TAG, a, action, "Got NULL Pointer")

/**
 * @brief   Malloc memory
 *
 * @param[in]  size   memory size
 *
 * @return
 *     - valid pointer on success
 *     - NULL when any errors
 */
void *ab_malloc(size_t size);

/**
 * @brief   Free memory
 *
 * @param[in]  ptr  memory pointer
 *
 * @return
 *     - void
 */
void ab_free(void *ptr);

/**
 * @brief  Malloc memory, if spi ram is enabled, it will malloc memory in the spi ram
 *
 * @param[in]  nmemb   number of block
 * @param[in]  size    block memory size
 *
 * @return
 *     - valid pointer on success
 *     - NULL when any errors
 */
void *ab_calloc(size_t nmemb, size_t size);

/**
 * @brief   Malloc memory, it will malloc to internal memory
 *
 * @param[in] nmemb   number of block
 * @param[in]  size   block memory size
 *
 * @return
 *     - valid pointer on success
 *     - NULL when any errors
 */
void *ab_calloc_inner(size_t nmemb, size_t size);

/**
 * @brief   Print heap memory status
 *
 * @param[in]  tag    tag of log
 * @param[in]  line   line of log
 * @param[in]  func   function name of log
 *
 * @return
 *     - void
 */
void ab_mem_print(const char *tag, int line, const char *func);

/**
 * @brief  Reallocate memory, if spi ram is enabled, it will allocate memory in the spi ram
 *
 * @param[in]  ptr   memory pointer
 * @param[in]  size  block memory size
 *
 * @return
 *     - valid pointer on success
 *     - NULL when any errors
 */
void *ab_realloc(void *ptr, size_t size);

/**
 * @brief   Duplicate given string.
 *
 *          Allocate new memory, copy contents of given string into it and return the pointer
 *
 * @param[in]  str   String to be duplicated
 *
 * @return
 *     - Pointer to new malloc'ed string
 *     - NULL otherwise
 */
char *ab_strdup(const char *str);

#ifdef __cplusplus
}
#endif

#endif