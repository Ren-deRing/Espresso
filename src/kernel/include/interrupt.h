#pragma once

#include <stdint.h>
#include <stdbool.h>

#define IDT_MAX_DESCRIPTORS 256

typedef struct {
	uint16_t    isr_low;      // ISR 하위 16비트
	uint16_t    kernel_cs;    // GDT 코드 세그먼트
	uint8_t	    ist;          // IST
	uint8_t     attributes;   // Type and attributes
	uint16_t    isr_mid;      // ISR 하위 32비트 중 상위 16비트
	uint32_t    isr_high;     // 그 반대
	uint32_t    reserved;     // 예약됨
} __attribute__((packed)) idt_entry_t;

typedef struct {
	uint16_t	limit;
	uint64_t	base;
} __attribute__((packed)) idtr_t;

typedef struct {
    uint64_t rax, rbx, rcx, rdx, rsi, rdi, rbp;
    uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
    
    uint64_t int_no, error_code;
    
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed)) interrupt_frame_t;

void isr_handler(void);
void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags);
void idt_init(void);

typedef uint64_t irq_state_t;

irq_state_t save_irq_disable();
void restore_irq(irq_state_t flags);