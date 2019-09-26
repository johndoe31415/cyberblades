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

#ifndef __CYBERBLADES_UI_H__
#define __CYBERBLADES_UI_H__

#include <stdbool.h>
#include <pthread.h>
#include "isleep.h"

#define MAX_TEXT_WIDTH		48

enum ui_screen_t {
	MAIN_SCREEN = 0,
	GAME_SCREEN = 1,
	FINISH_SCREEN = 2,
};

enum difficulty_level_t {
	EASY = 0,
	NORMAL = 1,
	HARD = 2,
	EXPERT = 3,
	EXPERTPLUS = 4,
};

struct song_metadata_t {
	char song_author[MAX_TEXT_WIDTH];
	char song_title[MAX_TEXT_WIDTH];
	char level_author[MAX_TEXT_WIDTH];
	enum difficulty_level_t difficulty;
};

struct performance_info_t {
	unsigned int score;
	unsigned int max_score;
	unsigned int combo;
	unsigned int max_combo;
	unsigned int hit_notes;
	unsigned int passed_notes;
	unsigned int missed_notes;
	char rank[4];
};

struct song_info_t {
	struct song_metadata_t meta;
	struct performance_info_t performance;
};

struct player_info_t {
	char name[MAX_TEXT_WIDTH];
	unsigned int playtime_today_secs;
	unsigned int total_score_today;
};

struct server_state_t {
	enum ui_screen_t ui_screen;
	double screen_shown_at_ts;

	struct player_info_t player;
	struct song_info_t current_song;

	struct player_info_t last_player;
	struct song_info_t last_song;

	struct historian_t *historian;
	struct isleep_t isleep;
	bool running;
	pthread_mutex_t shared_data_mutex;
	unsigned int frameno;
};

/*************** AUTO GENERATED SECTION FOLLOWS ***************/
/***************  AUTO GENERATED SECTION ENDS   ***************/

#endif
