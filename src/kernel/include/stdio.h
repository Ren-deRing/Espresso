#pragma once

#include <stddef.h>
#include <stdarg.h>

typedef struct {
    int dummy;
} FILE;

#define stdout ((FILE*)1)
#define stderr ((FILE*)2)
#define stdin  ((FILE*)0)

int printf(const char* format, ...);
int sprintf(char* str, const char* format, ...);
int snprintf(char* str, size_t size, const char* format, ...);
int vsnprintf(char* str, size_t size, const char* format, va_list ap);

size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream);
int fputc(int c, FILE* stream);
int putc(int c, FILE* stream);
int fputs(const char* s, FILE* stream);
int fflush(FILE* stream);