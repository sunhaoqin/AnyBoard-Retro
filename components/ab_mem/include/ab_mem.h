#ifndef _AB_MEM_H_
#define _AB_MEM_H_

#include <stddef.h>


void* ab_malloc(size_t size);

void *ab_realloc(void *ptr, size_t size);

void* ab_calloc(size_t nmemb, size_t size);

void ab_free(void *ptr);

#endif