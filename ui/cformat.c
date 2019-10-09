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
#include "cformat.h"

static char static_buffer[128];

void cformat_time_secs(char *dest, size_t size, int time_secs) {
	if (time_secs < 60) {
		snprintf(dest, size, "%d sec", time_secs);
	} else if (time_secs < 3600) {
		snprintf(dest, size, "%d:%02d m:s", time_secs / 60, time_secs % 60);
	} else if (time_secs < 86400) {
		snprintf(dest, size, "%d:%02d:%02d h:m:s", time_secs / 3600, time_secs % 3600 / 60, time_secs % 3600 % 60);
	} else {
		snprintf(dest, size, "%d-%02d:%02d:%02d d-h:m:s", time_secs / 86400, time_secs % 86400 / 3600, time_secs % 86400 % 3600 / 60, time_secs % 86400 % 3600 % 60);
	}
}

void cformat_si_float(char *dest, size_t size, double value) {
	if (value < 1e1) {
		snprintf(dest, size, "%.2f", value / 1e0);
	} else if (value < 1e2) {
		snprintf(dest, size, "%.1f", value / 1e0);
	} else if (value < 1e3) {
		snprintf(dest, size, "%.0f", value / 1e0);
	} else if (value < 1e4) {
		snprintf(dest, size, "%.2f k", value / 1e3);
	} else if (value < 1e5) {
		snprintf(dest, size, "%.1f k", value / 1e3);
	} else if (value < 1e6) {
		snprintf(dest, size, "%.0f k", value / 1e3);
	} else if (value < 1e7) {
		snprintf(dest, size, "%.2f M", value / 1e6);
	} else if (value < 1e8) {
		snprintf(dest, size, "%.1f M", value / 1e6);
	} else if (value < 1e9) {
		snprintf(dest, size, "%.0f M", value / 1e6);
	} else if (value < 1e10) {
		snprintf(dest, size, "%.2f G", value / 1e9);
	} else if (value < 1e11) {
		snprintf(dest, size, "%.1f G", value / 1e9);
	}  else {
		snprintf(dest, size, "%.0f G", value / 1e9);
	}
}

const char *cformat_sbuf_time_secs(int time_secs) {
	cformat_time_secs(static_buffer, sizeof(static_buffer), time_secs);
	return static_buffer;
}

const char *cformat_sbuf_si_float(double value) {
	cformat_si_float(static_buffer, sizeof(static_buffer), value);
	return static_buffer;
}
