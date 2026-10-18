"""Trade Central: one chip for one chip between two players in a channel.

A trade is proposed with the proposer's chip, answered with the other's,
and committed only when both have accepted the same pair as they saw it.
The server commits and records it; each game applies its side after its
visit returns. Nothing here reads or writes a save: chips are numbers."""

import asyncio
import json
import os
import time

from . import protocol, websocket

OFFERED, WAITING, ANSWERED, ACCEPTED, DONE, CANCELLED = range(1, 7)
PROPOSE, ANSWER, ACCEPT, CANCEL = range(1, 5)
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
        trade = self.open.get(ref)
        if not trade or player.id not in (trade.a, trade.b):
            await self.server.notice(player, 17, "That trade is over")
            return
        trade.time = time.monotonic()
        if action == CANCEL:
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
        if player.id in self.of or target.id in self.of or player.id in self.server.arena.of or target.id in self.server.arena.of:
            await self.server.notice(player, 16, "That Navi is busy")
            return
        trade = Trade(self.next, player.id, target.id, chip)
        self.next = self.next % 0xffffffff + 1
        self.open[trade.id] = trade
        self.of[player.id] = self.of[target.id] = trade.id
        self.send(player.id, trade, WAITING)
        self.send(target.id, trade, OFFERED)

    def commit(self, trade):
        players = self.server.players
        entry = {"time": time.time(), "trade": trade.id,
                 "a": players[trade.a].identity.hex() if trade.a in players else "",
                 "b": players[trade.b].identity.hex() if trade.b in players else "",
                 "a_gave": list(trade.chips[trade.a]), "b_gave": list(trade.chips[trade.b])}
        self.server.event("trade", trade=trade.id, a=trade.a, b=trade.b)
        self.end(trade, DONE)
        if self.server.store.directory:
            self.log_task = asyncio.create_task(asyncio.to_thread(self.write_log, entry))

    def write_log(self, entry):
        path = os.path.join(self.server.store.directory, "trades.jsonl")
        with open(path, "a", encoding="utf-8") as log:
            log.write(json.dumps(entry, separators=(",", ":")) + "\n")

    def left(self, player_id):
        trade = self.open.get(self.of.get(player_id, 0))
        if trade:
            self.end(trade)

    def expire(self, now):
        for trade in list(self.open.values()):
            if now - trade.time > TIMEOUT:
                self.end(trade)
