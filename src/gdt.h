#ifndef GDT_H
#define GDT_H

#include <stdint.h>

typedef uint64_t gdt_entry_t;

struct gdt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

void gdt_init(void);
void gdt_set_entry(int index, uint64_t base, uint32_t limit, uint8_t access, uint8_t flags);

#endif