"""What the state directory keeps and for how long (retention.py), the
players' log, a player's data forgotten, and the backups (backup.py);
no ROM needed."""

import asyncio
import configparser
import io
import json
from pathlib import Path
import sys
import tarfile
import tempfile
import time
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "server"))
sys.path.insert(0, str(Path(__file__).resolve().parent))

from cyberworld_server import admin, backup, retention, week
from cyberworld_server.app import Server
from cyberworld_server.identity import read_json, write_json
from cyberworld_server.retention import DAY, PlayersLog, Retention
from test_server import Client

NOW = week.week_start(2963) + 3 * DAY      # Thursday 22 October 2026, 00:00 UTC
ALPHA, BETA, GAMMA = "a" * 32, "b" * 32, "c" * 32


def lines(path):
    return [json.loads(line) for line in Path(path).read_text().splitlines()]


def jsonl(path, entries):
    Path(path).write_text("".join(json.dumps(entry) + "\n" for entry in entries))


def dive(identity, number, depth, at, name="Alpha"):
    return {"week": number, "identity": identity, "name": name, "depth": depth, "act": week.act_of(depth),
            "won": 0, "minutes": 30, "time": at, "seed": 1, "hash": 7}


class RetentionTests(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix="cyberworld-retention-")
        self.dir = Path(self.temporary.name)
        self.now = NOW
        self.server = None
        self.clients = []

    async def asyncTearDown(self):
        for client in self.clients:
            await client.close()
        if self.server:
            await self.server.close()
        self.temporary.cleanup()

    async def start(self, auto=False, **policy):
        """A server; its own passes held off (the tests call retain) unless `auto`."""
        self.server = Server(seed=7, capacity=8, player_cap=16, state_dir=self.dir, policy=Retention(**policy),
                             clock=lambda: self.now)
        if not auto:
            self.server.retention_time = time.monotonic()
        self.port = await self.server.start("127.0.0.1", 0)
        return self.server

    async def connect(self, name, key):
        client = Client()
        self.clients.append(client)
        welcome = await client.open(self.port, name, key=key)
        return client, welcome[14:30].hex()

    def test_the_policy_is_read_from_the_ini_and_bounded(self):
        self.assertEqual(Retention(), Retention(trade_days=14, board_weeks=2, log_days=14, backup_days=14))
        config = configparser.ConfigParser()
        config.read_string("[server]\ntrade_days = 7\nlog_days = 0\n")
        self.assertEqual(Retention.from_settings(config["server"]), Retention(trade_days=7, log_days=0))
        for wrong in ({"board_weeks": 1}, {"trade_days": -1}, {"backup_days": 0}, {"log_days": 366}, {"trade_days": True}):
            with self.assertRaises(ValueError):
                Retention(**wrong)
        config.read_string("[server]\nboard_weeks = many\n")
        with self.assertRaises(ValueError):
            Retention.from_settings(config["server"])

    async def test_trades_older_than_their_days_go(self):
        (self.dir / "identity.key").write_bytes(b"k" * 32)
        jsonl(self.dir / "trades.jsonl", [{"time": NOW - 20 * DAY, "a": ALPHA, "b": BETA},
                                          {"time": NOW - 13 * DAY, "a": ALPHA, "b": GAMMA},
                                          {"time": "yesterday", "a": BETA, "b": GAMMA}])
        with (self.dir / "trades.jsonl").open("a") as log:
            log.write("not json\n")
        await self.start()
        gone = await self.server.retain()
        self.assertEqual(gone, {"trades": 3})
        self.assertEqual([entry["b"] for entry in lines(self.dir / "trades.jsonl")], [GAMMA])
        # (nothing more to go: the file is left alone)
        before = (self.dir / "trades.jsonl").stat().st_mtime_ns
        self.assertEqual(await self.server.retain(), {})
        self.assertEqual((self.dir / "trades.jsonl").stat().st_mtime_ns, before)
        self.now += 2 * DAY
        self.assertEqual(await self.server.retain(), {"trades": 1})
        self.assertEqual((self.dir / "trades.jsonl").read_text(), "")

    async def test_the_pass_runs_at_the_start_and_then_hourly(self):
        await self.start(auto=True)
        await asyncio.sleep(1.3)
        first = self.server.retention_task
        self.assertIsNotNone(first)
        await first
        self.server.keep(self.server.retention_time + 10)
        self.assertIs(self.server.retention_task, first)
        # (a second past the hour: retention_time + PASS - retention_time can round below PASS)
        self.server.keep(self.server.retention_time + retention.PASS + 1)
        self.assertIsNot(self.server.retention_task, first)

    async def test_the_board_keeps_its_weeks_and_each_identitys_best(self):
        jsonl(self.dir / "weeks.jsonl", [dive(ALPHA, 2960, 9, NOW - 20 * DAY), dive(ALPHA, 2962, 3, NOW - 9 * DAY),
                                         dive(ALPHA, 2962, 6, NOW - 8 * DAY), dive(BETA, 2963, 4, NOW - DAY, "Beta"),
                                         {"week": 2963, "identity": GAMMA}])
        await self.start()
        kept = lines(self.dir / "weeks.jsonl")
        self.assertEqual([(entry["week"], entry["depth"]) for entry in kept], [(2962, 6), (2963, 4)])
        self.assertEqual(set(self.server.weeks.best), {(2962, ALPHA), (2963, BETA)})
        # (a week later 2962 is past the Board: the pass lets it go)
        self.now += 7 * DAY
        self.assertEqual(await self.server.retain(), {"board": 1})
        self.assertEqual([entry["week"] for entry in lines(self.dir / "weeks.jsonl")], [2963])
        # (a host keeping more weeks keeps them)
        await self.server.close()
        jsonl(self.dir / "weeks.jsonl", [dive(ALPHA, 2961, 9, NOW - 15 * DAY), dive(BETA, 2963, 4, NOW - DAY)])
        await self.start(board_weeks=4)
        self.assertEqual(await self.server.retain(), {})
        self.assertEqual(len(lines(self.dir / "weeks.jsonl")), 2)

    async def test_ended_kicks_and_hidden_entries_past_the_board_go_but_bans_stay(self):
        (self.dir / "identity.key").write_bytes(b"k" * 32)
        write_json(self.dir / "moderation.json", {
            "bans": {ALPHA: {"time": NOW - 400 * DAY, "reason": "spam"}},
            "mutes": {BETA: {"time": NOW - 400 * DAY, "reason": ""}},
            "kicks": {ALPHA: {"time": NOW - DAY, "expires": NOW - 1, "reason": ""},
                      GAMMA: {"time": NOW, "expires": NOW + 600, "reason": ""}},
            "hidden": {BETA: {"time": NOW - 30 * DAY, "week": 2959, "reason": ""},
                       GAMMA: {"time": NOW, "week": 2962, "reason": ""}}})
        await self.start()
        self.assertEqual(await self.server.retain(), {"moderation": 2})
        rules = read_json(self.dir / "moderation.json", {})
        self.assertEqual({action: sorted(entries) for action, entries in rules.items()},
                         {"bans": [ALPHA], "mutes": [BETA], "kicks": [GAMMA], "hidden": [GAMMA]})
        # (a file that will not load is never written over)
        (self.dir / "moderation.json").write_text("{broken")
        self.assertEqual(await self.server.retain(), {})
        self.assertEqual((self.dir / "moderation.json").read_text(), "{broken")

    async def test_names_and_identities_go_to_the_players_log_alone(self):
        await self.start()
        with self.assertLogs("cyberworld.server", "INFO") as console:
            _, identity = await self.connect("Alpha", b"p" * 16)
            await asyncio.sleep(0.05)
        self.assertTrue(any('"event":"join"' in line for line in console.output))
        self.assertFalse(any(identity in line or "Alpha" in line for line in console.output))
        today = PlayersLog.name(time.time())
        joined = lines(self.dir / "logs" / today)
        self.assertEqual((joined[0]["event"], joined[0]["identity"], joined[0]["name"]), ("join", identity, "Alpha"))
        # (the days past log_days go; with 0 none is written and all go)
        logs = PlayersLog(self.dir, 14)
        for days in (13, 14, 30):
            (self.dir / "logs" / PlayersLog.name(time.time() - days * DAY)).write_text("{}\n")
        self.assertEqual(logs.prune(time.time()), 2)
        self.assertEqual([name for _, name in logs.files()], [PlayersLog.name(time.time() - 13 * DAY), today])
        await self.server.close()
        await self.start(log_days=0)
        await self.connect("Beta", b"q" * 16)
        await asyncio.sleep(0.05)
        self.assertNotIn("Beta", (self.dir / "logs" / today).read_text())
        self.assertEqual(PlayersLog(self.dir, 0).prune(time.time()), 2)

    async def test_forget_takes_a_player_out_of_every_record_but_a_ban(self):
        await self.start()
        _, alpha = await self.connect("Alpha", b"a" * 16)
        _, beta = await self.connect("Beta", b"b" * 16)
        await asyncio.sleep(0.05)
        await self.server.close()
        self.server = None
        jsonl(self.dir / "trades.jsonl", [{"time": NOW, "a": alpha, "b": beta, "a_gave": [1, 0], "b_gave": [2, 0]}])
        jsonl(self.dir / "weeks.jsonl", [dive(alpha, 2963, 5, NOW), dive(beta, 2963, 4, NOW, "Beta")])
        admin.perform("kick", self.dir, alpha, minutes=60)
        admin.perform("hide", self.dir, alpha, week=2963)
        admin.perform("ban", self.dir, alpha, "harassment")
        result = admin.perform("forget", self.dir, alpha)
        self.assertEqual({key: result[key] for key in ("trades", "board", "logs", "moderation")},
                         {"trades": 1, "board": 1, "logs": 1, "moderation": 2})
        trade = lines(self.dir / "trades.jsonl")[0]
        self.assertEqual((trade["a"], trade["b"], trade["a_gave"]), ("", beta, [1, 0]))
        self.assertEqual([entry["identity"] for entry in lines(self.dir / "weeks.jsonl")], [beta])
        everything = "".join(path.read_text() for path in self.dir.rglob("*") if path.is_file() and path.suffix != ".key")
        rules = read_json(self.dir / "moderation.json", {})
        self.assertEqual(everything.count(alpha), 1)       # (the ban alone)
        self.assertIn(alpha, rules["bans"])
        self.assertIn(beta, everything)


class BackupTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix="cyberworld-backup-")
        root = Path(self.temporary.name)
        self.state, self.backups = root / "state", root / "backups"
        self.state.mkdir()
        self.backups.mkdir()
        (self.state / "identity.key").write_bytes(bytes(range(32)))
        write_json(self.state / "moderation.json", {"bans": {ALPHA: {"time": 1, "reason": ""}}})
        jsonl(self.state / "weeks.jsonl", [dive(ALPHA, 2963, 5, NOW)])
        jsonl(self.state / "trades.jsonl", [{"time": NOW, "a": ALPHA, "b": BETA}])
        (self.state / "players.json").write_text("{}")

    def tearDown(self):
        self.temporary.cleanup()

    def files(self):
        return {name: (self.state / name).read_bytes() for name in backup.FILES if (self.state / name).exists()}

    def test_an_archive_holds_the_key_the_moderation_and_the_board_alone(self):
        path = backup.archive(self.state, self.backups, NOW)
        self.assertEqual(path.name, "cyberworld-state-20261022-000000.tar.gz")
        with tarfile.open(path) as tar:
            self.assertEqual(sorted(tar.getnames()), sorted(backup.FILES))
        self.assertEqual(backup.read(path), self.files())
        self.assertEqual([p.name for p in self.backups.iterdir()], [path.name])
        (self.state / "identity.key").unlink()
        with self.assertRaises(ValueError):
            backup.archive(self.state, self.backups, NOW)

    def test_one_archive_a_day_kept_its_days(self):
        slept = []

        def sleep(seconds):
            slept.append(seconds)
            clock.now += seconds
            if len(slept) == 24 * 20:
                raise StopIteration

        class clock:
            now = NOW + 1800
        with self.assertRaises(StopIteration):
            backup.run(self.state, self.backups, 14, clock=lambda: clock.now, sleep=sleep)
        names = sorted(p.name for p in self.backups.iterdir())
        self.assertEqual(len(names), 14)
        self.assertEqual((names[0], names[-1]), ("cyberworld-state-20261028-003000.tar.gz",
                                                 "cyberworld-state-20261110-003000.tar.gz"))
        self.assertEqual(set(slept), {backup.LOOK})
        # (with once, one now whatever the day has)
        path = backup.run(self.state, self.backups, 14, once=True, clock=lambda: clock.now)
        self.assertTrue(path.exists())
        self.assertEqual(len(list(self.backups.iterdir())), 15)

    def test_a_failed_backup_is_tried_again_a_minute_later(self):
        (self.state / "identity.key").unlink()
        waits = []

        def sleep(seconds):
            waits.append(seconds)
            if len(waits) == 1:
                (self.state / "identity.key").write_bytes(bytes(range(32)))
            else:
                raise StopIteration
        with self.assertLogs("cyberworld.server", "ERROR"), self.assertRaises(StopIteration):
            backup.run(self.state, self.backups, 14, clock=lambda: NOW, sleep=sleep)
        self.assertEqual(waits, [backup.RETRY, backup.LOOK])
        self.assertEqual(len(list(self.backups.iterdir())), 1)

    def test_a_restore_puts_the_state_back_as_it_was(self):
        before = self.files()
        path = backup.archive(self.state, self.backups, NOW)
        (self.state / "identity.key").write_bytes(b"x" * 32)
        write_json(self.state / "moderation.json", {})
        (self.state / "weeks.jsonl").unlink()
        trades = (self.state / "trades.jsonl").read_bytes()
        self.assertEqual(backup.restore(path, self.state), sorted(backup.FILES))
        self.assertEqual(self.files(), before)
        self.assertEqual((self.state / "trades.jsonl").read_bytes(), trades)
        self.assertEqual((self.state / "identity.key").stat().st_mode & 0o777, 0o600)
        # (a file the archive lacks was not there when it was made: it goes)
        (self.state / "weeks.jsonl").unlink()
        sparse = backup.archive(self.state, self.backups, NOW + 60)
        backup.restore(path, self.state)
        backup.restore(sparse, self.state)
        self.assertFalse((self.state / "weeks.jsonl").exists())

    def test_a_restore_refuses_what_is_not_a_backup(self):
        def made(members):
            path = self.backups / "made.tar.gz"
            with tarfile.open(path, "w:gz") as tar:
                for info, content in members:
                    tar.addfile(info, io.BytesIO(content) if content is not None else None)
            return path

        def member(name, content=b"", kind=tarfile.REGTYPE):
            info = tarfile.TarInfo(name)
            info.type, info.size = kind, len(content or b"")
            return info, content
        key = bytes(range(32))
        link = member("moderation.json", None, tarfile.SYMTYPE)
        link[0].linkname = "/etc/passwd"
        for members in ([member("moderation.json", b"{}")], [member("identity.key", b"short")],
                        [member("identity.key", key), link],
                        [member("identity.key", key), member("identity.key", key)]):
            with self.assertRaises(ValueError):
                backup.restore(made(members), self.state)
        (self.backups / "plain.tar.gz").write_bytes(b"not an archive")
        with self.assertRaises(ValueError):
            backup.restore(self.backups / "plain.tar.gz", self.state)
        self.assertEqual((self.state / "identity.key").read_bytes(), key)
        # (members past the state's files are never extracted, a "./" is taken off)
        backup.restore(made([member("./identity.key", key), member("../escape", b"x"), member("logs/a", b"x")]),
                       self.state)
        self.assertFalse((self.state.parent / "escape").exists())
        self.assertFalse((self.state / "logs").exists())


if __name__ == "__main__":
    unittest.main()
