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
	def __init__(self, config):
		self._config = config
		self._db = sqlite3.connect(self._config["historian_db"])
		self._cursor = self._db.cursor()
		self._cursor.row_factory = self._row_dict_factory
		with contextlib.suppress(sqlite3.OperationalError):
			self._cursor.execute("""
			CREATE TABLE results (
				gameid integer PRIMARY KEY,
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

	@staticmethod
	def _row_dict_factory(cursor, row):
		return { column[0]: row[index] for (index, column) in enumerate(cursor.description) }

	def _insert_table(self, tablename, rowdata):
		keyvalues = list(rowdata.items())
		keys = ", ".join(key for (key, value) in keyvalues)
		values = ", ".join([ "?" ] * len(keyvalues))
		sql = "INSERT INTO %s (%s) VALUES (%s);" % (tablename, keys, values)
		self._cursor.execute(sql, tuple(value for (key, value) in keyvalues))

	def _insert_result(self, rowdata):
		return self._insert_table("results", rowdata)

	def _have_result(self, gamehash):
		result = self._cursor.execute("SELECT COUNT(*) AS cnt FROM results WHERE gamehash = ?;", (gamehash, )).fetchone()
		return result["cnt"] != 0

	def _file_seen(self, filesize, mtime_micros):
		result = self._cursor.execute("SELECT COUNT(*) AS cnt FROM seen_files WHERE size_bytes = ? AND mtime_micros = ?;", (filesize, mtime_micros)).fetchone()
		return result["cnt"] != 0

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

	def recent_results(self, limit = 10):
		return self._cursor.execute("""
			SELECT player
				FROM results
				ORDER BY endtime DESC
				LIMIT ?;
		""", (limit, )).fetchall()

	def get_last_game(self, player):
		return self._cursor.execute("""
			SELECT gameid, song_author, song_title, level_author, difficulty, score, max_score, rank, player, local_ts, max_combo, verdict
				FROM results
				WHERE player = ?
				ORDER BY endtime DESC
				LIMIT 1;
		""", (player, )).fetchone()

	def all_song_keys(self):
		return self._cursor.execute("""
			SELECT DISTINCT song_author, song_title, level_author, difficulty
				FROM results
				ORDER BY song_author ASC, song_title ASC, difficulty DESC;
		""").fetchall()

	def get_highscores(self, song_key, limit = 100):
		table = self._cursor.execute("""
			SELECT gameid, player, local_ts, score, max_score, rank, max_combo, verdict
			FROM results
			WHERE (song_title = ?) AND (song_author = ?) AND (level_author = ?) AND (difficulty = ?)
			ORDER BY score DESC, max_combo DESC
			LIMIT ?;
		""", (song_key["song_title"], song_key["song_author"], song_key["level_author"], song_key["difficulty"], limit)).fetchall()
		last_score = None
		rank = 0
		for entry in table:
			this_score = (entry["score"], entry["max_combo"])
			if this_score != last_score:
				rank += 1
				last_score = this_score
			entry["number"] = rank
		result = {
			"song_key": {
				"song_title":	song_key["song_title"],
				"song_author":	song_key["song_author"],
				"level_author":	song_key["level_author"],
				"difficulty":	song_key["difficulty"],
			},
			"table":	table,
		}
		return result

	def get_highscore_rank(self, song_key, score, max_combo):
		row = self._cursor.execute("""
			SELECT COUNT(DISTINCT COALESCE(score, '|', max_combo)) AS rank
				FROM results
				WHERE ((song_title = ?) AND (song_author = ?) AND (level_author = ?) AND (difficulty = ?))
					AND	((score > ?) OR ((score = ?) AND (max_combo > ?)));
			""", (song_key["song_title"], song_key["song_author"], song_key["level_author"], song_key["difficulty"], score, score, max_combo)).fetchone()
		return row["rank"] + 1

	def get_personal_highscores(self, player, limit = 10):
		last_game = self.get_last_game(player)
		if last_game is not None:
			highscores = self.get_highscores(song_key = last_game, limit = limit)
			highscore_game_ids = set(highscore["gameid"] for highscore in highscores["table"])
			if (last_game["gameid"] not in highscore_game_ids):
				number = self.get_highscore_rank(song_key = last_game, score = last_game["score"], max_combo = last_game["max_combo"])
				highscores["table"] = highscores["table"][:-1]
				last_game_highscore = { key: last_game[key] for key in [ "gameid", "local_ts", "max_combo", "max_score", "player", "rank", "verdict", "score" ] }
				last_game_highscore["number"] = number
				highscores["table"].append(last_game_highscore)
			for entry in highscores["table"]:
				if entry["gameid"] == last_game["gameid"]:
					entry["most_recent"] = True
				del entry["gameid"]
		else:
			highscores = None
		return highscores


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

		return self._cursor.execute("""
			SELECT player, COUNT(score) AS games_played, SUM(playtime) AS total_playtime_secs, SUM(score) AS total_score, SUM(max_score) AS total_max_score, SUM(passed_notes) AS total_passed_notes, SUM(missed_notes) AS total_missed_notes
			FROM results
			%s
			GROUP BY player
			ORDER BY total_playtime_secs DESC;""" % (where), parameters).fetchall()

	def get_playtimes_today(self, player = None):
		return self.get_playtimes(date = datetime.date.today(), player = player)

	def get_last_games(self, time_duration_secs = 3600 * 3, limit = 10):
		starttime_after = time.time() - time_duration_secs
		return self._cursor.execute("""
			SELECT player, score, max_score, rank, max_combo, verdict, local_ts, song_author, song_title, level_author, difficulty
			FROM results
			WHERE starttime_local > ?
			ORDER BY endtime DESC
			LIMIT ?;
		""", (starttime_after, limit)).fetchall()

	def get_player_info(self, player):
		def _first(rlist):
			if len(rlist) == 0:
				return None
			else:
				return rlist[0]
		result = {
			"today":		_first(self.get_playtimes_today(player = player)),
			"alltime":		_first(self.get_playtimes(player = player)),
			"highscore":	self.get_personal_highscores(player = player, limit = 10),
		}
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
