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

#ifndef __CAIRO_H__
#define __CAIRO_H__

#include <stdint.h>
#include <stdbool.h>
#include <cairo/cairo.h>
#include "colors.h"

struct cairo_swbuf_t {
	cairo_surface_t *surface;
	cairo_t *ctx;
	unsigned int width, height;
};

enum xanchor_t {
	XPOS_LEFT,
	XPOS_CENTER,
	XPOS_RIGHT,
};

enum yanchor_t {
	YPOS_TOP,
	YPOS_CENTER,
	YPOS_BOTTOM
};

struct anchored_placement_t {
	int xoffset, yoffset;
	struct {
		enum xanchor_t x;
		enum yanchor_t y;
	} dst_anchor;
	struct {
		enum xanchor_t x;
		enum yanchor_t y;
	} src_anchor;
};

struct font_placement_t {
	struct anchored_placement_t placement;
	const char *font_face;
	unsigned int font_size;
	uint32_t font_color;
	bool font_bold;
	unsigned int width_rounding;
};

struct rect_placement_t {
	struct anchored_placement_t placement;
	unsigned int width, height;
	unsigned int round;
	uint32_t color;
	bool fill;
};

struct coordinate_t {
	int x, y;
};

struct placement_t {
	struct coordinate_t top_left;
	struct coordinate_t bottom_right;
#if CAIRO_DEBUG
	struct coordinate_t anchor;
#endif
};


/*************** AUTO GENERATED SECTION FOLLOWS ***************/
struct cairo_swbuf_t *create_swbuf(unsigned int width, unsigned int height);
void swbuf_clear(struct cairo_swbuf_t *surface, uint32_t bgcolor);
uint32_t* swbuf_get_pixel_data(const struct cairo_swbuf_t *surface);
uint32_t swbuf_get_pixel(const struct cairo_swbuf_t *surface, unsigned int x, unsigned int y);
void swbuf_text(struct cairo_swbuf_t *surface, const struct font_placement_t *placement, const char *fmt, ...);
void swbuf_rect(struct cairo_swbuf_t *surface, const struct rect_placement_t *placement);
void swbuf_circle(struct cairo_swbuf_t *surface, unsigned int x, unsigned int y, unsigned int radius, uint32_t color);
void swbuf_dump(struct cairo_swbuf_t *surface, const char *png_filename);
void free_swbuf(struct cairo_swbuf_t *buffer);
void cairo_addfont(const char *font_ttf_filename);
void cairo_cleanup(void);
/***************  AUTO GENERATED SECTION ENDS   ***************/

#endif
