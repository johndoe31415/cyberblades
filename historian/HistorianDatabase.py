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
import sqlite3
import json
import contextlib
import tzlocal
import pytz
import datetime
import collections
import gzip
from ScoreKeeper import ScoreKeeper

class HistorianDatabase():
	_PlayResult = collections.namedtuple("PlayResult", [ "player", "starttime_local", "song_title", "song_author", "level_author", "difficulty", "playtime", "result_verdict", "result_score", "result_maxscore", "max_combo" ])
	_SongDifficulty = collections.namedtuple("SongDifficulty", [ "song_title", "song_author", "level_author", "difficulty" ])

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
				difficulty integer NOT NULL,
				playtime float NOT NULL,
				pausetime float NOT NULL,
				gamehash varchar NOT NULL,
				result_verdict varchar NOT NULL,
				result_score integer NOT NULL,
				result_maxscore integer NOT NULL,
				result_rank varchar NOT NULL,
				hit_bombs integer NOT NULL,
				missed_notes integer NOT NULL,
				passed_notes integer NOT NULL,
				max_combo integer NOT NULL,
				CHECK((result_verdict = 'pass') OR (result_verdict = 'fail')),
				UNIQUE(starttime_local, starttime, endtime, result_score)
			);
			""")
		with contextlib.suppress(sqlite3.OperationalError):
			self._cursor.execute("""
			CREATE TABLE seen_files (
				size_bytes integer NOT NULL,
				mtime_micros integer NOT NULL,
				PRIMARY KEY(size_bytes, mtime_micros)
			);
			""")
		self._db.commit()

	def _insert_result(self, rowdata):
		keyvalues = list(rowdata.items())
		keys = ", ".join(key for (key, value) in keyvalues)
		values = ", ".join([ "?" ] * len(keyvalues))
		sql = "INSERT INTO results (%s) VALUES (%s);" % (keys, values)
		self._cursor.execute(sql, tuple(value for (key, value) in keyvalues))

	def _have_result(self, gamehash):
		(count, ) = self._cursor.execute("SELECT COUNT(*) FROM results WHERE gamehash = ?;", (gamehash, )).fetchone()
		return count != 0

	def _file_seen(self, filesize, mtime_micros):
		(count, ) = self._cursor.execute("SELECT COUNT(*) FROM seen_files WHERE size_bytes = ? AND mtime_micros = ?;", (filesize, mtime_micros)).fetchone()
		return count != 0

	def _add_file_seen(self, filesize, mtime_micros):
		self._cursor.execute("INSERT INTO seen_files (size_bytes, mtime_micros) VALUES (?, ?);", (filesize, mtime_micros))

	def add_scorekeeper_results(self, player, starttime_local_timet, scorekeeper):
		localzone = tzlocal.get_localzone()
		localdatetime = localzone.fromutc(datetime.datetime.utcfromtimestamp(starttime_local_timet))
		play_day = localdatetime.date().strftime("%Y-%m-%d")

		if not self._have_result(scorekeeper.gamehash):
			skr = scorekeeper.to_dict()
			rowdata = {
				"player":			player,
				"play_day":			play_day,
				"starttime_local":	starttime_local_timet,
				"starttime":		skr["meta"]["start_ts"],
				"endtime":			skr["meta"]["end_ts"],
				"playtime":			skr["meta"]["playtime"],
				"pausetime":		skr["meta"]["pausetime"],
				"song_author":		skr["meta"]["song_author"],
				"song_title":		skr["meta"]["song_title"],
				"level_author":		skr["meta"]["level_author"],
				"difficulty":		skr["meta"]["difficulty"],
				"result_verdict":	skr["final"]["verdict"],
				"result_score":		skr["final"]["score"],
				"result_maxscore":	skr["final"]["max_score"],
				"result_rank":		skr["final"]["rank"],
				"hit_bombs":		skr["final"]["hit_bombs"],
				"missed_notes":		skr["final"]["missed_notes"],
				"passed_notes":		skr["final"]["passed_notes"],
				"max_combo":		skr["final"]["max_combo"],
				"gamehash":			scorekeeper.gamehash,
			}
			self._insert_result(rowdata)

	def _parse_history(self, filename):
		if filename.endswith(".gz"):
			with gzip.open(filename) as f:
				history = json.load(f)
		else:
			with open(filename) as f:
				history = json.load(f)
		sk = ScoreKeeper(advanced = True)
		sk.process_all(history["events"])
		player = history["meta"]["player"]
		starttime_local_timet = history["meta"]["songStartLocal"]
		self.add_scorekeeper_results(player = player, starttime_local_timet = starttime_local_timet, scorekeeper = sk)

	def parse_history(self, filename):
		stat = os.stat(filename)
		mtime_micros = int(stat.st_mtime * 1000000)
		filesize = stat.st_size
		if self._file_seen(filesize, mtime_micros):
			return
		self._parse_history(filename)
		self._add_file_seen(filesize, mtime_micros)
		self._db.commit()

	def recent_results(self, count = 10):
		fields = ", ".join(self._PlayResult._fields)
		return [ self._PlayResult(*row) for row in self._cursor.execute("SELECT %s FROM results ORDER BY endtime DESC LIMIT ?;" % (fields, count)).fetchall() ]

	def all_song_difficulties(self):
		fields = ", ".join(self._SongDifficulty._fields)
		return [ self._SongDifficulty(*row) for row in self._cursor.execute("SELECT DISTINCT %s FROM results ORDER BY song_author ASC, song_title ASC, difficulty DESC;" % (fields)).fetchall() ]

	def get_highscores(self, song_difficulty, count = 10):
		fields = ", ".join(self._PlayResult._fields)
		return [ self._PlayResult(*row) for row in self._cursor.execute("SELECT %s FROM results WHERE song_title = ? AND song_author = ? AND level_author = ? AND difficulty = ? ORDER BY result_score DESC LIMIT ?;" % (fields), (song_difficulty.song_title, song_difficulty.song_author, song_difficulty.level_author, song_difficulty.difficulty, count)).fetchall() ]
