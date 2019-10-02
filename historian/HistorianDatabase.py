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
import time
from ScoreKeeper import ScoreKeeper
from DAOObjects import DifficultyEnum

class HistorianDatabase():
	_PlayTimes = collections.namedtuple("PlayTimes", [ "player", "games_played", "playtime_secs", "score_sum", "max_score_sum", "passed_notes_sum", "missed_notes_sum" ])
	_PlayResult = collections.namedtuple("PlayResult", [ "player", "local_ts", "starttime_local", "song_title", "song_author", "level_author", "difficulty", "playtime", "pausetime", "verdict", "rank", "score", "max_score", "combo", "max_combo" ])
	_SongKey = collections.namedtuple("SongKey", [ "song_title", "song_author", "level_author", "difficulty" ])
	_DBReadHandlers = {
		"difficulty":		DifficultyEnum,
		"local_ts":			lambda x: datetime.datetime.strptime(x, "%Y-%m-%dT%H:%M:%S"),
	}

	def __init__(self, config):
		self._config = config
		self._db = sqlite3.connect(self._config["historian_db"])
		self._cursor = self._db.cursor()
		with contextlib.suppress(sqlite3.OperationalError):
			self._cursor.execute("""
			CREATE TABLE results (
				int id PRIMARY KEY,
				player varchar NULL,
				local_ts varchar NOT NULL,
				starttime_local float NOT NULL,
				gamehash varchar NOT NULL,

				starttime integer NOT NULL,
				endtime integer NOT NULL,
				song_author varchar NOT NULL,
				song_title varchar NOT NULL,
				level_author varchar NOT NULL,
				difficulty integer NOT NULL,

				playtime float NOT NULL,
				pausetime float NOT NULL,
				verdict varchar NOT NULL,
				rank varchar NOT NULL,
				score integer NOT NULL,
				max_score integer NOT NULL,
				combo integer NOT NULL,
				max_combo integer NOT NULL,
				passed_bombs integer NOT NULL,
				hit_bombs integer NOT NULL,
				passed_notes integer NOT NULL,
				missed_notes integer NOT NULL,

				CHECK((verdict = 'pass') OR (verdict = 'fail')),
				CHECK(difficulty >= 0),
				CHECK(difficulty <= 4),
				UNIQUE(starttime_local, starttime, endtime, score)
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
		local_ts = localdatetime.strftime("%Y-%m-%dT%H:%M:%S")

		if not self._have_result(scorekeeper.gamehash):
			skr = scorekeeper.to_dict()
			rowdata = {
				"player":			player,
				"local_ts":			local_ts,
				"starttime_local":	starttime_local_timet,
				"gamehash":			scorekeeper.gamehash,

				"starttime":		skr["meta"]["start_ts"],
				"endtime":			skr["meta"]["end_ts"],
				"song_author":		skr["meta"]["song_author"],
				"song_title":		skr["meta"]["song_title"],
				"level_author":		skr["meta"]["level_author"],
				"difficulty":		skr["meta"]["difficulty"],

				"playtime":			skr["meta"]["playtime"],
				"pausetime":		skr["meta"]["pausetime"],
				"verdict":			skr["final"]["verdict"],
				"rank":				skr["final"]["rank"],
				"score":			skr["final"]["score"],
				"max_score":		skr["final"]["max_score"],
				"combo":			skr["final"]["combo"],
				"max_combo":		skr["final"]["max_combo"],
				"passed_bombs":		skr["final"]["passed_bombs"],
				"hit_bombs":		skr["final"]["hit_bombs"],
				"passed_notes":		skr["final"]["passed_notes"],
				"missed_notes":		skr["final"]["missed_notes"],
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
		if sk.gamehash is None:
			print("No gamehash: %s" % (filename))
		else:
			player = history["meta"]["player"]
			starttime_local_timet = history["meta"]["songStartLocal"]
			self.add_scorekeeper_results(player = player, starttime_local_timet = starttime_local_timet, scorekeeper = sk)

	def mark_file_seen(self, filename):
		stat = os.stat(filename)
		mtime_micros = int(stat.st_mtime * 1000000)
		filesize = stat.st_size
		self._add_file_seen(filesize, mtime_micros)
		self._db.commit()

	def have_seen_file(self, filename):
		stat = os.stat(filename)
		mtime_micros = int(stat.st_mtime * 1000000)
		filesize = stat.st_size
		return self._file_seen(filesize, mtime_micros)

	def parse_history(self, filename):
		if self.have_seen_file(filename):
			return
		self._parse_history(filename)
		self.mark_file_seen(filename)

	def _results_select(self, sql_query, result_class, parameters = tuple(), insert_fields = True, process_fields = True):
		if insert_fields:
			fields = ", ".join(result_class._fields)
			sql_query = sql_query % (fields)
		results = [ list(row) for row in self._cursor.execute(sql_query, parameters).fetchall() ]
		if process_fields:
			for (i, fieldname) in enumerate(result_class._fields):
				handler = self._DBReadHandlers.get(fieldname)
				if handler is not None:
					for result in results:
						result[i] = handler(result[i])
		return [ result_class(*result) for result in results ]

	def recent_results(self, count = 10):
		return self._results_select("SELECT %s FROM results ORDER BY endtime DESC LIMIT ?;", parameters = (count, ), result_class = self._PlayResult)

	def get_last_game(self, player):
		return self._results_select("SELECT %s FROM results WHERE player = ? ORDER BY endtime DESC LIMIT 1;", parameters = (player, ), result_class = self._PlayResult, process_fields = False)

	def all_song_keys(self):
		return self._results_select("SELECT DISTINCT %s FROM results ORDER BY song_author ASC, song_title ASC, difficulty DESC;", result_class = self._SongKey)

	def get_highscores(self, song_key, count = 10, process_fields = True):
		return self._results_select("SELECT %s FROM results WHERE song_title = ? AND song_author = ? AND level_author = ? AND difficulty = ? ORDER BY score DESC LIMIT ?;", parameters = (song_key.song_title, song_key.song_author, song_key.level_author, song_key.difficulty, count), result_class = self._PlayResult, process_fields = process_fields)

	def get_highscore_rank(self, song_key, score):
		row = self._cursor.execute("SELECT COUNT(*) FROM results WHERE song_title = ? AND song_author = ? AND level_author = ? AND difficulty = ? AND score > ?;", (song_key.song_title, song_key.song_author, song_key.level_author, song_key.difficulty, score)).fetchone()
		return row[0] + 1

	def get_playtimes(self, date = None, player = None):
		where = [ ]
		parameters = [ ]
		if date is not None:
			date_str = date.strftime("%Y-%m-%d")
			where.append("local_ts LIKE '%s%%'" % (date_str))
		if player is not None:
			where.append("player = ?")
			parameters.append(player)

		if len(where) == 0:
			where = ""
		else:
			where = "WHERE %s" % (" AND ".join("(%s)" % (clause) for clause in where))
		return self._results_select("SELECT player, COUNT(score), SUM(playtime), SUM(score), SUM(max_score), SUM(passed_notes), SUM(missed_notes) AS playtime_secs FROM results %s GROUP BY player ORDER BY playtime_secs DESC;" % (where), parameters = parameters, result_class = self._PlayTimes, insert_fields = False)

	def get_playtimes_today(self, player = None):
		return self.get_playtimes(date = datetime.date.today(), player = player)

	def get_last_games(self, time_duration_secs = 3600 * 3, count = 10):
		starttime_after = time.time() - time_duration_secs
		return self._results_select("SELECT %s FROM results WHERE starttime_local > ? ORDER BY endtime DESC LIMIT ?;", parameters = (starttime_after, count), result_class = self._PlayResult)

	def get_player_info(self, player, process_fields = False):
		def _first(rlist):
			if len(rlist) == 0:
				return None
			else:
				return dict(rlist[0]._asdict())
		result = {
			"today":		_first(self.get_playtimes_today(player = player)),
			"alltime":		_first(self.get_playtimes(player = player)),
			"lastgame":		_first(self.get_last_game(player = player)),
		}
		if result["lastgame"] is not None:
			song_key = self._SongKey(**{ key: result["lastgame"][key] for key in [ "song_author", "song_title", "level_author", "difficulty" ] })
			result["highscore"] = [ hs._asdict() for hs in self.get_highscores(song_key, process_fields = process_fields) ]
			result["highscore_rank"] = self.get_highscore_rank(song_key, result["lastgame"]["score"])
		else:
			result["highscore"] = None
		return result

	def get_recent_players(self, time_duration_secs = 86400):
		starttime_after = time.time() - time_duration_secs
		recent_players = [ row[0] for row in self._cursor.execute("SELECT DISTINCT player FROM results WHERE starttime_local > ?", (starttime_after, )).fetchall() ]
		return recent_players


if __name__ == "__main__":
	from Configuration import Configuration
	config = Configuration("configuration.json")
	db = HistorianDatabase(config)
	print(db.get_recent_players())
