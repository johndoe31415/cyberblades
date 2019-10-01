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

#ifndef __HISTORIAN_H__
#define __HISTORIAN_H__

#include <stdio.h>
#include <pthread.h>
#include "ui_events.h"

enum historian_state_t {
	UNCONNECTED,
	CONNECTED_WAITING,
	CONNECTED_READY,
};

struct historian_t {
	const char *unix_socket;
	FILE *f_read, *f_write;
	pthread_mutex_t f_mutex;
	struct jsondom_t **response;
	pthread_cond_t response_cond;
	enum historian_state_t connection_state;
	ui_event_cb_t event_callback;
	void *event_callback_ctx;
	pthread_t connection_thread;
	pthread_t receive_thread;
	bool running;
};

/*************** AUTO GENERATED SECTION FOLLOWS ***************/
struct historian_t *historian_connect(const char *unix_socket, ui_event_cb_t historian_event_cb, void *callback_ctx);
void historian_command(struct historian_t *historian, const char *cmdname, const char *params, ...);
void historian_simple_command(struct historian_t *historian, const char *cmdname);
void historian_free(struct historian_t *historian);
/***************  AUTO GENERATED SECTION ENDS   ***************/

#endif
