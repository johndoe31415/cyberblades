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
#include "signals.h"
#include "cyberblades-ui.h"
#include "renderer_fullhd.h"

static void parse_game_info(struct song_info_t *song, struct jsondom_t *song_json) {
	struct jsondom_t *json_current_game_perf = jsondom_get_dict_dict(song_json, "performance");
	if (json_current_game_perf) {
		song->performance.score = jsondom_get_dict_int(json_current_game_perf, "score");
		song->performance.max_score = jsondom_get_dict_int(json_current_game_perf, "max_score");
		song->performance.combo = jsondom_get_dict_int(json_current_game_perf, "combo");
		song->performance.max_combo = jsondom_get_dict_int(json_current_game_perf, "max_combo");
		song->performance.hit_notes = jsondom_get_dict_int(json_current_game_perf, "hit_notes");
		song->performance.passed_notes = jsondom_get_dict_int(json_current_game_perf, "passed_notes");
		song->performance.missed_notes = jsondom_get_dict_int(json_current_game_perf, "missed_notes");
		const char *rank = jsondom_get_dict_str(json_current_game_perf, "rank");
		if (rank) {
			strncpy(song->performance.rank, rank, sizeof(song->performance.rank) - 1);
		} else {
			song->performance.rank[0] = 0;
		}
	}

	struct jsondom_t *json_current_game_meta = jsondom_get_dict_dict(song_json, "meta");
	if (json_current_game_meta) {
		const char *song_author = jsondom_get_dict_str(json_current_game_meta, "song_author");
		if (song_author) {
			strncpy(song->meta.song_author, song_author, sizeof(song->meta.song_author) - 1);
		}
		const char *song_title = jsondom_get_dict_str(json_current_game_meta, "song_title");
		if (song_title) {
			strncpy(song->meta.song_title, song_title, sizeof(song->meta.song_title) - 1);
		}
		const char *level_author = jsondom_get_dict_str(json_current_game_meta, "level_author");
		if (level_author) {
			strncpy(song->meta.level_author, level_author, sizeof(song->meta.level_author) - 1);
		}
	}
}

static void event_callback(enum ui_eventtype_t event_type, void *vevent, void *ctx) {
	struct server_state_t *server_state = (struct server_state_t*)ctx;

	pthread_mutex_lock(&server_state->shared_data_mutex);

	if (event_type == EVENT_QUIT) {
		exit(EXIT_SUCCESS);
	} else if (event_type == EVENT_KEYPRESS) {
		struct ui_event_keypress_t *event = (struct ui_event_keypress_t*)vevent;
		printf("%p\n", event);
	} else if (event_type == EVENT_HISTORIAN_MESSAGE) {
		struct ui_event_historian_msg_t *event = (struct ui_event_historian_msg_t*)vevent;
		//jsondom_dump(event->json);

		struct jsondom_t *json_status = jsondom_get_dict_dict(event->json, "status");
		struct jsondom_t *json_connection = jsondom_get_dict_dict(json_status, "connection");
		if (json_connection) {
			//jsondom_dump(json_connection);
			const char *current_player = jsondom_get_dict_str(json_connection, "current_player");
			if (current_player) {
				strncpy(server_state->player.name, current_player, sizeof(server_state->player.name) - 1);
			} else {
				server_state->player.name[0] = 0;
			}

			if (jsondom_get_dict_bool(json_connection, "in_game")) {
				server_state->ui_screen = GAME_SCREEN;
				server_state->screen_shown_at_ts = now();
			}
		}

		parse_game_info(&server_state->current_song, jsondom_get_dict_dict(json_status, "current_game"));


		isleep_interrupt(&server_state->isleep);
	} else if (event_type == EVENT_HISTORIAN_STATECHG) {
		if (server_state->historian->connection_state == UNCONNECTED) {
			server_state->ui_screen = MAIN_SCREEN;
			server_state->screen_shown_at_ts = now();
			isleep_interrupt(&server_state->isleep);
		}
	}
	pthread_mutex_unlock(&server_state->shared_data_mutex);
}

int main(int argc, char **argv) {
	struct server_state_t server_state = {
		.ui_screen = MAIN_SCREEN,
		.screen_shown_at_ts = now(),
		.isleep = ISLEEP_INITIALIZER,
		.running = true,
		.shared_data_mutex = PTHREAD_MUTEX_INITIALIZER,
	};

	struct display_t *display = NULL;
	if (argc == 2) {
		const char *filename = argv[1];
		display = display_init(&display_fb_calltable, (void*)filename);
#ifdef BUILD_WITH_SDL
	} else {
		struct display_sdl_init_t init_params = {
//			.width = 320, .height = 240,
			.width = 1920, .height = 1080,
		};
		display = display_init(&display_sdl_calltable, &init_params);
		display_sdl_register_events(display, event_callback, &server_state);
#endif
	}
	register_signal_handler(event_callback, &server_state);

	cairo_addfont("../external/beon/beon-webfont.ttf");
	cairo_addfont("../external/digital-7.mono.ttf");
	cairo_addfont("../external/digital-dream.fat.ttf");
	cairo_addfont("../external/oblivious/OBLIVIOUSFONT.TTF");

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
	while (server_state.running) {
		server_state.frameno++;
		pthread_mutex_lock(&server_state.shared_data_mutex);
		swbuf_render_full_hd(&server_state, swbuf);
		pthread_mutex_unlock(&server_state.shared_data_mutex);
		blit_swbuf_on_display(swbuf, display);
		display_commit(display);
		isleep(&server_state.isleep, 50);
	}
	historian_free(server_state.historian);
	free_swbuf(swbuf);
	display_free(display);

	cairo_cleanup();
	return 0;
}
