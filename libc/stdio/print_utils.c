#include "print_utils.h"

void itoa(int value, char* buffer, int base) {
	char* ptr = buffer;
	char* ptr1 = buffer;
	char tmp_char;
	int tmp_value;

	if (value == 0) {
		*ptr++ = '0';
		*ptr = '\0';
		return;
	}

	int is_negative = 0;
	if (value < 0 && base == 10) {
		is_negative = 1;
		value = -value;
	}

	while (value != 0) {
		tmp_value = value % base;
		*ptr++ = (tmp_value < 10) ? (tmp_value + '0') : (tmp_value - 10 + 'A');
		value /= base;
	}

	if (is_negative) {
		*ptr++ = '-';
	}

	*ptr = '\0';

	while (ptr1 < --ptr) {
		tmp_char = *ptr1;
		*ptr1++ = *ptr;
		*ptr = tmp_char;
	}
}

void itoa_ll(unsigned long long value, char* buffer, int base) {
	char* ptr = buffer;
	char* ptr1 = buffer;
	char tmp_char;
	unsigned long long tmp_value;

	if (value == 0) {
		*ptr++ = '0';
		*ptr = '\0';
		return;
	}

	while (value != 0) {
		tmp_value = value % base;
		*ptr++ = (tmp_value < 10) ? (tmp_value + '0') : (tmp_value - 10 + 'A');
		value /= base;
	}

	*ptr = '\0';

	while (ptr1 < --ptr) {
		tmp_char = *ptr1;
		*ptr1++ = *ptr;
		*ptr = tmp_char;
	}
}

void utoa_ll(unsigned long long value, char* buffer, int base) {
	char* ptr = buffer;
	char* ptr1 = buffer;
	char tmp_char;
	unsigned long long tmp_value;

	do {
		tmp_value = value;
		value /= base;
		*ptr++ = "0123456789abcdef"[tmp_value - value * base];
	} while (value);

	*ptr-- = '\0';

	while (ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr-- = *ptr1;
		*ptr1++ = tmp_char;
	}
}