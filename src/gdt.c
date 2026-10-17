#include "gdt.h"

static gdt_entry_t gdt[5];
static struct gdt_ptr gdt_r;

void gdt_set_entry(int index, uint64_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    
}

void gdt_init() {
    
}