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
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <string.h>
#include "display_fb.h"

#define BITMASK(bits)				((1 << (bits)) - 1)
#define TRUNCATE_TO_BITS(x, bits)	((((x) / (1 << (8 - (bits))) & BITMASK(bits))))

static unsigned int display_get_mapped_size(const struct display_t *display) {
	return display->width * display->height * display->bits_per_pixel / 8;
}

static void display_fill_16bit(struct display_t *display, uint16_t pixel) {
	struct display_fb_ctx_t *ctx = (struct display_fb_ctx_t*)display->drv_context;
	if (display->bits_per_pixel != 16) {
		fprintf(stderr, "not 16bpp screen\n");
		return;
	}
	uint16_t *screen = (uint16_t*)ctx->screen;
	for (unsigned int i = 0; i < display->width * display->height; i++) {
		*screen = pixel;
		screen++;
	}
}

static void display_fill_32bit(struct display_t *display, uint32_t pixel) {
	struct display_fb_ctx_t *ctx = (struct display_fb_ctx_t*)display->drv_context;
	if (display->bits_per_pixel != 32) {
		fprintf(stderr, "not 32bpp screen\n");
		return;
	}
	uint32_t *screen = (uint32_t*)ctx->screen;
	for (unsigned int i = 0; i < display->width * display->height; i++) {
		*screen = pixel;
		screen++;
	}
}

static void display_put_16bit(struct display_t *display, unsigned int x, unsigned int y, uint16_t pixel) {
	struct display_fb_ctx_t *ctx = (struct display_fb_ctx_t*)display->drv_context;
	uint16_t *screen = (uint16_t*)ctx->screen;
	screen[(y * display->width) + x] = pixel;
}

static void display_put_32bit(struct display_t *display, unsigned int x, unsigned int y, uint32_t pixel) {
	struct display_fb_ctx_t *ctx = (struct display_fb_ctx_t*)display->drv_context;
	uint32_t *screen = (uint32_t*)ctx->screen;
	screen[(y * display->width) + x] = pixel;
}

static uint16_t rgb_to_16bit(uint32_t rgb) {
	int r = TRUNCATE_TO_BITS(GET_R(rgb), 5);
	int g = TRUNCATE_TO_BITS(GET_G(rgb), 6);
	int b = TRUNCATE_TO_BITS(GET_B(rgb), 5);
	uint16_t pixel = (r << 11) | (g << 5) | (b << 0);
	return pixel;
}

static void display_fb_put_pixel(struct display_t *display, unsigned int x, unsigned int y, uint32_t rgb) {
	if (display->bits_per_pixel == 32) {
		display_put_32bit(display, x, y, rgb);
	} else if (display->bits_per_pixel == 16) {
		uint16_t pixel = rgb_to_16bit(rgb);
		display_put_16bit(display, x, y, pixel);
	} else {
		fprintf(stderr, "Don't know how to blit %d bpp screen.\n", display->bits_per_pixel);
	}
}

static void display_fb_fill(struct display_t *display, uint32_t rgb) {
	if (display->bits_per_pixel == 32) {
		display_fill_32bit(display, rgb);
	} else if (display->bits_per_pixel == 16) {
		uint16_t pixel = rgb_to_16bit(rgb);
		display_fill_16bit(display, pixel);
	} else {
		fprintf(stderr, "Don't know how to fill %d bpp screen.\n", display->bits_per_pixel);
	}
}

static bool display_fb_init(struct display_t *display, void *init_ctx) {
	const char *fbdev = (const char*)init_ctx;
	struct display_fb_ctx_t *ctx = (struct display_fb_ctx_t*)display->drv_context;

	if (!fbdev) {
		fprintf(stderr, "fbdev is NULL, not creating a hardware display.\n");
		return false;
	}

	ctx->fd = open(fbdev, O_RDWR);
	if (ctx->fd == -1) {
		perror(fbdev);
		return false;
	}

	struct fb_var_screeninfo vinfo;
	if (ioctl(ctx->fd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
		perror("ioctl(FBIOGET_VSCREENINFO)");
		display_free(display);
		return false;
	}
	display->width = vinfo.xres;
	display->height = vinfo.yres;
	display->bits_per_pixel = vinfo.bits_per_pixel;
	fprintf(stderr, "Initiated framebuffer device %d x %d pixels at %d BPP\n", display->width, display->height, display->bits_per_pixel);

	ctx->screen = (uint8_t*)mmap(0, display_get_mapped_size(display), PROT_READ | PROT_WRITE, MAP_SHARED, ctx->fd, 0);
	if (ctx->screen == (void*)-1) {
		perror("mmap");
		ctx->screen = NULL;
		return false;
	}

	return true;
}

static void display_fb_free(struct display_t *display) {
	struct display_fb_ctx_t *ctx = (struct display_fb_ctx_t*)display->drv_context;
	if (ctx->screen) {
		if (munmap(ctx->screen, display_get_mapped_size(display))) {
			perror("munmap");
		}
	}
	if (ctx->fd != -1) {
		close(ctx->fd);
	}
}

static unsigned int display_fb_get_ctx_size(void) {
	return sizeof(struct display_fb_ctx_t);
}

static bool display_fb_blit_buffer(struct display_t *display, uint32_t *source, unsigned int width, unsigned int height) {
	if (display->bits_per_pixel != 32) {
		return false;
	}
	if ((width != display->width) || (height != display->height)) {
		return false;
	}

	struct display_fb_ctx_t *ctx = (struct display_fb_ctx_t*)display->drv_context;
	memcpy(ctx->screen, source, sizeof(uint32_t) * width * height);
	return true;
}

const struct display_calltable_t display_fb_calltable = {
	.init = display_fb_init,
	.free = display_fb_free,
	.fill = display_fb_fill,
	.put_pixel = display_fb_put_pixel,
	.get_ctx_size = display_fb_get_ctx_size,
	.blit_buffer = display_fb_blit_buffer,
};
