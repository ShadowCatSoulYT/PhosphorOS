// =====================================================================
// Includes
// =====================================================================

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include "gdt.h"

// =====================================================================
// Macros / Defines
// =====================================================================

#define LMRQ   __attribute__((used, section(".limine_requests")))
#define LMRQ_S __attribute__((used, section(".limine_requests_start")))
#define LMRQ_E __attribute__((used, section(".limine_requests_end")))
#define STVL   static volatile

// =====================================================================
// Limine bootloader requests
// =====================================================================

LMRQ_S
STVL uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

LMRQ
STVL uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(6);

LMRQ
STVL struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

LMRQ_E
STVL uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

// =====================================================================
// Standard library-like helpers
// =====================================================================

void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
    uint8_t *restrict pdest = dest;
    const uint8_t *restrict psrc = src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = dest;
    const uint8_t *psrc = src;

    if ((uintptr_t)src > (uintptr_t)dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if ((uintptr_t)src < (uintptr_t)dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = s1;
    const uint8_t *p2 = s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

// =====================================================================
// Kernel utility functions
// =====================================================================

void hcf(void) {
    __asm__ ("cli");
    for (;;) {
        __asm__ ("hlt");
    }
}

int getlenght(char* str) {
    int i = 0;
    while (str[i] != '\0') {
        i++;
    }
    return i;
}

void itoa(unsigned int num, char* str) {
    int i = 0;
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    while (num > 0) {
        str[i] = (num % 10) + '0';
        num = num / 10;
        i++;
    }
    str[i] = '\0';
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

void sleep(uint64_t cycles) {
    for (volatile uint64_t i = 0; i < cycles; i++) {
        __asm__ volatile("pause");
    }
}

// =====================================================================
// Framebuffer drawing
// =====================================================================

void putpx(uint32_t x, uint32_t y, uint32_t r, uint32_t g, uint32_t b) {
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    uint32_t *fb = framebuffer->address;
    uint32_t pitch = framebuffer->pitch / sizeof(uint32_t);
    fb[y * pitch + x] = (r << 16) | (g << 8) | b;
}

void rect(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t r, uint32_t g, uint32_t b) {
    for (uint32_t i = y1; i < y2; i++) {
        for (uint32_t j = x1; j < x2; j++) {
            putpx(j, i, r, g, b);
        }
    }
}

// =====================================================================
// Terminal
// =====================================================================

void write(char* text, uint32_t r, uint32_t g, uint32_t b) {}

// =====================================================================
// Others i guess
// =====================================================================

void killcores(void) {
    
}

// =====================================================================
// Kernel entry point
// =====================================================================

void _start(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
        hcf();
    }

    init_gdt();

    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];

    rect(0, 0, fb->width, fb->height, 0, 0, 255);

    rect(50, 50, 150, 150, 255, 0, 0);

    volatile int x = 2/0;
    //i forgot if i added exceptions cuz im a dimwit
    
    hcf();
}
