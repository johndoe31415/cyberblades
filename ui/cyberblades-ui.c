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
#include "display_sdl.h"
#include "historian.h"
#include "tools.h"
#include "isleep.h"
#include "signals.h"
#include "cyberblades-ui.h"
#include "renderer_fullhd.h"

static bool string_is(const char *str1, const char *str2) {
	if (!str1 || !str2) {
		return false;
	}
	return !strcmp(str1, str2);
}

static void parse_performance(struct performance_info_t *performance, struct jsondom_t *json) {
	performance->score = jsondom_get_dict_int(json, "score");
	performance->max_score = jsondom_get_dict_int(json, "max_score");
	performance->combo = jsondom_get_dict_int(json, "combo");
	performance->max_combo = jsondom_get_dict_int(json, "max_combo");
	performance->hit_notes = jsondom_get_dict_int(json, "hit_notes");
	performance->passed_notes = jsondom_get_dict_int(json, "passed_notes");
	performance->missed_notes = jsondom_get_dict_int(json, "missed_notes");
	const char *rank = jsondom_get_dict_str(json, "rank");
	if (rank) {
		strncpy(performance->rank, rank, sizeof(performance->rank) - 1);
	} else {
		performance->rank[0] = 0;
	}
	performance->verdict_passed = string_is(jsondom_get_dict_str(json, "verdict"), "pass");
}

static void parse_game_info(struct song_info_t *song, struct jsondom_t *song_json) {
	struct jsondom_t *json_current_game_perf = jsondom_get_dict_dict(song_json, "performance");
	if (json_current_game_perf) {
		parse_performance(&song->performance, json_current_game_perf);
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

static void parse_player_stats(struct player_stats_t *stats, struct jsondom_t *stat_json) {
	stats->games_played = jsondom_get_dict_int(stat_json, "games_played");
	stats->total_playtime_secs = jsondom_get_dict_float(stat_json, "total_playtime_secs");
	stats->total_passed_notes = jsondom_get_dict_int(stat_json, "total_passed_notes");
	stats->total_missed_notes = jsondom_get_dict_int(stat_json, "total_missed_notes");
	stats->total_score = jsondom_get_dict_int(stat_json, "total_score");
	stats->total_max_score = jsondom_get_dict_int(stat_json, "total_max_score");
}

static void set_player(struct server_state_t *server_state, const char *new_player) {
	historian_command(server_state->historian, "set_player", "\"player\":\"%s\"", new_player);
}

static void request_player_information(struct server_state_t *server_state) {
	historian_command(server_state->historian, "playerinfo", "\"player\":\"%s\"", server_state->player.name);
}

static void event_handle_historian_status(struct server_state_t *server_state, struct jsondom_t *json) {
	struct jsondom_t *json_connection = jsondom_get_dict_dict(json, "connection");
	struct jsondom_t *current_game = jsondom_get_dict_dict(json, "current_game");
	if (json_connection) {
		if (strncpycmp(server_state->player.name, jsondom_get_dict_str(json_connection, "current_player"), sizeof(server_state->player.name))) {
			/* Player name has changed */
			request_player_information(server_state);
		}
		server_state->connected_to_beatsaber = jsondom_get_dict_bool(json_connection, "connected_to_beatsaber");

		bool in_game = current_game != NULL;
		if (in_game) {
			server_state->ui_screen = GAME_SCREEN;
			server_state->screen_shown_at_ts = now();
		} else {
			if (server_state->ui_screen == GAME_SCREEN) {
				/* Was playing a game, now back to main screen: Update
				 * highscores! */
				request_player_information(server_state);
			}
			server_state->ui_screen = MAIN_SCREEN;
			server_state->screen_shown_at_ts = now();
		}
	}

	parse_game_info(&server_state->current_song, current_game);
	isleep_interrupt(&server_state->isleep);
}

static void parse_highscore_entry(struct highscore_entry_t *entry, struct jsondom_t *json) {
	strncpycmp(entry->name, jsondom_get_dict_str(json, "player"), sizeof(entry->name));
	entry->number = jsondom_get_dict_int(json, "number");
	entry->most_recent = jsondom_get_dict_bool(json, "most_recent");
	parse_performance(&entry->performance, json);
}

static void event_handle_historian_playerinfo(struct server_state_t *server_state, struct jsondom_t *json) {
	jsondom_dump(json);
	const char *player = jsondom_get_dict_str(json, "player");
	if (!player || strcmp(player, server_state->player.name)) {
		/* No player set or different player given */
		return;
	}
	parse_player_stats(&server_state->player.today, jsondom_get_dict_dict(json, "today"));
	parse_player_stats(&server_state->player.alltime, jsondom_get_dict_dict(json, "alltime"));

	struct jsondom_t *highscore = jsondom_get_dict_dict(json, "highscore");
	struct jsondom_t *highscore_song_key = jsondom_get_dict_dict(highscore, "song_key");
	if (highscore_song_key) {
		strncpycmp(server_state->highscores.song_key.song_author, jsondom_get_dict_str(highscore_song_key, "song_author"), sizeof(server_state->highscores.song_key.song_author));
		strncpycmp(server_state->highscores.song_key.song_title, jsondom_get_dict_str(highscore_song_key, "song_title"), sizeof(server_state->highscores.song_key.song_title));
		strncpycmp(server_state->highscores.song_key.level_author, jsondom_get_dict_str(highscore_song_key, "level_author"), sizeof(server_state->highscores.song_key.level_author));
		server_state->highscores.song_key.difficulty = jsondom_get_dict_int(highscore_song_key, "difficulty");
	}

	struct jsondom_t *highscore_table = jsondom_get_dict_array(highscore, "table");
	if (highscore_table) {
		unsigned int highscore_entry_count = highscore_table->element.array.element_cnt;
		server_state->highscores.entry_count = (highscore_entry_count > MAX_HIGHSCORE_ENTRY_COUNT) ? MAX_HIGHSCORE_ENTRY_COUNT : highscore_entry_count;
		for (unsigned int i = 0; i < server_state->highscores.entry_count; i++) {
			struct jsondom_t *highscore_entry = jsondom_get_array_item(highscore_table, i);
			parse_highscore_entry(&server_state->highscores.entries[i], highscore_entry);
		}
	} else {
		server_state->highscores.entry_count = 0;
	}
}

static void event_callback(enum ui_eventtype_t event_type, void *vevent, void *ctx) {
	struct server_state_t *server_state = (struct server_state_t*)ctx;

	pthread_mutex_lock(&server_state->shared_data_mutex);

	if (event_type == EVENT_QUIT) {
		exit(EXIT_SUCCESS);
	} else if (event_type == EVENT_KEYPRESS) {
		struct ui_event_keypress_t *event = (struct ui_event_keypress_t*)vevent;
		if (event->key == SDLK_BACKSPACE) {
			char new_name[sizeof(server_state->player.name)];
			strcpy(new_name, server_state->player.name);
			int len = strlen(new_name);
			if (len) {
				new_name[len - 1] = 0;
			}
			set_player(server_state, new_name);
		}
	} else if (event_type == EVENT_TEXTDATA) {
		struct ui_event_textdata_t *event = (struct ui_event_textdata_t*)vevent;
		int len = strlen(server_state->player.name);
		int add_len = strlen(event->text);
		if (len + add_len < sizeof(server_state->player.name)) {
			char new_name[sizeof(server_state->player.name)];
			strcpy(new_name, server_state->player.name);
			strcat(new_name, event->text);
			set_player(server_state, new_name);
		}
	} else if (event_type == EVENT_HISTORIAN_MESSAGE) {
		struct ui_event_historian_msg_t *event = (struct ui_event_historian_msg_t*)vevent;
//		jsondom_dump(event->json);

		const char *msgtype = jsondom_get_dict_str(event->json, "msgtype");
		if (msgtype) {
			if (!strcmp(msgtype, "status")) {
				event_handle_historian_status(server_state, event->json);
			} else if (!strcmp(msgtype, "playerinfo")) {
				event_handle_historian_playerinfo(server_state, event->json);
			} else {
				fprintf(stderr, "Unhandled incoming message:\n");
				jsondom_dump(event->json);
			}
		} else {
			fprintf(stderr, "No 'msgtype' present:\n");
			jsondom_dump(event->json);
		}
	} else if (event_type == EVENT_HISTORIAN_STATECHG) {
		struct ui_event_historian_statechg_t *event = (struct ui_event_historian_statechg_t*)vevent;
		if (event->new_state == UNCONNECTED) {
			server_state->connected_to_beatsaber = false;
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
	} else {
		struct display_sdl_init_t init_params = {
//			.width = 320, .height = 240,
			.width = 1920, .height = 1080,
		};
		display = display_init(&display_sdl_calltable, &init_params);
		display_sdl_register_events(display, event_callback, &server_state);
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
