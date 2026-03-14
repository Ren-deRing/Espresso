#pragma once

#include <stddef.h>

#define KERNEL_HEAP_START 0xffffa00000000000
#define KERNEL_HEAP_SIZE  (128ULL * 1024 * 1024 * 1024)

void kmalloc_init();
void* kmalloc(size_t size);
void* kmalloc_aligned(size_t size, size_t align);
void kfree(void* ptr);