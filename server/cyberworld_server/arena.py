"""The Arena: Practice NetBattles between two players of a channel.

Each game runs both players' linked BN6 cores itself; the server pairs
the two, chooses which is the cable's first side and the input delay from
their latencies, and relays their loadouts, keys by frame and state hashes
to each other. It never sees a battle: the games compare their hashes.
Every battle is by full rules, between two games with the full set
(protocol v9, protocol.full_set): a challenge from or to a game without
it, or an answer from one, is refused with NOTICE notice_pass.

A Team NetBattle (protocol v12, teams.py) is a relay of these battles: a
challenge between players of two teams is a team challenge, whose Yes
begins the team match there; each of its bouts is a Match here, called by
teams.py, which both its fighters accept (ready, their last screen
closed) before START. A team player's challenge to a lone one, or to his
own partner, is refused with NOTICE notice_team."""

import math
import time

from . import protocol, websocket

CHALLENGED, WAITING, START, ENDED = range(1, 5)
CHALLENGE, ACCEPT, LEAVE, OVER = range(1, 5)
PENDING = 30          # seconds a challenge waits for its answer
FRAME_MS = 1000 / 60
LOCKSTEP = 1          # frames the linked cores need: one crosses into its next a little before its partner (v8; 3 before)
LEAST = LOCKSTEP      # (2 before issue #167: a tick for a key to be taken, which a rollback window covers as well)
MOST = 30


def delay_for(latency_a, latency_b):
    """Frames of input delay fitted to the pair (issue #128): the lockstep's
    frame, then the longer of the two latencies the games report: each its
    round trip with its jitter, less what its rollback window covers but a
    frame (protocol v8; docs/ONLINE_ARENA.md, Smoothed delay and Rollback).
    A key goes through the server, half a round trip of each game's; the
    games' time sync holds each one's keys' lag at its own round trip, so
    the delay covers the longer of the two, and a game that rolls back W
    frames covers W - 1 of it itself and the tick a key waits to be taken
    (issue #167: before, the server added that tick for every pair, so the
    least was 2 where a window covered the whole trip; a game without a
    window says a frame more for it)."""
    trip = max(latency_a, latency_b)
    return max(LEAST, min(MOST, LOCKSTEP + math.ceil(trip / FRAME_MS)))


class Match:
    def __init__(self, match_id, a, b, latency, team=0, teamed=False):
        self.id, self.a, self.b = match_id, a, b
        self.latency = {a: latency, b: 0}
        self.started = False
        self.finished = False   # over, from the cable's first (not a leave)
        self.team = team        # a team match's bout: its id (teams.py), both fighters' accepts before START
        self.teamed = teamed    # a team challenge: its Yes begins a team match, not a battle
        self.ready = set()
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
        player.writer.write(websocket.frame(protocol.arena_state(stage, match.id, match.other(player_id), identity, side, delay)))

    def end(self, match, told=()):
        if self.matches.pop(match.id, None) is None:
            return
        self.server.stands.end(match)
        for player_id in (match.a, match.b):
            self.of.pop(player_id, None)
            if player_id not in told:
                self.send(player_id, match, ENDED)
        if match.team:
            self.server.teams.bout_over(match, match.finished)

    def start(self, match):
        match.started = True
        delay = delay_for(match.latency[match.a], match.latency[match.b])
        self.send(match.a, match, START, 0, delay)
        self.send(match.b, match, START, 1, delay)
        self.server.event("arena", match=match.id, a=match.a, b=match.b, delay=delay, team=match.team)
        self.server.daily.count("battles_started")
        self.server.stands.begin(match, delay)
        if match.team:
            self.server.teams.fight(match)

    def bout(self, a, b, team_match):
        """A team match's bout (teams.py): a Match its two fighters accept when ready."""
        match = Match(self.next, a, b, 0, team=team_match.id)
        self.next = self.next % 0xffffffff + 1
        self.matches[match.id] = match
        self.of[a] = self.of[b] = match.id
        return match

    async def handle(self, player, data):
        action, ref, latency = protocol.read_arena(data)
        if not self.server.arena_on:
            await self.server.notice(player, 18, "The Arena is closed on this server")
            return
        # (a leave or an over only ends a battle: never refused)
        if action in (CHALLENGE, ACCEPT) and await self.server.without_pass(player):
            return
        if action == CHALLENGE:
            await self.challenge(player, ref, latency)
            return
        match = self.matches.get(ref)
        if not match or player.id not in (match.a, match.b):
            await self.server.notice(player, 17, "That battle is over")
            return
        if action == ACCEPT and match.team and not match.started:
            # (a bout: each fighter ready, the other's START waits for both)
            match.latency[player.id] = latency
            match.ready.add(player.id)
            if len(match.ready) == 2:
                self.start(match)
        elif action == ACCEPT and player.id == match.b and not match.started and match.teamed:
            # (a team challenge taken: the team match begins, teams.py; this Match was only its ask)
            if not self.server.teams.partners_free(match.a, match.b):
                self.end(match)
                return
            self.end(match, told=(match.a, match.b))
            self.server.teams.begin(match.a, match.b)
        elif action == ACCEPT and player.id == match.b and not match.started:
            match.latency[player.id] = latency
            self.start(match)
        else:
            # (a leave, a decline, or the battle over on one side: both told)
            if action == OVER and match.started:
                self.server.daily.count("battles_finished")
                match.finished = True
            self.end(match)

    async def challenge(self, player, target_id, latency):
        target = self.server.players.get(target_id)
        if not target or target.id == player.id or target.channel != player.channel:
            await self.server.notice(player, 17, "That Navi is no longer here")
            return
        if await self.server.without_pass(player, target):
            return
        teams = self.server.teams
        teamed = teams.challenge(player.id, target.id)
        if player.id in self.of or target.id in self.of or player.id in self.server.trades.of or target.id in self.server.trades.of \
                or self.server.stands.busy(player.id) or self.server.stands.busy(target.id) or self.server.holds.held(target) \
                or teams.busy(player.id) or teams.busy(target.id):
            await self.server.notice(player, 16, "That Navi is busy")
            return
        if teamed is None:
            await self.server.notice(player, protocol.SPEC["notice_team"], "A Team NetBattle takes two teams")
            return
        if teamed and not teams.partners_free(player.id, target.id):
            await self.server.notice(player, 16, "That Navi is busy")
            return
        match = Match(self.next, player.id, target.id, latency, teamed=teamed)
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
        self.server.stands.record(match, player.id, data)

    def left(self, player_id):
        match = self.matches.get(self.of.get(player_id, 0))
        if match:
            self.end(match, told=(player_id,))

    def expire(self, now):
        for match in list(self.matches.values()):
            if not match.started and now - match.time > PENDING:
                self.end(match)
