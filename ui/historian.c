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
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/un.h>
#include <unistd.h>

#include "historian.h"
#include "jsondom.h"

static bool truncate_crlf(char *string) {
	int length = strlen(string);
	bool truncated = false;
	if (length && (string[length - 1] == '\n')) {
		truncated = true;
		string[--length] = 0;
	}
	if (length && (string[length - 1] == '\r')) {
		truncated = true;
		string[--length] = 0;
	}
	return truncated;
}

static void handle_historian_connection(struct historian_t *historian) {
	FILE *f = fdopen(historian->historian_fd, "r+");
	if (!f) {
		perror("fdopen");
		return;
	}
	while (historian->running) {
		char line_buffer[1024 * 16];
		if (!fgets(line_buffer, sizeof(line_buffer), f)) {
			/* EOF */
			break;
		}

		if (!truncate_crlf(line_buffer)) {
			/* Either not complete line or empty line from the beginning */
			fprintf(stderr, "Received improper command line, severing connection.\n");
			historian->running = false;
			break;
		}

		/* Now try to parse the JSON message that we received */
		struct jsondom_t *json = jsondom_parse(line_buffer);
		if (!json) {
			fprintf(stderr, "Failed to parse server JSON, severing connection.\n");
			historian->running = false;
			break;
		}

		char *value = jsondom_get_dict_str(json, "msgtype");
		if (value && !strcmp(value, "event")) {
			/* Event recived */
			if (historian->event_callback) {
				struct jsondom_t *status = jsondom_get_dict_dict(json, "status");
				if (status) {
				 	enum historian_state_t current_connection_state = jsondom_get_dict_bool(status, "connected_to_beatsaber") ? CONNECTED_READY : CONNECTED_WAITING;
					if (current_connection_state != historian->connection_state) {
						historian->connection_state = current_connection_state;
						historian->event_callback(EVENT_HISTORIAN_STATECHG, &((struct ui_event_historian_statechg_t){ .historian = historian }), historian->event_callback_ctx);
					}
				}

				historian->event_callback(EVENT_HISTORIAN_MESSAGE, &((struct ui_event_historian_msg_t){ .json = json }), historian->event_callback_ctx);
			}
		} else if (value && !strcmp(value, "response")) {
			/* Response recived */
		} else {
			fprintf(stderr, "Unrecognized msgtype received: %s\n", value);
		}
		jsondom_free(json);
	}
}

static void* historian_connection_thread_fnc(void *vhistorian) {
	struct historian_t *historian = (struct historian_t*)vhistorian;
	while (historian->running) {
		/* Try to establish a connection */
		historian->historian_fd = socket(AF_UNIX, SOCK_STREAM, 0);
		if (historian->historian_fd == -1) {
			perror("socket");
			sleep(1);
			continue;
		}

		struct sockaddr_un destination = {
		   .sun_family = AF_UNIX,
		};
		strncpy(destination.sun_path, historian->unix_socket, UNIX_PATH_MAX);

		if (connect(historian->historian_fd, (struct sockaddr*)&destination, sizeof(destination)) == -1) {
			perror("connect");
			sleep(3);
			continue;
		}

		historian->connection_state = CONNECTED_WAITING;
		if (historian->event_callback) {
			historian->event_callback(EVENT_HISTORIAN_STATECHG, &((struct ui_event_historian_statechg_t){ .historian = historian }), historian->event_callback_ctx);
		}

		handle_historian_connection(historian);
		shutdown(historian->historian_fd, SHUT_RDWR);
		close(historian->historian_fd);

		historian->connection_state = UNCONNECTED;
		if (historian->event_callback) {
			historian->event_callback(EVENT_HISTORIAN_STATECHG, &((struct ui_event_historian_statechg_t){ .historian = historian }), historian->event_callback_ctx);
		}
	}
	return NULL;
}

struct historian_t *historian_connect(const char *unix_socket, ui_event_cb_t historian_event_cb, void *callback_ctx) {
	struct historian_t *historian = calloc(sizeof(struct historian_t), 1);
	if (!historian) {
		perror("calloc");
		return NULL;
	}

	historian->connection_state = UNCONNECTED;
	historian->unix_socket = unix_socket;
	historian->event_callback = historian_event_cb;
	historian->event_callback_ctx = callback_ctx;
	historian->running = true;
	if (pthread_create(&historian->connection_thread, NULL, historian_connection_thread_fnc, historian)) {
		perror("pthread_create");
		free(historian);
		return NULL;
	}

	return historian;
}

void historian_free(struct historian_t *historian) {
	if (!historian) {
		return;
	}
	historian->running = false;
	shutdown(historian->historian_fd, SHUT_RDWR);
	pthread_join(historian->connection_thread, NULL);
	free(historian);
}

#ifdef TEST_HISTORIAN

// gcc -Wall -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=500 -Wall -Wmissing-prototypes -Wstrict-prototypes -Werror=implicit-function-declaration -Werror=format -Wshadow -Wswitch -pthread -std=c11 -DTEST_HISTORIAN historian.c -o historian -ggdb3 -fsanitize=address -fsanitize=undefined -fsanitize=leak -fno-omit-frame-pointer -D_FORTITY_SOURCE=2 && ./historian

static void event_callback(struct historian_t *historian, enum historian_event_t event_type, void *event) {
	printf("RX event\n");
}

int main(void) {
	struct historian_t *historian = historian_connect("../unix_sock", event_callback);
	//while (true) {
	for (int i = 0; i < 3; i++) {
		sleep(1);
	}
	printf("Shutdown\n");
	historian_free(historian);
}
#endif
