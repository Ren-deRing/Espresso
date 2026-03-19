#include <limine.h>

#include "include/string.h"

#include "serial.h"

#include "mm.h"
#include "pmm.h"

uint64_t g_hhdm_offset;
struct limine_memmap_response *g_memmap;

buddy_pmm_t g_pmm;

void pmm_init(void) {
    if (g_memmap == NULL) {
        return;
    }

    uint64_t entry_count = g_memmap->entry_count;
    uint64_t top_addr = 0;

    for (uint64_t i = 0; i < entry_count; i++) {
        struct limine_memmap_entry *entry = g_memmap->entries[i];
        uint64_t end = entry->base + entry->length;
        if (end > top_addr) top_addr = end;
    }
    
    g_pmm.total_pages = ALIGN_UP(top_addr, PAGE_SIZE) / PAGE_SIZE;
    uint64_t array_size = ALIGN_UP(g_pmm.total_pages * sizeof(page_t), PAGE_SIZE);

    uint64_t array_phys_addr = 0;
    for (uint64_t i = 0; i < entry_count; i++) {
        struct limine_memmap_entry *entry = g_memmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            uint64_t start = ALIGN_UP(entry->base, PAGE_SIZE);
            uint64_t end = ALIGN_DOWN(entry->base + entry->length, PAGE_SIZE);
            if (end > start && (end - start) >= array_size) {
                array_phys_addr = start;
                break;
            }
        }
    }

    if (array_phys_addr == 0) return; 

    g_pmm.all_pages_array = (page_t*)(p2v(array_phys_addr));
    memset(g_pmm.all_pages_array, 0, array_size);

    uint64_t array_end = array_phys_addr + array_size;

    for (uint64_t i = 0; i < entry_count; i++) {
        struct limine_memmap_entry *entry = g_memmap->entries[i];
        if (entry->type != LIMINE_MEMMAP_USABLE) continue;

        uint64_t start = ALIGN_UP(entry->base, PAGE_SIZE);
        uint64_t end = ALIGN_DOWN(entry->base + entry->length, PAGE_SIZE);

        for (uint64_t curr = start; curr < end; ) {
            if (curr >= array_phys_addr && curr < array_end) {
                curr = array_end;
                continue;
            }

            uint64_t limit = end;
            if (curr < array_phys_addr && end > array_phys_addr) {
                limit = array_phys_addr;
            }

            uint64_t remaining = limit - curr;
            uint8_t order = 0;

            for (int o = MAX_ORDER - 1; o >= 0; o--) {
                uint64_t block_size = (uint64_t)PAGE_SIZE << o;
                if (remaining >= block_size && (curr % block_size) == 0) {
                    order = o;
                    break;
                }
            }

            page_t* page = phys_to_page(curr);
            page->order = order;
            page->is_free = true;
            add_to_list(&g_pmm.free_lists[order], page);

            curr += ((uint64_t)PAGE_SIZE << order);
        }
    }
}

page_t* pmm_alloc_pages(uint8_t order) {
    uint8_t current_order;
    page_t* page = NULL;

    // 충분히 크지만 가장 작은 블록을 찾음
    for (current_order = order; current_order < MAX_ORDER; current_order++) {
        if (g_pmm.free_lists[current_order] != NULL) {
            page = g_pmm.free_lists[current_order];
            remove_from_list(&g_pmm.free_lists[current_order], page);
            break;
        }
    }

    if (page == NULL) {
        kprintf("PMM: Out of memory at order %d\n", order);
        return NULL; // 맞는 블록이 없음!!
    }

    // 블록을 맞는 형태까지 자름
    while (current_order > order) {
        current_order--;
        
        // 다른 반은 우리 Buddy
        page_t* buddy = get_buddy(page, current_order);
        
        // Buddy로 추가하고 free list에 추가
        buddy->is_free = true;
        buddy->order = current_order;
        add_to_list(&g_pmm.free_lists[current_order], buddy);
    }

    page->is_free = false;
    page->order = order;
    
    return page;
}

void pmm_free_pages(page_t* page, uint8_t order) {
    page->is_free = true;
    page->order = order;

    // buddy들과 합병 시도
    while (order < MAX_ORDER - 1) {
        page_t* buddy = get_buddy(page, order);

        // Buddy가 준비됐나요?
        if (buddy < g_pmm.all_pages_array ||
            buddy >= g_pmm.all_pages_array + g_pmm.total_pages ||
            !buddy->is_free ||
            buddy->order != order) {
            break; // 아니요?
        }

        // 합병 가능하므로 free list에서 삭제
        remove_from_list(&g_pmm.free_lists[order], buddy);

        // 새 큰 블록 생성
        if (buddy < page) {
            page = buddy;
        }

        // order를 키워 다음 단계 반복
        order++;
        page->order = order;
    }

    // 모두 합병된 블록 free list에 추가
    add_to_list(&g_pmm.free_lists[order], page);
}

void add_to_list(page_t** head, page_t* node) {
    node->next = *head;
    node->prev = NULL;
    if (*head != NULL) {
        (*head)->prev = node;
    }
    *head = node;
}

void remove_from_list(page_t** head, page_t* target) {
    if (!head || !target) return;
    
    page_t* next = target->next;
    page_t* prev = target->prev;

    if (prev != NULL) {
        prev->next = next;
    } else {
        *head = next;
    }

    if (next != NULL) {
        next->prev = prev;
    }

    target->next = NULL;
    target->prev = NULL;
}

uint64_t page_to_phys(page_t* page) {
    if (!page) return 0;
    uint64_t idx = page - g_pmm.all_pages_array;
    return idx * PAGE_SIZE; 
}

page_t* phys_to_page(uint64_t phys) {
    uint64_t idx = phys / PAGE_SIZE;
    if (idx >= g_pmm.total_pages) return NULL;
    return &g_pmm.all_pages_array[idx];
}

page_t* get_buddy(page_t* page, uint8_t order) {
    uint64_t phys = page_to_phys(page);
    uint64_t buddy_phys = phys ^ ((uint64_t)PAGE_SIZE << order);
    return phys_to_page(buddy_phys);
}