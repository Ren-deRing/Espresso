#pragma once

#include <stdint.h>

extern uint64_t g_hhdm_offset;
extern struct limine_memmap_response *g_memmap;

static inline void* p2v(uintptr_t phys) {
    return (void*)(phys + g_hhdm_offset);
}

static inline uintptr_t v2p(void* virt) {
    return (uintptr_t)virt - g_hhdm_offset;
}

#define ALIGN_DOWN(addr, align) ((addr) & ~((align) - 1))
#define ALIGN_UP(addr, align)   (((addr) + (align) - 1) & ~((align) - 1))


#define PAGE_SIZE 4096

#define PAGE_PRESENT    (1ULL << 0)   // 페이지가 존재
#define PAGE_WRITABLE   (1ULL << 1)   // 쓰기 가능
#define PAGE_USER       (1ULL << 2)   // 사용자 모드 접근 가능
#define PAGE_PWT        (1ULL << 3)   // Write-Through 캐싱
#define PAGE_PCD        (1ULL << 4)   // 캐시 비활성화
#define PAGE_ACCESSED   (1ULL << 5)   // CPU가 이 페이지를 읽거나 씀
#define PAGE_DIRTY      (1ULL << 6)   // CPU가 이 페이지에 씀 (PT 엔트리 전용)
#define PAGE_HUGE       (1ULL << 7)   // 거대 페이지
#define PAGE_GLOBAL     (1ULL << 8)   // CR3 교체 시 TLB에서 삭제되지 않음
#define PAGE_NX         (1ULL << 63)  // 실행 금지

#define PAGE_MMIO       (PAGE_PRESENT | PAGE_WRITABLE | PAGE_NX | PAGE_PCD | PAGE_PWT)

#define PAGE_ADDR_MASK  0x000ffffffffff000ull

#define VMM_FLAGS_KERNEL_CODE (PAGE_PRESENT)
#define VMM_FLAGS_KERNEL_DATA (PAGE_PRESENT | PAGE_WRITABLE | PAGE_NX)

typedef uint64_t pt_entry_t;

typedef struct {
    pt_entry_t entries[512];
} page_table_t;

#define GET_PML4_IDX(v) (((v) >> 39) & 0x1FF)
#define GET_PDPT_IDX(v) (((v) >> 30) & 0x1FF)
#define GET_PD_IDX(v)   (((v) >> 21) & 0x1FF)
#define GET_PT_IDX(v)   (((v) >> 12) & 0x1FF)

#define ENTRY_TO_VIRT(e) (p2v(e & PAGE_ADDR_MASK))
