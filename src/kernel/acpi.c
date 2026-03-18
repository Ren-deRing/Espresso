#include <limine.h>

#include "string.h"

#include "acpi/acpi.h"
#include "serial.h"

#include "mm.h"

struct acpi_info g_acpi_info = {0};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST_ID,
    .revision = 0
};

void acpi_init() {
    struct rsdp_descriptor* rsdp = (struct rsdp_descriptor*)rsdp_request.response->address;

    if (rsdp->revision == 0) {
        return; 
    }

    struct xsdt* xsdt = (struct xsdt*)(p2v(rsdp->xsdt_address));
    int entries = (xsdt->header.length - sizeof(struct acpi_sdt_header)) / sizeof(uint64_t);

    for (int i = 0; i < entries; i++) {
        struct acpi_sdt_header* table = (struct acpi_sdt_header*)(p2v(xsdt->tables[i]));

        // MADT
        if (memcmp(table->signature, "APIC", 4) == 0) {
            g_acpi_info.madt = (struct madt*)table;
            madt_init(g_acpi_info.madt);
        } 
        // FADT
        else if (memcmp(table->signature, "FACP", 4) == 0) {
            g_acpi_info.fadt = (struct fadt*)table;
        }
        // HPET
        else if (memcmp(table->signature, "HPET", 4) == 0) {
            struct hpet* h = (struct hpet*)table;
    
            uint64_t val = h->address.address;
            g_acpi_info.hpet_addr = (uintptr_t)(p2v(val));
        }
    }
}

void madt_init(struct madt* m) {
    g_acpi_info.lapic_addr = (uintptr_t)p2v(m->lapic_addr);

    uint8_t* ptr = m->entries;
    uint8_t* end = (uint8_t*)m + m->header.length;

    while (ptr < end) {
        struct madt_entry_header* entry = (struct madt_entry_header*)ptr;
        if (entry->length < 2) break;

        switch (entry->type) {
            case 0: { // Processor Local APIC
                uint32_t flags = *(uint32_t*)&ptr[4];
                if ((flags & 1) && g_acpi_info.cpu_count < MAX_CORES) {
                    g_acpi_info.cpu_ids[g_acpi_info.cpu_count++] = ptr[3]; // APIC ID
                }
                break;
            }
            case 1: { // I/O APIC
                g_acpi_info.ioapic_addr = (uintptr_t)(p2v(*(uint32_t*)&ptr[4]));
                break;
            }
            case 2: { // Interrupt Source Override
                if (g_acpi_info.override_count < MAX_ISO) {
                    g_acpi_info.int_overrides[g_acpi_info.override_count].source = ptr[3];
                    g_acpi_info.int_overrides[g_acpi_info.override_count].target_gsi = *(uint32_t*)&ptr[4];
                    g_acpi_info.override_count++;
                }
                break;
            }
        }
        ptr += entry->length;
    }
}