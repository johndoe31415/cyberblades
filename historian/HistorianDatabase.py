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

import sqlite3
import json
import contextlib

class HistorianDatabase():
	def __init__(self, config):
		self._config = config
		self._db = sqlite3.connect(self._config.get("historian_db"))
		self._cursor = self._db.cursor()
		with contextlib.suppress(sqlite3.OperationalError):
			self._cursor.execute("""
			CREATE TABLE results (
				int id PRIMARY KEY,
				player varchar NULL,
				play_day varchar NOT NULL,
				starttime_local float NOT NULL,
				starttime integer NOT NULL,
				endtime integer NOT NULL,
				song_title varchar NOT NULL,
				song_author varchar NOT NULL,
				level_author varchar NOT NULL,
				result_verdict varchar NOT NULL,
				result_score integer NOT NULL,
				result_maxscore integer NOT NULL,
				result_rank varchar NOT NULL,
				hit_bombs integer NOT NULL,
				missed_notes integer NOT NULL,
				passed_notes integer NOT NULL,
				max_combo integer NOT NULL,
				playtime float NOT NULL,
				CHECK((result_verdict = 'pass') OR (result_verdict = 'fail')),
				UNIQUE(starttime_local, starttime, endtime, result_score)
			);
			""")
		self._db.commit()

