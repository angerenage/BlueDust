#pragma once

#include <stdint.h>

void init_idt();
void set_idt_entry(int vector, void (*handler)(), uint8_t type_attr);