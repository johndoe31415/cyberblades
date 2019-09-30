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

import os
import json
import datetime

class Configuration():
	def __init__(self, configfile):
		with open(configfile) as f:
			self._config = json.load(f)
		self._base_dir = os.path.dirname(os.path.realpath(__file__))
		if not self._base_dir.endswith("/"):
			self._base_dir += "/"

	def _subs(self, text):
		text = text.replace("${base_dir}", self._base_dir)
		text = text.replace("${datetime_utc}", datetime.datetime.utcnow().strftime("%Y_%m_%d_%H_%M_%S"))
		return text

	def has(self, key):
		return key in self._config

	def __getitem__(self, key):
		return self._subs(self._config[key])
