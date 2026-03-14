#include "include/string.h"

size_t strlen(const string str)
{
	size_t i = 0;
	while(str[i] != 0) i++;
	return i;
}

void atoi(char *str, int* a)
{
	int k = 0;
	while(*str)
	{
		k = (k<<3)+(k<<1)+(*str)-'0';
		str++;
	}
	*a = k;
}

size_t strcrl(string str, const char what, const char with)
{
	size_t i = 0;
	while(str[i] != 0)
	{
		if(str[i] == what) str[i] = with;
		i++;
	}
	return i;
}

size_t strcount(string str, char c)
{
	size_t i = 0;
	while(*str--)
		if(*str == c) i++;
	return i;
}

size_t str_backspace(string str, char c)
{
	size_t i = strlen(str);
	i--;
	while(i)
	{
		i--;
		if(str[i] == c)
		{
			str[i+1] = 0;
			return 1;
		}
	}
	return 0;
}

size_t strsplit(string str, char delim)
{
	size_t n = 0;
	uint32_t i = 0;
	while(str[i])
	{
		if(str[i] == delim)
		{
			str[i] = 0;
			n++;
		}
		i++;
	}
	n++;
	return n;
}

size_t str_begins_with(const string str, const string with)
{
	size_t j = strlen(with);
	size_t i = 0;
	size_t ret = 1;
	while(with[j] != 0)
	{
		if(str[i] != with[i]) { ret = 0; break; }
		j--;
		i++;
	}
	return ret;
}

size_t strcmp(string str1, string str2)
{
	size_t res=0;
	while (!(res = *(unsigned char*)str1 - *(unsigned char*)str2) && *str2)
		++str1, ++str2;

	return res;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char *p1 = s1, *p2 = s2;
    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] - p2[i];
        }
    }
    return 0;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *restrict pdest = (uint8_t *restrict)dest;
    const uint8_t *restrict psrc = (const uint8_t *restrict)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}