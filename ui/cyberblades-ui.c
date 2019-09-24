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
#include "cairo.h"
#include "cairoglue.h"
#ifdef BUILD_WITH_SDL
#include "display_sdl.h"
#endif
#include "historian.h"

static void swbuf_render(struct cairo_swbuf_t *swbuf) {
	swbuf_clear(swbuf, COLOR_BS_DARKBLUE);

	{
		struct font_placement_t placement = {
			.font_face = "Beon",
			.font_size = 32,
			.font_color = COLOR_BS_RED,
			.xanchor = XPOS_CENTER,
			.yanchor = YPOS_TOP,
			.yoffset = 10,
		};
		swbuf_text(swbuf, &placement, "Cyber");
	}
	{
		struct font_placement_t placement = {
			.font_face = "Beon",
			.font_size = 32,
			.font_color = COLOR_BS_BLUE,
			.xanchor = XPOS_CENTER,
			.yanchor = YPOS_TOP,
			.yoffset = 10 + 32,
		};
		swbuf_text(swbuf, &placement, "Blades");
	}
}

static void event_callback(enum ui_eventtype_t event_type, void *vevent) {
	if (event_type == EVENT_QUIT) {
		exit(EXIT_SUCCESS);
	} else if (event_type == EVENT_KEYPRESS) {
		struct ui_event_keypress_t *event = (struct ui_event_keypress_t*)vevent;
	} else if (event_type == EVENT_HISTORIAN_MESSAGE) {
		struct ui_event_historian_msg_t *event = (struct ui_event_historian_msg_t*)vevent;
		jsondom_dump(event->json);
	}
}

int main(int argc, char **argv) {
	struct display_t *display = NULL;
	if (argc == 2) {
		const char *filename = argv[1];
		display = display_init(&display_fb_calltable, (void*)filename);
#ifdef BUILD_WITH_SDL
	} else {
		struct display_sdl_init_t init_params = {
			.width = 320,
			.height = 240,
		};
		display = display_init(&display_sdl_calltable, &init_params);
		display_sdl_register_events(display, event_callback);
#endif
	}
	cairo_addfont("beon/beon-webfont.ttf");

	if (!display) {
		fprintf(stderr, "Could not create display.\n");
		exit(EXIT_FAILURE);
	}

	/* Start historian connection */
	struct historian_t *historian = historian_connect("../historian/unix_sock", event_callback);
	if (!historian) {
		fprintf(stderr, "Could not create historian connection instance.\n");
		exit(EXIT_FAILURE);
	}

	struct cairo_swbuf_t *swbuf = create_swbuf(display->width, display->height);
	for (int i = 0; i < 10; i++) {
		swbuf_render(swbuf);
		blit_swbuf_on_display(swbuf, display);
		display_commit(display);
		sleep(1);
	}
	historian_free(historian);
	free_swbuf(swbuf);
	display_free(display);

	cairo_cleanup();
	return 0;
}
