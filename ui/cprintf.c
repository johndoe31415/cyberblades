/*
	pibeatsaber - Beat Saber historian application that tracks players
	Copyright (C) 2019-2019 Johannes Bauer

	This file is part of pibeatsaber.

	pibeatsaber is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; this program is ONLY licensed under
	version 3 of the License, later versions are explicitly excluded.

	pibeatsaber is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.

	Johannes Bauer <JohannesBauer@gmx.de>
*/

#include <stdio.h>
#include <stdarg.h>
#include <printf.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "cprintf.h"

#define PRINTF_FLAG_CHARACTERS					"#0- +'I"
#define PRINTF_PRECISION_CHARACTERS				"*.0123456789"
#define PRINTF_LENGTH_MODIFIER_CHARACTERS		"hlqLjzZt"
#define PRINTF_NON_CONVERSION_SPECIFIER			(PRINTF_FLAG_CHARACTERS PRINTF_PRECISION_CHARACTERS PRINTF_LENGTH_MODIFIER_CHARACTERS)

static int cprintf_arg(char *str, size_t size, const char *fmt, va_list ap) {
	if (!strcmp(fmt, CPRINTF_FMT_TIME_SECS)) {
		unsigned int secs = va_arg(ap, unsigned int);
		if (secs < 60) {
			return snprintf(str, size, "%d sec", secs);
		} else if (secs < 3600) {
			return snprintf(str, size, "%d:%02d m:s", secs / 60, secs % 60);
		} else if (secs < 86400) {
			return snprintf(str, size, "%d:%02d:%02d h:m:s", secs / 3600, secs % 3600 / 60, secs % 3600 % 60);
		} else {
			return snprintf(str, size, "%d-%02d:%02d:%02d d-h:m:s", secs / 86400, secs % 86400 / 3600, secs % 86400 % 3600 / 60, secs % 86400 % 3600 % 60);
		}
	} else if (!strcmp(fmt, CPRINTF_FMT_SIZE_FLOAT)) {
		double value = va_arg(ap, double);
		if (value < 1e1) {
			return snprintf(str, size, "%.2f", value / 1e0);
		} else if (value < 1e2) {
			return snprintf(str, size, "%.1f", value / 1e0);
		} else if (value < 1e3) {
			return snprintf(str, size, "%.0f", value / 1e0);
		} else if (value < 1e4) {
			return snprintf(str, size, "%.2f k", value / 1e3);
		} else if (value < 1e5) {
			return snprintf(str, size, "%.1f k", value / 1e3);
		} else if (value < 1e6) {
			return snprintf(str, size, "%.0f k", value / 1e3);
		} else if (value < 1e7) {
			return snprintf(str, size, "%.2f M", value / 1e6);
		} else if (value < 1e8) {
			return snprintf(str, size, "%.1f M", value / 1e6);
		} else if (value < 1e9) {
			return snprintf(str, size, "%.0f M", value / 1e6);
		} else if (value < 1e10) {
			return snprintf(str, size, "%.2f G", value / 1e9);
		} else if (value < 1e11) {
			return snprintf(str, size, "%.1f G", value / 1e9);
		}  else {
			return snprintf(str, size, "%.0f G", value / 1e9);
		}
	} else {
		return snprintf(str, size, "%s", fmt);
	}
}

void cvsnprintf(char *str, size_t size, const char *fmt, va_list ap) {
	if (size < 1) {
		return;
	} else if (size == 1) {
		str[0] = 0;
		return;
	}

	const char *cur = fmt;
	char fmtstr[32];
	char c;
	unsigned int fmtstr_offset = 0;
	unsigned int out_offset = 0;
	while (((c = (*cur++)) != 0) && (out_offset < size - 1)) {
		if (fmtstr_offset == 0) {
			if (c == '%') {
				/* Begin of format string */
				fmtstr[0] = '%';
				fmtstr_offset = 1;
			} else {
				/* Regular character */
				str[out_offset++] = c;
			}
		} else {
			/* Are currently in format string */
			fmtstr[fmtstr_offset++] = c;
			bool modifier = (strchr(PRINTF_NON_CONVERSION_SPECIFIER, c) != NULL);
			if (!modifier || (fmtstr_offset == sizeof(fmtstr) - 1)) {
				/* Final character */
				if (c == '%') {
					str[out_offset++] = '%';
				} else if (c == '!') {
					/* Custom formatter */
					fmtstr[fmtstr_offset++] = 0;
					out_offset += cprintf_arg(str + out_offset, size - out_offset, fmtstr, ap);
				} else {
					fmtstr[fmtstr_offset++] = 0;
					out_offset += vsnprintf(str + out_offset, size - out_offset, fmtstr, ap);
				}
				if (out_offset > size - 1) {
					out_offset = size - 1;
				}
				fmtstr_offset = 0;
			}
		}
	}
	str[out_offset] = 0;
}

void csnprintf(char *str, size_t size, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	cvsnprintf(str, size, fmt, ap);
	va_end(ap);
}

void cprintf(const char *fmt, ...) {
	char buffer[1024];
	va_list ap;
	va_start(ap, fmt);
	cvsnprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);
	printf("%s", buffer);
}

#ifdef __TEST_CPRINTF__
// gcc -std=c11 -Wall -o cprintf -D__TEST_CPRINTF__ cprintf.c && ./cprintf

int main(void) {
	cprintf("Foo! >%2!< %% >%1!< Muh\n", 929993456.1234, 123456);
	return 0;
}
#endif
