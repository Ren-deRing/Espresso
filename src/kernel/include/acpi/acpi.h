#pragma once

#include <stdint.h>

#include "acpi/hpet.h"
#include "acpi/fadt.h"

#include "acpi/acpi_types.h"

#define MAX_CORES 256
#define MAX_ISO 16


struct rsdp_descriptor {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;
    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__((packed));

struct xsdt {
    struct acpi_sdt_header header;
    uint64_t tables[];
} __attribute__((packed));

struct madt {
    struct acpi_sdt_header header;
    uint32_t lapic_addr;
    uint32_t flags;
    uint8_t  entries[];
} __attribute__((packed));

struct madt_entry_header {
    uint8_t type;
    uint8_t length;
} __attribute__((packed));

struct acpi_info {
    uintptr_t local_intc_addr;  // x86: LAPIC,  ARM: GICC/GICR
    uintptr_t dist_intc_addr;   // x86: IOAPIC, ARM: GICD
    uintptr_t timer_addr;       // x86: HPET,   ARM: Generic Timer/GTDT

    struct madt* madt; 
    struct fadt* fadt;
    struct hpet* hpet;

    struct acpi_sdt_header* timer_table; // HPET(x86) or GTDT(ARM)

    uint32_t cpu_ids[MAX_CORES]; // x86: APIC ID, ARM: Interface Number/MPIDR
    int cpu_count;

    struct {
        uint32_t source;
        uint32_t target_gsi;
        uint16_t flags;
    } int_overrides[MAX_ISO];
    int override_count;

    enum { ARCH_X86_64, ARCH_AARCH64 } arch_type;
    uint8_t gic_version;
};

extern struct acpi_info g_acpi_info;

void acpi_init();
void madt_init(struct madt* m);