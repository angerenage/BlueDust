#pragma once

#include <stdint.h>

void outb(unsigned short port, unsigned char value);
uint8_t inb(unsigned short port);