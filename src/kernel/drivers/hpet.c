#include "mm.h"
#include "vmm.h"

#include "acpi/acpi.h"

#include "drivers/hpet.h"

#include "serial.h"

static volatile uint64_t* hpet_base = NULL;
static uint32_t clk_period = 0;

void hpet_init() {
    uint64_t hpet_virt_addr = g_acpi_info.hpet_addr;
    uint64_t hpet_phys_addr = v2p((void*)hpet_virt_addr);

    vmm_map(g_kernel_pagemap,
            hpet_virt_addr,
            hpet_phys_addr,
            PAGE_PRESENT | PAGE_WRITABLE | PAGE_PCD | PAGE_NX);

    hpet_base = (volatile uint64_t*)hpet_virt_addr;

    clk_period = (uint32_t)(hpet_base[0] >> 32);

    if (clk_period == 0) {
        printf("Error: HPET clk_period is 0. Check VM settings.\n");
        return;
    }

    hpet_base[0x10 / 8] |= 0x01; 
}

uint64_t hpet_get_nanos() {
    return hpet_base[0xF0 / 8] * (clk_period / 1000000);
}

void hpet_usleep(uint64_t us) {
    uint64_t start = hpet_get_nanos();
    uint64_t delta = us * 1000;
    
    while ((hpet_get_nanos() - start) < delta) {
        __asm__ volatile ("pause");
    }
}