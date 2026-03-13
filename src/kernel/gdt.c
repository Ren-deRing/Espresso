#include <stdint.h>

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

static struct gdt_entry gdt[4];
static struct gdt_ptr   gp;

extern void gdt_flush(uintptr_t gdt_ptr);

void gdt_init(void) {
    // 0: Null
    gdt[0] = (struct gdt_entry){0, 0, 0, 0, 0, 0};

    // 1: Kernel Code (0x08) - Access: 0x9A, Granularity: 0x20 (64-bit)
    gdt[1] = (struct gdt_entry){0, 0, 0, 0x9A, 0x20, 0};

    // 2: Kernel Data (0x10) - Access: 0x92, Granularity: 0x00
    gdt[2] = (struct gdt_entry){0, 0, 0, 0x92, 0x00, 0};

    gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gp.base  = (uintptr_t)&gdt;

    gdt_flush((uintptr_t)&gp);
}