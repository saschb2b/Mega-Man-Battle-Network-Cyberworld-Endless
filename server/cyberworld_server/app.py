"""One ephemeral, bounded presence square. Nothing is written to game saves."""

import asyncio
import contextlib
import dataclasses
import json
import logging
import struct
import time

from . import protocol, websocket
from .identity import IdentityStore

LOG = logging.getLogger("cyberworld.server")


@dataclasses.dataclass
class Player:
    id: int
    name: str
    writer: asyncio.StreamWriter
    identity: bytes = bytes(16)
    channel: int = 1
    position: tuple = (0, 0, 0, 0)
    last_log: float = 0
    joined: float = dataclasses.field(default_factory=time.time)
    profile: bytes = protocol.DEFAULT_PROFILE
    social_tokens: float = 2
    social_time: float = dataclasses.field(default_factory=time.monotonic)
    profile_time: float = 0


class Server:
    def __init__(self, seed=120, capacity=8, tick_hz=20, origins=(), event_log=None,
                 player_cap=64, state_dir=None, name="Cyberworld Endless", motd="Welcome to the Net Square", whitelist=()):
        if not 0 <= seed <= 0xffffffff or not 1 <= capacity <= protocol.SPEC["max_players"]:
            raise ValueError("seed must be uint32; capacity must be 1..8")
        if not 1 <= tick_hz <= 30:
            raise ValueError("tick_hz must be 1..30")
        if not capacity <= player_cap <= 256:
            raise ValueError("player_cap must be room capacity..256")
        self.seed, self.capacity, self.tick_hz = seed, capacity, tick_hz
        self.player_cap = player_cap
        self.max_channels = (player_cap + capacity - 1) // capacity
        self.name, self.motd = name[:64], motd[:160]
        self.store = IdentityStore(state_dir)
        self.whitelist = set(whitelist)
        self.last_moderation = 0
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
        self.store.roster({})
        # Recent asyncio versions wait for accepted connections here too.
        # Cancel them first so shutdown cannot wait for idle players.
        if self.listener:
            await self.listener.wait_closed()

    async def handle(self, reader, writer):
        task = asyncio.current_task()
        player = None
        upgraded = False
        if len(self.connections) >= self.player_cap + 32:
            writer.close()
            return
        self.connections.add(task)
        try:
            selected = await websocket.upgrade(reader, writer, self.origins, self.health())
            if not selected:
                return
            upgraded = True
            opcode, data = await asyncio.wait_for(websocket.receive(reader), 5)
            if opcode != 2:
                raise protocol.ProtocolError("Expected binary HELLO")
            name, key, requested = protocol.read_hello(data)
            identity = self.store.identity(key)
            self.store.reload()
            if self.store.contains("bans", identity):
                raise protocol.ProtocolError("This profile is banned", 10)
            if self.whitelist and identity.hex() not in self.whitelist:
                raise protocol.ProtocolError("This server uses an allowlist", 11)
            if any(p.identity == identity for p in self.players.values()):
                raise protocol.ProtocolError("This profile is already connected", 12)
            channel = self.choose_channel(requested)
            if not channel or len(self.players) >= self.player_cap:
                raise protocol.ProtocolError("Square or channel is full", 7)
            player = Player(self.next_id, name, writer, identity, channel)
            self.next_id = self.next_id % 0xffffffff + 1
            self.players[player.id] = player
            self.event("join", id=player.id, identity=identity.hex(), name=name, channel=channel, players=len(self.players))
            await self.welcome(player)
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
            try:
                if player:
                    self.players.pop(player.id, None)
                    self.event("leave", id=player.id, players=len(self.players))
            finally:
                writer.close()
                try:
                    with contextlib.suppress(ConnectionError, asyncio.TimeoutError):
                        await asyncio.wait_for(writer.wait_closed(), 2)
                finally:
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
            if self.event_log and now - player.last_log >= 1:
                player.last_log = now
                self.event("position", id=player.id, position=player.position)
        elif len(data) == 5 and data[0] == protocol.SPEC["ping"]:
            await websocket.send(player.writer, bytes((protocol.SPEC["pong"],)) + data[1:])
        elif data and data[0] == protocol.SPEC["profile"]:
            profile = protocol.read_profile(data)
            if now - player.profile_time >= 1:
                player.profile, player.profile_time = profile, now
        elif len(data) == 5 and data[0] == protocol.SPEC["inspect"]:
            await self.inspect(player, struct.unpack("!I", data[1:])[0])
        elif data and data[0] == protocol.SPEC["social"]:
            await self.social(player, data, now)
        elif len(data) == 3 and data[0] == protocol.SPEC["channel"]:
            await self.switch_channel(player, struct.unpack("!H", data[1:])[0])
        else:
            raise protocol.ProtocolError("Unknown or out-of-order message")

    def health(self):
        return {"status": "ok", "protocol": protocol.SPEC["version"], "name": self.name,
                "motd": self.motd, "players": len(self.players), "player_cap": self.player_cap,
                "room_capacity": self.capacity, "channels": self.max_channels,
                "rules": {"free_text": False, "trading": False, "arena": False}}

    def choose_channel(self, requested, exclude=0):
        if requested > self.max_channels:
            return 0
        for channel in ([requested] if requested else range(1, self.max_channels + 1)):
            count = sum(p.channel == channel and p.id != exclude for p in self.players.values())
            if count < self.capacity:
                return channel
        return 0

    async def welcome(self, player):
        await websocket.send(player.writer, protocol.WELCOME.pack(protocol.SPEC["welcome"], self.seed,
                             player.id, self.tick_hz, self.capacity, player.channel, player.identity, self.max_channels))

    async def notice(self, player, code, text):
        await websocket.send(player.writer, struct.pack("!BH", protocol.SPEC["notice"], code) + text.encode("ascii")[:96])

    async def switch_channel(self, player, requested):
        channel = self.choose_channel(requested, player.id)
        if not channel:
            await self.notice(player, 10, "Channel is full or unavailable")
            return
        player.channel = channel
        await self.welcome(player)
        self.event("channel", id=player.id, channel=channel)

    async def inspect(self, player, target):
        # Every accepted request needs a reply: reopening a card otherwise
        # waits forever for a serial that a silent per-card throttle dropped.
        # serve_player already bounds all messages with its token bucket.
        peer = self.players.get(target)
        if not peer or peer.channel != player.channel:
            await self.notice(player, 13, "Player is no longer in this channel")
            return
        await websocket.send(player.writer, struct.pack("!BI16s", protocol.SPEC["card"], peer.id, peer.identity) + peer.profile)

    async def social(self, player, data, now):
        kind, choice = protocol.read_social(data)
        if self.store.contains("mutes", player.identity):
            await self.notice(player, 11, "This profile is muted")
            return
        player.social_tokens = min(2, player.social_tokens + (now - player.social_time) / 2)
        player.social_time = now
        if player.social_tokens < 1:
            await self.notice(player, 12, "Please wait before sending another phrase or emote")
            return
        player.social_tokens -= 1
        event = struct.pack("!BIBB", protocol.SPEC["event"], player.id, kind, choice)
        for peer in list(self.players.values()):
            if peer.channel == player.channel and not peer.writer.is_closing():
                peer.writer.write(websocket.frame(event))

    async def moderation(self):
        try:
            self.store.reload()
        except (ValueError, OSError) as exc:
            LOG.error("Moderation reload failed; keeping last rules: %s", exc)
            return
        for player in list(self.players.values()):
            banned = self.store.contains("bans", player.identity)
            if banned or self.store.kicked(player.identity, player.joined):
                with contextlib.suppress(ConnectionError, asyncio.TimeoutError):
                    await websocket.send(player.writer, protocol.error(protocol.ProtocolError("Removed by a moderator", 14)))
                player.writer.close()
        try:
            self.store.roster(self.players)
        except OSError as exc:
            LOG.error("Could not write local player roster: %s", exc)

    async def tick(self):
        while True:
            await asyncio.sleep(1 / self.tick_hz)
            now = time.monotonic()
            if now - self.last_moderation >= 1:
                self.last_moderation = now
                await self.moderation()
            for player in list(self.players.values()):
                if player.writer.is_closing():
                    continue
                if player.writer.transport.get_write_buffer_size() > 8192:
                    player.writer.close()
                    continue
                player.writer.write(websocket.frame(protocol.snapshot(self.players, player.id)))
