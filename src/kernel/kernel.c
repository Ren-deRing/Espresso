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

#include "wasm3.h"

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

    kprintf("HHDM Offset: 0x%lx\n", g_hhdm_offset);
    kprintf("FrameBuffer Addr: 0x%lx\n", (uint64_t)framebuffer->address);

    kprintf("Initializing Wasm3...\n");
    
    IM3Environment env = m3_NewEnvironment();
    if (!env) {
        kprintf("Failed to create environment\n");
        return;
    }

    IM3Runtime runtime = m3_NewRuntime(env, 1024 * 64, NULL); // 64KB 스택
    if (!runtime) {
        kprintf("Failed to create runtime\n");
        return;
    }

    kprintf("Wasm3 Online!\n");

    unsigned char fib_wasm[] = {
        0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x06, 0x01, 0x60, 0x01, 0x7f, 0x01, 0x7f, 
        0x03, 0x02, 0x01, 0x00, 0x07, 0x07, 0x01, 0x03, 0x66, 0x69, 0x62, 0x00, 0x00, 0x0a, 0x1e, 0x01, 
        0x1c, 0x00, 0x20, 0x00, 0x41, 0x02, 0x49, 0x04, 0x7f, 0x20, 0x00, 0x05, 0x20, 0x00, 0x41, 0x01, 
        0x6b, 0x10, 0x00, 0x20, 0x00, 0x41, 0x02, 0x6b, 0x10, 0x00, 0x6a, 0x0b, 0x0b
    };

    IM3Module module;
    M3Result result = m3_ParseModule(env, &module, fib_wasm, sizeof(fib_wasm));
    if (result) {
        kprintf("Parse error: %s\n", result);
        return;
    }

    result = m3_LoadModule(runtime, module);
    if (result) {
        kprintf("Load error: %s\n", result);
        return;
    }

    IM3Function f;
    result = m3_FindFunction(&f, runtime, "fib");
    if (result) {
        kprintf("FindFunction error: %s\n", result);
        return;
    }

    result = m3_CallV(f, 20);
    if (result) {
        kprintf("Call error: %s\n", result);
        return;
    }

    int value = 0;
    m3_GetResultsV(f, &value);
    kprintf("WASM fib(20) = %d\n", value);

    while (1) {
        __asm__ volatile ("hlt");
    }
}