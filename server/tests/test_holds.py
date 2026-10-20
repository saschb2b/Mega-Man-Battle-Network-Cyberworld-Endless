"""A place held through a dropped link, and taken back (issue #153, protocol
v10, holds.py); the maintenance line (server.ini)."""

import asyncio
import json
from pathlib import Path
import struct
import sys
import tempfile
import time
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "server"))

from cyberworld_server import admin, protocol
from cyberworld_server.app import Server
from test_server import Client, masked
from test_play import arena, trade

SPEC = protocol.SPEC


async def until(condition, timeout=2.0):
    deadline = time.monotonic() + timeout
    while not condition():
        if time.monotonic() > deadline:
            raise AssertionError("Condition not reached")
        await asyncio.sleep(0.01)


class HoldTests(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        self.dir = tempfile.TemporaryDirectory()
        self.server = None
        self.clients = []

    async def start(self, **options):
        options = dict(dict(seed=7, capacity=8, player_cap=16, state_dir=self.dir.name, trading=True, arena=True), **options)
        self.server = Server(**options)
        self.port = await self.server.start("127.0.0.1", 0)
        return self.server

    async def asyncTearDown(self):
        for client in self.clients:
            await client.close()
        if self.server:
            await self.server.close()
        self.dir.cleanup()

    async def connect(self, name, key=None, resume=False, channel=0):
        client = Client()
        self.clients.append(client)
        welcome = await client.open(self.port, name, key=key, bn5=True, resume=resume, channel=channel)
        return client, welcome

    async def dropped(self, client):
        """The link gone without a word (no BYE, no close frame)."""
        await client.drop()
        self.clients.remove(client)

    async def peers(self, client):
        """The ids in the next SNAPSHOT."""
        data = await client.message(SPEC["snapshot"])
        return [protocol.PEER.unpack_from(data, 2 + k * SPEC["peer_bytes"])[0] for k in range(data[1])]

    async def test_a_dropped_link_keeps_its_place_and_takes_it_back(self):
        server = await self.start()
        key = b"h" * 16
        a, first = await self.connect("Alpha", key)
        a_id = struct.unpack("!I", first[5:9])[0]
        b, _ = await self.connect("Beta")
        await a.send(protocol.STATE.pack(SPEC["state"], 40, 50, 0, 3))
        await until(lambda: server.players[a_id].position == (40, 50, 0, 3))
        await self.dropped(a)
        await until(lambda: server.holds.held(server.players[a_id]))
        # (Beta still sees him, standing where he stood)
        for _ in range(10):
            self.assertEqual(await self.peers(b), [a_id])
        self.assertEqual(server.players[a_id].position, (40, 50, 0, 3))
        a, again = await self.connect("Alpha", key, resume=True)
        self.assertEqual(again[0], SPEC["welcome"])
        self.assertEqual(again[5:9], first[5:9])        # his id
        self.assertEqual(again[12:14], first[12:14])    # his channel
        self.assertFalse(server.holds.held(server.players[a_id]))
        self.assertEqual(await self.peers(b), [a_id])
        self.assertEqual(len(server.players), 2)
        # (the same visit: not one more of the day's joins, stats.py)
        self.assertEqual(server.daily.today["joins"], 2)

    async def test_the_hold_ends_and_a_goodbye_frees_at_once(self):
        server = await self.start(hold=0.3)
        a, first = await self.connect("Alpha")
        a_id = struct.unpack("!I", first[5:9])[0]
        b, _ = await self.connect("Beta")
        await self.dropped(a)
        await until(lambda: a_id in server.holds.until)
        await until(lambda: a_id not in server.players)
        for _ in range(20):
            if await self.peers(b) == []:
                break
        else:
            self.fail("Alpha still shown after his hold")
        # (BYE: gone the moment he says so)
        c, third = await self.connect("Gamma")
        c_id = struct.unpack("!I", third[5:9])[0]
        await c.close()
        self.clients.remove(c)
        await until(lambda: c_id not in server.players, 0.2)
        self.assertNotIn(c_id, server.holds.until)

    async def test_a_resume_with_no_place_held_joins_its_channel_anew(self):
        # (a server restarted: nothing held, the resume asks for its channel)
        server = await self.start()
        a, welcome = await self.connect("Alpha", resume=True, channel=2)
        self.assertEqual(welcome[0], SPEC["welcome"])
        self.assertEqual(struct.unpack("!H", welcome[12:14])[0], 2)
        self.assertFalse(server.players[struct.unpack("!I", welcome[5:9])[0]].resumed)

    async def test_a_new_visit_while_held_arrives_anew(self):
        server = await self.start()
        key = b"n" * 16
        a, first = await self.connect("Alpha", key)
        await self.dropped(a)
        a_id = struct.unpack("!I", first[5:9])[0]
        await until(lambda: a_id in server.holds.until)
        a, second = await self.connect("Alpha", key)
        self.assertNotEqual(second[5:9], first[5:9])
        self.assertNotIn(a_id, server.players)
        self.assertNotIn(a_id, server.holds.until)

    async def trade_state(self, client):
        return protocol.TRADE_STATE.unpack(await client.message(SPEC["trade_state"]))

    async def test_a_trade_under_way_is_off_for_both_and_told_again(self):
        server = await self.start()
        key = b"t" * 16
        a, first = await self.connect("Alpha", key)
        b, second = await self.connect("Beta")
        b_id = struct.unpack("!I", second[5:9])[0]
        await a.send(trade(1, b_id, (5, 2)))
        trade_id = (await self.trade_state(a))[2]
        await self.trade_state(b)
        await self.dropped(a)
        self.assertEqual((await self.trade_state(b))[1:3], (6, trade_id))
        # (a trade aimed at a Navi whose place is held: busy)
        c, _ = await self.connect("Gamma")
        await c.send(trade(1, struct.unpack("!I", first[5:9])[0], (5, 2)))
        self.assertEqual(struct.unpack("!H", (await c.message(SPEC["notice"]))[1:3])[0], 16)
        a, _ = await self.connect("Alpha", key, resume=True)
        self.assertEqual((await self.trade_state(a))[1:3], (6, trade_id))
        self.assertFalse(server.trades.open)

    async def test_a_trade_committed_as_the_link_dropped_still_comes_home(self):
        server = await self.start()
        key = b"d" * 16
        a, _ = await self.connect("Alpha", key)
        b, second = await self.connect("Beta")
        b_id = struct.unpack("!I", second[5:9])[0]
        await a.send(trade(1, b_id, (5, 2)))
        trade_id = (await self.trade_state(a))[2]
        await self.trade_state(b)
        await b.send(trade(2, trade_id, (10, 0), (5, 2)))
        await self.trade_state(a)
        await self.trade_state(b)
        await a.send(trade(3, trade_id, (5, 2), (10, 0)))
        await self.trade_state(a)
        # (Alpha's link drops; the server has not noticed yet when Beta's Deal commits)
        a.writer.transport.pause_reading()
        await b.send(trade(3, trade_id, (10, 0), (5, 2)))
        self.assertEqual((await self.trade_state(b))[1], 5)
        await self.dropped(a)
        a, _ = await self.connect("Alpha", key, resume=True)
        done = await self.trade_state(a)
        self.assertEqual((done[1], done[2], done[5:]), (5, trade_id, (5, 2, 10, 0)))
        # (one trade a visit holds across the drop: the same visit)
        await a.send(trade(1, b_id, (6, 0)))
        self.assertEqual(struct.unpack("!H", (await a.message(SPEC["notice"]))[1:3])[0], 21)

    async def test_a_netbattle_ends_for_the_other_and_no_challenge_reaches_a_held_place(self):
        server = await self.start()
        a, first = await self.connect("Alpha")
        a_id = struct.unpack("!I", first[5:9])[0]
        b, second = await self.connect("Beta")
        b_id = struct.unpack("!I", second[5:9])[0]
        await a.send(arena(1, b_id))
        match = protocol.ARENA_STATE.unpack(await a.message(SPEC["arena_state"]))[2]
        await b.message(SPEC["arena_state"])
        await b.send(arena(2, match))
        await a.message(SPEC["arena_state"])
        self.assertEqual(protocol.ARENA_STATE.unpack(await b.message(SPEC["arena_state"]))[1], 3)
        await self.dropped(a)
        ended = protocol.ARENA_STATE.unpack(await b.message(SPEC["arena_state"]))
        self.assertEqual(ended[1:3], (4, match))
        await until(lambda: a_id in server.holds.until)
        await b.send(arena(1, a_id))
        self.assertEqual(struct.unpack("!H", (await b.message(SPEC["notice"]))[1:3])[0], 16)

    async def test_a_moderator_frees_a_held_place_at_once(self):
        server = await self.start()
        server.housekeeping = 0.02
        a, first = await self.connect("Alpha")
        a_id = struct.unpack("!I", first[5:9])[0]
        await self.dropped(a)
        await until(lambda: a_id in server.holds.until)
        admin.perform("ban", Path(self.dir.name), first[14:30].hex())
        await until(lambda: a_id not in server.players)

    async def test_a_stalled_links_backlog_of_positions_is_no_flood(self):
        server = await self.start()
        a, first = await self.connect("Alpha")
        a_id = struct.unpack("!I", first[5:9])[0]
        # (five seconds of positions let through at once, as a stall clears)
        a.writer.write(b"".join(masked(protocol.STATE.pack(SPEC["state"], k, 0, 0, 0)) for k in range(100)))
        await a.writer.drain()
        await until(lambda: server.players[a_id].position == (99, 0, 0, 0))
        await a.send(bytes((SPEC["ping"],)) + b"1234")
        self.assertEqual(await a.message(SPEC["pong"]), bytes((SPEC["pong"],)) + b"1234")

    async def test_positions_among_a_netbattles_keys_never_run_dry(self):
        # (a battle's keys 60 a second between the positions' 20: the
        # positions' bucket refilled by the time between any two messages)
        server = await self.start()
        a, first = await self.connect("Alpha")
        a_id = struct.unpack("!I", first[5:9])[0]
        for k in range(300):
            await a.send(protocol.ARENA_INPUT.pack(SPEC["arena_input"], 1, k, 1) + b"\0\0")
            if k % 3 == 0:
                await a.send(protocol.STATE.pack(SPEC["state"], k, 0, 0, 0))
            await asyncio.sleep(1 / 60)
        await until(lambda: server.players[a_id].position == (297, 0, 0, 0))

    async def test_maintenance_turns_every_game_away_with_its_line(self):
        with self.assertRaises(ValueError):
            Server(maintenance="x" * (SPEC["maintenance_bytes"] + 1))
        server = await self.start(maintenance="until  14:00 UTC")
        _, refused = await self.connect("Alpha")
        self.assertEqual(refused[:3], struct.pack("!BH", SPEC["error"], SPEC["error_maintenance"]))
        self.assertEqual(refused[3:], b"until 14:00 UTC")
        self.assertEqual(server.health()["maintenance"], "until 14:00 UTC")
        self.assertFalse(server.players)
        self.assertEqual(json.loads(json.dumps(server.health()))["hold_seconds"], 40.0)


if __name__ == "__main__":
    unittest.main()
