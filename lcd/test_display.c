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
#include <time.h>
#include <unistd.h>

#include "framebuffer.h"
#include "cairo.h"
#include "clock.h"

static void swbuf_blit(struct cairo_swbuf_t *swbuf, struct display_t *target) {
	for (int y = 0; y < swbuf->height; y++) {
		for (int x = 0; x < swbuf->width; x++) {
			uint32_t rgb = swbuf_get_pixel(swbuf, x, y);
			display_put_pixel(target, x, y, rgb);
		}
	}
}

static void swbuf_render(struct cairo_swbuf_t *swbuf) {
	time_t now = time(NULL);

	char time_local[16], time_utc[16], time_chicago[16], time_singapore[16], day_local[32];
	format_time_hh_mm(now, time_local, sizeof(time_local), "%H:%M", "Europe/Berlin");
	format_time_hh_mm(now, day_local, sizeof(day_local), "%A, %d.%m.%Y", "Europe/Berlin");
	format_time_hh_mm(now, time_utc, sizeof(time_utc), "%a %H:%M", "UTC");
	format_time_hh_mm(now, time_chicago, sizeof(time_chicago), "%a %H:%M", "America/Chicago");
	format_time_hh_mm(now, time_singapore, sizeof(time_singapore), "%a %H:%M", "Asia/Singapore");

	swbuf_clear(swbuf, COLOR_BLACK);

	{
		struct font_placement_t placement = {
			.font_face = "Latin Modern Mono",
			.font_size = 92,
			.font_color = COLOR_WHITE,
			.xanchor = XPOS_CENTER,
			.yanchor = YPOS_TOP,
			.yoffset = 10,
		};
		swbuf_text(swbuf, &placement, "%s", time_local);
	}
	{
		struct font_placement_t placement = {
			.font_face = "Latin Modern Sans",
			.font_size = 20,
			.font_color = COLOR_WHITE,
			.xanchor = XPOS_CENTER,
			.yanchor = YPOS_TOP,
			.yoffset = 80,
		};
		swbuf_text(swbuf, &placement, "%s", day_local);
	}
	{
		struct font_placement_t placement = {
			.font_face = "Latin Modern Sans",
			.font_size = 14,
			.font_color = COLOR_WHITE,
			.xanchor = XPOS_LEFT,
			.yanchor = YPOS_TOP,
			.yoffset = 114,
			.xoffset = 10,
		};
		swbuf_text(swbuf, &placement, "Sgp");
	}
	{
		struct font_placement_t placement = {
			.font_face = "Latin Modern Sans",
			.font_size = 14,
			.font_color = COLOR_WHITE,
			.xanchor = XPOS_LEFT,
			.yanchor = YPOS_TOP,
			.yoffset = 130,
			.xoffset = 10,
		};
		swbuf_text(swbuf, &placement, "%s", time_singapore);
	}
	{
		struct font_placement_t placement = {
			.font_face = "Latin Modern Sans",
			.font_size = 14,
			.font_color = COLOR_WHITE,
			.xanchor = XPOS_RIGHT,
			.yanchor = YPOS_TOP,
			.yoffset = 114,
			.xoffset = -10,
		};
		swbuf_text(swbuf, &placement, "NBK");
	}
	{
		struct font_placement_t placement = {
			.font_face = "Latin Modern Sans",
			.font_size = 14,
			.font_color = COLOR_WHITE,
			.xanchor = XPOS_RIGHT,
			.yanchor = YPOS_TOP,
			.yoffset = 130,
			.xoffset = -10,
		};
		swbuf_text(swbuf, &placement, "%s", time_chicago);
	}
	{
		struct font_placement_t placement = {
			.font_face = "Latin Modern Sans",
			.font_size = 14,
			.font_color = COLOR_WHITE,
			.xanchor = XPOS_CENTER,
			.yanchor = YPOS_TOP,
			.yoffset = 114,
		};
		swbuf_text(swbuf, &placement, "UTC");
	}
	{
		struct font_placement_t placement = {
			.font_face = "Latin Modern Sans",
			.font_size = 14,
			.font_color = COLOR_WHITE,
			.xanchor = XPOS_CENTER,
			.yanchor = YPOS_TOP,
			.yoffset = 130,
		};
		swbuf_text(swbuf, &placement, "%s", time_utc);
	}
}

int main(int argc, char **argv) {
	const char *filename = argv[1];
	struct display_t *display = display_init(filename);
	if (!display) {
		display = display_sw_init(320, 240);
		fprintf(stderr, "Using SW framebuffer.\n");
	}

	if (display) {
		struct cairo_swbuf_t *swbuf = create_swbuf(display->width, display->height);
		while (true) {
			swbuf_render(swbuf);
			swbuf_blit(swbuf, display);
			sleep(1);
		}
		if (!display->mmapped) {
			swbuf_dump(swbuf, "output.png");
		}
		free_swbuf(swbuf);
	}
	display_free(display);
	cairo_cleanup();
	return 0;
}
