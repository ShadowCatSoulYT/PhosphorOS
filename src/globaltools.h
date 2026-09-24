#ifndef GLOBALTOOLS_H
#define GLOBALTOOLS_H

#include <stdint.h>

extern void itoa(unsigned int num, char* str);
extern void u64_to_hex(uint64_t value, char *text);
extern void panic(int vector);

#endif