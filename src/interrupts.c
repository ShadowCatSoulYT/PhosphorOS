#include "interrupts.h"
#include "terminal.h"
#include "globaltools.h"

struct IDT_entery idt[256];
struct IDTP idtr;
extern void exception_0(void);
extern void exception_13(void);
extern void exception_14(void);

void idt_set_gate(int vector, uint64_t handler_address, uint8_t attributes) {

    idt[vector].offset_low = (uint16_t)handler_address;

    idt[vector].offset_middle = (uint16_t)(handler_address >> 16);

    idt[vector].offset_high = (uint32_t)(handler_address >> 32);

    idt[vector].selector = 0x08;

    idt[vector].type_attr = attributes;
    idt[vector].ist = 0;
    idt[vector].zero = 0;

}

void general_handler(registers_t* r, uint64_t cr2) {
    switch (r->vector) {
        case 0:
            nwrite("[EXC] Division Error (#DE)", 3, false);
            r->rax = 0;
            r->rdx = 0;
            r->rip += 3;
            break;
        case 13:
            nwrite("[EXC] General Protection Fault (#GP)", 3, false);
            break;
        case 14:
            nwrite("[EXC] Page Fault (#PF)", 3, false);
            break;
    }
    char buf[19];
    u64_to_hex(r->error_code, buf);
    write("    [ERC] 0x", 3, false);
    nwrite(buf, 3, false);
    if (r->vector == 14) {
        char cr2b[19];
        u64_to_hex(cr2, cr2b);
        write("    [CR2] 0x", 3, false);
        nwrite(cr2b, 3, false);
    }
}

void InitiateIDT(void) {
    idt_set_gate(0, (uint64_t)exception_0, 0x8E);
    idt_set_gate(13, (uint64_t)exception_13, 0x8E);
    idt_set_gate(14, (uint64_t)exception_14, 0x8E);
    
    idtr.limit = sizeof(idt) - 1;
    idtr.base = (uint64_t)&idt;
    __asm__ volatile ("lidt %0" : : "m"(idtr));
    nwrite("[LOAD] IDT loaded", 3, true);
}

