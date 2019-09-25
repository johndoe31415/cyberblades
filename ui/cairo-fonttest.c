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
#include "cairo.h"
#include "colors.h"

int main(void) {
	struct cairo_swbuf_t *surface = create_swbuf(1000, 850);
	swbuf_clear(surface, COLOR_WHITE);

	const unsigned int k = 224;
	swbuf_circle(surface, 500 - k, 75, 3, COLOR_BLUE);
	swbuf_circle(surface, 500 + k, 75, 3, COLOR_BLUE);
	swbuf_circle(surface, 500, 100, 5, COLOR_RED);
	swbuf_text(surface, &(const struct font_placement_t){
		.font_face = "Beon",
		.font_size = 64,
		.font_color = COLOR_BLACK,
		.placement = {
			.src_anchor = {
				.x = XPOS_CENTER,
				.y = YPOS_BOTTOM,
			},
			.xoffset = 500,
			.yoffset = 100,
		},
	}, "ABC W j y 12345");

	swbuf_text(surface, &(const struct font_placement_t){
		.font_face = "Latin Modern Sans",
		.font_size = 64,
		.font_color = COLOR_BLACK,
		.placement = {
			.src_anchor = {
				.x = XPOS_CENTER,
				.y = YPOS_BOTTOM,
			},
			.xoffset = 500,
			.yoffset = 300,
		},
	}, "ABC W j y 12345");

	swbuf_text(surface, &(const struct font_placement_t){
		.font_face = "Latin Modern Sans",
		.font_size = 64,
		.font_color = COLOR_BLACK,
		.placement = {
			.src_anchor = {
				.y = YPOS_BOTTOM,
			},
			.xoffset = 100,
			.yoffset = 200,
		},
	}, "ABC");
	swbuf_text(surface, &(const struct font_placement_t){
		.font_face = "Latin Modern Sans",
		.font_size = 64,
		.font_color = COLOR_BLACK,
		.placement = {
			.src_anchor = {
				.y = YPOS_BOTTOM,
			},
			.xoffset = 250,
			.yoffset = 200,
		},
	}, "i");
	swbuf_text(surface, &(const struct font_placement_t){
		.font_face = "Latin Modern Sans",
		.font_size = 64,
		.font_color = COLOR_BLACK,
		.placement = {
			.src_anchor = {
				.y = YPOS_BOTTOM,
			},
			.xoffset = 300,
			.yoffset = 200,
		},
	}, "y");

	swbuf_text(surface, &(const struct font_placement_t){
		.font_face = "Digital-7 Mono",
		.font_size = 64,
		.font_color = COLOR_BLACK,
		.placement = {
			.src_anchor = {
				.x = XPOS_CENTER,
				.y = YPOS_BOTTOM,
			},
			.xoffset = 500,
			.yoffset = 400,
		},
	}, "ABC W j y 12345");

	swbuf_dump(surface, "cairo-fonttest.png");
	return 0;
}
