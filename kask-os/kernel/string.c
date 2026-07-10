#include "../shared/kernel.h"

/* ============================================================
 * Freestanding String Utilities
 * ============================================================ */

int k_strlen(const char* s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}

int k_strcmp(const char* a, const char* b) {
    while (*a && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

int k_strncmp(const char* a, const char* b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] != b[i]) return (unsigned char)a[i] - (unsigned char)b[i];
        if (a[i] == '\0') return 0;
    }
    return 0;
}

char* k_strcpy(char* dest, const char* src) {
    char* orig = dest;
    while ((*dest++ = *src++));
    return orig;
}

char* k_strncpy(char* dest, const char* src, int n) {
    int i;
    for (i = 0; i < n && src[i]; i++) dest[i] = src[i];
    for (; i < n; i++) dest[i] = '\0';
    return dest;
}

char* k_strcat(char* dest, const char* src) {
    char* end = dest + k_strlen(dest);
    while ((*end++ = *src++));
    return dest;
}

void* k_memset(void* ptr, int value, int num) {
    unsigned char* p = (unsigned char*)ptr;
    for (int i = 0; i < num; i++) p[i] = (unsigned char)value;
    return ptr;
}

void* k_memcpy(void* dest, const void* src, int num) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    for (int i = 0; i < num; i++) d[i] = s[i];
    return dest;
}

void k_itoa(int value, char* buf, int base) {
    char tmp[32];
    int i = 0, negative = 0;

    if (value == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    if (value < 0 && base == 10) { negative = 1; value = -value; }

    unsigned int uval = (unsigned int)value;
    while (uval > 0) {
        int rem = uval % base;
        tmp[i++] = (rem < 10) ? ('0' + rem) : ('a' + rem - 10);
        uval /= base;
    }

    int j = 0;
    if (negative) buf[j++] = '-';
    while (i > 0) buf[j++] = tmp[--i];
    buf[j] = '\0';
}

int k_isalpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int k_isdigit(char c) {
    return c >= '0' && c <= '9';
}

void* memset(void* ptr, int value, unsigned long num) {
    return k_memset(ptr, value, (int)num);
}

void* memcpy(void* dest, const void* src, unsigned long num) {
    return k_memcpy(dest, src, (int)num);
}
