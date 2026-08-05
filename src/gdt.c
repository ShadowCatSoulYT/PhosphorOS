#include "gdt.h"

static struct gdt_entry gdt[3];
static struct gdt_ptr   gp;

static void gdt_set_gate(int num, uint8_t access, uint8_t granularity) {
    gdt[num].base_low    = 0;
    gdt[num].base_middle = 0;
    gdt[num].base_high   = 0;
    gdt[num].limit_low   = 0;

    gdt[num].access      = access;
    gdt[num].granularity = granularity & 0xF0;
}

void init_gdt(void) {
    gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gp.base  = (uint64_t)&gdt;

    gdt_set_gate(0, 0, 0);

    gdt_set_gate(1, GDT_PRESENT | GDT_RING0 | GDT_SYSTEM | GDT_EXECUTABLE | GDT_READ_WRITE, GDT_LONG_MODE);

    gdt_set_gate(2, GDT_PRESENT | GDT_RING0 | GDT_SYSTEM | GDT_READ_WRITE, 0);

    __asm__ volatile (
        "lgdt %0\n\t"
        "push $0x08\n\t"
        "lea 1f(%%rip), %%rax\n\t"
        "push %%rax\n\t"
        "lretq\n\t"
        "1:\n\t"
        "mov $0x10, %%ax\n\t"
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"
        "mov %%ax, %%ss\n\t"
        :
        : "m" (gp)
        : "rax", "memory"
    );
}