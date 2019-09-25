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
#include <errno.h>

#include "historian.h"
#include "jsondom.h"
#include "tools.h"

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
	while (historian->running) {
		char line_buffer[1024 * 16];
		if (!fgets(line_buffer, sizeof(line_buffer), historian->f_read)) {
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
				struct jsondom_t *connection = jsondom_get_dict_dict(jsondom_get_dict_dict(json, "status"), "connection");
				if (connection) {
				 	enum historian_state_t current_connection_state = jsondom_get_dict_bool(connection, "connected_to_beatsaber") ? CONNECTED_READY : CONNECTED_WAITING;
					if (current_connection_state != historian->connection_state) {
						historian->connection_state = current_connection_state;
						historian->event_callback(EVENT_HISTORIAN_STATECHG, &((struct ui_event_historian_statechg_t){ .historian = historian }), historian->event_callback_ctx);
					}
				}

				historian->event_callback(EVENT_HISTORIAN_MESSAGE, &((struct ui_event_historian_msg_t){ .json = json }), historian->event_callback_ctx);
			}
		} else if (value && !strcmp(value, "response")) {
			/* Response recived */
			fprintf(stderr, "Received response message in reader thread\n");
			bool free_json = false;
			pthread_mutex_lock(&historian->f_mutex);
			if (historian->response == NULL) {
				free_json = true;
			} else {
				*(historian->response) = json;
				pthread_cond_broadcast(&historian->response_cond);
			}
			pthread_mutex_unlock(&historian->f_mutex);

			if (!free_json) {
				return;
			}
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
		int fd = socket(AF_UNIX, SOCK_STREAM, 0);
		if (fd == -1) {
			perror("socket");
			sleep(1);
			continue;
		}

		struct sockaddr_un destination = {
		   .sun_family = AF_UNIX,
		};
		strncpy(destination.sun_path, historian->unix_socket, UNIX_PATH_MAX - 1);

		if (connect(fd, (struct sockaddr*)&destination, sizeof(destination)) == -1) {
			perror("connect");
			sleep(3);
			continue;
		}

		int dupfd = dup(fd);
		if (dupfd == -1) {
			perror("dup");
			close(fd);
			sleep(3);
			continue;
		}

		historian->connection_state = CONNECTED_WAITING;
		if (historian->event_callback) {
			historian->event_callback(EVENT_HISTORIAN_STATECHG, &((struct ui_event_historian_statechg_t){ .historian = historian }), historian->event_callback_ctx);
		}

		pthread_mutex_lock(&historian->f_mutex);
		historian->f_read = fdopen(fd, "r");
		if (!historian->f_read) {
			perror("fdopen");
			pthread_mutex_unlock(&historian->f_mutex);
			close(fd);
			close(dupfd);
			sleep(3);
			continue;
		}
		historian->f_write = fdopen(dupfd, "w");
		if (!historian->f_write) {
			perror("fdopen");
			fclose(historian->f_read);
			historian->f_read = NULL;
			pthread_mutex_unlock(&historian->f_mutex);
			close(dupfd);
			sleep(3);
			continue;
		}
		pthread_mutex_unlock(&historian->f_mutex);

		handle_historian_connection(historian);
		shutdown(fd, SHUT_RDWR);

		pthread_mutex_lock(&historian->f_mutex);
		fclose(historian->f_read);
		fclose(historian->f_write);
		historian->f_read = NULL;
		historian->f_write = NULL;
		pthread_mutex_unlock(&historian->f_mutex);

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

	pthread_mutex_init(&historian->f_mutex, NULL);
	pthread_cond_init(&historian->response_cond, NULL);
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

static struct jsondom_t *historian_command_once(struct historian_t *historian, const char *command_query) {
	pthread_mutex_lock(&historian->f_mutex);
	if (!historian->f_write || historian->response) {
		/* Either we cannot write or someone else is currently writing -- we're
		 * not doing this. */
		pthread_mutex_unlock(&historian->f_mutex);
		return NULL;
	}

	/* Alright, we can technically write and receive a response. Let's do it. */
	struct jsondom_t *response = NULL;
	historian->response = &response;
	fprintf(historian->f_write, "{ \"cmd\": \"%s\" }\n", command_query);
	fflush(historian->f_write);

	struct timespec abstime;
	get_abs_timespec_offset(&abstime, 2000);
	while (!response) {
		if (pthread_cond_timedwait(&historian->response_cond, &historian->f_mutex, &abstime) == ETIMEDOUT) {
			break;
		}
	}
	historian->response = NULL;
	pthread_mutex_unlock(&historian->f_mutex);

	return response;
}

struct jsondom_t *historian_command(struct historian_t *historian, const char *command_query) {
	for (unsigned int i = 0; i < 3; i++) {
		struct jsondom_t *response = historian_command_once(historian, command_query);
		if (response) {
			return response;
		}
		sleep(1);
	}
	return NULL;
}

void historian_free(struct historian_t *historian) {
	if (!historian) {
		return;
	}
	historian->running = false;
	pthread_mutex_lock(&historian->f_mutex);
	if (historian->f_read) {
		shutdown(fileno(historian->f_read), SHUT_RDWR);
	}
	pthread_mutex_unlock(&historian->f_mutex);
	pthread_join(historian->connection_thread, NULL);
	free(historian);
}

#ifdef TEST_HISTORIAN

// gcc -Wall -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=500 -Wall -Wmissing-prototypes -Wstrict-prototypes -Werror=implicit-function-declaration -Werror=format -Wshadow -Wswitch -pthread -std=c11 -DTEST_HISTORIAN historian.c jsondom.c tools.c -o historian -ggdb3 -fsanitize=address -fsanitize=undefined -fsanitize=leak -fno-omit-frame-pointer -D_FORTITY_SOURCE=2 `pkg-config --cflags --libs yajl` && ./historian

static void event_callback(enum ui_eventtype_t event_type, void *event, void *ctx) {
	if (event_type == EVENT_HISTORIAN_MESSAGE) {
		struct ui_event_historian_msg_t *msg = (struct ui_event_historian_msg_t *)event;
		printf("RX event:\n");
		jsondom_dump(msg->json);
	} else if (event_type == EVENT_HISTORIAN_STATECHG) {
		struct ui_event_historian_statechg_t *msg = (struct ui_event_historian_statechg_t *)event;
		printf("Historian state now %d\n", msg->historian->connection_state);
	}
}

int main(void) {
	struct historian_t *historian = historian_connect("../historian/unix_sock", event_callback, NULL);
	for (int i = 0; i < 2; i++) {
		struct jsondom_t *response = historian_command(historian, "status");
		fprintf(stderr, "historian_command %d: %p\n", i, response);
		if (response) {
			fprintf(stderr, "Received response message %d in API:\n", i);
			jsondom_dump(response);
			jsondom_free(response);
		}
	}
	for (int i = 0; i < 3; i++) {
		sleep(1);
	}
	printf("Shutdown\n");
	historian_free(historian);
}
#endif
