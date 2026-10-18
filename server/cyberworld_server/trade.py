"""Trade Central: one chip for one chip between two players in a channel.

A trade is proposed with the proposer's chip, answered with the other's,
and committed only when both have accepted the same pair as they saw it.
The server commits and records it; each game applies its side after its
visit returns. Nothing here reads or writes a save: chips are numbers.

The board: each player may have one offer up in his channel, the chip he
has and the kind he wants (a number the games name; the server never
reads a chip's element), there while he is in the channel and until he
trades. Taking one starts a trade with both chips named, the poster's
the one on the board; his game checks the want before its Deal. A
session trades once: its game keeps one swap until it is home."""

import asyncio
import json
import os
import time

from . import protocol, websocket

OFFERED, WAITING, ANSWERED, ACCEPTED, DONE, CANCELLED = range(1, 7)
PROPOSE, ANSWER, ACCEPT, CANCEL, TAKE = range(1, 6)
OFFER_WAIT = 1.0      # seconds between a player's posts on the board
TIMEOUT = 300         # seconds a trade may rest without progress: a walk to the desk and a chip picked
NONE = (0, 0)


class Trade:
    def __init__(self, trade_id, a, b, chip):
        self.id, self.a, self.b = trade_id, a, b
        self.chips = {a: chip, b: None}
        self.accepted = set()
        self.time = time.monotonic()

    def other(self, player_id):
        return self.b if player_id == self.a else self.a


class Trades:
    def __init__(self, server):
        self.server = server
        self.open = {}         # trade id -> Trade
        self.of = {}           # player id -> trade id
        self.next = 1
        self.log_task = None
        self.offers = {}       # player id -> (chip, want): the board's
        self.done = set()      # players whose session has traded

    def send(self, player_id, trade, stage):
        player = self.server.players.get(player_id)
        if not player or player.writer.is_closing():
            return
        other = self.server.players.get(trade.other(player_id))
        identity = other.identity if other else bytes(16)
        mine, theirs = trade.chips.get(player_id) or NONE, trade.chips.get(trade.other(player_id)) or NONE
        player.writer.write(websocket.frame(protocol.trade_state(stage, trade.id, trade.other(player_id), identity, mine, theirs)))

    def end(self, trade, stage=CANCELLED):
        self.open.pop(trade.id, None)
        for player_id in (trade.a, trade.b):
            self.of.pop(player_id, None)
            self.send(player_id, trade, stage)

    async def handle(self, player, data):
        action, ref, give, get = protocol.read_trade(data)
        if not self.server.trading:
            await self.server.notice(player, 15, "Trading is off on this server")
            return
        if action == PROPOSE:
            await self.propose(player, ref, give)
            return
        if action == TAKE:
            await self.take(player, ref, give, get)
            return
        trade = self.open.get(ref)
        if not trade or player.id not in (trade.a, trade.b):
            await self.server.notice(player, 17, "That trade is over")
            return
        trade.time = time.monotonic()
        if action == CANCEL:
            self.end(trade)
        elif action == ANSWER and not self.held_by_both(trade, give):
            await self.server.notice(player, 19, "That chip needs BN5 on both games")
            self.end(trade)
        elif action == ANSWER and player.id == trade.b and trade.chips[trade.b] is None and get == trade.chips[trade.a]:
            trade.chips[trade.b] = give
            for player_id in (trade.a, trade.b):
                self.send(player_id, trade, ANSWERED)
        elif action == ACCEPT and None not in trade.chips.values() and \
                give == trade.chips[player.id] and get == trade.chips[trade.other(player.id)]:
            trade.accepted.add(player.id)
            if len(trade.accepted) == 2:
                self.commit(trade)
            else:
                self.send(player.id, trade, ACCEPTED)
        else:
            # (a stale or mismatched step ends the trade: neither side swaps)
            self.end(trade)

    async def propose(self, player, target_id, chip):
        target = self.server.players.get(target_id)
        if not target or target.id == player.id or target.channel != player.channel:
            await self.server.notice(player, 17, "That Navi is no longer here")
            return
        if not await self.free(player, target, chip):
            return
        trade = self.begin(player, target, chip)
        self.send(player.id, trade, WAITING)
        self.send(target.id, trade, OFFERED)

    async def free(self, player, target, *chips):
        """Whether the two may trade these chips now: neither busy nor traded, the chips their common ground."""
        if player.id in self.of or target.id in self.of or player.id in self.server.arena.of or target.id in self.server.arena.of:
            await self.server.notice(player, 16, "That Navi is busy")
            return False
        if player.id in self.done or target.id in self.done:
            await self.server.notice(player, 21, "One trade a visit; it goes home first")
            return False
        if any(protocol.chip_bn5(chip[0]) for chip in chips) and not player.roms & target.roms & protocol.SPEC["roms_bn5"]:
            await self.server.notice(player, 19, "That chip needs BN5 on both games")
            return False
        return True

    def begin(self, player, target, chip):
        trade = Trade(self.next, player.id, target.id, chip)
        self.next = self.next % 0xffffffff + 1
        self.open[trade.id] = trade
        self.of[player.id] = self.of[target.id] = trade.id
        return trade

    async def take(self, player, poster_id, give, get):
        """An offer on the board answered: a trade with both chips named, the poster's as posted."""
        poster, offer = self.server.players.get(poster_id), self.offers.get(poster_id)
        if not poster or poster.id == player.id or poster.channel != player.channel or not offer or offer[0] != get:
            await self.server.notice(player, 20, "That offer is gone")
            return
        if not await self.free(player, poster, give, get):
            return
        trade = self.begin(player, poster, give)
        trade.chips[poster.id] = get
        for player_id in (player.id, poster.id):
            self.send(player_id, trade, ANSWERED)

    async def offer(self, player, data):
        chip, want = protocol.read_offer(data)
        if not self.server.trading:
            await self.server.notice(player, 15, "Trading is off on this server")
            return
        now = time.monotonic()
        if now - player.offer_time < OFFER_WAIT:
            await self.server.notice(player, 22, "Please wait before posting again")
            return
        player.offer_time = now
        if not chip[0]:
            if self.offers.pop(player.id, None):
                self.board(player.channel)
            return
        if player.id in self.done:
            await self.server.notice(player, 21, "One trade a visit; it goes home first")
            return
        if protocol.chip_bn5(chip[0]) and not player.roms & protocol.SPEC["roms_bn5"]:
            await self.server.notice(player, 19, "That chip needs BN5 on both games")
            return
        self.offers[player.id] = (chip, want)
        self.board(player.channel)

    def board_for(self, player):
        """The offers of his channel he may see: BN5's chips only where his game has BN5."""
        players, bn5 = self.server.players, player.roms & protocol.SPEC["roms_bn5"]
        return [(pid, chip, want) for pid, (chip, want) in sorted(self.offers.items())
                if pid in players and players[pid].channel == player.channel and (bn5 or not protocol.chip_bn5(chip[0]))]

    def board(self, channel):
        for player in list(self.server.players.values()):
            if player.channel == channel and not player.writer.is_closing():
                player.writer.write(websocket.frame(protocol.board(self.board_for(player))))

    def withdraw(self, player_ids, channel):
        """Offers taken down (a trade done, a player gone): the channel told where one was up."""
        if [self.offers.pop(player_id) for player_id in player_ids if player_id in self.offers]:
            self.board(channel)

    def held_by_both(self, trade, chip):
        """BN6's own chips trade between any two games, BN5's only between two with its ROM."""
        players = [self.server.players.get(p) for p in (trade.a, trade.b)]
        return not protocol.chip_bn5(chip[0]) or all(p and p.roms & protocol.SPEC["roms_bn5"] for p in players)

    def commit(self, trade):
        players = self.server.players
        entry = {"time": time.time(), "trade": trade.id,
                 "a": players[trade.a].identity.hex() if trade.a in players else "",
                 "b": players[trade.b].identity.hex() if trade.b in players else "",
                 "a_gave": list(trade.chips[trade.a]), "b_gave": list(trade.chips[trade.b])}
        self.server.event("trade", trade=trade.id, a=trade.a, b=trade.b)
        self.end(trade, DONE)
        self.done.update((trade.a, trade.b))
        if trade.a in players:
            self.withdraw((trade.a, trade.b), players[trade.a].channel)
        if self.server.store.directory:
            self.log_task = asyncio.create_task(asyncio.to_thread(self.write_log, entry))

    def write_log(self, entry):
        path = os.path.join(self.server.store.directory, "trades.jsonl")
        with open(path, "a", encoding="utf-8") as log:
            log.write(json.dumps(entry, separators=(",", ":")) + "\n")

    def left(self, player):
        """A player gone from his channel (left the server, replaced, or changed channel)."""
        trade = self.open.get(self.of.get(player.id, 0))
        if trade:
            self.end(trade)
        self.withdraw((player.id,), player.channel)

    def expire(self, now):
        for trade in list(self.open.values()):
            if now - trade.time > TIMEOUT:
                self.end(trade)
