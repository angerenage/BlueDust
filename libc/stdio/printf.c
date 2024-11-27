#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include <kernel/tty.h>

#include "print_utils.h"

static void print(const char* str) {
	while (*str != '\0') {
		terminal_putchar(*str);
		str++;
	}
}

void printf(const char* format, ...) {
	va_list args;
	va_start(args, format);

	char buffer[32];
	const char* p = format;

	while (*p != '\0') {
		if (*p == '%' && *(p + 1) != '\0') {
			p++;
			switch (*p) {
				case 'd': {  // entier signé en base 10
					int i = va_arg(args, int);
					itoa(i, buffer, 10);
					print(buffer);
					break;
				}
				case 'x': {  // entier non signé en base 16
					int i = va_arg(args, int);
					itoa(i, buffer, 16);
					print(buffer);
					break;
				}
				case 'c': {  // caractère
					char c = (char)va_arg(args, int);
					char str[2] = {c, '\0'};
					print(str);
					break;
				}
				case 's': {  // chaîne de caractères
					char* s = va_arg(args, char*);
					print(s);
					break;
				}
				case 'l': {
					if (*(p + 1) == 'l') {
						if (*(p + 2) == 'x') {  // %llx
							p += 2;
							unsigned long long ull = va_arg(args, unsigned long long);
							utoa_ll(ull, buffer, 16);
							print(buffer);
						}
						else if (*(p + 2) == 'u') {  // %llu
							p += 2;
							unsigned long long ull = va_arg(args, unsigned long long);
							utoa_ll(ull, buffer, 10);
							print(buffer);
						}
					}
					break;
				}
				case 'z': {  // pour %zu
					if (*(p + 1) == 'u') {
						p++;
						size_t z = va_arg(args, size_t);
						utoa_ll(z, buffer, 10);
						print(buffer);
					}
					break;
				}
				case 'p': {  // pointeur
					void* ptr = va_arg(args, void*);
					utoa_ll((unsigned long long)ptr, buffer, 16);
					print("0x");
					print(buffer);
					break;
				}
				default: {  // caractère inconnu
					print("%");
					print((const char[]){*p, '\0'});
					break;
				}
			}
		}
		else {
			print((const char[]){*p, '\0'});
		}
		p++;
	}

	va_end(args);
}