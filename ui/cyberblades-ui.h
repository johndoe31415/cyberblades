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

#define MAX_TEXT_WIDTH					48
#define MAX_HIGHSCORE_ENTRY_COUNT		10


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
	bool verdict_passed;
	char rank[4];
};

struct song_info_t {
	struct song_metadata_t meta;
	struct performance_info_t performance;
};

struct highscore_entry_t {
	char name[MAX_TEXT_WIDTH];
	bool most_recent;
	unsigned int number;
	struct performance_info_t performance;
};

struct highscore_table_t {
	unsigned int entry_count;
	struct song_metadata_t song_key;
	struct highscore_entry_t entries[MAX_HIGHSCORE_ENTRY_COUNT];
};

struct player_stats_t {
	unsigned int games_played;
	unsigned int total_playtime_secs;
	unsigned int total_score;
	unsigned int total_max_score;
	unsigned int total_passed_notes;
	unsigned int total_missed_notes;
};

struct player_info_t {
	char name[MAX_TEXT_WIDTH];
	struct player_stats_t today;
	struct player_stats_t alltime;
};

struct server_state_t {
	enum ui_screen_t ui_screen;
	double screen_shown_at_ts;

	bool connected_to_beatsaber;
	struct player_info_t player;
	struct song_info_t current_song;
	struct highscore_table_t highscores;

	struct historian_t *historian;
	struct isleep_t isleep;
	bool running;
	pthread_mutex_t shared_data_mutex;
	unsigned int frameno;
};

/*************** AUTO GENERATED SECTION FOLLOWS ***************/
/***************  AUTO GENERATED SECTION ENDS   ***************/

#endif
