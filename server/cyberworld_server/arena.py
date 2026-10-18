"""The Arena: Practice NetBattles between two players of a channel.

Each game runs both players' linked BN6 cores itself; the server pairs
the two, chooses which is the cable's first side and the input delay from
their latencies, and relays their loadouts, keys by frame and state hashes
to each other. It never sees a battle: the games compare their hashes."""

import math
import time

from . import protocol, websocket

CHALLENGED, WAITING, START, ENDED = range(1, 5)
CHALLENGE, ACCEPT, LEAVE, OVER = range(1, 5)
PENDING = 30          # seconds a challenge waits for its answer
FRAME_MS = 1000 / 60


def delay_for(latency_a, latency_b):
    """Frames of input delay: three of the cores' own, then the one-way trip."""
    one_way = max(latency_a, latency_b) / 2
    return max(6, min(30, 3 + math.ceil(one_way / FRAME_MS) + 2))


class Match:
    def __init__(self, match_id, a, b, latency, rules=0):
        self.id, self.a, self.b, self.rules = match_id, a, b, rules
        self.latency = {a: latency, b: 0}
        self.started = False
        self.time = time.monotonic()

    def other(self, player_id):
        return self.b if player_id == self.a else self.a


class Arena:
    def __init__(self, server):
        self.server = server
        self.matches = {}
        self.of = {}
        self.next = 1

    def send(self, player_id, match, stage, side=0, delay=0):
        player = self.server.players.get(player_id)
        if not player or player.writer.is_closing():
            return
        other = self.server.players.get(match.other(player_id))
        identity = other.identity if other else bytes(16)
        player.writer.write(websocket.frame(protocol.arena_state(stage, match.id, match.other(player_id), identity, side, delay,
                                                                 match.rules)))

    def end(self, match, told=()):
        self.matches.pop(match.id, None)
        for player_id in (match.a, match.b):
            self.of.pop(player_id, None)
            if player_id not in told:
                self.send(player_id, match, ENDED)

    async def handle(self, player, data):
        action, ref, latency, rules = protocol.read_arena(data)
        if not self.server.arena_on:
            await self.server.notice(player, 18, "The Arena is closed on this server")
            return
        if action == CHALLENGE:
            await self.challenge(player, ref, latency, rules)
            return
        match = self.matches.get(ref)
        if not match or player.id not in (match.a, match.b):
            await self.server.notice(player, 17, "That battle is over")
            return
        if action == ACCEPT and player.id == match.b and not match.started:
            match.latency[player.id] = latency
            match.started = True
            delay = delay_for(match.latency[match.a], latency)
            self.send(match.a, match, START, 0, delay)
            self.send(match.b, match, START, 1, delay)
            self.server.event("arena", match=match.id, a=match.a, b=match.b, delay=delay)
        else:
            # (a leave, a decline, or the battle over on one side: both told)
            self.end(match)

    async def challenge(self, player, target_id, latency, rules=0):
        target = self.server.players.get(target_id)
        if not target or target.id == player.id or target.channel != player.channel:
            await self.server.notice(player, 17, "That Navi is no longer here")
            return
        if player.id in self.of or target.id in self.of or player.id in self.server.trades.of or target.id in self.server.trades.of:
            await self.server.notice(player, 16, "That Navi is busy")
            return
        # (the rules the challenge names: BN6's for any pair, full ones, BN5's
        # chips and Souls, only where both games have BN5's ROM)
        if rules == protocol.SPEC["format_full"] and not player.roms & target.roms & protocol.SPEC["roms_bn5"]:
            await self.server.notice(player, 19, "Those rules need BN5 on both games")
            return
        match = Match(self.next, player.id, target.id, latency, rules)
        self.next = self.next % 0xffffffff + 1
        self.matches[match.id] = match
        self.of[player.id] = self.of[target.id] = match.id
        self.send(player.id, match, WAITING)
        self.send(target.id, match, CHALLENGED)

    def relay(self, player, data):
        match = self.matches.get(protocol.arena_relay(data))
        if not match or not match.started or player.id not in (match.a, match.b):
            return
        other = self.server.players.get(match.other(player.id))
        if other and not other.writer.is_closing():
            other.writer.write(websocket.frame(data))

    def left(self, player_id):
        match = self.matches.get(self.of.get(player_id, 0))
        if match:
            self.end(match, told=(player_id,))

    def expire(self, now):
        for match in list(self.matches.values()):
            if not match.started and now - match.time > PENDING:
                self.end(match)
