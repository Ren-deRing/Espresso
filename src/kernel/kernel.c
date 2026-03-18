#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <string.h>

#include <limine.h>

#include "cpu.h"
#include "interrupt.h"
#include "serial.h"

#include "acpi/acpi.h"
#include "drivers/lapic.h"
#include "drivers/hpet.h"
#include "drivers/io.h"

#include "time.h"

#include "mm.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"

__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_response = {
    .id = LIMINE_HHDM_REQUEST_ID, .revision = 0
};
__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_response = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID, .revision = 0
};
__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0
};

void kmain() {
    __asm__ volatile ("cli");

    __asm__ volatile(
        "mov %%cr4, %%rax\n\t"
        "or $(1 << 9), %%rax\n\t"   // OSFXSR
        "or $(1 << 10), %%rax\n\t"  // OSXMMEXCPT
        "or $(1 << 18), %%rax\n\t"  // OSXSAVE 활성화
        "mov %%rax, %%cr4\n\t"
        
        "xor %%rcx, %%rcx\n\t"      // XCR0은 첫번째
        "mov $7, %%rax\n\t"         // 1: x87, 2: SSE, 4: AVX
        "xor %%rdx, %%rdx\n\t"
        "xsetbv\n\t"
        : : : "rax", "rcx", "rdx"
    );
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);

    g_hhdm_offset = hhdm_response.response->offset;
    g_memmap = memmap_request.response;
    
    serial_init();

    gdt_init();
    idt_init();

    pmm_init();
    vmm_init();

    kmalloc_init();

    acpi_init();

    hpet_init();
    lapic_init();

    lapic_timer_calibrate();

    time_init();

    __asm__ volatile ("sti");

    struct limine_framebuffer *framebuffer = framebuffer_response.response->framebuffers[0];

    printf("HHDM Offset: 0x%lx\n", g_hhdm_offset);
    printf("FrameBuffer Addr: 0x%lx\n", (uint64_t)framebuffer->address);

    struct timespec ts_rel;
    getTimeoutAbsolute(&ts_rel, 1500, 0);
    printf("Absolute +1500ms: %llu sec, %llu nsec\n", ts_rel.tv_sec, ts_rel.tv_nsec);

    while (1) {
        __asm__ volatile ("hlt");
    }
}