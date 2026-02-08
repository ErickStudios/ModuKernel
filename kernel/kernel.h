#include <stdint.h>

typedef unsigned long long size_t;

#define NULL 0

// E/S

static uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "dN"(port));
    return ret;
}

static void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__("outb %0, %1" : : "a"(val), "dN"(port));
}
static uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ __volatile__("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
static inline void outw(unsigned short port, unsigned short value)
{
    __asm__ __volatile__("outw %0, %1" : : "a"(value), "Nd"(port));
}
void IntToString2Digits(int value, char* out) {
    out[0] = '0' + (value / 10);
    out[1] = '0' + (value % 10);
    out[2] = '\0';
}

unsigned long long InternalGetNumberOfTicksFromMachineStart();

char* InternalReadLine();

void * InternalMemoryCopy(void *dest, const void *src, unsigned int n) {
    unsigned char *d = (unsigned char*)dest;
    const unsigned char *s = (const unsigned char*)src;
    for (unsigned int i = 0; i < n; i++) {
        d[i] = s[i];
    }
    return dest;
}

void* InternalMemorySet(void* dest, int value, unsigned int count)
{
    unsigned char* ptr = (unsigned char*)dest;
    while (count--)
        *ptr++ = (unsigned char)value;

    return dest;
}

#define min(a,b) ((a) < (b) ? (a) : (b))

int InternalMemoryComp(const void *s1, const void *s2, unsigned int n) {
    const unsigned char *p1 = (const unsigned char*)s1;
    const unsigned char *p2 = (const unsigned char*)s2;
    for (unsigned int i = 0; i < n; i++) {
        if (p1[i] != p2[i]) return (int)p1[i] - (int)p2[i];
    }
    return 0;
}

void FreePool(void* ptr);
void * InternalMemMove(void *dest, const void *src, int n) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;

    if (d < s) {
        // Copiar hacia adelante
        for (int i = 0; i < n; i++) {
            d[i] = s[i];
        }
    } else if (d > s) {
        // Copiar hacia atrás (para manejar solapamiento)
        for (int i = n; i > 0; i--) {
            d[i-1] = s[i-1];
        }
    }
    return dest;
}
