"""One ephemeral, bounded presence square. Nothing is written to game saves."""

import asyncio
import contextlib
import dataclasses
import json
import logging
import math
import struct
import time

from . import protocol, websocket
from .arena import Arena
from .trade import Trades
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
                 player_cap=64, state_dir=None, name="Cyberworld Endless", motd="Welcome to the Net Square", whitelist=(),
                 pending_cap=16, handshake_timeout=5.0, trading=False, arena=False):
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
        self.housekeeping = 1.0     # seconds between moderation checks and roster writes
        self.origins, self.event_log = origins, event_log
        self.players = {}
        self.next_id = 1
        self.connections = set()    # every accepted socket's task, for shutdown
        # Unfinished handshakes have a small cap and a deadline of their own:
        # idle sockets are dropped and never hold a player's place.
        self.pending = set()
        self.pending_cap, self.handshake_timeout = pending_cap, handshake_timeout
        self.roster_written = None
        self.roster_task = None
        self.roster_failed = False
        self.listener = self.ticker = None
        self.trading, self.arena_on = trading, arena
        self.trades, self.arena = Trades(self), Arena(self)

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
        if self.roster_task:
            await asyncio.gather(self.roster_task, return_exceptions=True)
        try:
            self.store.roster([])
        except (OSError, ValueError) as exc:
            LOG.error("Could not clear the local player roster: %s", exc)
        # Recent asyncio versions wait for accepted connections here too.
        # Cancel them first so shutdown cannot wait for idle players.
        if self.listener:
            await self.listener.wait_closed()

    async def handle(self, reader, writer):
        task = asyncio.current_task()
        player = None
        upgraded = False
        if len(self.pending) >= self.pending_cap:
            writer.close()
            return
        self.connections.add(task)
        self.pending.add(task)
        try:
            try:
                async with asyncio.timeout(self.handshake_timeout):
                    if not await websocket.upgrade(reader, writer, self.origins, self.health()):
                        return
                    upgraded = True
                    opcode, data = await websocket.receive(reader)
            finally:
                self.pending.discard(task)
            if opcode != 2:
                raise protocol.ProtocolError("Expected binary HELLO")
            player = self.join(writer, *protocol.read_hello(data))
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
                # A replaced session has already handed its place on.
                if player and self.players.get(player.id) is player:
                    del self.players[player.id]
                    self.trades.left(player.id)
                    self.arena.left(player.id)
                    self.event("leave", id=player.id, players=len(self.players))
            finally:
                writer.close()
                try:
                    with contextlib.suppress(ConnectionError, asyncio.TimeoutError):
                        await asyncio.wait_for(writer.wait_closed(), 2)
                finally:
                    self.connections.discard(task)

    def join(self, writer, name, key, requested):
        identity = self.store.identity(key)
        self.store.refresh()
        if self.store.contains("bans", identity):
            raise protocol.ProtocolError("This profile is banned", 10)
        if self.whitelist and identity.hex() not in self.whitelist:
            raise protocol.ProtocolError("This server uses an allowlist", 11)
        now = time.time()
        kick = self.store.kick(identity, now)
        if kick:
            minutes = max(1, math.ceil((kick.get("expires", now) - now) / 60))
            raise protocol.ProtocolError(f"Kicked by a moderator; try again in {minutes} min", 14)
        # The same profile again (a reconnect after a drop the server has not
        # noticed yet) takes over its old session, and by default its channel.
        old = next((p for p in self.players.values() if p.identity == identity), None)
        channel = self.choose_channel(requested or (old.channel if old else 0), old.id if old else 0)
        if not channel or len(self.players) - bool(old) >= self.player_cap:
            raise protocol.ProtocolError("Square or channel is full", 7)
        player = Player(self.next_id, name, writer, identity, channel)
        self.next_id = self.next_id % 0xffffffff + 1
        if old:
            del self.players[old.id]
            self.trades.left(old.id)
            self.arena.left(old.id)
            self.event("replace", id=old.id, by=player.id, identity=identity.hex())
            self.drop(old, protocol.ProtocolError("This profile connected again from another session", 12), 1000)
        self.players[player.id] = player
        self.event("join", id=player.id, identity=identity.hex(), name=name, channel=channel, players=len(self.players))
        return player

    def drop(self, player, exc, close_code=1008):
        """Tell a session why it ends and close it, without waiting on its socket."""
        if not player.writer.is_closing():
            player.writer.write(websocket.frame(protocol.error(exc)))
            reason = str(exc).encode("ascii")[:123]
            player.writer.write(websocket.frame(struct.pack("!H", close_code) + reason, 8))
        player.writer.close()

    async def serve_player(self, reader, player):
        # (the Arena's keys come 60 times a second: a bucket of their own)
        tokens, relay, previous = 40.0, 150.0, time.monotonic()
        relayed = (protocol.SPEC["arena_input"], protocol.SPEC["arena_hash"], protocol.SPEC["arena_loadout"])
        while True:
            opcode, data = await asyncio.wait_for(websocket.receive(reader), 15)
            now = time.monotonic()
            elapsed, previous = now - previous, now
            if opcode == 2 and data and data[0] in relayed:
                relay = min(150, relay + elapsed * 120) - 1
                tokens = min(40, tokens + elapsed * 30)
                if relay < 0:
                    raise protocol.ProtocolError("Message rate exceeded", 8)
                self.arena.relay(player, data)
                continue
            tokens = min(40, tokens + elapsed * 30) - 1
            relay = min(150, relay + elapsed * 120)
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
        elif data and data[0] == protocol.SPEC["trade"]:
            await self.trades.handle(player, data)
        elif data and data[0] == protocol.SPEC["arena"]:
            await self.arena.handle(player, data)
        else:
            raise protocol.ProtocolError("Unknown or out-of-order message")

    def health(self):
        return {"status": "ok", "protocol": protocol.SPEC["version"], "name": self.name,
                "motd": self.motd, "players": len(self.players), "player_cap": self.player_cap,
                "room_capacity": self.capacity, "channels": self.max_channels,
                "rules": {"free_text": False, "trading": self.trading, "arena": self.arena_on}}

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
        self.trades.left(player.id)
        self.arena.left(player.id)
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
        # Re-read only when moderation.json changed; a bad file keeps the last rules.
        self.store.refresh()
        now = time.time()
        for player in list(self.players.values()):
            kick = self.store.kick(player.identity, now)
            if self.store.contains("bans", player.identity) or (kick and kick["time"] >= player.joined):
                self.drop(player, protocol.ProtocolError("Removed by a moderator", 14))
        self.save_roster()

    def save_roster(self):
        """Write the local roster when it changed: in a thread, one write at a time."""
        roster = [{"id": p.id, "identity": p.identity.hex(), "name": p.name, "channel": p.channel}
                  for p in self.players.values()]
        if not self.store.directory or roster == self.roster_written:
            return
        if self.roster_task and not self.roster_task.done():
            return
        self.roster_task = asyncio.create_task(self.write_roster(roster))

    async def write_roster(self, roster):
        try:
            await asyncio.to_thread(self.store.roster, roster)
        except (OSError, ValueError) as exc:
            if not self.roster_failed:
                LOG.error("Could not write the local player roster: %s", exc)
            self.roster_failed = True
            return
        self.roster_written, self.roster_failed = roster, False

    async def tick(self):
        while True:
            await asyncio.sleep(1 / self.tick_hz)
            now = time.monotonic()
            if now - self.last_moderation >= self.housekeeping:
                self.last_moderation = now
                await self.moderation()
                self.trades.expire(now)
                self.arena.expire(now)
            for player in list(self.players.values()):
                if player.writer.is_closing():
                    continue
                if player.writer.transport.get_write_buffer_size() > 8192:
                    player.writer.close()
                    continue
                player.writer.write(websocket.frame(protocol.snapshot(self.players, player.id)))
