#pragma once

#include <stdint.h>
#include <stdbool.h>

#define MAX_ORDER 11  // 0(4KB) ~ 10(4MB)
#define PAGE_SIZE 4096

typedef struct page {
    struct page* next;
    struct page* prev;
    uint8_t order;
    bool is_free;
    uint32_t ref_count;
} page_t;

typedef struct {
    page_t* free_lists[MAX_ORDER];
    page_t* all_pages_array;
    uint64_t total_pages;
    uint64_t last_phys_addr;
} buddy_pmm_t;

extern buddy_pmm_t g_pmm;
void pmm_init();

page_t* pmm_alloc_pages(uint8_t order);
void pmm_free_pages(page_t* page, uint8_t order);

void add_to_list(page_t** head, page_t* node);
void remove_from_list(page_t** head, page_t* target);

page_t* phys_to_page(uint64_t phys);
uint64_t page_to_phys(page_t* page);
page_t* get_buddy(page_t* page, uint8_t order);