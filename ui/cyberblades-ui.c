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
#include <string.h>
#include "display.h"
#include "display_fb.h"
#include "cairo.h"
#include "cairoglue.h"
#ifdef BUILD_WITH_SDL
#include "display_sdl.h"
#endif
#include "historian.h"
#include "tools.h"
#include "isleep.h"

enum ui_screen_t {
	MAIN_SCREEN = 0,
	GAME_SCREEN = 1,
	FINISH_SCREEN = 2,
};

struct server_state_t {
	enum ui_screen_t ui_screen;
	double screen_shown_at_ts;
	char current_player[32];
	unsigned int current_playtime;
	unsigned int score_sum;
	const char *song_author;
	const char *song_title;
	const char *level_author;
	unsigned int current_score;
	struct historian_t *historian;
	struct isleep_t isleep;
};

static void swbuf_render(const struct server_state_t *server_state, struct cairo_swbuf_t *swbuf) {
	swbuf_clear(swbuf, COLOR_BS_DARKBLUE);
	if (server_state->ui_screen == MAIN_SCREEN) {
		{
			const struct font_placement_t placement = {
				.font_face = "Beon",
				.font_size = 32,
				.font_color = COLOR_BS_RED,
				.placement = {
					.src_anchor = {
						.x = XPOS_RIGHT,
						.y = YPOS_TOP,
					},
					.dst_anchor = {
						.x = XPOS_CENTER,
						.y = YPOS_TOP,
					},
					.yoffset = 4,
					.xoffset = -12,
				}
			};
			swbuf_text(swbuf, &placement, "Cyber");
		}
		{
			const struct font_placement_t placement = {
				.font_face = "Beon",
				.font_size = 32,
				.font_color = COLOR_BS_BLUE,
				.placement = {
					.src_anchor = {
						.x = XPOS_LEFT,
						.y = YPOS_TOP,
					},
					.dst_anchor = {
						.x = XPOS_CENTER,
						.y = YPOS_TOP,
					},
					.yoffset = 4,
					.xoffset = 0,
				}
			};
			swbuf_text(swbuf, &placement, "Blades");
		}

		{
			const struct font_placement_t text_placement = {
				.font_face = "Latin Modern Sans",
				.font_size = 16,
				.font_color = COLOR_BS_BLUE,
				.placement = {
					.src_anchor = {
						.x = XPOS_CENTER,
						.y = YPOS_CENTER,
					},
					.dst_anchor = {
						.x = XPOS_CENTER,
						.y = YPOS_BOTTOM,
					},
					.yoffset = -20,
				}
			};
			const struct anchored_placement_t rect_placement = {
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
						.color = COLOR_POMEGRANATE,
						.fill = true,
						.round = 10,
						.width = 200,
						.height = 25,
					});
					swbuf_text(swbuf, &text_placement, "Historian unavailable");
					break;

				case CONNECTED_WAITING:
					swbuf_rect(swbuf, &(const struct rect_placement_t){
						.placement = rect_placement,
						.color = COLOR_SUN_FLOWER,
						.fill = true,
						.round = 10,
						.width = 200,
						.height = 25,
					});
					swbuf_text(swbuf, &text_placement, "Unconnected");
					break;

				case CONNECTED_READY:
					swbuf_rect(swbuf, &(const struct rect_placement_t){
						.placement = rect_placement,
						.color = COLOR_EMERLAND,
						.fill = true,
						.round = 10,
						.width = 200,
						.height = 25,
					});
					swbuf_text(swbuf, &text_placement, "Ready for action");
					break;
			}
		}

		{
			const struct font_placement_t text_placement = {
				.font_face = "Latin Modern Sans",
				.font_size = 24,
				.font_color = server_state->current_player[0] ? COLOR_SILVER : COLOR_POMEGRANATE,
				.placement = {
					.src_anchor = {
						.x = XPOS_CENTER,
						.y = YPOS_CENTER,
					},
					.dst_anchor = {
						.x = XPOS_CENTER,
						.y = YPOS_TOP,
					},
					.yoffset = 55,
				}
			};
			swbuf_text(swbuf, &text_placement, "Player: %s", server_state->current_player[0] ? server_state->current_player : "—");
		}
		if (server_state->current_player) {
			{
				const struct font_placement_t text_placement = {
					.font_face = "Latin Modern Sans",
					.font_size = 24,
					.font_color = server_state->current_player ? COLOR_SILVER : COLOR_POMEGRANATE,
					.placement = {
						.src_anchor = {
							.x = XPOS_CENTER,
							.y = YPOS_CENTER,
						},
						.dst_anchor = {
							.x = XPOS_CENTER,
							.y = YPOS_TOP,
						},
						.yoffset = 55 + 24,
					}
				};
				swbuf_text(swbuf, &text_placement, "Playtime today: %d sec", server_state->current_playtime);
			}
		}

	} if (server_state->ui_screen == GAME_SCREEN) {
		{
			const struct font_placement_t placement = {
				.font_face = "Beon",
				.font_size = 32,
				.font_color = COLOR_BS_RED,
				.placement = {
					.src_anchor = {
						.x = XPOS_CENTER,
						.y = YPOS_TOP,
					},
					.dst_anchor = {
						.x = XPOS_CENTER,
						.y = YPOS_TOP,
					},
					.yoffset = 4,
					.xoffset = -12,
				}
			};
			swbuf_text(swbuf, &placement, "Game On");
		}
		{
			const struct font_placement_t text_placement = {
				.font_face = "Latin Modern Sans",
				.font_size = 24,
				.font_color = server_state->current_player[0] ? COLOR_SILVER : COLOR_POMEGRANATE,
				.placement = {
					.src_anchor = {
						.x = XPOS_CENTER,
						.y = YPOS_CENTER,
					},
					.dst_anchor = {
						.x = XPOS_CENTER,
						.y = YPOS_TOP,
					},
					.yoffset = 55,
				}
			};
			swbuf_text(swbuf, &text_placement, "%ld", server_state->current_score);
		}
	} if (server_state->ui_screen == FINISH_SCREEN) {
	}
}

static void event_callback(enum ui_eventtype_t event_type, void *vevent, void *ctx) {
	struct server_state_t *server_state = (struct server_state_t*)ctx;

	if (event_type == EVENT_QUIT) {
		exit(EXIT_SUCCESS);
	} else if (event_type == EVENT_KEYPRESS) {
		struct ui_event_keypress_t *event = (struct ui_event_keypress_t*)vevent;

	} else if (event_type == EVENT_HISTORIAN_MESSAGE) {
		struct ui_event_historian_msg_t *event = (struct ui_event_historian_msg_t*)vevent;
		jsondom_dump(event->json);

		struct jsondom_t *json_status = jsondom_get_dict_dict(event->json, "status");
		struct jsondom_t *json_connection = jsondom_get_dict_dict(json_status, "connection");
		if (json_connection) {
			jsondom_dump(json_connection);
			const char *current_player = jsondom_get_dict_str(json_connection, "current_player");
			if (current_player) {
				strncpy(server_state->current_player, current_player, sizeof(server_state->current_player) - 1);
			} else {
				server_state->current_player[0] = 0;
			}

			if (jsondom_get_dict_bool(json_connection, "in_game")) {
				server_state->ui_screen = GAME_SCREEN;
				server_state->screen_shown_at_ts = now();
			}
		}

		struct jsondom_t *json_current_game_perf = jsondom_get_dict_dict(jsondom_get_dict_dict(json_status, "current_game"), "performance");
		if (json_current_game_perf) {
			server_state->current_score = jsondom_get_dict_int(json_current_game_perf, "score");
		}

//		struct jsondom_t *json_current_game_meta = jsondom_get_dict_dict(jsondom_get_dict_dict(json_status, "current_game"), "meta");

		isleep_interrupt(&server_state->isleep);
	} else if (event_type == EVENT_HISTORIAN_STATECHG) {
		if (server_state->historian->connection_state == UNCONNECTED) {
			server_state->ui_screen = MAIN_SCREEN;
			server_state->screen_shown_at_ts = now();
			isleep_interrupt(&server_state->isleep);
		}
	}
}

int main(int argc, char **argv) {
	struct server_state_t server_state = {
		.ui_screen = MAIN_SCREEN,
		.screen_shown_at_ts = now(),
		.isleep = ISLEEP_INITIALIZER,
	};

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
		display_sdl_register_events(display, event_callback, &server_state);
#endif
	}
	cairo_addfont("../external/beon/beon-webfont.ttf");

	if (!display) {
		fprintf(stderr, "Could not create display.\n");
		exit(EXIT_FAILURE);
	}

	/* Start historian connection */
	server_state.historian = historian_connect("../historian/unix_sock", event_callback, &server_state);
	if (!server_state.historian) {
		fprintf(stderr, "Could not create historian connection instance.\n");
		exit(EXIT_FAILURE);
	}

	struct cairo_swbuf_t *swbuf = create_swbuf(display->width, display->height);
	for (int i = 0; i < 1000; i++) {
		swbuf_render(&server_state, swbuf);
		blit_swbuf_on_display(swbuf, display);
		display_commit(display);
		isleep(&server_state.isleep, 1000);
	}
	historian_free(server_state.historian);
	free_swbuf(swbuf);
	display_free(display);

	cairo_cleanup();
	return 0;
}
