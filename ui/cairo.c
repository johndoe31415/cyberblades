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
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <fontconfig/fontconfig.h>
#include "cairo.h"

struct cairo_swbuf_t *create_swbuf(unsigned int width, unsigned int height) {
	struct cairo_swbuf_t *buffer = calloc(sizeof(struct cairo_swbuf_t), 1);
	if (!buffer) {
		perror("calloc");
		return NULL;
	}

	buffer->width = width;
	buffer->height = height;
	buffer->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
	if (!buffer->surface) {
		free_swbuf(buffer);
		return NULL;
	}

	buffer->ctx = cairo_create(buffer->surface);
	return buffer;
}

static void swbuf_set_source_rgb(struct cairo_swbuf_t *surface, uint32_t bgcolor) {
	cairo_set_source_rgb(surface->ctx, GET_R(bgcolor) / 255.0, GET_G(bgcolor) / 255.0, GET_B(bgcolor) / 255.0);
}

void swbuf_clear(struct cairo_swbuf_t *surface, uint32_t bgcolor) {
	swbuf_set_source_rgb(surface, bgcolor);
	cairo_rectangle(surface->ctx, 0, 0, surface->width, surface->height);
	cairo_fill(surface->ctx);
}

uint32_t swbuf_get_pixel(const struct cairo_swbuf_t *surface, unsigned int x, unsigned int y) {
	uint32_t *data = (uint32_t*)cairo_image_surface_get_data(surface->surface);
	return data[(y * surface->width) + x] & 0xffffff;
}

static const char *xanchor_to_str(enum xanchor_t anchor) {
	switch (anchor) {
		case XPOS_LEFT:		return "left";
		case XPOS_CENTER:	return "hcenter";
		case XPOS_RIGHT:	return "right";
		default:			return "?";
	}
}

static const char *yanchor_to_str(enum yanchor_t anchor) {
	switch (anchor) {
		case YPOS_TOP:		return "top";
		case YPOS_CENTER:	return "vcenter";
		case YPOS_BOTTOM:	return "bottom";
		default:			return "?";
	}
}

static struct placement_t swbuf_calculate_placement(const struct cairo_swbuf_t *surface, const struct anchored_placement_t *anchored_placement) {
	struct placement_t placement;

	/* First we assume that we're placing the top left corner of the object
	 * onto the surface */
	switch (anchored_placement->dst_anchor.x) {
		case XPOS_LEFT:
			placement.top_left.x = 0;
			break;

		case XPOS_CENTER:
			placement.top_left.x = surface->width / 2;
			break;

		case XPOS_RIGHT:
			placement.top_left.x = surface->width;
			break;
	}

	switch (anchored_placement->dst_anchor.y) {
		case YPOS_TOP:
			placement.top_left.y = 0;
			break;

		case YPOS_CENTER:
			placement.top_left.y = surface->height / 2;
			break;

		case YPOS_BOTTOM:
			placement.top_left.y = surface->height;
			break;
	}

	/* Then, if the source wants to be placed somewhere else, we take that into
	 * account */
	switch (anchored_placement->src_anchor.x) {
		case XPOS_LEFT:
			break;

		case XPOS_CENTER:
			placement.top_left.x -= anchored_placement->width / 2;
			break;

		case XPOS_RIGHT:
			placement.top_left.x -= anchored_placement->width;
			break;
	}

	switch (anchored_placement->src_anchor.y) {
		case YPOS_TOP:
			break;

		case YPOS_CENTER:
			placement.top_left.y -= anchored_placement->height / 2;
			break;

		case YPOS_BOTTOM:
			placement.top_left.y -= anchored_placement->height;
			break;
	}

	/* Finally we add the user offset */
	placement.top_left.x += anchored_placement->xoffset;
	placement.top_left.y += anchored_placement->yoffset;

	/* And calculate the second set of coordinates */
	placement.bottom_right.x = placement.top_left.x + anchored_placement->width;
	placement.bottom_right.y = placement.top_left.y + anchored_placement->height;

	printf("Abs position of %d x %d object: %s/%s corner is placed on %s/%s (%+d / %+d) => %d, %d\n",
			anchored_placement->width, anchored_placement->height,
			yanchor_to_str(anchored_placement->src_anchor.y), xanchor_to_str(anchored_placement->src_anchor.x),
			yanchor_to_str(anchored_placement->dst_anchor.y), xanchor_to_str(anchored_placement->dst_anchor.x),
			anchored_placement->xoffset, anchored_placement->yoffset,
			placement.top_left.x, placement.top_left.y
	);

	return placement;
}

void swbuf_text(struct cairo_swbuf_t *surface, const struct font_placement_t *options, const char *fmt, ...) {
	char text[512];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(text, sizeof(text), fmt, ap);
	va_end(ap);

	cairo_text_extents_t extents;
	cairo_select_font_face(surface->ctx, options->font_face, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(surface->ctx, options->font_size);
	cairo_text_extents (surface->ctx, text, &extents);

	int base_x = 0;
	switch (options->xanchor) {
		case XPOS_LEFT:
			base_x = 0;
			break;

		case XPOS_CENTER:
			base_x = (surface->width - extents.width) / 2 - extents.x_bearing;
			break;

		case XPOS_RIGHT:
			base_x = surface->width - extents.width - extents.x_bearing;
			break;
	}

	int base_y = 0;
	switch (options->yanchor) {
		case YPOS_TOP:
			base_y = extents.height;
			break;

		case YPOS_CENTER:
			base_y = (surface->height - extents.height) / 2 - extents.y_bearing;
			break;

		case YPOS_BOTTOM:
			base_y = surface->height - extents.height - extents.y_bearing;
			break;
	}

	swbuf_set_source_rgb(surface, options->font_color);
	cairo_move_to(surface->ctx, base_x + options->xoffset, base_y + options->yoffset);
	cairo_show_text(surface->ctx, text);
}

void swbuf_rect(struct cairo_swbuf_t *surface, const struct rect_placement_t *placement) {
	struct placement_t abs_placement = swbuf_calculate_placement(surface, &placement->placement);

	swbuf_set_source_rgb(surface, placement->fill_color);
	cairo_set_line_width(surface->ctx, 0);
	if (placement->round == 0) {
		cairo_rectangle(surface->ctx, abs_placement.top_left.x, abs_placement.top_left.y, placement->placement.width, placement->placement.height);
	} else {
		/* Calculate rounded path */
		cairo_move_to(surface->ctx, abs_placement.top_left.x + placement->round, abs_placement.top_left.y);
		cairo_line_to(surface->ctx, abs_placement.bottom_right.x - placement->round, abs_placement.top_left.y);
		cairo_arc(surface->ctx, abs_placement.bottom_right.x - placement->round, abs_placement.top_left.y + placement->round, placement->round, -M_PI / 2, 0);
		cairo_line_to(surface->ctx, abs_placement.bottom_right.x, abs_placement.bottom_right.y - placement->round);
		cairo_arc(surface->ctx, abs_placement.bottom_right.x - placement->round, abs_placement.bottom_right.y - placement->round, placement->round, 0, M_PI / 2);
		cairo_line_to(surface->ctx, abs_placement.top_left.x + placement->round, abs_placement.bottom_right.y);
		cairo_arc(surface->ctx, abs_placement.top_left.x + placement->round, abs_placement.bottom_right.y - placement->round, placement->round, M_PI / 2, M_PI);
		cairo_line_to(surface->ctx, abs_placement.top_left.x, abs_placement.top_left.y + placement->round);
		cairo_arc(surface->ctx, abs_placement.top_left.x + placement->round, abs_placement.top_left.y + placement->round, placement->round, M_PI, M_PI * 3 / 2);
	}
	cairo_fill(surface->ctx);
}

void swbuf_dump(struct cairo_swbuf_t *surface, const char *png_filename) {
	cairo_surface_write_to_png(surface->surface, png_filename);
}

void free_swbuf(struct cairo_swbuf_t *buffer) {
	if (!buffer) {
		return;
	}
	cairo_destroy(buffer->ctx);
	cairo_surface_destroy(buffer->surface);
	free(buffer);
}

void cairo_addfont(const char *font_ttf_filename) {
	FcConfigAppFontAddFile(FcConfigGetCurrent(), (uint8_t*)font_ttf_filename);
}

void cairo_cleanup(void) {
	/* Super stupid workaround to be able to properly memcheck -- WE (!) need
	 * to tell fontconfig to get its shit together even though it's Cairo's (!!)
	 * transitive dependency. What a bunch of garbage.
	 */
	cairo_debug_reset_static_data();
	FcFini();
}
