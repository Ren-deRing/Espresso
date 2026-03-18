#pragma once

#include <stdint.h>

#define LAPIC_ID          0x020
#define LAPIC_VER         0x030
#define LAPIC_TPR         0x080
#define LAPIC_EOI         0x0B0
#define LAPIC_LDR         0x0D0
#define LAPIC_DFR         0x0E0
#define LAPIC_SIV         0x0F0
#define LAPIC_ICR_LOW     0x300
#define LAPIC_ICR_HIGH    0x310
#define LAPIC_LVT_TIMER   0x320
#define LAPIC_INIT_COUNT  0x380
#define LAPIC_CURR_COUNT  0x390
#define LAPIC_DIV_CONF    0x3E0

void lapic_init();

void lapic_write(uint32_t reg, uint32_t val);
uint32_t lapic_read(uint32_t reg);

uint32_t lapic_timer_calibrate();
void lapic_timer_start(uint32_t ms, uint8_t vector);
void lapic_timer_oneshot(uint32_t ms, uint8_t vector);
void lapic_eoi();