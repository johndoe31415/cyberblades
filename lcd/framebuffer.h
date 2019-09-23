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

#ifndef __FRAMEBUFFER_H__
#define __FRAMEBUFFER_H__

#include <stdint.h>
#include <stdbool.h>

#include "colors.h"

struct display_t {
	int fd;
	uint8_t *screen;
	bool mmapped;
	unsigned int width;
	unsigned int height;
	unsigned int bits_per_pixel;
};

/*************** AUTO GENERATED SECTION FOLLOWS ***************/
void display_put_pixel(struct display_t *display, unsigned int x, unsigned int y, uint32_t rgb);
void display_fill(struct display_t *display, uint32_t rgb);
struct display_t* display_init(const char *fbdev);
struct display_t* display_sw_init(unsigned int width, unsigned int height);
void display_test(struct display_t *display);
void display_free(struct display_t *display);
/***************  AUTO GENERATED SECTION ENDS   ***************/

#endif
