#	pibeatsaber - Beat Saber historian application that tracks players
#	Copyright (C) 2019-2019 Johannes Bauer
#
#	This file is part of pibeatsaber.
#
#	pibeatsaber is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; this program is ONLY licensed under
#	version 3 of the License, later versions are explicitly excluded.
#
#	pibeatsaber is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
#	Johannes Bauer <JohannesBauer@gmx.de>

import enum

class DifficultyEnum(enum.IntEnum):
	Easy = 0
	Normal = 1
	Hard = 2
	Expert = 3
	ExpertPlus = 4

	@classmethod
	def byname(cls, name):
		name = name.replace("+", "Plus")
		return getattr(cls, name)
