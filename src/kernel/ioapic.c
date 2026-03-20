#include "ioapic.h"
#include "acpi/acpi.h"

#include "vmm.h"

volatile uint32_t* ioapic_base;

void ioapic_init() {
    uint64_t ioapic_virt_addr = g_acpi_info.ioapic_addr;
    uint64_t ioapic_phys_addr = v2p((void*)ioapic_virt_addr);

    vmm_map(g_kernel_pagemap, ioapic_virt_addr, ioapic_phys_addr, PAGE_MMIO);

    ioapic_base = (volatile uint32_t*)ioapic_virt_addr;
}

void ioapic_write(uint32_t reg, uint32_t val) {
    *(volatile uint32_t*)((uintptr_t)ioapic_base + IOAPIC_REG_INDEX) = reg;
    *(volatile uint32_t*)((uintptr_t)ioapic_base + IOAPIC_REG_DATA) = val;
}

uint32_t ioapic_read(uint32_t reg) {
    *(volatile uint32_t*)((uintptr_t)ioapic_base + IOAPIC_REG_INDEX) = reg;
    return *(volatile uint32_t*)((uintptr_t)ioapic_base + IOAPIC_REG_DATA);
}

void ioapic_set_irq(uint8_t irq, uint64_t apic_id, uint8_t vector) {
    uint32_t low = vector;
    uint32_t high = (uint32_t)(apic_id << 24);

    ioapic_write(0x10 + irq * 2, low);
    ioapic_write(0x10 + irq * 2 + 1, high);
}