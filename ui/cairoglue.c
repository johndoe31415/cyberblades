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

#include "cairoglue.h"

void blit_swbuf_on_display(struct cairo_swbuf_t *swbuf, struct display_t *target) {
	/* Try to fast blit first */
	if ((target->calltable->blit_buffer) && target->calltable->blit_buffer(target, swbuf_get_pixel_data(swbuf), swbuf->width, swbuf->height)) {
		/* Success! */
		return;
	}

	/* Else fallback to slow per-pixel copies */
	for (int y = 0; y < swbuf->height; y++) {
		for (int x = 0; x < swbuf->width; x++) {
			uint32_t rgb = swbuf_get_pixel(swbuf, x, y);
			display_put_pixel(target, x, y, rgb);
		}
	}
}
