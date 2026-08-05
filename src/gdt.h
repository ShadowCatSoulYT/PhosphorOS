#ifndef GDT_H
#define GDT_H

#include <stdint.h>

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

#define GDT_PRESENT       (1 << 7)
#define GDT_RING0         (0 << 5)
#define GDT_SYSTEM        (1 << 4)
#define GDT_EXECUTABLE    (1 << 3)
#define GDT_READ_WRITE    (1 << 1)
#define GDT_LONG_MODE     (1 << 5)

void init_gdt(void);

#endif