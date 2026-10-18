"""Trade Central and the Arena over real sockets; no ROM needed."""

import asyncio
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "server"))
sys.path.insert(0, str(Path(__file__).resolve().parent))

from cyberworld_server import protocol
from cyberworld_server.app import Server
from cyberworld_server.arena import delay_for
from test_server import Client

SPEC = protocol.SPEC


def trade(action, ref, give=(0, 0), get=(0, 0)):
    return protocol.TRADE.pack(SPEC["trade"], action, ref, give[0], give[1], get[0], get[1])


def offer(chip=(0, 0), want=0):
    return protocol.OFFER.pack(SPEC["offer"], chip[0], chip[1], want)


def arena(action, ref, latency=0, rules=0):
    return protocol.ARENA.pack(SPEC["arena"], action, ref, latency, rules)


class PlayTests(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        self.dir = tempfile.TemporaryDirectory()
        self.server = Server(seed=7, capacity=8, player_cap=16, state_dir=self.dir.name, trading=True, arena=True)
        self.port = await self.server.start("127.0.0.1", 0)
        self.clients = []

    async def asyncTearDown(self):
        for client in self.clients:
            await client.close()
        await self.server.close()
        self.dir.cleanup()

    async def connect(self, name, bn5=False):
        client = Client()
        self.clients.append(client)
        welcome = await client.open(self.port, name, bn5=bn5)
        return client, struct.unpack("!I", welcome[5:9])[0]

    async def trade_state(self, client):
        return protocol.TRADE_STATE.unpack(await client.message(SPEC["trade_state"]))

    async def board(self, client, count=None):
        """The next BOARD (of `count` offers, where given) as (session, chip, code, want) tuples."""
        for _ in range(20):
            data = await client.message(SPEC["board"])
            size = protocol.BOARD_ENTRY.size
            self.assertEqual(len(data), 2 + data[1] * size)
            entries = [protocol.BOARD_ENTRY.unpack_from(data, 2 + k * size) for k in range(data[1])]
            if count is None or len(entries) == count:
                return entries
        raise AssertionError("No such board")

    async def notice(self, client):
        return struct.unpack("!H", (await client.message(SPEC["notice"]))[1:3])[0]

    async def arena_state(self, client):
        return protocol.ARENA_STATE.unpack(await client.message(SPEC["arena_state"]))

    async def test_trade_commits_only_the_pair_both_saw(self):
        a, a_id = await self.connect("Alpha")
        b, b_id = await self.connect("Beta")
        await a.send(trade(1, b_id, (5, 2)))
        _, stage, trade_id, other, _, *mine_theirs = await self.trade_state(a)
        self.assertEqual((stage, other, mine_theirs), (2, b_id, [5, 2, 0, 0]))
        _, stage, offered_id, other, _, *mine_theirs = await self.trade_state(b)
        self.assertEqual((stage, offered_id, other, mine_theirs), (1, trade_id, a_id, [0, 0, 5, 2]))
        await b.send(trade(2, trade_id, (10, 0), (5, 2)))
        self.assertEqual((await self.trade_state(a))[1], 3)
        self.assertEqual((await self.trade_state(b))[1], 3)
        await a.send(trade(3, trade_id, (5, 2), (10, 0)))
        self.assertEqual((await self.trade_state(a))[1], 4)
        await b.send(trade(3, trade_id, (10, 0), (5, 2)))
        done_a, done_b = await self.trade_state(a), await self.trade_state(b)
        self.assertEqual((done_a[1], done_a[5:]), (5, (5, 2, 10, 0)))
        self.assertEqual((done_b[1], done_b[5:]), (5, (10, 0, 5, 2)))
        await asyncio.sleep(0.2)
        entry = json.loads((Path(self.dir.name) / "trades.jsonl").read_text().splitlines()[-1])
        self.assertEqual((entry["a_gave"], entry["b_gave"]), ([5, 2], [10, 0]))

    async def test_a_mismatched_accept_cancels_for_both(self):
        a, _ = await self.connect("Alpha")
        b, b_id = await self.connect("Beta")
        await a.send(trade(1, b_id, (5, 2)))
        trade_id = (await self.trade_state(a))[2]
        await self.trade_state(b)
        await b.send(trade(2, trade_id, (10, 0), (5, 2)))
        await self.trade_state(a)
        await self.trade_state(b)
        await a.send(trade(3, trade_id, (5, 2), (11, 0)))
        self.assertEqual((await self.trade_state(a))[1], 6)
        self.assertEqual((await self.trade_state(b))[1], 6)
        self.assertFalse(self.server.trades.open)

    async def test_bn5_chips_and_leaving_players_never_trade(self):
        a, _ = await self.connect("Alpha")
        b, b_id = await self.connect("Beta")
        await a.send(trade(1, b_id, (SPEC["chip_last"] + 1, 0)))
        self.assertEqual((await a.message(SPEC["error"]))[:3], struct.pack("!BH", SPEC["error"], 1))
        c, _ = await self.connect("Gamma")
        await c.send(trade(1, b_id, (5, 2)))
        trade_id = (await self.trade_state(c))[2]
        await self.trade_state(b)
        await b.close()
        self.clients.remove(b)
        self.assertEqual((await self.trade_state(c))[1:3], (6, trade_id))

    async def test_trading_off_is_a_notice(self):
        self.server.trading = False
        a, _ = await self.connect("Alpha")
        _, b_id = await self.connect("Beta")
        await a.send(trade(1, b_id, (5, 2)))
        self.assertEqual((await a.message(SPEC["notice"]))[:3], struct.pack("!BH", SPEC["notice"], 15))

    async def test_arena_pairs_sides_delay_and_relays_only_to_the_opponent(self):
        a, a_id = await self.connect("Alpha")
        b, b_id = await self.connect("Beta")
        c, _ = await self.connect("Gamma")
        await a.send(arena(1, b_id, 40))
        _, stage, match_id, other, _, _, _, rules = await self.arena_state(a)
        self.assertEqual((stage, other, rules), (2, b_id, SPEC["format_bn6"]))
        self.assertEqual((await self.arena_state(b))[1:4], (1, match_id, a_id))
        await b.send(arena(2, match_id, 100))
        start_a, start_b = await self.arena_state(a), await self.arena_state(b)
        self.assertEqual((start_a[1], start_a[5], start_a[6]), (3, 0, delay_for(40, 100)))
        self.assertEqual((start_b[1], start_b[5], start_b[6]), (3, 1, delay_for(40, 100)))
        for frame in range(120):
            keys = protocol.ARENA_INPUT.pack(SPEC["arena_input"], match_id, frame * 2, 2) + struct.pack("!HH", frame, frame + 1)
            await a.send(keys)
        received = 0
        while received < 120:
            message = await b.message(SPEC["arena_input"])
            self.assertEqual(protocol.ARENA_INPUT.unpack_from(message)[2], received * 2)
            received += 1
        hashed = protocol.ARENA_HASH.pack(SPEC["arena_hash"], match_id, 59, 0x1234)
        await b.send(hashed)
        self.assertEqual(await a.message(SPEC["arena_hash"]), hashed)
        await a.send(arena(3, match_id))
        self.assertEqual((await self.arena_state(b))[1], 4)
        self.assertFalse(self.server.arena.matches)
        await c.send(hashed)    # (no match of its own: dropped, not an error)
        await asyncio.sleep(0.1)
        self.assertIn(c, self.clients)

    async def test_bn5_chips_trade_only_between_two_games_with_bn5(self):
        a, a_id = await self.connect("Alpha", bn5=True)
        b, b_id = await self.connect("Beta")
        c, c_id = await self.connect("Gamma", bn5=True)
        for chip in (411, 214, SPEC["chip_last"]):     # (past BN6's ids, one of its blank records, the last)
            await a.send(trade(1, b_id, (chip, 12)))
            self.assertEqual((await a.message(SPEC["notice"]))[:3], struct.pack("!BH", SPEC["notice"], 19))
        self.assertFalse(self.server.trades.open)
        await a.send(trade(1, c_id, (411, 12)))
        trade_id = (await self.trade_state(a))[2]
        self.assertEqual((await self.trade_state(c))[5:], (0, 0, 411, 12))
        await c.send(trade(2, trade_id, (450, 3), (411, 12)))
        self.assertEqual((await self.trade_state(a))[1:2] + (await self.trade_state(c))[1:2], (3, 3))
        await a.send(trade(3, trade_id, (411, 12), (450, 3)))
        await self.trade_state(a)
        await c.send(trade(3, trade_id, (450, 3), (411, 12)))
        done = await self.trade_state(c)
        self.assertEqual((done[1:4], done[5:]), ((5, trade_id, a_id), (450, 3, 411, 12)))
        # (a game with BN5 answering a BN6-only game's offer with BN5's chip: refused, the trade off)
        d, d_id = await self.connect("Delta", bn5=True)
        await b.send(trade(1, d_id, (5, 2)))
        trade_id = (await self.trade_state(b))[2]
        await self.trade_state(d)
        await d.send(trade(2, trade_id, (411, 12), (5, 2)))
        self.assertEqual((await d.message(SPEC["notice"]))[:3], struct.pack("!BH", SPEC["notice"], 19))
        self.assertEqual((await self.trade_state(b))[1], 6)

    async def test_an_offer_on_the_board_taken_trades_once(self):
        a, a_id = await self.connect("Alpha")
        b, b_id = await self.connect("Beta")
        self.assertEqual(await self.board(b, 0), [])
        await a.send(offer((5, 2), 1))
        self.assertEqual(await self.board(a, 1), [(a_id, 5, 2, 1)])
        self.assertEqual(await self.board(b, 1), [(a_id, 5, 2, 1)])
        # (a chip not the one posted: the offer is gone for that taker)
        await b.send(trade(5, a_id, (10, 0), (6, 2)))
        self.assertEqual(await self.notice(b), 20)
        await b.send(trade(5, a_id, (10, 0), (5, 2)))
        taken_b, taken_a = await self.trade_state(b), await self.trade_state(a)
        self.assertEqual((taken_b[1], taken_b[3], taken_b[5:]), (3, a_id, (10, 0, 5, 2)))
        self.assertEqual((taken_a[1], taken_a[3], taken_a[5:]), (3, b_id, (5, 2, 10, 0)))
        await a.send(trade(3, taken_a[2], (5, 2), (10, 0)))
        self.assertEqual((await self.trade_state(a))[1], 4)
        await b.send(trade(3, taken_b[2], (10, 0), (5, 2)))
        self.assertEqual((await self.trade_state(a))[1], 5)
        self.assertEqual((await self.trade_state(b))[1], 5)
        self.assertEqual(await self.board(b, 0), [])
        # (one trade a visit: no more posts, proposals or takes this session)
        await asyncio.sleep(1.05)
        await a.send(offer((7, 1), 0))
        self.assertEqual(await self.notice(a), 21)
        c, c_id = await self.connect("Gamma")
        await c.send(trade(1, a_id, (8, 0)))
        self.assertEqual(await self.notice(c), 21)
        self.assertFalse(self.server.trades.offers)

    async def test_offers_go_with_their_poster_and_by_the_pair_common_ground(self):
        a, a_id = await self.connect("Alpha", bn5=True)
        b, _ = await self.connect("Beta")
        c, _ = await self.connect("Gamma", bn5=True)
        await b.send(offer((411, 12), 0))
        self.assertEqual(await self.notice(b), 19)
        await a.send(offer((411, 12), 3))
        self.assertEqual(await self.board(c, 1), [(a_id, 411, 12, 3)])
        self.assertEqual(await self.board(b, 0), [])
        await b.send(trade(5, a_id, (5, 2), (411, 12)))
        self.assertEqual(await self.notice(b), 19)
        # (a second post within the second: refused; then the offer changed)
        await a.send(offer((5, 2), 0))
        self.assertEqual(await self.notice(a), 22)
        await asyncio.sleep(1.05)
        await a.send(offer((5, 2), 9))
        self.assertEqual(await self.board(b, 1), [(a_id, 5, 2, 9)])
        await a.send(offer((SPEC["chip_last"] + 1, 0), 0))
        self.assertEqual((await a.message(SPEC["error"]))[:3], struct.pack("!BH", SPEC["error"], 1))
        self.assertEqual(await self.board(b, 0), [])
        self.assertFalse(self.server.trades.offers)

    async def test_arena_rules_full_only_between_two_games_with_bn5(self):
        a, _ = await self.connect("Alpha", bn5=True)
        _, b_id = await self.connect("Beta")
        c, c_id = await self.connect("Gamma", bn5=True)
        await a.send(arena(1, b_id, 0, SPEC["format_full"]))
        self.assertEqual((await a.message(SPEC["notice"]))[:3], struct.pack("!BH", SPEC["notice"], 19))
        self.assertFalse(self.server.arena.matches)
        await a.send(arena(1, c_id, 0, SPEC["format_full"]))
        self.assertEqual((await self.arena_state(a))[-1], SPEC["format_full"])
        challenged = await self.arena_state(c)
        self.assertEqual((challenged[1], challenged[-1]), (1, SPEC["format_full"]))
        await c.send(arena(2, challenged[2], 0))
        start = await self.arena_state(c)
        self.assertEqual((start[1], start[-1]), (3, SPEC["format_full"]))
        await a.send(arena(1, c_id, 0, 2))
        self.assertEqual((await a.message(SPEC["error"]))[:3], struct.pack("!BH", SPEC["error"], 1))

    def test_delay_covers_the_trip(self):
        self.assertEqual(delay_for(0, 0), 6)
        self.assertEqual(delay_for(40, 100), 8)
        self.assertEqual(delay_for(4000, 0), 30)


if __name__ == "__main__":
    unittest.main()
