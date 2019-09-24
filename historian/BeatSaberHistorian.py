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

class BeatSaberHistorian():
	def __init__(self, config, args):
		self._config = config
		self._args = args
		self._current_player = None
		self._current_data = None

	def _subs(self, text):
		text = text.replace("${player}", self._current_player or "unknown_player")
		return self._config.subs(text)

	def _local_command_status(self, query):
		return {
			"msgtype":	"response",
			"success":	True,
			"data": {
				"current_player":		self._current_player,
			},
		}

	def _local_command_set(self, query):
		if ("current_player" in query) and (isinstance(query["current_player"], (type(None), str))):
			self._current_player = query["current_player"]
		return self._local_command_status(query)

	def _process_local_command(self, query):
		if not isinstance(query, dict):
			return {
				"success":	False,
				"data":		"Invalid data type provided, expected dict.",
			}

		cmd = query.get("cmd", "")
		handler = getattr(self, "_local_command_%s" % (cmd), None)
		if handler is not None:
			return handler(query)
		else:
			return {
				"msgtype":	"response",
				"success":	False,
				"data":		"No such command: \"%s\"" % (cmd),
			}

	async def _local_server_commands(self, reader, writer):
		try:
			while not writer.is_closing():
				msg = await reader.readline()
				if len(msg) == 0:
					break
				try:
					msg = json.loads(msg)
					response = self._process_local_command(msg)
					writer.write((json.dumps(response) + "\n").encode("ascii"))
				except json.decoder.JSONDecodeError as e:
					writer.write((json.dumps({
						"msgtype":	"response",
						"success":	False,
						"data":		"Could not decode command: %s" % (str(e)),
					}) + "\n").encode("ascii"))
		except (ConnectionResetError, BrokenPipeError) as e:
			print("Local UNIX server caught exception:", e)
			writer.close()

	async def _local_server_events(self, reader, writer):
		while not writer.is_closing():
			await asyncio.sleep(1)
			writer.write((json.dumps({
				"msgtype":	"event",
				"data":		"Some event data",
			}) + "\n").encode("ascii"))

	async def _local_server_tasks(self, reader, writer):
		await asyncio.gather(
			self._local_server_commands(reader, writer),
			self._local_server_events(reader, writer),
		)
		writer.close()

	async def _create_local_server(self):
		await asyncio.start_unix_server(self._local_server_tasks, path = self._subs(self._config["unix_socket"]))

	def _finish_song(self):
		destination_filename = self._subs(self._config["history_files"])
		with contextlib.suppress(FileExistsError):
			os.makedirs(os.path.dirname(destination_filename))
		with open(destination_filename, "w") as f:
			json.dump(self._current_data, f)
			f.write("\n")
		self._current_data = None

	def _handle_beatsaber_event(self, event):
		if event["event"] == "songStart":
			self._current_data = {
				"meta": {
					"songStartLocal":	time.time(),
					"player":			self._current_player,
				},
				"events":		[ ],
			}
			self._current_data["events"].append(event)
			print("Player %s started %s - %s (%s)" % (self._current_player, event["status"]["beatmap"]["songAuthorName"], event["status"]["beatmap"]["songName"], event["status"]["beatmap"]["difficulty"]))
		elif (self._current_data is not None) and ((event["event"] == "finished") or (event["event"] == "failed")):
			self._current_data["events"].append(event)
			self._finish_song()
		elif self._current_data is not None:
			self._current_data["events"].append(event)

	async def _connect_beatsaber(self):
		uri = self._config.subs(self._config["beatsaber_websocket_uri"])
		while True:
			try:
				async with websockets.connect(uri) as websocket:
					print("Connection to BeatSaber established at %s" % (uri))
					while True:
						msg = await websocket.recv()
						msg = json.loads(msg)
						self._handle_beatsaber_event(msg)
			except (OSError, ConnectionRefusedError, websockets.exceptions.ConnectionClosed):
				await asyncio.sleep(1)

	async def _feed_mock_data(self):
		await asyncio.sleep(3)
		with open("mock_events.txt") as f:
			for line in f:
				msg = json.loads(line)
				self._handle_beatsaber_event(msg)
#				await asyncio.sleep(0.01)

	def start(self):
		loop = asyncio.get_event_loop()
		loop.create_task(self._create_local_server())
		loop.create_task(self._connect_beatsaber())
#		loop.create_task(self._feed_mock_data())
		try:
			loop.run_forever()
		except KeyboardInterrupt:
			with contextlib.suppress(FileNotFoundError):
				os.unlink(self._subs(self._config["unix_socket"]))
