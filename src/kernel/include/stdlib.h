#pragma once

#include <stddef.h>
#include <stdint.h>

unsigned long      strtoul(const char* nptr, char** endptr, int base);
unsigned long long strtoull(const char* nptr, char** endptr, int base);
double             strtod(const char* nptr, char** endptr);

void* malloc(size_t size);
void* calloc(size_t nmemb, size_t size);
void* realloc(void* ptr, size_t size);
void free(void* ptr);

int atoi(const char* str);

void abort(void) __attribute__((noreturn));