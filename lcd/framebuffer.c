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
#include "framebuffer.h"

#define BITMASK(bits)				((1 << (bits)) - 1)
#define TRUNCATE_TO_BITS(x, bits)	((((x) / (1 << (8 - (bits))) & BITMASK(bits))))

static unsigned int display_get_mapped_size(const struct display_t *display) {
	return display->width * display->height * display->bits_per_pixel / 8;
}

static void display_fill_16bit(struct display_t *display, uint16_t pixel) {
	if (display->bits_per_pixel != 16) {
		fprintf(stderr, "not 16bpp screen\n");
		return;
	}
	uint16_t *screen = (uint16_t*)display->screen;
	for (unsigned int i = 0; i < display->width * display->height; i++) {
		*screen = pixel;
		screen++;
	}
}

static void display_put_16bit(struct display_t *display, unsigned int x, unsigned int y, uint16_t pixel) {
	uint16_t *screen = (uint16_t*)display->screen;
	screen[(y * display->width) + x] = pixel;
}

static uint16_t rgb_to_16bit(uint32_t rgb) {
	int r = TRUNCATE_TO_BITS(GET_R(rgb), 5);
	int g = TRUNCATE_TO_BITS(GET_G(rgb), 6);
	int b = TRUNCATE_TO_BITS(GET_B(rgb), 5);
	uint16_t pixel = (r << 11) | (g << 5) | (b << 0);
	return pixel;
}

void display_put_pixel(struct display_t *display, unsigned int x, unsigned int y, uint32_t rgb) {
	if (display->bits_per_pixel == 16) {
		uint16_t pixel = rgb_to_16bit(rgb);
		display_put_16bit(display, x, y, pixel);
	} else {
		fprintf(stderr, "Don't know how to blit %d bpp screen.\n", display->bits_per_pixel);
	}
}

void display_fill(struct display_t *display, uint32_t rgb) {
	if (display->bits_per_pixel == 16) {
		uint16_t pixel = rgb_to_16bit(rgb);
		display_fill_16bit(display, pixel);
	} else {
		fprintf(stderr, "Don't know how to fill %d bpp screen.\n", display->bits_per_pixel);
	}
}

struct display_t* display_init(const char *fbdev) {
	if (!fbdev) {
		fprintf(stderr, "fbdev is NULL\n");
		return NULL;
	}

	struct display_t *display = calloc(sizeof(struct display_t), 1);
	if (!display) {
		perror("calloc");
		return NULL;
	}

	display->fd = open(fbdev, O_RDWR);
	if (display->fd == -1) {
		perror(fbdev);
		free(display);
		return NULL;
	}

	struct fb_var_screeninfo vinfo;
	if (ioctl(display->fd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
		perror("ioctl(FBIOGET_VSCREENINFO)");
		display_free(display);
		return NULL;
	}
	display->width = vinfo.xres;
	display->height = vinfo.yres;
	display->bits_per_pixel = vinfo.bits_per_pixel;

	display->screen = (uint8_t*)mmap(0, display_get_mapped_size(display), PROT_READ | PROT_WRITE, MAP_SHARED, display->fd, 0);
	if (display->screen == (void*)-1) {
		perror("mmap");
		display->screen = NULL;
		display_free(display);
		return NULL;
	}
	display->mmapped = true;

	return display;
}

struct display_t* display_sw_init(unsigned int width, unsigned int height) {
	const unsigned int bits_per_pixel = 16;
	const unsigned int length = width * height * bits_per_pixel / 8;

	struct display_t *display = calloc(sizeof(struct display_t) + length, 1);
	if (!display) {
		perror("calloc");
		return NULL;
	}

	display->fd = -1;
	display->screen = (uint8_t*)&display[1];
	display->width = width;
	display->bits_per_pixel = bits_per_pixel;
	display->height = height;
	display->mmapped = false;

	return display;
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

void display_free(struct display_t *display) {
	if (!display) {
		return;
	}
	if (display->screen && display->mmapped) {
		if (munmap(display->screen, display_get_mapped_size(display))) {
			perror("munmap");
		}
	}
	if (display->fd != -1) {
		close(display->fd);
	}
	free(display);
}
