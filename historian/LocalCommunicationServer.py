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

import json
import asyncio

class CommunicationError(Exception): pass

class LocalCommunicationServer():
	def __init__(self, historian):
		self._historian = historian
		self._change_event = asyncio.Event()

	def _command_recentplayers(self, query = None):
		fixed_players = self._historian.config["permanent_players"]
		recent_players = self._historian.db.get_recent_players()
		players = list(fixed_players)
		playerset = set(players)
		for player in sorted(recent_players):
			if player not in playerset:
				playerset.add(player)
				players.append(player)
		return {
			"players":	players,
		}

	def _command_playerinfo(self, query):
		self._assert_prerequisite(("player" in query) and isinstance(query["player"], str), "'player' property not set or not of the correct type.")
		info = self._historian.db.get_player_info(query["player"])
		info["player"] = query["player"]
		return info

	def _command_status(self, query = None):
		return {
			"connection": {
				"connected_to_beatsaber":	self._historian.connected_to_beatsaber,
				"current_player":			self._historian.current_player,
			},
			"current_game":					self._historian.current_score.to_dict() if (self._historian.current_score is not None) else None,
		}

	def _command_set_player(self, query):
		self._assert_prerequisite(("player" in query) and isinstance(query["player"], (str, type(None))), "'player' property not set or not of the correct type.")
		self._historian.current_player = query["player"]

	def _assert_prerequisite(self, condition, error_msg):
		if not condition:
			raise CommunicationError(error_msg)

	def _process_local_command(self, query):
		self._assert_prerequisite(isinstance(query, dict), "Invalid data type provided, expected dict.")
		self._assert_prerequisite(("cmd" in query) and isinstance(query["cmd"], str), "No command given or command of wrong type.")
		cmd = query["cmd"]
		handler = getattr(self, "_command_%s" % (cmd), None)
		if handler is None:
			raise CommunicationError("No such command: \"%s\"" % (cmd))
		response = handler(query)
		if response is not None:
			response["msgtype"] = cmd
		return response

	def _process_local_raw_command(self, raw_query):
		query = json.loads(raw_query)
		return self._process_local_command(query)

	async def _respond(self, writer, response):
		writer.write((json.dumps(response) + "\n").encode("ascii"))

	async def _local_server_commands(self, reader, writer):
		try:
			while not writer.is_closing():
				msg = await reader.readline()
				if len(msg) == 0:
					break
				try:
					response = self._process_local_raw_command(msg)
				except (CommunicationError, json.decoder.JSONDecodeError) as e:
					response = {
						"msgtype":	"error",
						"text":		str(e),
					}
				if response is not None:
					await self._respond(writer, response)
		except (ConnectionResetError, BrokenPipeError) as e:
			print("Local UNIX server caught exception:", e)
			writer.close()

	def change_event(self):
		self._change_event.set()

	async def _local_server_events(self, reader, writer):
		self._change_event.set()
		while not writer.is_closing():
			await self._change_event.wait()
			self._change_event.clear()
			await self._respond(writer, self._process_local_command({ "cmd": "status" }))

	async def _local_server_tasks(self, reader, writer):
		await asyncio.gather(
			self._local_server_commands(reader, writer),
			self._local_server_events(reader, writer),
		)
		writer.close()

	async def create_server(self):
		await asyncio.start_unix_server(self._local_server_tasks, path = self._historian.config["unix_socket"])
