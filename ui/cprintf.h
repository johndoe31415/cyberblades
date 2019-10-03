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

#ifndef __CPRINTF_H__
#define __CPRINTF_H__

#include <stdarg.h>

#define CPRINTF_FMT_TIME_SECS			"%1!"
#define CPRINTF_FMT_SIZE_FLOAT			"%2!"

/*************** AUTO GENERATED SECTION FOLLOWS ***************/
void cvsnprintf(char *str, size_t size, const char *fmt, va_list ap);
void csnprintf(char *str, size_t size, const char *fmt, ...);
void cprintf(const char *fmt, ...);
/***************  AUTO GENERATED SECTION ENDS   ***************/

#endif
