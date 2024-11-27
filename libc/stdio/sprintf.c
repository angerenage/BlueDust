#include <stdio.h>

#include <stddef.h>

#include "print_utils.h"

void agregate(char* buffer, const char *input, int* i) {
	while (*input != '\0') {
		buffer[*i] = *input;
		(*i)++;
		input++;
	}
}

void sprintf(char* buffer, const char* __restrict format, ...) {
	va_list args;
	va_start(args, format);
	
	char tmp_buffer[32];
	const char* p = format;
	int j = 0;

	while (*p != '\0') {
		if (*p == '%' && *(p + 1) != '\0') {
			p++;
			switch (*p) {
				case 'd': {  // entier signé en base 10
					int i = va_arg(args, int);
					itoa(i, tmp_buffer, 10);
					agregate(buffer, tmp_buffer, &j);
					break;
				}
				case 'x': {  // entier non signé en base 16
					int i = va_arg(args, int);
					itoa(i, tmp_buffer, 16);
					agregate(buffer, tmp_buffer, &j);
					break;
				}
				case 'c': {  // caractère
					char c = (char)va_arg(args, int);
					char str[2] = {c, '\0'};
					agregate(buffer, str, &j);
					break;
				}
				case 's': {  // chaîne de caractères
					char* s = va_arg(args, char*);
					agregate(buffer, s, &j);
					break;
				}
				case 'l': {
					if (*(p + 1) == 'l') {
						if (*(p + 2) == 'x') {  // %llx
							p += 2;
							unsigned long long ull = va_arg(args, unsigned long long);
							utoa_ll(ull, tmp_buffer, 16);
							agregate(buffer, tmp_buffer, &j);
						}
						else if (*(p + 2) == 'u') {  // %llu
							p += 2;
							unsigned long long ull = va_arg(args, unsigned long long);
							utoa_ll(ull, tmp_buffer, 10);
							agregate(buffer, tmp_buffer, &j);
						}
					}
					break;
				}
				case 'z': {  // pour %zu
					if (*(p + 1) == 'u') {
						p++;
						size_t z = va_arg(args, size_t);
						utoa_ll(z, tmp_buffer, 10);
						agregate(buffer, tmp_buffer, &j);
					}
					break;
				}
				case 'p': {  // pointeur
					void* ptr = va_arg(args, void*);
					utoa_ll((unsigned long long)ptr, tmp_buffer, 16);

					agregate(buffer, "0x", &j);
					agregate(buffer, tmp_buffer, &j);
					break;
				}
				default: {  // caractère inconnu
					agregate(buffer, "%", &j);
					agregate(buffer, (const char[]){*p, '\0'}, &j);
					break;
				}
			}
		}
		else {
			agregate(buffer, (const char[]){*p, '\0'}, &j);
		}
		p++;
	}

	va_end(args);
}