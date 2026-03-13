#include "interrupt.h"
#include "serial.h"

static idtr_t volatile idtr;
static bool volatile vectors[IDT_MAX_DESCRIPTORS];

extern void* volatile isr_stub_table[];

__attribute__((aligned(0x10))) 
static volatile idt_entry_t idt[256];

__attribute__((noreturn))
void isr_handler() {
    printf("Interrupt!");
    __asm__ volatile ("cli; hlt");
}

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
    idt_entry_t* descriptor = &idt[vector];

    descriptor->isr_low        = (uint64_t)isr & 0xFFFF;
    descriptor->kernel_cs      = 0x08;
    descriptor->ist            = 0;
    descriptor->attributes     = flags;
    descriptor->isr_mid        = ((uint64_t)isr >> 16) & 0xFFFF;
    descriptor->isr_high       = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
    descriptor->reserved       = 0;
}

void idt_init() {
    idtr.base = (uintptr_t)&idt[0];
    idtr.limit = (uint16_t)sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS - 1;

    for (uint8_t vector = 0; vector < 32; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
        vectors[vector] = true;
    }

    __asm__ volatile ("lidt %0" : : "m"(idtr));
    __asm__ volatile ("sti");
}

irq_state_t save_irq_disable() {
    irq_state_t flags;
    __asm__ volatile (
        "pushfq\n\t" // eflags 읽기
        "pop %0\n\t" // 스택에서 변수로 팝
        "cli" // 인터럽트 끄기
        : "=rm"(flags)
        :
        : "memory"
    );
    return flags;
}

void restore_irq(irq_state_t flags) {
    // push %0 -> popfq
    __asm__ volatile (
        "push %0\n\t"
        "popfq"
        :
        : "rm"(flags)
        : "memory", "cc"
    );
}