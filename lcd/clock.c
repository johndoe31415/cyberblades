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
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "clock.h"

static void set_timezone(const char *tzname) {
	if (tzname) {
		char env_var[strlen(tzname) + 2];
		snprintf(env_var, sizeof(env_var), ":%s", tzname);
		setenv("TZ", env_var, 1);
	} else {
		unsetenv("TZ");
	}
	tzset();
}

void format_time_hh_mm(time_t ts, char *result, size_t maxlen, const char *fmt, const char *tzname) {
	const char *previous_tz = getenv("TZ");
	set_timezone(tzname);
	struct tm tm;
	localtime_r(&ts, &tm);
	strftime(result, maxlen, fmt, &tm);
	set_timezone(previous_tz);
}
