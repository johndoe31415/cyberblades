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

#include "renderer_fullhd.h"
#include "cyberblades-ui.h"
#include "cairo.h"
#include "historian.h"
#include "cformat.h"

#define STR_ENDASH								"–"
#define STR_EMDASH								"—"

#define FONT_HEADING_SIZE						128
#define FONT_HEADING							.font_face = "Beon", .font_size = FONT_HEADING_SIZE
#define TEXT_PLACEMENT(xoff, yoff, color)		&(const struct font_placement_t) {		\
													.font_face = "Roboto",				\
													.font_size = 40,					\
													.font_color = (color),				\
													.placement = {						\
														 .src_anchor = {				\
															.x = XPOS_CENTER,			\
															.y = YPOS_BOTTOM,			\
														},								\
														.dst_anchor = {					\
															.x = XPOS_CENTER,			\
															.y = YPOS_TOP,				\
														},								\
														.xoffset = (xoff),				\
														.yoffset = (yoff),				\
													}									\
												}

static void swbuf_render_heading(struct cairo_swbuf_t *swbuf, const char *text) {
	swbuf_text(swbuf, &(const struct font_placement_t) {
		FONT_HEADING,
		.font_color = COLOR_BS_RED,
		.placement = {
			.src_anchor = {	.x = XPOS_CENTER, .y = YPOS_BOTTOM, },
			.dst_anchor = { .x = XPOS_CENTER, .y = YPOS_TOP, },
			.yoffset = FONT_HEADING_SIZE - 8,
		}
	}, text);
}

static void swbuf_render_main_screen_bottom_box(const struct server_state_t *server_state, struct cairo_swbuf_t *swbuf) {
	uint32_t fgcolor, bgcolor;
	const char *text = NULL;

	switch (server_state->historian->connection_state) {
		case UNCONNECTED:
			fgcolor = COLOR_WHITE;
			bgcolor = COLOR_POMEGRANATE;
			text = "Historian unavailable";
			break;

		case CONNECTED:
			if (!server_state->connected_to_beatsaber) {
				fgcolor = COLOR_BLACK;
				bgcolor = COLOR_SUN_FLOWER;
				text = "Not connected to BeatSaber";
			} else {
				fgcolor = COLOR_WHITE;
				bgcolor = COLOR_EMERLAND;
				text = "Ready for action!";
			}
			break;
	}

	if (text) {
		swbuf_rect(swbuf, &(const struct rect_placement_t){
			.placement = {
				.src_anchor = { .x = XPOS_CENTER, .y = YPOS_BOTTOM },
				.dst_anchor = {	.x = XPOS_CENTER, .y = YPOS_BOTTOM },
				.yoffset = -10,
			},
			.color = bgcolor,
			.fill = true,
			.round = 15,
			.width = 1000,
			.height = 75,
		});
		swbuf_text(swbuf, TEXT_PLACEMENT(0, 1045, fgcolor), text);
	}
}

static void render_highscore_table_entry(char *dest_buf, unsigned int dest_buf_length, unsigned int x, const struct highscore_entry_t *entry) {
	switch (x) {
		case 0:
			snprintf(dest_buf, dest_buf_length, "%d", entry->number);
			break;

		case 1:
			snprintf(dest_buf, dest_buf_length, "%s", entry->name[0] ? entry->name : STR_EMDASH);
			break;

		case 2:
			snprintf(dest_buf, dest_buf_length, "%u", entry->performance.score);
			break;

		case 3:
			snprintf(dest_buf, dest_buf_length, "%u", entry->performance.max_combo);
			break;

		case 4:
			snprintf(dest_buf, dest_buf_length, "%u", entry->performance.missed_notes);
			break;

		case 5:
			/* TODO: Modifiers */
			break;

		case 6:
			if (entry->performance.max_score) {
				snprintf(dest_buf, dest_buf_length, "%.1f%%", 100. * entry->performance.score / entry->performance.max_score);
			} else {
				snprintf(dest_buf, dest_buf_length, STR_EMDASH);
			}
			break;

		case 7:
			snprintf(dest_buf, dest_buf_length, "%s", entry->performance.rank);
			break;
	}
}

static void render_highscore_table(char *dest_buf, unsigned int dest_buf_length, struct font_placement_t *placement, unsigned int x, unsigned int y, void *ctx) {
	const struct server_state_t *server_state = (const struct server_state_t*)ctx;

	if (y == 0) {
		const char *column_headings[] = {
			"#",
			"Player",
			"Score",
			"Combo",
			"Miss",
			"Mods",
			"%",
			"Rank",
		};
		const unsigned int col_count = sizeof(column_headings) / sizeof(column_headings[0]);
	 	if (x < col_count) {
			strncpy(dest_buf, column_headings[x], dest_buf_length);
			placement->font_bold = true;
		}
		return;
	}

	const unsigned int highscore_index = y - 1;
	if (highscore_index >= server_state->highscores.entry_count) {
		return;
	}

	const struct highscore_entry_t *highscore_entry = &server_state->highscores.entries[highscore_index];

	if (!highscore_entry->performance.verdict_passed) {
		placement->font_color = COLOR_ASBESTOS;
	}
	if (highscore_entry->most_recent) {
		placement->font_color = highscore_entry->performance.verdict_passed ? COLOR_SUN_FLOWER : COLOR_ORANGE;
	}

	render_highscore_table_entry(dest_buf, dest_buf_length, x, highscore_entry);
}

static const char *difficulty_str(enum difficulty_level_t difficulty) {
	switch (difficulty) {
		case EASY:			return "Easy";
		case NORMAL:		return "Normal";
		case HARD:			return "Hard";
		case EXPERT:		return "Expert";
		case EXPERTPLUS:	return "Expert+";
	}
	return "?";
}

static void swbuf_render_main_screen(const struct server_state_t *server_state, struct cairo_swbuf_t *swbuf) {
	const int cyberblades_offset = -5;
	swbuf_text(swbuf, &(const struct font_placement_t) {
		FONT_HEADING,
		.font_color = COLOR_BS_RED,
		.placement = {
			.src_anchor = {	.x = XPOS_RIGHT, .y = YPOS_BOTTOM, },
			.dst_anchor = { .x = XPOS_CENTER, .y = YPOS_TOP, },
			.yoffset = FONT_HEADING_SIZE - 8,
			.xoffset = -5 + cyberblades_offset,
		}
	}, "Cyber");
	swbuf_text(swbuf, &(const struct font_placement_t) {
		FONT_HEADING,
		.font_color = COLOR_BS_BLUE,
		.placement = {
			.src_anchor = { .x = XPOS_LEFT, .y = YPOS_BOTTOM, },
			.dst_anchor = { .x = XPOS_CENTER, .y = YPOS_TOP, },
			.yoffset = FONT_HEADING_SIZE - 8,
			.xoffset = 5 + cyberblades_offset,
		}
	}, "Blades");

	if (server_state->player.name[0]) {
		const struct font_placement_t player_placement = {
			.font_face = "Roboto",
			.font_size = 40,
			.font_color = COLOR_CLOUDS,
			.placement = {
				 .src_anchor = {
					.x = XPOS_LEFT,
					.y = YPOS_BOTTOM,
				},
				.dst_anchor = {
					.x = XPOS_LEFT,
					.y = YPOS_TOP,
				},
				.xoffset = 100,
				.yoffset = 200,
			}
		};
		swbuf_text(swbuf, &player_placement, "%s", server_state->player.name);


		const struct font_placement_t song_placement = {
			.font_face = "Roboto",
			.font_size = 40,
			.font_color = COLOR_CLOUDS,
			.placement = {
				 .src_anchor = {
					.x = XPOS_RIGHT,
					.y = YPOS_BOTTOM,
				},
				.dst_anchor = {
					.x = XPOS_RIGHT,
					.y = YPOS_TOP,
				},
				.xoffset = -100,
				.yoffset = 200,
			}
		};
		if (server_state->highscores.song_key.song_title[0]) {
			if (server_state->highscores.song_key.song_author[0]) {
				swbuf_text(swbuf, &song_placement, "%s - %s (%s)", server_state->highscores.song_key.song_author, server_state->highscores.song_key.song_title, difficulty_str(server_state->highscores.song_key.difficulty));
			} else {
				swbuf_text(swbuf, &song_placement, "%s (%s)", server_state->highscores.song_key.song_title, difficulty_str(server_state->highscores.song_key.difficulty));
			}
		}

		swbuf_text(swbuf, TEXT_PLACEMENT(-360 * 2, 200 + 45 * 2, COLOR_CLOUDS), "Playtime");
		swbuf_text(swbuf, TEXT_PLACEMENT(-360 * 1, 200 + 45 * 2, COLOR_CLOUDS), "Notes Cut");
		swbuf_text(swbuf, TEXT_PLACEMENT(0, 200 + 45 * 2, COLOR_CLOUDS), "Games Played");
		swbuf_text(swbuf, TEXT_PLACEMENT(360 * 1, 200 + 45 * 2, COLOR_CLOUDS), "Total Score");
		swbuf_text(swbuf, TEXT_PLACEMENT(360 * 2, 200 + 45 * 2, COLOR_CLOUDS), "Percentage");

		swbuf_text(swbuf, TEXT_PLACEMENT(-360 * 2, 200 + 45 * 3, COLOR_CLOUDS), "%s", cformat_sbuf_time_secs(server_state->player.today.total_playtime_secs));
		swbuf_text(swbuf, TEXT_PLACEMENT(-360 * 1, 200 + 45 * 3, COLOR_CLOUDS), "%s", cformat_sbuf_si_float((double)(server_state->player.today.total_passed_notes - server_state->player.today.total_missed_notes)));
		swbuf_text(swbuf, TEXT_PLACEMENT(0, 200 + 45 * 3, COLOR_CLOUDS), "%u", server_state->player.today.games_played);
		swbuf_text(swbuf, TEXT_PLACEMENT(360 * 1, 200 + 45 * 3, COLOR_CLOUDS), "%s", cformat_sbuf_si_float((double)server_state->player.today.total_score));
		if (server_state->player.today.total_max_score) {
			swbuf_text(swbuf, TEXT_PLACEMENT(360 * 2, 200 + 45 * 3, COLOR_CLOUDS), "%.1f%%", 100. * server_state->player.today.total_score / server_state->player.today.total_max_score);
		} else {
			swbuf_text(swbuf, TEXT_PLACEMENT(360 * 2, 200 + 45 * 3, COLOR_CLOUDS), STR_EMDASH);
		}

		swbuf_text(swbuf, TEXT_PLACEMENT(-360 * 2, 200 + 45 * 4, COLOR_CLOUDS), "%s", cformat_sbuf_time_secs(server_state->player.alltime.total_playtime_secs));
		swbuf_text(swbuf, TEXT_PLACEMENT(-360 * 1, 200 + 45 * 4, COLOR_CLOUDS), "%s", cformat_sbuf_si_float((double)(server_state->player.alltime.total_passed_notes - server_state->player.alltime.total_missed_notes)));
		swbuf_text(swbuf, TEXT_PLACEMENT(0, 200 + 45 * 4, COLOR_CLOUDS), "%u", server_state->player.alltime.games_played);
		swbuf_text(swbuf, TEXT_PLACEMENT(360 * 1, 200 + 45 * 4, COLOR_CLOUDS), "%s", cformat_sbuf_si_float((double)server_state->player.alltime.total_score));
		if (server_state->player.alltime.total_max_score) {
			swbuf_text(swbuf, TEXT_PLACEMENT(360 * 2, 200 + 45 * 4, COLOR_CLOUDS), "%.1f%%", 100. * server_state->player.alltime.total_score / server_state->player.alltime.total_max_score);
		} else {
			swbuf_text(swbuf, TEXT_PLACEMENT(360 * 2, 200 + 45 * 4, COLOR_CLOUDS), STR_EMDASH);
		}

		const struct table_definition_t table = {
			.rows = 1 + server_state->highscores.entry_count,
			.columns = 8,
			.row_height = 45,
			.column_widths = (unsigned int[]) { 100, 250, 200, 150, 150, 150, 150, 100 },
			.anchor = {
				 .src_anchor = {
					.x = XPOS_CENTER,
					.y = YPOS_TOP,
				},
				.dst_anchor = {
					.x = XPOS_CENTER,
					.y = YPOS_TOP,
				},
				.xoffset = 0,
				.yoffset = 420,
			},
			.rendering_callback = render_highscore_table,
			.font_default = {
				.font_face = "Roboto",
				.font_size = 40,
				.font_color = COLOR_CLOUDS,
			},
		};
		swbuf_render_table(swbuf, &table, (void*)server_state);
	} else {
		swbuf_text(swbuf, TEXT_PLACEMENT(0, 200 + 45 * 0, COLOR_POMEGRANATE), "No player selected");
	}

	swbuf_render_main_screen_bottom_box(server_state, swbuf);
}

static void swbuf_render_game_screen(const struct server_state_t *server_state, struct cairo_swbuf_t *swbuf) {
	static unsigned int last_score_width = 0;
	swbuf_render_heading(swbuf, "Game On");
	last_score_width = swbuf_text(swbuf, &(const struct font_placement_t){
		.font_face = "Instruction",
		.font_size = 140,
		.font_color = COLOR_SUN_FLOWER,
		.last_width = last_score_width,
		.max_width_deviation = 10,
		.placement = {
			.src_anchor = {
				.x = XPOS_CENTER,
				.y = YPOS_BOTTOM,
			},
			.dst_anchor = {
				.x = XPOS_CENTER,
				.y = YPOS_TOP,
			},
			.yoffset = 200 + 96,
		}
	}, "%ld", server_state->current_song.performance.score);

	static unsigned int last_percentage_width = 0;
	last_percentage_width = swbuf_text(swbuf, &(const struct font_placement_t){
		.font_face = "Roboto",
		.font_size = 80,
		.font_color = COLOR_ORANGE,
		.last_width = last_percentage_width,
		.max_width_deviation = 10,
		.placement = {
			.src_anchor = {
				.x = XPOS_CENTER,
				.y = YPOS_BOTTOM,
			},
			.dst_anchor = {
				.x = XPOS_CENTER,
				.y = YPOS_TOP,
			},
			.xoffset = 10 - 200,
			.yoffset = 200 + 96 + 96,
		}
	}, "%.1f%%", server_state->current_song.performance.max_score ? 100. * server_state->current_song.performance.score / server_state->current_song.performance.max_score : 0);

	swbuf_text(swbuf, &(const struct font_placement_t){
		.font_face = "Roboto",
		.font_size = 80,
		.font_color = COLOR_ORANGE,
		.last_width = last_percentage_width,
		.max_width_deviation = 10,
		.placement = {
			.src_anchor = {
				.x = XPOS_CENTER,
				.y = YPOS_BOTTOM,
			},
			.dst_anchor = {
				.x = XPOS_CENTER,
				.y = YPOS_TOP,
			},
			.xoffset = 10 + 200,
			.yoffset = 200 + 96 + 96,
		}
	}, "%s", server_state->current_song.performance.rank[0] ? server_state->current_song.performance.rank : STR_EMDASH);

	swbuf_text(swbuf, TEXT_PLACEMENT(-360 * 2, 500, COLOR_CLOUDS), "Combo");
	swbuf_text(swbuf, TEXT_PLACEMENT(-360 * 2, 500 + 40, (server_state->current_song.performance.combo != server_state->current_song.performance.max_combo) ? COLOR_CLOUDS : COLOR_EMERLAND), "%d", server_state->current_song.performance.combo);

	swbuf_text(swbuf, TEXT_PLACEMENT(-360, 500, COLOR_CLOUDS), "Missed Notes");
	swbuf_text(swbuf, TEXT_PLACEMENT(-360, 500 + 40, server_state->current_song.performance.missed_notes ? COLOR_POMEGRANATE : COLOR_EMERLAND), "%d", server_state->current_song.performance.missed_notes);

	swbuf_text(swbuf, TEXT_PLACEMENT(0, 500, COLOR_CLOUDS), "Total Notes");
	swbuf_text(swbuf, TEXT_PLACEMENT(0, 500 + 40, COLOR_CLOUDS), "%d", server_state->current_song.performance.passed_notes);

	swbuf_text(swbuf, TEXT_PLACEMENT(360, 500, COLOR_CLOUDS), "Note Percentage");
	swbuf_text(swbuf, TEXT_PLACEMENT(360, 500 + 40, COLOR_CLOUDS), "%.1f%%", server_state->current_song.performance.passed_notes ? 100. * (server_state->current_song.performance.passed_notes - server_state->current_song.performance.missed_notes) / server_state->current_song.performance.passed_notes : 0);

	swbuf_text(swbuf, TEXT_PLACEMENT(360 * 2, 500, COLOR_CLOUDS), "Max Combo");
	swbuf_text(swbuf, TEXT_PLACEMENT(360 * 2, 500 + 40, COLOR_CLOUDS), "%d", server_state->current_song.performance.max_combo);
}

void swbuf_render_full_hd(const struct server_state_t *server_state, struct cairo_swbuf_t *swbuf) {
	swbuf_clear(swbuf, COLOR_BS_DARKBLUE);
	if (server_state->ui_screen == MAIN_SCREEN) {
		swbuf_render_main_screen(server_state, swbuf);
	} if (server_state->ui_screen == GAME_SCREEN) {
		swbuf_render_game_screen(server_state, swbuf);

	} if (server_state->ui_screen == FINISH_SCREEN) {
	}
}
