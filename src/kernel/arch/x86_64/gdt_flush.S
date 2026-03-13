.section .text
.global gdt_flush

gdt_flush:
    lgdt (%rdi)

    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    pushq $0x08
    leaq .flush(%rip), %rax
    pushq %rax
    lretq

.flush:
    ret