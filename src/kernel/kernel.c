#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <string.h>

#include <limine.h>

#include "cpu.h"
#include "interrupt.h"
#include "serial.h"

#include "acpi/acpi.h"

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
        "mov %%cr0, %%rax\n\t"
        "and $~(1 << 2), %%rax\n\t" // EM 비트 지우기
        "or $(1 << 1), %%rax\n\t"   // MP 비트 설정
        "mov %%rax, %%cr0\n\t"
        "mov %%cr4, %%rax\n\t"
        "or $(1 << 9), %%rax\n\t"   // OSFXSR 비트 설정
        "or $(1 << 10), %%rax\n\t"  // OSXMMEXCPT 비트 설정
        "mov %%rax, %%cr4"
        : : : "rax"
    );
    g_hhdm_offset = hhdm_response.response->offset;
    g_memmap = memmap_request.response;
    serial_init();

    acpi_init();

    gdt_init();
    idt_init();

    pmm_init();
    vmm_init();
    kmalloc_init();

    struct limine_framebuffer *framebuffer = framebuffer_response.response->framebuffers[0];

    printf("HHDM Offset: 0x%lx\n", g_hhdm_offset);
    printf("FrameBuffer Addr: 0x%lx\n", (uint64_t)framebuffer->address);

    __asm__ volatile ("sti");

    __asm__ volatile ("hlt");
}