#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <elf.h>

#include "mm.h"
#include "vmm.h"
#include "pmm.h"
#include "string.h"

static volatile struct limine_executable_address_request executable_address_request = {
    .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST_ID,
    .revision = 0
};

static volatile struct limine_executable_file_request executable_file_request = {
    .id = LIMINE_EXECUTABLE_FILE_REQUEST_ID,
    .revision = 0
};

page_table_t* g_kernel_pagemap = NULL;

static inline void set_cr3(uintptr_t pml4_phys) {
    asm volatile("mov %0, %%cr3" : : "r"(pml4_phys) : "memory");
}

static inline uintptr_t get_cr3(void) {
    uintptr_t pml4_phys;
    asm volatile("mov %%cr3, %0" : "=r"(pml4_phys));
    return pml4_phys;
}

static inline void invlpg(uint64_t vaddr) {
    asm volatile("invlpg (%0)" : : "r"(vaddr) : "memory");
}

pt_entry_t* vmm_get_pte(page_table_t* pml4, uint64_t virt, bool allocate) {
    page_table_t* current_table = pml4;
    uint64_t indices[4] = { GET_PML4_IDX(virt), GET_PDPT_IDX(virt), GET_PD_IDX(virt), GET_PT_IDX(virt) };

    for (int i = 0; i < 3; i++) {
        pt_entry_t* entry = &current_table->entries[indices[i]];
        
        if (!(*entry & PAGE_PRESENT)) {
            if (!allocate) return NULL;

            page_t* new_table_page = pmm_alloc_pages(0);
            if (!new_table_page) return NULL;

            uint64_t new_table_phys = page_to_phys(new_table_page);
            memset(p2v(new_table_phys), 0, PAGE_SIZE);

            new_table_page->ref_count = 0;

            page_t* parent_page = phys_to_page(v2p(current_table));
            parent_page->ref_count++;

            *entry = new_table_phys | PAGE_PRESENT | PAGE_WRITABLE;
        }
        
        current_table = (page_table_t*)ENTRY_TO_VIRT(*entry);
    }

    return &current_table->entries[indices[3]];
}

bool vmm_map(page_table_t* pml4, uint64_t virt, uint64_t phys, uint64_t flags) {
    pt_entry_t* pte = vmm_get_pte(pml4, virt, true);
    if (!pte) return false;

    if (!(*pte & PAGE_PRESENT)) {
        uintptr_t pt_vaddr = ALIGN_DOWN((uintptr_t)pte, PAGE_SIZE);
        page_t* pt_page = phys_to_page(v2p((void*)pt_vaddr));
        pt_page->ref_count++;
    }

    *pte = (phys & PAGE_ADDR_MASK) | flags;
    invlpg(virt);
    return true;
}

void vmm_init(void) {
    // 커널 PML4 생성
    page_t* pml4_page = pmm_alloc_pages(0);
    g_kernel_pagemap = (page_table_t*)p2v(page_to_phys(pml4_page));
    memset(g_kernel_pagemap, 0, PAGE_SIZE);
    pml4_page->ref_count = 1;

    // 전체 물리 메모리 매핑
    for (uint64_t i = 0; i < g_memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = g_memmap->entries[i];
        
        uint64_t base = ALIGN_DOWN(entry->base, PAGE_SIZE);
        uint64_t top = ALIGN_UP(entry->base + entry->length, PAGE_SIZE);

        for (uint64_t phys = base; phys < top; phys += PAGE_SIZE) {
            // Here
            uint64_t virt = (uint64_t)p2v(phys);
            vmm_map(g_kernel_pagemap, virt, phys, PAGE_PRESENT | PAGE_WRITABLE);
        }
    }

    // 커널 바이너리 매핑
    struct limine_executable_address_response* kaddr = executable_address_request.response;
    struct limine_file* kfile = executable_file_request.response->executable_file;

    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)kfile->address;
    Elf64_Phdr* phdr = (Elf64_Phdr*)((uintptr_t)kfile->address + ehdr->e_phoff);

    for (size_t i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type != PT_LOAD) continue;

        uint64_t virt_start = ALIGN_DOWN(phdr[i].p_vaddr, PAGE_SIZE);
        uint64_t phys_start = (virt_start - kaddr->virtual_base) + kaddr->physical_base;
        uint64_t size = ALIGN_UP(phdr[i].p_memsz + (phdr[i].p_vaddr % PAGE_SIZE), PAGE_SIZE);

        uint64_t flags = PAGE_PRESENT;
        if (phdr[i].p_flags & PF_W) flags |= PAGE_WRITABLE;
        if (!(phdr[i].p_flags & PF_X)) flags |= PAGE_NX;

        for (uint64_t offset = 0; offset < size; offset += PAGE_SIZE) {
            vmm_map(g_kernel_pagemap, virt_start + offset, phys_start + offset, flags);
        }
    }

    set_cr3(v2p(g_kernel_pagemap));
}

void vmm_unmap(page_table_t* pml4, uint64_t virt) {
    uint64_t idxs[4] = { GET_PML4_IDX(virt), GET_PDPT_IDX(virt), GET_PD_IDX(virt), GET_PT_IDX(virt) };
    pt_entry_t* entries[4];
    page_table_t* tables[4];

    tables[0] = pml4;
    for (int i = 0; i < 3; i++) {
        entries[i] = &tables[i]->entries[idxs[i]];
        if (!(*entries[i] & PAGE_PRESENT)) return;
        tables[i+1] = (page_table_t*)ENTRY_TO_VIRT(*entries[i]);
    }
    entries[3] = &tables[3]->entries[idxs[3]];

    if (!(*entries[3] & PAGE_PRESENT)) return;

    *entries[3] = 0;
    invlpg(virt);

    for (int i = 3; i > 0; i--) {
        page_t* table_page = phys_to_page(v2p(tables[i]));
        
        if (--table_page->ref_count == 0) {
            *entries[i-1] = 0;
            pmm_free_pages(table_page, 0);
        } else {
            break;
        }
    }
}

page_table_t* vmm_get_current_pml4(void) {
    return (page_table_t*)p2v(get_cr3());
}

uint64_t vmm_get_phys(page_table_t* pml4, uint64_t virt) {
    pt_entry_t* pte = vmm_get_pte(pml4, virt, false);
    if (!pte || !(*pte & PAGE_PRESENT)) {
        return 0;
    }
    return (*pte & PAGE_ADDR_MASK) + (virt & (PAGE_SIZE - 1));
}