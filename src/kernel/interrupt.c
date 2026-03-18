#include "interrupt.h"
#include "serial.h"

#include "drivers/lapic.h"


#include "drivers/io.h" // TODO: Remove

static idtr_t volatile idtr;
static bool volatile vectors[IDT_MAX_DESCRIPTORS];

extern void* volatile isr_stub_table[];

__attribute__((aligned(0x10))) 
static volatile idt_entry_t idt[256];

static interrupt_handler_t volatile interrupt_handlers[256];

void register_interrupt_handler(uint8_t vector, interrupt_handler_t handler) {
    interrupt_handlers[vector] = handler;
}

void kernel_panic(const char* reason, interrupt_frame_t* frame) {
    __asm__ volatile ("cli");

    printf("kernel panic!\n\n");
    printf("REASON: %s\n", reason);
    printf("----\n");
    printf("  INT NO: %d | ERROR CODE: %d\n", frame->int_no, frame->error_code);
    printf("  RIP: %016llx | CS: %02llx\n", frame->rip, frame->cs);
    printf("  RFLAGS: %016llx | RSP: %016llx\n", frame->rflags, frame->rsp);
    printf("----\n");
    printf("SYSTEM HALTED.\n");

    while (1) {
        __asm__ volatile ("hlt");
    }
}

void isr_handler(interrupt_frame_t* frame) {
    if (frame->int_no < 32) {
        kernel_panic("EXCEPTION", frame);
        return;
    }

    interrupt_handler_t handler = (interrupt_handler_t)interrupt_handlers[frame->int_no];

    if (handler) {
        handler(frame);
    } 

    if (frame->int_no >= 32 && frame->int_no <= 47) {
        lapic_eoi();
    }
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
    idtr.limit = (uint16_t)sizeof(idt_entry_t) * 256 - 1;

    for (int vector = 0; vector < 256; vector++) {
        idt_set_descriptor((uint8_t)vector, isr_stub_table[vector], 0x8E);
        vectors[vector] = true;
    }

    __asm__ volatile ("lidt %0" : : "m"(idtr));
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