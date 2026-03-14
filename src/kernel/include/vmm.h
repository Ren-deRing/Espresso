#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "mm.h"

extern page_table_t* g_kernel_pagemap;

void vmm_init(void);
bool vmm_map(page_table_t* pml4, uint64_t virt, uint64_t phys, uint64_t flags);
void vmm_unmap(page_table_t* pml4, uint64_t virt);
pt_entry_t* vmm_get_pte(page_table_t* pml4, uint64_t virt, bool allocate);
page_table_t* vmm_get_current_pml4(void);
uint64_t vmm_get_phys(page_table_t* pml4, uint64_t virt);
