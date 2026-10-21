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


def arena(action, ref, latency=0):
    return protocol.ARENA.pack(SPEC["arena"], action, ref, latency)


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

    async def connect(self, name, full=True):
        """A game in the channel: with the full set (BN6's and BN5's ROMs), or BN6's alone."""
        client = Client()
        self.clients.append(client)
        welcome = await client.open(self.port, name, bn5=full)
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

    async def no_notice(self, client):
        """A PING answered with no NOTICE before its PONG: what was sent before it was taken."""
        await client.send(bytes((SPEC["ping"],)) + b"\0\0\0\1")
        for _ in range(100):
            message = await client.receive()
            self.assertNotEqual(message[0], SPEC["notice"])
            if message[0] == SPEC["pong"]:
                return
        raise AssertionError("No PONG")

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

    async def test_chips_past_the_tables_and_leaving_players_never_trade(self):
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
        _, stage, match_id, other, _, _, _ = await self.arena_state(a)
        self.assertEqual((stage, other), (2, b_id))
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

    async def battle(self, challenger, challenger_id, other, other_id):
        """A battle asked, seen, taken and over: its START's sides, (challenger's, other's)."""
        await challenger.send(arena(1, other_id, 40))
        match_id = (await self.arena_state(challenger))[2]
        self.assertEqual((await self.arena_state(other))[1:3], (1, match_id))
        await other.send(arena(SPEC["arena_seen"], match_id))
        self.assertEqual((await self.arena_state(challenger))[1:3], (SPEC["arena_seen"], match_id))
        await other.send(arena(2, match_id, 40))
        sides = (await self.arena_state(challenger))[5], (await self.arena_state(other))[5]
        await (challenger if sides[0] == 0 else other).send(arena(4, match_id))
        self.assertEqual((await self.arena_state(challenger))[1], 4)
        self.assertEqual((await self.arena_state(other))[1], 4)
        return sides

    async def test_a_rematch_keeps_the_sides_whoever_asks(self):
        # (v13: both games go into a rematch from the pair they kept, its sides the last battle's)
        a, a_id = await self.connect("Alpha")
        b, b_id = await self.connect("Beta")
        self.assertEqual(await self.battle(a, a_id, b, b_id), (0, 1))
        self.assertEqual(await self.battle(b, b_id, a, a_id), (1, 0))
        self.assertEqual(await self.battle(a, a_id, b, b_id), (0, 1))
        # (a new pair: the challenger first again)
        c, c_id = await self.connect("Gamma")
        self.assertEqual(await self.battle(c, c_id, b, b_id), (0, 1))
        await b.close()
        self.clients.remove(b)
        await asyncio.sleep(0.1)
        self.assertFalse([pair for pair in self.server.arena.sides if b_id in pair])

    async def test_a_challenge_waits_longer_unseen_and_its_thirty_seconds_run_from_its_question(self):
        a, a_id = await self.connect("Alpha")
        b, b_id = await self.connect("Beta")
        await a.send(arena(1, b_id, 40))
        match_id = (await self.arena_state(a))[2]
        await self.arena_state(b)
        match = self.server.arena.matches[match_id]
        match.time -= 45
        self.server.arena.expire(match.time + 45)
        self.assertIn(match_id, self.server.arena.matches)
        # (seen only once, by the challenged alone)
        await a.send(arena(SPEC["arena_seen"], match_id))
        await self.no_notice(a)
        self.assertFalse(match.seen)
        await b.send(arena(SPEC["arena_seen"], match_id))
        self.assertEqual((await self.arena_state(a))[1], SPEC["arena_seen"])
        self.assertTrue(match.seen)
        self.server.arena.expire(match.time + 29)
        self.assertIn(match_id, self.server.arena.matches)
        self.server.arena.expire(match.time + 31)
        self.assertNotIn(match_id, self.server.arena.matches)
        self.assertEqual((await self.arena_state(a))[1], 4)

    async def snapshot_byte(self, client, peer_id, want):
        for _ in range(60):
            data = await client.message(SPEC["snapshot"])
            for k in range(data[1]):
                peer = protocol.PEER.unpack_from(data, 2 + k * protocol.PEER.size)
                if peer[0] == peer_id and peer[-1] == want:
                    return
        raise AssertionError(f"No such byte {want:#x}")

    async def test_the_square_sees_who_is_busy_and_each_link(self):
        a, a_id = await self.connect("Alpha")
        b, b_id = await self.connect("Beta")
        c, _ = await self.connect("Gamma")
        bn5 = SPEC["roms_bn5"]
        # (a round trip said in PING: Beta's link three bars, then one)
        await b.send(bytes((SPEC["ping"],)) + b"\0\0\0\1" + struct.pack("!H", 60))
        await self.snapshot_byte(c, b_id, bn5 | 3 << SPEC["link_shift"])
        await b.send(bytes((SPEC["ping"],)) + b"\0\0\0\2" + struct.pack("!H", 250))
        await self.snapshot_byte(c, b_id, bn5 | 1 << SPEC["link_shift"])
        # (a challenge waiting is not busy; a battle under way is, both of them)
        await a.send(arena(1, b_id, 40))
        match_id = (await self.arena_state(a))[2]
        await self.snapshot_byte(c, a_id, bn5)
        await b.send(arena(2, match_id, 40))
        busy = SPEC["busy_battle"] << SPEC["busy_shift"]
        await self.snapshot_byte(c, a_id, bn5 | busy)
        await self.snapshot_byte(c, b_id, bn5 | busy | 1 << SPEC["link_shift"])
        self.assertEqual(protocol.link_bars(0), 0)
        self.assertEqual([protocol.link_bars(ms) for ms in (99, 100, 199, 200)], [3, 2, 2, 1])

    async def test_bn5_chips_trade_between_two_full_sets(self):
        a, a_id = await self.connect("Alpha")
        c, c_id = await self.connect("Gamma")
        await a.send(trade(1, c_id, (411, 12)))
        trade_id = (await self.trade_state(a))[2]
        self.assertEqual((await self.trade_state(c))[5:], (0, 0, 411, 12))
        await c.send(trade(2, trade_id, (214, 3), (411, 12)))     # (past BN6's ids; one of its blank records)
        self.assertEqual((await self.trade_state(a))[1:2] + (await self.trade_state(c))[1:2], (3, 3))
        await a.send(trade(3, trade_id, (411, 12), (214, 3)))
        await self.trade_state(a)
        await c.send(trade(3, trade_id, (214, 3), (411, 12)))
        done = await self.trade_state(c)
        self.assertEqual((done[1:4], done[5:]), ((5, trade_id, a_id), (214, 3, 411, 12)))

    async def test_trade_central_needs_the_full_set(self):
        a, a_id = await self.connect("Alpha")
        b, b_id = await self.connect("Beta", full=False)
        c, c_id = await self.connect("Gamma")
        # (from a game without it: a proposal, a take, a post; to one: a proposal, a take of its offer)
        await b.send(trade(1, a_id, (5, 2)))
        self.assertEqual(await self.notice(b), SPEC["notice_pass"])
        await a.send(trade(1, b_id, (5, 2)))
        self.assertEqual(await self.notice(a), SPEC["notice_pass"])
        self.assertFalse(self.server.trades.open)
        await c.send(offer((5, 2), 0))
        self.assertEqual(await self.board(b, 1), [(c_id, 5, 2, 0)])     # (the board is every game's to read)
        await b.send(trade(5, c_id, (10, 0), (5, 2)))
        self.assertEqual(await self.notice(b), SPEC["notice_pass"])
        await b.send(offer((5, 2), 0))
        self.assertEqual(await self.notice(b), SPEC["notice_pass"])
        self.assertNotIn(b_id, self.server.trades.offers)
        await b.send(offer())       # (a take-down: never refused)
        await self.no_notice(b)
        self.server.trades.offers[b_id] = ((7, 1), 0)    # (an offer of a game without it, as no wire puts one up)
        await a.send(trade(5, b_id, (10, 0), (7, 1)))
        self.assertEqual(await self.notice(a), SPEC["notice_pass"])
        self.assertFalse(self.server.trades.open)
        # (an answer from a game without it: refused before it touches the trade, which goes on)
        await a.send(trade(1, c_id, (5, 2)))
        trade_id = (await self.trade_state(a))[2]
        await self.trade_state(c)
        await b.send(trade(2, trade_id, (10, 0), (5, 2)))
        self.assertEqual(await self.notice(b), SPEC["notice_pass"])
        await c.send(trade(2, trade_id, (10, 0), (5, 2)))
        self.assertEqual((await self.trade_state(a))[1], 3)

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

    async def test_offers_go_with_their_poster_and_bn5_chips_where_they_can_be_named(self):
        a, a_id = await self.connect("Alpha")
        b, _ = await self.connect("Beta", full=False)
        c, _ = await self.connect("Gamma")
        await a.send(offer((411, 12), 3))
        self.assertEqual(await self.board(c, 1), [(a_id, 411, 12, 3)])
        self.assertEqual(await self.board(b, 0), [])     # (BN5's chip left out for a game without its ROM)
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

    async def test_the_arena_needs_the_full_set(self):
        a, a_id = await self.connect("Alpha")
        b, b_id = await self.connect("Beta", full=False)
        c, c_id = await self.connect("Gamma")
        # (a challenge from a game without it, and to one)
        await b.send(arena(1, a_id, 0))
        self.assertEqual(await self.notice(b), SPEC["notice_pass"])
        await a.send(arena(1, b_id, 0))
        self.assertEqual(await self.notice(a), SPEC["notice_pass"])
        self.assertFalse(self.server.arena.matches)
        # (an answer from one: refused before the battle is looked at; a leave never is)
        await a.send(arena(1, c_id, 0))
        match_id = (await self.arena_state(a))[2]
        self.assertEqual((await self.arena_state(c))[1:3], (1, match_id))
        await b.send(arena(2, match_id, 0))
        self.assertEqual(await self.notice(b), SPEC["notice_pass"])
        await b.send(arena(3, match_id))
        self.assertEqual(await self.notice(b), 17)
        self.assertFalse(self.server.arena.matches[match_id].started)
        await c.send(arena(2, match_id, 0))
        self.assertEqual((await self.arena_state(c))[1], 3)
        # (v8's ARENA, a rules byte at its end: malformed)
        await a.send(arena(1, c_id, 0) + b"\1")
        self.assertEqual((await a.message(SPEC["error"]))[:3], struct.pack("!BH", SPEC["error"], 1))

    def test_delay_covers_the_trip(self):
        # (the lockstep's frame, the longer latency said: 1 at least, 30 at most; a game that rolls
        # back says its round trip less what its window covers but a frame, protocol v8 and issue #167)
        self.assertEqual(delay_for(0, 0), 1)
        self.assertEqual(delay_for(20, 10), 3)
        self.assertEqual(delay_for(40, 100), 7)
        self.assertEqual(delay_for(4000, 0), 30)
        # (a game whose window covers its trip asks for 0: the floor 1, where the server once gave 2;
        # past the window's edge the delay is what it was, the window's frame said back by the game)
        self.assertEqual(delay_for(16, 0), 2)


if __name__ == "__main__":
    unittest.main()
