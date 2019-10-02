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
import socket
import asyncio
import websockets
import json
import time
import datetime
import contextlib
import gzip
from ScoreKeeper import ScoreKeeper
from HistorianDatabase import HistorianDatabase
from LocalCommunicationServer import LocalCommunicationServer

class BeatSaberHistorian():
	def __init__(self, config, args):
		self._config = config
		self._args = args
		self._current_player = None
		self._current_songdata = None
		self._connected_to_beatsaber = False
		self._current_score = None
		self._score_change = asyncio.Event()
		self._db = HistorianDatabase(self._config)
		self._local_server = LocalCommunicationServer(self)

	@property
	def config(self):
		return self._config

	@property
	def current_player(self):
		return self._current_player

	@current_player.setter
	def current_player(self, new_player):
		if new_player != self._current_player:
			self._current_player = new_player
			print("Player changed: %s" % (new_player))
			self._local_server.change_event()

	@property
	def connected_to_beatsaber(self):
		return self._connected_to_beatsaber

	@property
	def current_score(self):
		return self._current_score

	@property
	def db(self):
		return self._db

	def _finish_song(self):
		now = datetime.datetime.now().strftime("%Y_%m_%d_%H_%M_%S")
		destination_filename = self._config["history_directory"] + "/" + (self._current_player if (self._current_player is not None) else "unknown_player") + "/" + now + ".json.gz"
		with contextlib.suppress(FileExistsError):
			os.makedirs(os.path.dirname(destination_filename))
		with gzip.open(destination_filename, "wt") as f:
			json.dump(self._current_songdata, f)
			f.write("\n")
		self._db.add_scorekeeper_results(self._current_songdata["meta"]["player"], self._current_songdata["meta"]["songStartLocal"], self._current_score)
		self._db.mark_file_seen(destination_filename)
		self._current_songdata = None

	def _handle_beatsaber_event(self, event):
		if event["event"] == "songStart":
			self._current_score = ScoreKeeper(player_name = self._current_player, advanced = True)
			self._current_songdata = {
				"meta": {
					"songStartLocal":	time.time(),
					"player":			self._current_player,
				},
				"events":		[ ],
			}
			print("Player %s started %s - %s (%s)" % (self._current_player, event["status"]["beatmap"]["songAuthorName"], event["status"]["beatmap"]["songName"], event["status"]["beatmap"]["difficulty"]))

		if self._current_songdata is not None:
			self._current_songdata["events"].append(event)

		if self._current_score is not None:
			if self._current_score.process(event):
				self._local_server.change_event()

		if (self._current_songdata is not None) and ((event["event"] == "finished") or (event["event"] == "failed")):
			self._finish_song()
			self._current_score = None
			self._local_server.change_event()

	async def _connect_beatsaber(self):
		uri = self._config["beatsaber_websocket_uri"]
		while True:
			try:
				async with websockets.connect(uri) as websocket:
					print("Connection to BeatSaber established at %s" % (uri))
					self._connected_to_beatsaber = True
					while True:
						msg = await websocket.recv()
						msg = json.loads(msg)
						self._handle_beatsaber_event(msg)
			except (OSError, ConnectionRefusedError, websockets.exceptions.ConnectionClosed) as e:
				if self._connected_to_beatsaber:
					if self._current_songdata is not None:
						print("Disconnected from BeatSaber: %s - %s; warning: data of current song is lost." % (e.__class__.__name__, str(e)))
					else:
						print("Disconnected from BeatSaber: %s - %s" % (e.__class__.__name__, str(e)))
				self._current_songdata = None
				self._connected_to_beatsaber = False
				await asyncio.sleep(1)

	async def _connect_heartrate_monitor(self):
		socket = self._config["heartrate_monitor"]
		while True:
			await asyncio.sleep(1)

	def start(self):
		loop = asyncio.get_event_loop()
		loop.create_task(self._local_server.create_server())
		loop.create_task(self._connect_beatsaber())
		if self._config.has("heartrate_monitor"):
			loop.create_task(self._connect_heartrate_monitor())
		try:
			loop.run_forever()
		except KeyboardInterrupt:
			with contextlib.suppress(FileNotFoundError):
				os.unlink(self._config["unix_socket"])
