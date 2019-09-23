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
#define COLOR_GREEN					MK_RGB(0xff, 0xff, 0x00)
#define COLOR_BLUE					MK_RGB(0xff, 0x00, 0xff)
#define COLOR_WHITE					MK_RGB(0xff, 0xff, 0xff)

#endif
