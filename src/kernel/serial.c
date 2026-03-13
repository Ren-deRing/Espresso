#include <stdarg.h>
#include <stdint.h>

#include "serial.h"
#include "interrupt.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

#pragma GCC diagnostic pop

static volatile int serial_lock = 0;

static void lock_serial() {
  while (__atomic_test_and_set(&serial_lock, __ATOMIC_ACQUIRE)) {
    __asm__ volatile ("pause");
  }
}

static void unlock_serial() {
  __atomic_clear(&serial_lock, __ATOMIC_RELEASE);
}

static inline uint8_t inb(uint16_t port) {
  uint8_t ret;
  asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
  asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void serial_init() {
  outb(0x3F9, 0x00);  // 인터럽트 금지
  outb(0x3FB, 0x80);  // Enable DLAB
  outb(0x3F8, 0x01);  // 115200 Baud
  outb(0x3F9, 0x00);  // MSB
  outb(0x3FB, 0x03);  // 8비트, 패리티 없음, 스탑 비트 1개 (DLAB off)
  outb(0x3FA, 0xC7);  // FIFO 활성화, 큐 초기화
  outb(0x3FC, 0x0B);  // IRQs 활성화, RTS/DTR 세팅
}

static char* serial_write_cb(const char* buf, void* user, int len) {
  (void)user;

  for (int i = 0; i < len; i++) {
    while ((inb(0x3F8 + 5) & 0x20) == 0);
    outb(0x3F8, buf[i]);
  }
  return (char*)buf;
}

void printf(const char* format, ...) {
  char tmp[STB_SPRINTF_MIN];
  va_list args;
  
  irq_state_t state = save_irq_disable();
  
  lock_serial();

  va_start(args, format);
  stbsp_vsprintfcb(serial_write_cb, NULL, tmp, format, args);
  va_end(args);

  unlock_serial();
  restore_irq(state);
}