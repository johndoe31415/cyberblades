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
#include <stddef.h>
#include <sys/time.h>
#include "tools.h"

double now(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + (1e-6 * tv.tv_usec);
}

void add_timespec_offset(struct timespec *timespec, int32_t offset_milliseconds) {
	int32_t offset_full_seconds = offset_milliseconds / 1000;
	int32_t offset_full_nanoseconds = 1000000 * (offset_milliseconds % 1000);
	timespec->tv_sec += offset_full_seconds;
	timespec->tv_nsec += offset_full_nanoseconds;
	if (timespec->tv_nsec < 0) {
		timespec->tv_sec -= 1;
		timespec->tv_nsec += 1000000000;
	} else if (timespec->tv_nsec >= 1000000000) {
		timespec->tv_sec += 1;
		timespec->tv_nsec -= 1000000000;
	}
}

void get_timespec_now(struct timespec *timespec) {
	struct timeval now;
	if (gettimeofday(&now, NULL) != 0) {
		perror("gettimeofday");
		return;
	}
	timespec->tv_sec = now.tv_sec;
	timespec->tv_nsec = now.tv_usec * 1000;
}

void get_abs_timespec_offset(struct timespec *timespec, int32_t offset_milliseconds) {
	get_timespec_now(timespec);
	add_timespec_offset(timespec, offset_milliseconds);
}
