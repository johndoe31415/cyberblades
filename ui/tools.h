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

#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>

/*************** AUTO GENERATED SECTION FOLLOWS ***************/
double now(void);
void add_timespec_offset(struct timespec *timespec, int32_t offset_milliseconds);
void get_timespec_now(struct timespec *timespec);
void get_abs_timespec_offset(struct timespec *timespec, int32_t offset_milliseconds);
bool strncpycmp(char *dest, const char *src, unsigned int dest_buffer_size);
/***************  AUTO GENERATED SECTION ENDS   ***************/

#endif
