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
#include <unistd.h>
#include <stdlib.h>

#include "display.h"
#include "display_fb.h"
#include "display_sdl.h"
#include "cairo.h"
#include "cairoglue.h"

static void swbuf_render(struct cairo_swbuf_t *swbuf) {
	static uint8_t counter;
	swbuf_clear(swbuf, COLOR_GRAY);

	{
		struct font_placement_t placement = {
			//.font_face = "Latin Modern Mono",
			.font_face = "Beon",
			.font_size = 32,
			.font_color = COLOR_BS_RED,
			.xanchor = XPOS_CENTER,
			.yanchor = YPOS_TOP,
			.yoffset = 10,
		};
		swbuf_text(swbuf, &placement, "Cyber Blades %d", counter++);
	}
}

int main(int argc, char **argv) {
	struct display_t *display = NULL;
	if (argc == 2) {
		const char *filename = argv[1];
		display = display_init(&display_fb_calltable, (void*)filename);
	} else {
		struct display_sdl_init_t init_params = {
			.width = 320,
			.height = 240,
		};
		display = display_init(&display_sdl_calltable, &init_params);
	}

	if (!display) {
		fprintf(stderr, "Could not create display.\n");
		exit(EXIT_FAILURE);
	}

	struct cairo_swbuf_t *swbuf = create_swbuf(display->width, display->height);
	for (int i = 0; i < 10; i++) {
		swbuf_render(swbuf);
		blit_swbuf_on_display(swbuf, display);
		display_commit(display);
#if 0
		if (!display->mmapped) {
			swbuf_dump(swbuf, "output.png");
			break;
		}
#endif
		sleep(1);
	}
	free_swbuf(swbuf);
	display_free(display);

	cairo_cleanup();
	return 0;
}
