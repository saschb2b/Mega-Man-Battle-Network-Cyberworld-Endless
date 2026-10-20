"""The week's net over real sockets (issue #127); no ROM needed."""

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

from cyberworld_server import admin, protocol, week
from cyberworld_server.app import Server
from test_server import Client

SPEC = protocol.SPEC
MONDAY = week.week_start(2963)          # a Monday 00:00 UTC (19 October 2026)


def board(data):
    """A WEEK_BOARD as (week, place, mine, top, last), each entry (name, depth, act, won, minutes)."""
    _, number, count, past, place = week.BOARD_HEAD.unpack_from(data)
    size = week.ENTRY.size
    assert len(data) == week.BOARD_HEAD.size + size * (1 + count + past)
    entries = [week.ENTRY.unpack_from(data, week.BOARD_HEAD.size + k * size) for k in range(1 + count + past)]
    entries = [(name.rstrip(b"\0").decode(), *rest) for name, *rest in entries]
    return number, place, entries[0], entries[1:1 + count], entries[1 + count:]


class WeekTests(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        self.dir = tempfile.TemporaryDirectory()
        self.now = MONDAY + 3600
        await self.start()
        self.clients = []

    async def start(self):
        self.server = Server(seed=7, capacity=8, player_cap=16, state_dir=self.dir.name)
        self.server.weeks.clock = lambda: self.now
        self.port = await self.server.start("127.0.0.1", 0)

    async def asyncTearDown(self):
        for client in self.clients:
            await client.close()
        await self.server.close()
        self.dir.cleanup()

    async def connect(self, name, full=True):
        """A game: with the full set (BN6's and BN5's ROMs), or BN6's alone."""
        client = Client()
        self.clients.append(client)
        welcome = await client.open(self.port, name, key=name.encode().ljust(16, b"k"), bn5=full)
        seed, number = struct.unpack("!IH", welcome[32:38])
        return client, seed, number

    async def board(self, client):
        return board(await client.message(SPEC["week_board"]))

    async def notice(self, client):
        return struct.unpack("!H", (await client.message(SPEC["notice"]))[1:3])[0]

    async def test_welcome_names_the_week_and_its_own_seed(self):
        a, seed, number = await self.connect("Alpha")
        self.assertEqual(number, 2963)
        self.assertEqual(seed, self.server.weeks.seed(2963))
        self.assertNotEqual(seed, self.server.weeks.seed(2962))
        self.assertEqual(week.week_of(MONDAY - 1), 2962)
        self.assertEqual(week.week_of(MONDAY + 7 * 86400 - 1), 2963)
        # (another server's secret, another week's net)
        with tempfile.TemporaryDirectory() as other:
            self.assertNotEqual(Server(state_dir=other).weeks.seed(2963), seed)
        number, place, mine, top, last = await self.board(a)
        self.assertEqual((number, place, mine[1], top, last), (2963, 0, 0, [], []))

    async def test_the_best_dive_a_week_is_kept_and_shown(self):
        a, seed, number = await self.connect("Alpha")
        b, _, _ = await self.connect("Beta")
        await self.board(a)
        await self.board(b)
        await a.send(week.post(number, seed, 5, 2, False, 40, 0x1234))
        number_seen, place, mine, top, _ = await self.board(a)
        self.assertEqual((place, mine), (1, ("Alpha", 5, 2, 0, 40)))
        self.assertEqual((await self.board(b))[3], [("Alpha", 5, 2, 0, 40)])
        # (a shallower dive keeps the best; a win on the last layer beats it)
        await a.send(week.post(number, seed, 3, 1, False, 20, 0x1234))
        self.assertEqual((await self.board(a))[2][1], 5)
        await b.send(week.post(number, seed, 10, 4, True, 70, 0x99))
        _, place, mine, top, _ = await self.board(b)
        self.assertEqual((place, [e[0] for e in top]), (1, ["Beta", "Alpha"]))
        self.assertEqual(top[0], ("Beta", 10, 4, 1, 70))
        await asyncio.sleep(0.2)
        kept = [json.loads(line) for line in (Path(self.dir.name) / "weeks.jsonl").read_text().splitlines()]
        self.assertEqual([(e["name"], e["depth"]) for e in kept], [("Alpha", 5), ("Beta", 10)])
        # (kept across a restart, and shown as last week's once the week turns)
        await self.server.close()
        for client in self.clients:
            await client.close()
        self.clients = []
        self.now += 7 * 86400
        await self.start()
        c, seed, number = await self.connect("Gamma")
        self.assertEqual(number, 2964)
        _, _, _, top, last = await self.board(c)
        self.assertEqual((top, [e[0] for e in last]), ([], ["Beta", "Alpha"]))

    async def test_implausible_dives_are_refused(self):
        a, seed, number = await self.connect("Alpha")
        await self.board(a)
        for message, code in ((week.post(number - 2, self.server.weeks.seed(number - 2), 4, 2, False, 9, 1), 23),
                              (week.post(number, seed ^ 1, 4, 2, False, 9, 1), 24),
                              (week.post(number, seed, 11, 4, False, 9, 1), 25),
                              (week.post(number, seed, 4, 1, False, 9, 1), 25),
                              (week.post(number, seed, 5, 2, True, 9, 1), 25),
                              (week.post(number, seed, 5, 2, False, 9, 0), 25)):
            await a.send(message)
            self.assertEqual(await self.notice(a), code)
        # (last week's net is still taken: a dive begun before Monday)
        await a.send(week.post(number - 1, self.server.weeks.seed(number - 1), 4, 2, False, 9, 1))
        self.assertEqual((await self.board(a))[4], [("Alpha", 4, 2, 0, 9)])
        await a.send(bytes((SPEC["week_post"], 0)))
        self.assertEqual((await a.message(SPEC["error"]))[:3], struct.pack("!BH", SPEC["error"], 1))

    async def test_posting_needs_the_full_set_and_the_board_is_every_games(self):
        a, seed, number = await self.connect("Alpha")
        b, _, _ = await self.connect("Beta", full=False)
        await self.board(a)
        await self.board(b)
        await a.send(week.post(number, seed, 5, 2, False, 40, 0x1234))
        self.assertEqual((await self.board(a))[3], [("Alpha", 5, 2, 0, 40)])
        self.assertEqual((await self.board(b))[3], [("Alpha", 5, 2, 0, 40)])
        await b.send(week.post(number, seed, 7, 3, False, 30, 0x99))
        self.assertEqual(await self.notice(b), SPEC["notice_pass"])
        self.assertEqual(len(self.server.weeks.best), 1)
        # (a malformed post is malformed before it is refused)
        await b.send(bytes((SPEC["week_post"], 0)))
        self.assertEqual((await b.message(SPEC["error"]))[:3], struct.pack("!BH", SPEC["error"], 1))

    async def test_a_moderator_hides_an_entry(self):
        a, seed, number = await self.connect("Alpha")
        await self.board(a)
        await a.send(week.post(number, seed, 6, 2, False, 30, 7))
        self.assertEqual((await self.board(a))[1], 1)
        identity = self.server.players[next(iter(self.server.players))].identity.hex()
        result = admin.perform("hide", self.dir.name, identity, "test", week=number)
        self.assertEqual(result["week"], number)
        self.server.store.reload()
        await a.send(week.post(number, seed, 7, 3, False, 30, 7))
        self.assertEqual(await self.notice(a), 26)
        b, _, _ = await self.connect("Beta")
        self.assertEqual((await self.board(b))[3], [])
        with self.assertRaises(ValueError):
            admin.perform("ban", self.dir.name, identity, week=number)


if __name__ == "__main__":
    unittest.main()
