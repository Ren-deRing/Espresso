#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <string.h>

#include <limine.h>

#include "cpu.h"
#include "interrupt.h"
#include "serial.h"

#include "graphics.h"

#include "acpi/acpi.h"
#include "drivers/lapic.h"
#include "drivers/hpet.h"
#include "drivers/io.h"
#include "ioapic.h"

#include "time.h"

#include "mm.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"

#include "runtime/runtime.h"

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

static uint8_t hex_to_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return 0;
}

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
    g_framebuffer = framebuffer_response.response->framebuffers[0];
    
    serial_init();

    gdt_init();
    idt_init();

    pmm_init();
    vmm_init();

    kmalloc_init();

    acpi_init();

    hpet_init();
    lapic_init();
    ioapic_init();

    lapic_timer_calibrate();

    time_init();

    while (inb(0x64) & 0x01) {
        inb(0x60);
    }

    runtime_init();

    __asm__ volatile ("sti");

    char input[16384];
    int index = 0;

    while (1) {
        kprintf("espresso@os:~$ ");
        index = 0;
        memset(input, 0, sizeof(input));

        while (1) {
            char c = serial_getc();

            if (c == '\r' || c == '\n') {
                serial_putc('\n');
                input[index] = '\0';
                break;
            }
            else if (c == 0x08 || c == 0x7F) {
                if (index > 0) {
                    index--;
                    serial_putc('\b');
                    serial_putc(' ');
                    serial_putc('\b');
                }
            }
            else if (index < 16383) {
                input[index++] = c;
                serial_putc(c);
            }
        }

        if (strlen(input) == 0) continue;

        if (strcmp(input, "help") == 0) {
            kprintf("Commands: help, run, clear\n");
        } 
        else if (strncmp(input, "run ", 4) == 0) {
            char* hex_str = input + 4;
            
            uint32_t str_len = strlen(hex_str);
            uint8_t* wasm_bin = (uint8_t*)kmalloc(str_len / 2 + 1);
            uint32_t bin_idx = 0;

            for (uint32_t i = 0; i < str_len; ) {
                if (hex_str[i] == ' ' || hex_str[i] == '\t') {
                    i++;
                    continue;
                }

                if (hex_str[i+1] == '\0') break;

                wasm_bin[bin_idx++] = (hex_to_val(hex_str[i]) << 4) | hex_to_val(hex_str[i+1]);
                
                i += 2;
            }

            if (bin_idx > 0) {
                kprintf("Converting hex string to %d bytes binary...\n", bin_idx);
                runtime_execute(wasm_bin, bin_idx);
            }
            
            kfree(wasm_bin);
        }
        else if (strcmp(input, "clear") == 0) {
            kprintf("\033[2J\033[H");
        }
        else {
            kprintf("Unknown command: %s\n", input);
        }
    }

    while (1) {
        __asm__ volatile ("hlt");
    }
}