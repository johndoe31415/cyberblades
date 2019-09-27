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

import hashlib
from StatisticalEval import StatisticalEval
from DAOObjects import DifficultyEnum

class _SaberStatistic():
	def __init__(self):
		self._cut_count = 0
		self._correct_cut_count = 0
		self._saberspeed = StatisticalEval()
		self._dist_to_center = StatisticalEval()
		self._direction_deviation = StatisticalEval()
		self._time_deviation = StatisticalEval()

	def process(self, event):
		if event["event"] == "noteFullyCut":
			self._saberspeed.append(event["noteCut"]["saberSpeed"])
			self._dist_to_center.append(event["noteCut"]["cutDistanceToCenter"])
			self._direction_deviation.append(event["noteCut"]["cutDirectionDeviation"])
			self._time_deviation.append(event["noteCut"]["timeDeviation"])
			self._cut_count += 1
			if event["noteCut"]["saberTypeOK"]:
				self._correct_cut_count += 1

	def to_dict(self):
		return {
			"cuts":					self._cut_count,
			"correct_cuts":			self._correct_cut_count,
			"saber_speed":			self._saberspeed.to_dict(),
			"distance_to_center":	self._dist_to_center.to_dict(),
			"direction_deviation":	self._direction_deviation.to_dict(),
			"time_deviation":		self._time_deviation.to_dict(),
		}

class ScoreKeeper():
	def __init__(self, advanced = False):
		self._data = None
		self._advanced = advanced
		self._pause_begin = None
		self._total_pause_duration = 0
		self._song_begin_time = None
		self._saber_statistics = {
			"SaberA":	_SaberStatistic(),
			"SaberB":	_SaberStatistic(),
		}
		self._hashdata = [ ]
		self._gamehash = None

	@property
	def gamehash(self):
		if (self._gamehash is None) and len(self._hashdata) == 7:
			assert(all(isinstance(item, (str, int)) for item in self._hashdata))
			hash_bindata = ("\n".join(str(item) for item in self._hashdata)).encode("utf-8")
			self._gamehash = hashlib.md5(hash_bindata).hexdigest()
		return self._gamehash

	@staticmethod
	def _get_performance(event):
		return {
			"score":			event["status"]["performance"]["score"],
			"max_score":		event["status"]["performance"]["currentMaxScore"],
			"combo":			event["status"]["performance"]["combo"],
			"max_combo":		event["status"]["performance"]["maxCombo"],
			"passed_notes":		event["status"]["performance"]["passedNotes"],
			"hit_notes":		event["status"]["performance"]["hitNotes"],
			"missed_notes":		event["status"]["performance"]["missedNotes"],
			"hit_bombs":		event["status"]["performance"]["hitBombs"],
			"passed_bombs":		event["status"]["performance"]["passedBombs"],
			"rank":				event["status"]["performance"]["rank"],
		}

	def process(self, event):
		etype = event["event"]
		if etype == "scoreChanged":
			self._data["performance"] = self._get_performance(event)
		elif etype == "songStart":
			self._data = {
				"meta": {
					"start_ts":			event["time"],
					"song_author":		event["status"]["beatmap"]["songAuthorName"],
					"song_title":		event["status"]["beatmap"]["songName"],
					"level_author":		event["status"]["beatmap"]["levelAuthorName"],
					"bpm":				event["status"]["beatmap"]["songBPM"],
					"max_score":		event["status"]["beatmap"]["maxScore"],
					"notes_cnt":		event["status"]["beatmap"]["notesCount"],
					"difficulty":		int(DifficultyEnum.byname(event["status"]["beatmap"]["difficulty"])),
				},
			}
			self._hashdata += [ self._data["meta"]["start_ts"], self._data["meta"]["song_author"], self._data["meta"]["song_title"], self._data["meta"]["level_author"], self._data["meta"]["difficulty"], self._data["meta"]["notes_cnt"] ]
		elif (etype == "finished") or (etype == "failed"):
			self._data["meta"].update({
				"end_ts":		event["time"],
				"playtime":		(event["time"] - self._data["meta"]["start_ts"] - self._total_pause_duration) / 1000,
				"pausetime":	self._total_pause_duration / 1000,
			})
			self._data["final"] = self._get_performance(event)
			self._data["final"].update({
					"verdict":			"fail" if (etype == "failed") else "pass",
			})
			self._data["sabers"] = {
					"left":				self._saber_statistics["SaberA"].to_dict(),
					"right":			self._saber_statistics["SaberB"].to_dict(),
			}
			self._hashdata += [ self._data["meta"]["end_ts"] ]
		elif self._advanced and (etype == "noteFullyCut"):
			stype = event["noteCut"]["saberType"]
			self._saber_statistics[stype].process(event)
		elif self._advanced and (etype == "pause"):
			self._pause_begin = event["time"]
		elif self._advanced and (etype == "resume"):
			pause_duration = event["time"] - self._pause_begin
			self._pause_begin = None
			self._total_pause_duration += pause_duration
#		else:
#			print(etype)

	def process_all(self, events):
		for event in events:
			self.process(event)

	def to_dict(self):
		return self._data

if __name__ == "__main__":
	import json
	import os
	dirname = "history/joe/"
	#for filename in [ "2019_09_24_20_24_44.json" ]:
	for filename in os.listdir(dirname):
		filename = dirname + filename
		with open(filename) as f:
			history = json.load(f)

		sk = ScoreKeeper(advanced = True)
		sk.process_all(history["events"])
		print(sk.gamehash)
		#print(json.dumps(sk.to_dict(), indent = 4, sort_keys = True))
