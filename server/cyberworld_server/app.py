"""One ephemeral, bounded presence square. Nothing is written to game saves."""

import asyncio
import contextlib
import dataclasses
import json
import logging
import struct
import time

from . import protocol, websocket

LOG = logging.getLogger("cyberworld.server")


@dataclasses.dataclass
class Player:
    id: int
    name: str
    writer: asyncio.StreamWriter
    position: tuple = (0, 0, 0, 0)
    last_log: float = 0


class Server:
    def __init__(self, seed=120, capacity=8, tick_hz=20, origins=(), event_log=None):
        if not 0 <= seed <= 0xffffffff or not 1 <= capacity <= protocol.SPEC["max_players"]:
            raise ValueError("seed must be uint32; capacity must be 1..8")
        if not 1 <= tick_hz <= 30:
            raise ValueError("tick_hz must be 1..30")
        self.seed, self.capacity, self.tick_hz = seed, capacity, tick_hz
        self.origins, self.event_log = origins, event_log
        self.players = {}
        self.next_id = 1
        self.connections = set()
        self.listener = self.ticker = None

    def event(self, kind, **fields):
        entry = dict(event=kind, **fields)
        LOG.info("%s", json.dumps(entry, separators=(",", ":")))
        if self.event_log:
            self.event_log.write(json.dumps(dict(time=time.time(), **entry)) + "\n")
            self.event_log.flush()

    async def start(self, host, port, ssl=None):
        self.listener = await asyncio.start_server(self.handle, host, port, ssl=ssl, limit=8192)
        self.ticker = asyncio.create_task(self.tick())
        self.event("listening", host=host, port=self.listener.sockets[0].getsockname()[1], seed=self.seed)
        return self.listener.sockets[0].getsockname()[1]

    async def close(self):
        if self.listener:
            self.listener.close()
        if self.ticker:
            self.ticker.cancel()
            with contextlib.suppress(asyncio.CancelledError):
                await self.ticker
        tasks = list(self.connections)
        for task in tasks:
            task.cancel()
        await asyncio.gather(*tasks, return_exceptions=True)
        # Recent asyncio versions wait for accepted connections here too.
        # Cancel them first so shutdown cannot wait for idle players.
        if self.listener:
            await self.listener.wait_closed()

    async def handle(self, reader, writer):
        task = asyncio.current_task()
        player = None
        upgraded = False
        if len(self.connections) >= 32:
            writer.close()
            return
        self.connections.add(task)
        try:
            await websocket.upgrade(reader, writer, self.origins)
            upgraded = True
            opcode, data = await asyncio.wait_for(websocket.receive(reader), 5)
            if opcode != 2:
                raise protocol.ProtocolError("Expected binary HELLO")
            name = protocol.read_hello(data)
            if len(self.players) >= self.capacity:
                raise protocol.ProtocolError("Square is full", 7)
            player = Player(self.next_id, name, writer)
            self.next_id = self.next_id % 0xffffffff + 1
            self.players[player.id] = player
            self.event("join", id=player.id, name=name, players=len(self.players))
            await websocket.send(writer, struct.pack("!BIIHB", protocol.SPEC["welcome"],
                                                    self.seed, player.id, self.tick_hz, self.capacity))
            await self.serve_player(reader, player)
        except protocol.ProtocolError as exc:
            self.event("reject", reason=str(exc), code=exc.code)
            with contextlib.suppress(ConnectionError, asyncio.TimeoutError):
                if upgraded:
                    await websocket.send(writer, protocol.error(exc))
                    await websocket.send(writer, struct.pack("!H", 1008), 8)
                else:
                    writer.write(b"HTTP/1.1 400 Bad Request\r\nConnection: close\r\nContent-Length: 0\r\n\r\n")
                    await asyncio.wait_for(writer.drain(), 2)
        except (ConnectionError, asyncio.IncompleteReadError, asyncio.TimeoutError):
            pass
        finally:
            if player:
                self.players.pop(player.id, None)
                self.event("leave", id=player.id, players=len(self.players))
            writer.close()
            with contextlib.suppress(ConnectionError, asyncio.TimeoutError):
                await asyncio.wait_for(writer.wait_closed(), 2)
            self.connections.discard(task)

    async def serve_player(self, reader, player):
        tokens, previous = 40.0, time.monotonic()
        while True:
            opcode, data = await asyncio.wait_for(websocket.receive(reader), 15)
            now = time.monotonic()
            tokens = min(40, tokens + (now - previous) * 30) - 1
            previous = now
            if tokens < 0:
                raise protocol.ProtocolError("Message rate exceeded", 8)
            if opcode == 8:
                await websocket.send(player.writer, struct.pack("!H", 1000), 8)
                return
            if opcode == 9:
                await websocket.send(player.writer, data, 10)
            elif opcode == 2:
                await self.message(player, data, now)

    async def message(self, player, data, now):
        if data and data[0] == protocol.SPEC["state"]:
            player.position = protocol.read_state(data)
            if now - player.last_log >= 1:
                player.last_log = now
                self.event("position", id=player.id, position=player.position)
        elif len(data) == 5 and data[0] == protocol.SPEC["ping"]:
            await websocket.send(player.writer, bytes((protocol.SPEC["pong"],)) + data[1:])
        else:
            raise protocol.ProtocolError("Unknown or out-of-order message")

    async def tick(self):
        while True:
            await asyncio.sleep(1 / self.tick_hz)
            for player in list(self.players.values()):
                if player.writer.is_closing():
                    continue
                if player.writer.transport.get_write_buffer_size() > 8192:
                    player.writer.close()
                    continue
                player.writer.write(websocket.frame(protocol.snapshot(self.players, player.id)))
