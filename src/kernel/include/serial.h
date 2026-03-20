#pragma once

#include <stddef.h>

void serial_init();
void kprintf(const char* format, ...);
char serial_getc();
void serial_putc(char c);