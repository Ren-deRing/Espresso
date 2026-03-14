#pragma once

#include <stddef.h>
#include <stdint.h>

typedef char* string;

size_t strlen(const string str);
size_t strcmp(const string str1, const string str2);

void atoi(char *str, int* a);

size_t strcrl(string str, const char what, const char with);
size_t str_begins_with(const string str, const string with);
size_t str_backspace(string str, char c);
size_t strcount(string str, char c);
size_t strsplit(string str, char delim);
int memcmp(const void* s1, const void* s2, size_t n);
void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);