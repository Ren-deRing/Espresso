OUTPUT_FORMAT(elf64-x86-64)

/* entry point */
ENTRY(kmain)

/* 부트로더 헤더 */
PHDRS
{
    limine_requests PT_LOAD;
    text PT_LOAD;
    rodata PT_LOAD;
    data PT_LOAD;
}

SECTIONS
{
    /* HHDM */
    . = 0xffffffff80000000;

    /* 헤더로 Limine Request를 지정할 수 있도록 함 */
    .limine_requests : {
        KEEP(*(.limine_requests_start))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end))
    } :limine_requests

    /* .text 까지 정렬 */
    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .text : {
        *(.text .text.*)
    } :text

    /* .rodata 까지 정렬 */
    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .rodata : {
        *(.rodata .rodata.*)
    } :rodata

    /* 링커 명령어 */
    .note.gnu.build-id : {
        *(.note.gnu.build-id)
    } :rodata

    /* .data 까지 정렬 */
    . = ALIGN(CONSTANT(MAXPAGESIZE));

    .data : {
        *(.data .data.*)
    } :data

    /* .bss는 모든 데이터와 정렬 데이터들 뒤에 배치되어야 함 */
    /* 추가적인 공간이 필요하다면 이 위에 작성 */
    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    } :data

    /DISCARD/ : {
        *(.eh_frame*)
        *(.note .note.*)
    }
}