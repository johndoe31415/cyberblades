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
		swbuf_text(swbuf, TEXT_PLACEMENT(0, 200 + 45 * 0, COLOR_SILVER), "Current player: %s", server_state->player.name);
		swbuf_text(swbuf, TEXT_PLACEMENT(0, 200 + 45 * 1, COLOR_SILVER), "Playtime: %2d:%02d", server_state->player.playtime_today_secs / 60, server_state->player.playtime_today_secs % 60);
		swbuf_text(swbuf, TEXT_PLACEMENT(0, 200 + 45 * 2, COLOR_SILVER), "Scoresum: %.1f k", server_state->player.total_score_today / 1000.);
	} else {
		swbuf_text(swbuf, TEXT_PLACEMENT(0, 200 + 45 * 0, COLOR_POMEGRANATE), "No player selected");
	}

	swbuf_render_main_screen_bottom_box(server_state, swbuf);
}

static void swbuf_render_game_screen(const struct server_state_t *server_state, struct cairo_swbuf_t *swbuf) {
	static unsigned int last_score_width = 0;
	swbuf_render_heading(swbuf, "Game On");
	last_score_width = swbuf_text(swbuf, &(const struct font_placement_t){
//		.font_face = "Digital Dream Fat",
		.font_face = "Oblivious Font",
		.font_size = 96,
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
		.font_face = "Oblivious Font",
		.font_size = 64,
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
		.font_face = "Oblivious Font",
		.font_size = 64,
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
	}, "%s", server_state->current_song.performance.rank[0] ? server_state->current_song.performance.rank : "-");

	swbuf_text(swbuf, TEXT_PLACEMENT(-360 * 2, 500, COLOR_SILVER), "Combo");
	swbuf_text(swbuf, TEXT_PLACEMENT(-360 * 2, 500 + 40, (server_state->current_song.performance.combo != server_state->current_song.performance.max_combo) ? COLOR_SILVER : COLOR_EMERLAND), "%d", server_state->current_song.performance.combo);

	swbuf_text(swbuf, TEXT_PLACEMENT(-360, 500, COLOR_SILVER), "Missed Notes");
	swbuf_text(swbuf, TEXT_PLACEMENT(-360, 500 + 40, server_state->current_song.performance.missed_notes ? COLOR_POMEGRANATE : COLOR_EMERLAND), "%d", server_state->current_song.performance.missed_notes);

	swbuf_text(swbuf, TEXT_PLACEMENT(0, 500, COLOR_SILVER), "Total Notes");
	swbuf_text(swbuf, TEXT_PLACEMENT(0, 500 + 40, COLOR_SILVER), "%d", server_state->current_song.performance.passed_notes);

	swbuf_text(swbuf, TEXT_PLACEMENT(360, 500, COLOR_SILVER), "Note Percentage");
	swbuf_text(swbuf, TEXT_PLACEMENT(360, 500 + 40, COLOR_SILVER), "%.1f%%", server_state->current_song.performance.passed_notes ? 100. * (server_state->current_song.performance.passed_notes - server_state->current_song.performance.missed_notes) / server_state->current_song.performance.passed_notes : 0);

	swbuf_text(swbuf, TEXT_PLACEMENT(360 * 2, 500, COLOR_SILVER), "Max Combo");
	swbuf_text(swbuf, TEXT_PLACEMENT(360 * 2, 500 + 40, COLOR_SILVER), "%d", server_state->current_song.performance.max_combo);
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
