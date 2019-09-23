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

#include <stdlib.h>
#include <unistd.h>
#include "display.h"

struct display_t* display_init(const struct display_calltable_t *calltable, void *init_ctx) {
	unsigned int drv_size = calltable->get_ctx_size();
	struct display_t *display = calloc(sizeof(struct display_t) + drv_size, 1);
	display->calltable = calltable;
	display->calltable->init(display, init_ctx);
	return display;
}

void display_free(struct display_t *display) {
	display->calltable->free(display);
}

void display_fill(struct display_t *display, uint32_t color) {
	display->calltable->fill(display, color);
}

void display_commit(struct display_t *display) {
	if (display->calltable->commit) {
		display->calltable->commit(display);
	}
}

void display_put_pixel(struct display_t *display, unsigned int x, unsigned int y, uint32_t color) {
	display->calltable->put_pixel(display, x, y, color);
}

void display_test(struct display_t *display) {
	for (int i = 0; i < 256; i++) {
		display_fill(display, MK_RGB(i, i, i));
		usleep(10 * 1000);
	}

	display_fill(display, MK_RGB(0xff, 0, 0));
	sleep(1);
	display_fill(display, MK_RGB(0, 0xff, 0));
	sleep(1);
	display_fill(display, MK_RGB(0, 0, 0xff));
	sleep(1);
	display_fill(display, 0);
}
