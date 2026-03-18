.global isr_common_stub
.extern isr_handler

.macro ISR_NOERRCODE num
.global isr\num
isr\num:
    pushq $0           # 가짜 에러 코드
    pushq $\num        # 인터럽트 번호
    jmp isr_common_stub
.endm

.macro ISR_ERRCODE num
.global isr\num
isr\num:
    # CPU가 이미 push 완료
    pushq $\num        # 인터럽트 번호 push
    jmp isr_common_stub
.endm

ISR_NOERRCODE 0  # Divide By Zero
ISR_NOERRCODE 1  # Debug
ISR_NOERRCODE 2
ISR_NOERRCODE 3  # Breakpoint
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8  # Double Fault
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13 # General Protection Fault
ISR_ERRCODE   14 # Page Fault
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_ERRCODE   17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_ERRCODE   21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_ERRCODE   29
ISR_ERRCODE   30
ISR_NOERRCODE 31

.altmacro

.macro do_isr_noerr num
    ISR_NOERRCODE \num
.endm

i = 32
.rept 224
    do_isr_noerr %i
    i = i + 1
.endr

.noaltmacro

isr_common_stub:
    pushq %r15; pushq %r14; pushq %r13; pushq %r12
    pushq %r11; pushq %r10; pushq %r9;  pushq %r8
    pushq %rbp; pushq %rdi; pushq %rsi; pushq %rdx
    pushq %rcx; pushq %rbx; pushq %rax

    movq %rsp, %rdi
    call isr_handler

    popq %rax;  popq %rbx;  popq %rcx;  popq %rdx
    popq %rsi;  popq %rdi;  popq %rbp
    popq %r8;   popq %r9;   popq %r10;  popq %r11
    popq %r12;  popq %r13;  popq %r14;  popq %r15

    cmpq $0, 16(%rsp)   # iretq가 읽을 RIP 위치(16바이트 뒤)가 0인지 확인
    jne .fine
    ud2 

.fine:
    addq $16, %rsp
    iretq

.section .data
.align 8
.global isr_stub_table

.altmacro

.macro isr_entry index
    .quad isr\index
.endm

isr_stub_table:
    i = 0
    .rept 32
        isr_entry %i
        i = i + 1
    .endr

    i = 32
    .rept 224
        isr_entry %i
        i = i + 1
    .endr

.noaltmacro