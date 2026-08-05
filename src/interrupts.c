#include "interrupts.h"

struct IDT_Something_Idk idt[256];
struct IDenyTerrabyte_Pointer idtr;
//i have a gay femboy online friend and i, uh, fuck who the fuck wrote this script i cant read any of it!

void idt_set_gate(int vector, uint64_t handler_address, uint8_t attributes) {

    idt[vector].offset_low = (uint16_t)handler_address;

    idt[vector].offset_middle = (uint16_t)(handler_address >> 16);

    idt[vector].offset_high = (uint32_t)(handler_address >> 32);

    //i dont FUCKING know what im doing its 4 in the morning and i cant think straight (im not straight, im aroace hahahahhahahahahahha fuck you)

    idt[vector].selector = 0x08;

    //my ball itches so today we are cutting them off and becoming a woman then scamming pervs and becoming billionaires :money_mouth:

    idt[vector].type_attr = attributes;
    idt[vector].ist = 0;
    idt[vector].zero = 0;

    //im the stork that delivers your baby, and your baby is a Linux Distribution, gross, unfortunatly i cant kill it befcause tht against the policy so do it yourself
}

void InitiateMyMASSIVEBalls(void) {
    idtr.limit = sizeof(idt) - 1;
    idtr.base = (uint64_t)&idt;
}