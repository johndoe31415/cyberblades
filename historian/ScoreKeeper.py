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

class ScoreKeeper():
	def __init__(self):
		self._data = None

	def process(self, event):
		etype = event["event"]
		if etype == "songStart":
			self._data = {
				"meta": {
					"song_author":		event["status"]["beatmap"]["songAuthorName"],
					"song_title":		event["status"]["beatmap"]["songName"],
					"level_author":		event["status"]["beatmap"]["levelAuthorName"],
					"bpm":				event["status"]["beatmap"]["songBPM"],
					"max_score":		event["status"]["beatmap"]["maxScore"],
					"notes_cnt":		event["status"]["beatmap"]["notesCount"],
				},
			}
		elif etype == "scoreChanged":
			self._data.update({
				"performance": {
					"score":			event["status"]["performance"]["score"],
					"max_score":		event["status"]["performance"]["currentMaxScore"],
					"combo":			event["status"]["performance"]["combo"],
					"max_combo":		event["status"]["performance"]["maxCombo"],
					"passed_notes":		event["status"]["performance"]["passedNotes"],
					"hit_notes":		event["status"]["performance"]["hitNotes"],
					"missed_notes":		event["status"]["performance"]["missedNotes"],
				}
			})

	def to_dict(self):
		return self._data
