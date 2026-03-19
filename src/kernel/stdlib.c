#include "stdlib.h"
#include "heap.h"
#include "string.h"
#include "serial.h"
#include "errno.h"

int errno = 0;

void* malloc(size_t size) {
    return kmalloc(size);
}

void free(void* ptr) {
    kfree(ptr);
}

void* calloc(size_t nmemb, size_t size) {
    size_t total = nmemb * size;
    void* ptr = kmalloc(total);
    if (ptr) {
        memset(ptr, 0, total);
    }
    return ptr;
}

void* realloc(void* ptr, size_t size) {
    return krealloc(ptr, size);
}

int atoi(const char* str) {
    int res = 0;
    while (*str >= '0' && *str <= '9') {
        res = res * 10 + (*str - '0');
        str++;
    }
    return res;
}

void abort(void) {
    kprintf("WASM PANIC!\n");
    
    while (1) {
        __asm__ volatile ("hlt");
    }
}