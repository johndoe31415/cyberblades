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

struct server_state_t {
	struct historian_t *historian;
};

static void swbuf_render(const struct server_state_t *server_state, struct cairo_swbuf_t *swbuf) {
	swbuf_clear(swbuf, COLOR_BS_DARKBLUE);

	{
		const struct font_placement_t placement = {
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
		const struct font_placement_t placement = {
			.font_face = "Beon",
			.font_size = 32,
			.font_color = COLOR_BS_BLUE,
			.xanchor = XPOS_CENTER,
			.yanchor = YPOS_TOP,
			.yoffset = 10 + 32,
		};
		swbuf_text(swbuf, &placement, "Blades");
	}

	{
		const struct font_placement_t text_placement = {
			.font_face = "Latin Modern Sans",
			.font_size = 16,
			.font_color = COLOR_BS_BLUE,
			.xanchor = XPOS_CENTER,
			.yanchor = YPOS_BOTTOM,
			.yoffset = -9,
		};
		const struct anchored_placement_t rect_placement = {
			.width = 200,
			.height = 25,
			.src_anchor = {
				.x = XPOS_CENTER,
				.y = YPOS_BOTTOM,
			},
			.dst_anchor = {
				.x = XPOS_CENTER,
				.y = YPOS_BOTTOM,
			},
			.yoffset = -3,
		};
		switch (server_state->historian->connection_state) {
			case UNCONNECTED:
				swbuf_rect(swbuf, &(const struct rect_placement_t){
					.placement = rect_placement,
					.fill_color = COLOR_POMEGRANATE,
					.round = 10,
				});
				swbuf_text(swbuf, &text_placement, "Historian unavailable");
				break;

			case CONNECTED_WAITING:
				swbuf_rect(swbuf, &(const struct rect_placement_t){
					.placement = rect_placement,
					.fill_color = COLOR_SUN_FLOWER,
					.round = 10,
				});
				swbuf_text(swbuf, &text_placement, "Unconnected");
				break;

			case CONNECTED_READY:
				swbuf_rect(swbuf, &(const struct rect_placement_t){
					.placement = rect_placement,
					.fill_color = COLOR_EMERLAND,
					.round = 10,
				});
				swbuf_text(swbuf, &text_placement, "Ready for action");
				break;
		}
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
	cairo_addfont("../external/beon/beon-webfont.ttf");

	if (!display) {
		fprintf(stderr, "Could not create display.\n");
		exit(EXIT_FAILURE);
	}

	struct server_state_t server_state = { 0 };

	/* Start historian connection */
	server_state.historian = historian_connect("../historian/unix_sock", event_callback);
	if (!server_state.historian) {
		fprintf(stderr, "Could not create historian connection instance.\n");
		exit(EXIT_FAILURE);
	}

	struct cairo_swbuf_t *swbuf = create_swbuf(display->width, display->height);
	for (int i = 0; i < 1000; i++) {
		swbuf_render(&server_state, swbuf);
		blit_swbuf_on_display(swbuf, display);
		display_commit(display);
		sleep(1);
	}
	historian_free(server_state.historian);
	free_swbuf(swbuf);
	display_free(display);

	cairo_cleanup();
	return 0;
}
