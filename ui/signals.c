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
#include <stddef.h>
#include <signal.h>
#include <pthread.h>
#include "ui_events.h"

static ui_event_cb_t ui_event_callback;
static void *ui_callback_ctx;

static void *signal_thread(void *csigno) {
	ui_event_callback(EVENT_QUIT, NULL, ui_callback_ctx);
	return NULL;
}

static void sigint_handler(int signo) {
	/* Handle signal asynchronously */
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_t thread;
	pthread_create(&thread, &attr, signal_thread, NULL);
	pthread_attr_destroy(&attr);
}

bool register_signal_handler(ui_event_cb_t event_callback, void *ctx) {
	ui_event_callback = event_callback;
	ui_callback_ctx = ctx;

	struct sigaction action = {
		.sa_handler = sigint_handler,
		.sa_flags = SA_RESTART,
	};
	sigemptyset(&action.sa_mask);
	if (sigaction(SIGINT, &action, NULL)) {
		perror("sigaction");
		return false;
	}
	return true;
}
