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

#ifndef __UI_EVENTS_H__
#define __UI_EVENTS_H__
#include <stdint.h>
#include <SDL2/SDL.h>
#include "jsondom.h"

enum ui_eventtype_t {
	EVENT_QUIT,
	EVENT_KEYPRESS,
	EVENT_TEXTDATA,
	EVENT_HISTORIAN_MESSAGE,
	EVENT_HISTORIAN_STATECHG,
};

struct ui_event_keypress_t {
	SDL_Keycode key;
	uint16_t mod;
};

struct ui_event_textdata_t {
	char text[32];
};

struct historian_t;

struct ui_event_historian_msg_t {
	struct historian_t *historian;
	struct jsondom_t* json;
};

struct ui_event_historian_statechg_t {
	struct historian_t *historian;
	unsigned int old_state, new_state;
};

typedef void (*ui_event_cb_t)(enum ui_eventtype_t event_type, void *event, void *ctx);

/*************** AUTO GENERATED SECTION FOLLOWS ***************/
/***************  AUTO GENERATED SECTION ENDS   ***************/

#endif
