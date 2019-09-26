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
	char song_author[48];
	char song_title[48];
	char level_author[48];
	unsigned int current_score;
	unsigned int current_maxscore;
	struct historian_t *historian;
	struct isleep_t isleep;
	bool running;
	pthread_mutex_t shared_data_mutex;
	unsigned int frameno;
};

/*************** AUTO GENERATED SECTION FOLLOWS ***************/
/***************  AUTO GENERATED SECTION ENDS   ***************/

#endif
