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

#ifndef __COLORS_H__
#define __COLORS_H__

#define UINT8(x)					((x) & 0xff)
#define GET_R(rgb)					UINT8((rgb) >> 16)
#define GET_G(rgb)					UINT8((rgb) >> 8)
#define GET_B(rgb)					UINT8((rgb) >> 0)

#define MK_RGB(r, g, b)				((UINT8(r) << 16) | (UINT8(g) << 8) | (UINT8(b) << 0))

#define COLOR_BLACK					MK_RGB(0x00, 0x00, 0x00)
#define COLOR_RED					MK_RGB(0xff, 0x00, 0x00)
#define COLOR_GREEN					MK_RGB(0x00, 0xff, 0x00)
#define COLOR_BLUE					MK_RGB(0x00, 0x00, 0xff)
#define COLOR_WHITE					MK_RGB(0xff, 0xff, 0xff)
#define COLOR_GRAY					MK_RGB(0x80, 0x80, 0x80)
#define COLOR_BS_RED				MK_RGB(0xdf, 0x20, 0x33)
#define COLOR_BS_BLUE				MK_RGB(0x38, 0x6c, 0xb6)
#define COLOR_BS_DARKBLUE			MK_RGB(0x0c, 0x1d, 0x2f)

#define COLOR_TURQUOISE				MK_RGB(0x1a, 0xbc, 0x9c)
#define COLOR_GREEN_SEA				MK_RGB(0x16, 0xa0, 0x85)
#define COLOR_EMERLAND				MK_RGB(0x2e, 0xcc, 0x71)
#define COLOR_NEPHRITIS				MK_RGB(0x27, 0xae, 0x60)
#define COLOR_PETER_RIVER			MK_RGB(0x34, 0x98, 0xdb)
#define COLOR_BELIZE_HOLE			MK_RGB(0x29, 0x80, 0xb9)
#define COLOR_AMETHYST				MK_RGB(0x9b, 0x59, 0xb6)
#define COLOR_WISTERIA				MK_RGB(0x8e, 0x44, 0xad)
#define COLOR_WET_ASPHALT			MK_RGB(0x34, 0x49, 0x5e)
#define COLOR_MIDNIGHT_BLUE			MK_RGB(0x2c, 0x3e, 0x50)
#define COLOR_SUN_FLOWER			MK_RGB(0xf1, 0xc4, 0x0f)
#define COLOR_ORANGE				MK_RGB(0xf3, 0x9c, 0x12)
#define COLOR_CARROT				MK_RGB(0xe6, 0x7e, 0x22)
#define COLOR_PUMPKIN				MK_RGB(0xd3, 0x54, 0x00)
#define COLOR_ALIZARIN				MK_RGB(0xe7, 0x4c, 0x3c)
#define COLOR_POMEGRANATE			MK_RGB(0xc0, 0x39, 0x2b)
#define COLOR_CLOUDS				MK_RGB(0xec, 0xf0, 0xf1)
#define COLOR_SILVER				MK_RGB(0xbd, 0xc3, 0xc7)
#define COLOR_CONCRETE				MK_RGB(0x95, 0xa5, 0xa6)
#define COLOR_ASBESTOS				MK_RGB(0x7f, 0x8c, 0x8d)

#endif
