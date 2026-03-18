#include "mm.h"
#include "vmm.h"

#include "acpi/acpi.h"

#include "drivers/lapic.h"
#include "drivers/hpet.h"

static uint32_t lapic_ticks_per_ms = 0;
static volatile uint32_t* lapic_base;

uint32_t g_lapic_ticks_per_ms = 0;

void lapic_init() {
    uint64_t lapic_virt_addr = g_acpi_info.lapic_addr;
    uint64_t lapic_phys_addr = v2p((void*)lapic_virt_addr);

    vmm_map(g_kernel_pagemap, lapic_virt_addr, lapic_phys_addr, PAGE_MMIO);
    
    lapic_base = (volatile uint32_t*)lapic_virt_addr;

    lapic_write(LAPIC_SIV, lapic_read(LAPIC_SIV) | 0x100 | 0xFF); 
}

uint32_t lapic_timer_calibrate() {
    lapic_write(LAPIC_DIV_CONF, 0x03); // 16분주기
    
    lapic_write(LAPIC_LVT_TIMER, 0x10000); 
    lapic_write(LAPIC_INIT_COUNT, 0xFFFFFFFF);

    hpet_usleep(10000);

    uint32_t ticks_passed = 0xFFFFFFFF - lapic_read(LAPIC_CURR_COUNT);
    
    g_lapic_ticks_per_ms = ticks_passed / 10;

    lapic_write(LAPIC_INIT_COUNT, 0);

    return g_lapic_ticks_per_ms;
}

void lapic_timer_start(uint32_t ms, uint8_t vector) {
    lapic_write(LAPIC_LVT_TIMER, 0x20000 | vector);
    lapic_write(LAPIC_DIV_CONF, 0x03); // 16분주기
    
    lapic_write(LAPIC_INIT_COUNT, g_lapic_ticks_per_ms * ms);
}

void lapic_timer_oneshot(uint32_t ms, uint8_t vector) {
    lapic_write(LAPIC_LVT_TIMER, vector); 
    lapic_write(LAPIC_DIV_CONF, 0x03);
    lapic_write(LAPIC_INIT_COUNT, g_lapic_ticks_per_ms * ms);
}

void lapic_write(uint32_t reg, uint32_t val) {
    *(volatile uint32_t*)((uintptr_t)lapic_base + reg) = val;
}

uint32_t lapic_read(uint32_t reg) {
    return *(volatile uint32_t*)((uintptr_t)lapic_base + reg);
}

void lapic_eoi() {
    lapic_write(LAPIC_EOI, 0);
}
