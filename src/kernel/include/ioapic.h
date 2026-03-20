#pragma once

#include <stdint.h>

#define IOAPIC_REG_INDEX  0x00
#define IOAPIC_REG_DATA   0x10

void ioapic_set_irq(uint8_t irq, uint64_t apic_id, uint8_t vector);
void ioapic_init();