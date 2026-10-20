"""The server's daily totals (issue #156, stats.py): what a day counts,
GET /stats, stats.jsonl written at a day's end and as the server stops,
taken up again at a restart, pruned by the retention, and never an
identity or a name in either; no ROM needed."""

import asyncio
import configparser
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "server"))
sys.path.insert(0, str(Path(__file__).resolve().parent))

from cyberworld_server import protocol, retention, stats, week
from cyberworld_server.app import Server
from cyberworld_server.retention import DAY, Retention
from test_play import trade
from test_server import Client
from test_stands import arena, watch

SPEC = protocol.SPEC
NOW = week.week_start(2963) + 3 * DAY + 3600      # Thursday 22 October 2026, 01:00 UTC
NAMES = ("Alpha", "Beta", "Gamma")


async def get(port, path):
    """An HTTP GET of the server's: its status line and JSON body."""
    reader, writer = await asyncio.open_connection("127.0.0.1", port)
    writer.write(f"GET {path} HTTP/1.1\r\nHost: localhost\r\n\r\n".encode())
    await writer.drain()
    response = await reader.read()
    writer.close()
    head, body = response.split(b"\r\n\r\n", 1)
    return head.split(b"\r\n")[0].decode(), json.loads(body) if body else None


class DailyTests(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix="cyberworld-stats-")
        self.dir = Path(self.temporary.name)
        self.now = NOW
        self.server = None
        self.clients = []
        self.identities = []

    async def asyncTearDown(self):
        await self.stop()
        self.temporary.cleanup()

    async def start(self, **policy):
        self.server = Server(seed=7, capacity=8, player_cap=16, state_dir=self.dir, policy=Retention(**policy),
                             trading=True, arena=True, clock=lambda: self.now)
        self.port = await self.server.start("127.0.0.1", 0)

    async def stop(self):
        for client in self.clients:
            await client.close()
        self.clients = []
        if self.server:
            await self.server.close()
            self.server = None

    async def connect(self, name, full=True, key=None):
        client = Client()
        self.clients.append(client)
        welcome = await client.open(self.port, name, key=key or name.encode().ljust(16, b"k"), bn5=full)
        self.identities.append(welcome[14:30].hex())
        return client, struct.unpack("!I", welcome[5:9])[0]

    def assert_nothing_personal(self, text):
        """No public identity, no name, no guest key in `text`."""
        for identity in self.identities:
            self.assertNotIn(identity, text)
        for name in NAMES:
            self.assertNotIn(name, text)
            self.assertNotIn(name.encode().ljust(16, b"k").hex(), text)
        self.assertNotIn("identity\"", text.replace("identities", ""))

    async def test_a_day_counts_joins_identities_the_peak_and_the_versions(self):
        await self.start()
        a, _ = await self.connect("Alpha")
        b, _ = await self.connect("Beta", full=False)
        # (Alpha again from another session: a join, the same identity)
        await self.connect("Alpha")
        # (a game of another protocol version: refused, its version counted)
        refused = Client()
        self.clients.append(refused)
        reply = await refused.open(self.port, "Gamma", key=b"g" * 16, version=SPEC["version"] - 1)
        self.assertEqual(struct.unpack("!H", reply[1:3])[0], 2)
        status, body = await get(self.port, "/stats")
        self.assertEqual(status, "HTTP/1.1 200 OK")
        today = body["today"]
        self.assertEqual(today["day"], "2026-10-22")
        self.assertEqual((today["joins"], today["identities"], today["peak"]), (3, 2, 2))
        self.assertEqual(today["versions"], {str(SPEC["version"]): 3, str(SPEC["version"] - 1): 1})
        self.assertEqual(today["passes"], {"full": 2, "square": 1})
        self.assertEqual(body["days"], [])
        self.assert_nothing_personal(json.dumps(body))

    async def test_trades_battles_watches_and_dives_are_counted(self):
        await self.start()
        a, a_id = await self.connect("Alpha")
        b, b_id = await self.connect("Beta")
        c, _ = await self.connect("Gamma")
        # a trade
        await a.send(trade(1, b_id, (5, 2)))
        trade_id = protocol.TRADE_STATE.unpack(await a.message(SPEC["trade_state"]))[2]
        await b.message(SPEC["trade_state"])
        await b.send(trade(2, trade_id, (10, 0), (5, 2)))
        await a.message(SPEC["trade_state"])
        await b.message(SPEC["trade_state"])
        await a.send(trade(3, trade_id, (5, 2), (10, 0)))
        await a.message(SPEC["trade_state"])
        await b.send(trade(3, trade_id, (10, 0), (5, 2)))
        self.assertEqual(protocol.TRADE_STATE.unpack(await b.message(SPEC["trade_state"]))[1], 5)
        # a NetBattle, watched, then over; another started and left
        await a.send(arena(1, b_id, 40))
        match = protocol.ARENA_STATE.unpack(await a.message(SPEC["arena_state"]))[2]
        await b.message(SPEC["arena_state"])
        await b.send(arena(2, match, 40))
        await a.message(SPEC["arena_state"])
        await b.message(SPEC["arena_state"])
        await c.send(watch(1, match))
        await c.message(SPEC["watch_state"])
        await a.send(arena(4, match))
        await a.message(SPEC["arena_state"])
        await b.message(SPEC["arena_state"])
        await a.send(arena(1, b_id, 40))
        match = protocol.ARENA_STATE.unpack(await a.message(SPEC["arena_state"]))[2]
        await b.message(SPEC["arena_state"])
        await b.send(arena(2, match, 40))
        await a.message(SPEC["arena_state"])
        await a.send(arena(3, match))
        await b.message(SPEC["arena_state"])
        # a dive posted
        number = week.week_of(self.now)
        await c.send(week.post(number, self.server.weeks.seed(number), 5, 2, False, 40, 0x1234))
        for _ in range(100):
            today = (await get(self.port, "/stats"))[1]["today"]
            if today["dives"]:
                break
            await asyncio.sleep(0.02)
        self.assertEqual({field: today[field] for field in ("trades", "battles_started", "battles_finished", "watched", "dives")},
                         {"trades": 1, "battles_started": 2, "battles_finished": 1, "watched": 1, "dives": 1})
        self.assertEqual(today["peak"], 3)

    async def test_a_day_is_written_at_its_end_and_as_the_server_stops(self):
        await self.start()
        await self.connect("Alpha")
        await self.connect("Beta")
        # (past midnight: the day's line written, the next day begun with its identities dropped)
        self.now += DAY
        for _ in range(100):
            await asyncio.sleep(0.05)
            if (self.dir / stats.LOG).exists():
                break
        days = [json.loads(line) for line in (self.dir / stats.LOG).read_text().splitlines()]
        self.assertEqual([(d["day"], d["joins"], d["identities"], d["peak"]) for d in days], [("2026-10-22", 2, 2, 2)])
        self.assertEqual(self.server.daily.snapshot()["identities"], 0)
        await self.connect("Gamma")
        status, body = await get(self.port, "/stats")
        self.assertEqual((body["today"]["day"], body["today"]["joins"]), ("2026-10-23", 1))
        self.assertEqual([d["day"] for d in body["days"]], ["2026-10-22"])
        # (the server stopped: today so far written; a restart the same day takes it up again)
        await self.stop()
        days = [json.loads(line) for line in (self.dir / stats.LOG).read_text().splitlines()]
        self.assertEqual([(d["day"], d["joins"]) for d in days], [("2026-10-22", 2), ("2026-10-23", 1)])
        await self.start()
        await self.connect("Alpha")
        today = (await get(self.port, "/stats"))[1]["today"]
        self.assertEqual((today["joins"], today["identities"]), (2, 2))
        await self.stop()
        days = [json.loads(line) for line in (self.dir / stats.LOG).read_text().splitlines()]
        self.assertEqual([(d["day"], d["joins"]) for d in days], [("2026-10-22", 2), ("2026-10-23", 2)])
        # nothing personal in the file, nor at /stats
        self.assert_nothing_personal((self.dir / stats.LOG).read_text())
        self.assertEqual(set(days[0]), {"day", "joins", "identities", "peak", "trades", "battles_started",
                                        "battles_finished", "watched", "dives", "versions", "passes"})

    async def test_kept_stats_days_by_the_retention_and_none_where_zero(self):
        old = [dict(stats.fresh(stats.day_of(NOW - k * DAY)), joins=k) for k in (400, 366, 365, 364, 1)]
        (self.dir / stats.LOG).write_text("".join(json.dumps(entry) + "\n" for entry in old))
        await self.start()
        self.assertEqual([d["joins"] for d in (await get(self.port, "/stats"))[1]["days"]], [400, 366, 365, 364, 1])
        gone = await self.server.retain()
        self.assertEqual(gone.get("stats"), 3)
        days = [json.loads(line)["joins"] for line in (self.dir / stats.LOG).read_text().splitlines()]
        self.assertEqual(days, [364, 1])
        await self.stop()
        # (stats_days = 0: no line written, what was there pruned)
        await self.start(stats_days=0)
        await self.connect("Alpha")
        await self.server.retain()
        await self.stop()
        self.assertEqual((self.dir / stats.LOG).read_text(), "")

    def test_the_ini_key_and_its_bounds(self):
        self.assertEqual(Retention().stats_days, 365)
        config = configparser.ConfigParser()
        config.read_string("[server]\nstats_days = 30\n")
        self.assertEqual(Retention.from_settings(config["server"]).stats_days, 30)
        for wrong in (-1, 3651, True):
            with self.assertRaises(ValueError):
                Retention(stats_days=wrong)
        official = configparser.ConfigParser()
        official.read(ROOT / "deploy/coolify/server.ini")
        self.assertEqual(Retention.from_settings(official["server"]).stats_days, 365)

    def test_a_line_read_back_keeps_only_the_totals(self):
        line = dict(stats.fresh("2026-10-22"), joins=4, identity="a" * 32, name="Alpha", identities=True)
        kept = stats.checked(line)
        self.assertEqual(set(kept), set(stats.fresh("")))
        self.assertEqual((kept["joins"], kept["identities"]), (4, 0))
        self.assertIsNone(stats.checked({"joins": 1}))
        daily = stats.Daily(None, 365, lambda: NOW)
        for version in range(40):
            daily.hello(version)
        self.assertEqual(len(daily.snapshot()["versions"]), stats.VERSIONS + 1)
        self.assertEqual(daily.snapshot()["versions"]["other"], 40 - stats.VERSIONS)


if __name__ == "__main__":
    unittest.main()
