#include <limine.h>

#include "tlsf.h"
#include "vmm.h"
#include "pmm.h"
#include "heap.h"

#include "serial.h"

static tlsf_t g_tlsf_handle = NULL;
static uint64_t g_heap_current_limit = KERNEL_HEAP_START;

static bool expand_heap(size_t size) {
    size_t aligned_size = ALIGN_UP(size, PAGE_SIZE);
    uint64_t start_addr = g_heap_current_limit;
    uint64_t allocated_pages_count = 0;

    for (uint64_t i = 0; i < aligned_size; i += PAGE_SIZE) {
        page_t* page = pmm_alloc_pages(0); 
        if (!page) {
            for (uint64_t j = 0; j < allocated_pages_count; j++) {
                uintptr_t vaddr = start_addr + (j * PAGE_SIZE);
                
                pt_entry_t* pte = vmm_get_pte(g_kernel_pagemap, vaddr, false);
                if (pte && (*pte & PAGE_PRESENT)) {
                    uint64_t phys = *pte & PAGE_ADDR_MASK;
                    pmm_free_pages(phys_to_page(phys), 0);
                }
                vmm_unmap(g_kernel_pagemap, vaddr);
            }
            return false;
        }

        uint64_t phys = page_to_phys(page);
        vmm_map(g_kernel_pagemap, start_addr + i, phys, PAGE_PRESENT | PAGE_WRITABLE | PAGE_NX);
        allocated_pages_count++;
    }

    if (!g_tlsf_handle) {
        g_tlsf_handle = tlsf_create_with_pool((void*)start_addr, aligned_size);
    } else {
        tlsf_add_pool(g_tlsf_handle, (void*)start_addr, aligned_size);
    }

    g_heap_current_limit += aligned_size;
    return true;
}

void kmalloc_init() {
    expand_heap(4 * 1024 * 1024);
}

void* kmalloc(size_t size) {
    void* ptr = tlsf_malloc(g_tlsf_handle, size);
    
    if (!ptr) {
        size_t expansion_size = ALIGN_UP(size + 4096, 1024 * 1024);
        if (expand_heap(expansion_size)) {
            ptr = tlsf_malloc(g_tlsf_handle, size);
        }
    }
    return ptr;
}

void* kmalloc_aligned(size_t size, size_t align) {
    void* ptr = tlsf_memalign(g_tlsf_handle, align, size);
    
    if (!ptr) {
        size_t expansion_size = ALIGN_UP(size + align + 4096, 1024 * 1024);
        if (expand_heap(expansion_size)) {
            ptr = tlsf_memalign(g_tlsf_handle, align, size);
        }
    }
    return ptr;
}

void* krealloc(void* ptr, size_t size) {
    if (!ptr) return kmalloc(size);
    if (size == 0) {
        kfree(ptr);
        return NULL;
    }

    void* new_ptr = tlsf_realloc(g_tlsf_handle, ptr, size);

    if (!new_ptr) {
        size_t expansion_size = ALIGN_UP(size + 4096, 1024 * 1024);
        if (expand_heap(expansion_size)) {
            new_ptr = tlsf_realloc(g_tlsf_handle, ptr, size);
        }
    }

    return new_ptr;
}

void kfree(void* ptr) { 
    if (ptr) tlsf_free(g_tlsf_handle, ptr);
}