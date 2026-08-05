#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>

struct IDT_Something_Idk {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  ist;
    uint8_t  type_attr;
    uint16_t offset_middle;
    uint32_t offset_high;
    uint32_t zero;
}__attribute__((packed));

struct  {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

#endif