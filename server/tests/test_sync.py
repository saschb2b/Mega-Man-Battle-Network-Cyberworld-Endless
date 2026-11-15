"""Saves kept in step (shared/online/README.md, Saves kept in step;
sync.py): pull, push and drop over real sockets, the slots on disk, their
bounds, retention, backups, forget and the daily totals; no ROM needed."""

import asyncio
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tarfile
import tempfile
import time
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "server"))
sys.path.insert(0, str(Path(__file__).resolve().parent))

from cyberworld_server import admin, backup, protocol, sync, week
from cyberworld_server.app import Server
from cyberworld_server.retention import DAY, Retention
from cyberworld_server.sync import Slots
from test_server import Client
from test_stats import get
from test_transfer import TransferClient

SPEC = protocol.SPEC
SLOT, OTHER, THIRD = bytes(range(16)), bytes(range(16, 32)), bytes(range(32, 48))
AUTH, WRONG = bytes(range(32)), bytes(range(1, 33))
NOW = week.week_start(2963) + 3 * DAY + 3600      # Thursday 22 October 2026, 01:00 UTC
PULL, PUSH, DROP = SPEC["sync_pull"], SPEC["sync_push"], SPEC["sync_drop"]


def ask(op, slot=SLOT, generation=0, size=0, auth=AUTH, version=None):
    return protocol.SYNC.pack(SPEC["sync"], SPEC["sync_version"] if version is None else version,
                              op, slot, auth, generation, size)


def state(name, generation=0, size=0):
    return protocol.sync_state(SPEC["sync_" + name], generation, size)


class Clock:
    """A monotonic clock the test moves."""

    def __init__(self):
        self.now = 1000.0

    def __call__(self):
        return self.now


class SyncTests(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix="cyberworld-sync-")
        self.dir = Path(self.temporary.name)
        self.now, self.monotonic = NOW, Clock()
        self.server = None
        await self.start()

    async def start(self, on=True, policy=None, **limits):
        """A server on the test's state directory, its own retention passes
        held off (the tests call retain), its slots on the test's clocks."""
        self.server = Server(seed=123456, capacity=2, player_cap=2, state_dir=self.dir, clock=lambda: self.now,
                             policy=policy, sync=on)
        self.server.retention_time = time.monotonic()
        self.server.slots = Slots(self.dir, on, self.server.daily.count, lambda: self.now, self.monotonic,
                                  days=self.server.retention.cloud_days, **limits)
        self.port = await self.server.start("127.0.0.1", 0)
        self.clients = []

    async def asyncTearDown(self):
        await self.stop()
        self.temporary.cleanup()

    async def stop(self):
        for client in self.clients:
            await client.drop()
        if self.server:
            await self.server.close()
        self.server = None

    async def restart(self, on=True, policy=None, **limits):
        await self.stop()
        await self.start(on, policy, **limits)

    async def open(self, first):
        """A connection whose first message is `first`; its answer."""
        client = TransferClient()
        self.clients.append(client)
        await client.upgrade(self.port)
        await client.send(first)
        return client, await client.message()

    async def answer(self, first, expected):
        """An exchange answered `expected` at once, then closed."""
        client, answer = await self.open(first)
        self.assertEqual(answer, expected)
        await client.closed()

    async def push(self, blob, generation=0, slot=SLOT, auth=AUTH):
        """A push that is taken: the slot's new generation."""
        client, answer = await self.open(ask(PUSH, slot, generation, len(blob), auth))
        self.assertEqual(answer, state("ready", generation))
        await client.data(blob)
        answer = await client.message()
        self.assertEqual(answer[:2], bytes((SPEC["sync_state"], SPEC["sync_stored"])))
        await client.closed()
        return protocol.SYNC_STATE.unpack(answer)[2]

    async def pull(self, generation=0, slot=SLOT, auth=AUTH):
        """A pull: (status's name, generation, blob or None)."""
        client, answer = await self.open(ask(PULL, slot, generation, 0, auth))
        _, status, got, size = protocol.SYNC_STATE.unpack(answer)
        name = next(name for name, value in sync.STATUS.items() if value == status)
        blob = await client.blob(size) if name == "found" else None
        await client.closed()
        return name, got, blob

    async def refused(self, client, answer=None):
        """An exchange ended with an ERROR and a close."""
        answer = answer or await client.message()
        self.assertEqual(answer[0], SPEC["error"])
        await client.closed(1008)

    def meta(self, slot=SLOT):
        return json.loads((self.dir / "cloud" / (slot.hex() + ".json")).read_text())

    async def test_push_pull_and_generations(self):
        first, second = os.urandom(1500), os.urandom(700)
        self.assertEqual(await self.push(first), 1)
        self.assertEqual(await self.pull(0), ("found", 1, first))
        self.assertEqual(await self.pull(1), ("same", 1, None))
        self.assertEqual(await self.push(second, 1), 2)
        await self.answer(ask(PUSH, SLOT, 1, 5), state("conflict", 2))
        # (generation 0 to a slot the server has: a CONFLICT like any other)
        await self.answer(ask(PUSH, SLOT, 0, 5), state("conflict", 2))
        self.assertEqual(await self.pull(1), ("found", 2, second))
        # (any other generation than the slot's, a newer one too, is FOUND)
        self.assertEqual(await self.pull(7), ("found", 2, second))
        # (on disk: the blob as sent, the auth's hash, never the auth)
        self.assertEqual((self.dir / "cloud" / (SLOT.hex() + ".blob")).read_bytes(), second)
        meta = self.meta()
        self.assertEqual(meta, {"auth": hashlib.sha256(AUTH).hexdigest(), "generation": 2, "size": 700,
                                "created": "2026-10-22", "used": "2026-10-22"})
        self.assertNotIn(AUTH.hex(), (self.dir / "cloud" / (SLOT.hex() + ".json")).read_text())
        # (no visit: no player, nothing pending, nothing under way, no join counted)
        today = (await get(self.port, "/stats"))[1]["today"]
        self.assertEqual((today["sync_pulls"], today["sync_pushes"], today["sync_slots"]), (4, 2, 1))
        self.assertEqual((self.server.players, self.server.pending, today["joins"]), ({}, set(), 0))
        self.assertEqual((self.server.slots.under_way, self.server.slots.filling), (0, {}))
        self.assertTrue((await get(self.port, "/health"))[1]["rules"]["sync"])

    async def test_the_largest_save(self):
        blob = os.urandom(SPEC["sync_max"])
        self.assertEqual(await self.push(blob), 1)
        self.assertEqual(await self.pull(), ("found", 1, blob))

    async def test_auth_and_slots_not_there(self):
        await self.push(b"saved")
        for first in (ask(PULL, auth=WRONG), ask(PUSH, generation=1, size=5, auth=WRONG), ask(DROP, auth=WRONG),
                      ask(PUSH, size=5, auth=WRONG)):
            await self.answer(first, state("denied"))
        for first in (ask(PULL, OTHER), ask(PULL, OTHER, 3), ask(PUSH, OTHER, 3, 5), ask(DROP, OTHER)):
            await self.answer(first, state("none"))
        self.assertEqual(sorted(path.name for path in (self.dir / "cloud").iterdir()),
                         [SLOT.hex() + ".blob", SLOT.hex() + ".json"])
        await self.answer(ask(DROP), state("dropped"))
        self.assertEqual(list((self.dir / "cloud").iterdir()), [])
        for first in (ask(PULL), ask(PUSH, generation=1, size=5), ask(DROP)):
            await self.answer(first, state("none"))
        # (made anew by a push from 0, the new auth its own from then on)
        self.assertEqual(await self.push(b"again", auth=WRONG), 1)
        await self.answer(ask(PULL), state("denied"))
        self.assertEqual(await self.pull(auth=WRONG), ("found", 1, b"again"))
        self.assertEqual(self.server.daily.snapshot()["sync_slots"], 1)

    async def test_malformed_exchanges_end_with_an_error(self):
        for first in (ask(PUSH, size=0), ask(PUSH, size=SPEC["sync_max"] + 1), ask(PUSH, size=5, version=2),
                      ask(9), ask(0), ask(PULL, size=5), ask(DROP, size=1), ask(PUSH, size=5)[:-1],
                      ask(PULL) + b"x", bytes((SPEC["sync"],))):
            client, answer = await self.open(first)
            await self.refused(client, answer)
        # (a DATA past the size, an empty one, and another message in a push)
        for data in (b"toolong", b"", None):
            client, answer = await self.open(ask(PUSH, size=5))
            self.assertEqual(answer, state("ready"))
            if data is None:
                await client.send(ask(PULL))
            else:
                await client.send(bytes((SPEC["transfer_data"],)) + data)
            await self.refused(client)
        self.assertFalse((self.dir / "cloud").exists())
        self.assertEqual((self.server.slots.filling, self.server.slots.under_way, self.server.slots.index), ({}, 0, {}))
        # (a push cut short leaves the slot as it was)
        await self.push(b"saved")
        client, answer = await self.open(ask(PUSH, generation=1, size=10))
        self.assertEqual(answer, state("ready", 1))
        await client.data(b"half")
        await client.drop()
        await asyncio.sleep(0.05)
        self.assertEqual(await self.pull(), ("found", 1, b"saved"))

    async def test_sync_off(self):
        await self.restart(on=False)
        for first in (ask(PULL), ask(PUSH, size=5), ask(DROP), ask(PUSH, size=5, version=2)):
            await self.answer(first, state("off"))
        self.assertFalse((await get(self.port, "/health"))[1]["rules"]["sync"])
        # (a server without a state directory keeps no slots either)
        self.assertFalse(Slots(None).on)
        self.assertFalse(Server().slots.on)

    async def test_a_second_push_to_a_slot_filling_is_busy(self):
        coming, answer = await self.open(ask(PUSH, size=5))
        self.assertEqual(answer, state("ready"))
        await self.answer(ask(PUSH, size=5), state("busy", 0, sync.AT_ONCE_WAIT))
        # (a pull and a drop of another slot go on)
        await self.answer(ask(PULL, OTHER), state("none"))
        await coming.data(b"first")
        self.assertEqual(await coming.message(), state("stored", 1))
        await coming.closed()
        coming, answer = await self.open(ask(PUSH, generation=1, size=6))
        self.assertEqual(answer, state("ready", 1))
        await self.answer(ask(PUSH, generation=1, size=5), state("busy", 0, sync.AT_ONCE_WAIT))
        # (a pull meanwhile has the slot as it is)
        self.assertEqual(await self.pull(), ("found", 1, b"first"))
        await coming.data(b"second")
        self.assertEqual(await coming.message(), state("stored", 2))

    async def test_slots_kept_at_most(self):
        await self.restart(slots=2)
        await self.push(b"a", slot=SLOT)
        await self.push(b"b", slot=OTHER)
        await self.answer(ask(PUSH, THIRD, size=1), state("busy", 0, sync.FULL_WAIT))
        # (a slot kept still takes its pushes; one dropped makes room)
        self.assertEqual(await self.push(b"aa", 1, SLOT), 2)
        await self.answer(ask(DROP, OTHER), state("dropped"))
        self.assertEqual(await self.push(b"c", slot=THIRD), 1)
        # (a slot being made counts among them)
        await self.answer(ask(DROP, THIRD), state("dropped"))
        coming, answer = await self.open(ask(PUSH, THIRD, size=1))
        self.assertEqual(answer, state("ready"))
        await self.answer(ask(PUSH, OTHER, size=1), state("busy", 0, sync.FULL_WAIT))
        await coming.data(b"c")
        self.assertEqual(await coming.message(), state("stored", 1))

    async def test_bytes_kept_at_most(self):
        self.server.slots.most_bytes = 100
        await self.push(b"a" * 60, slot=SLOT)
        await self.answer(ask(PUSH, OTHER, size=41), state("busy", 0, sync.FULL_WAIT))
        # (a push replaces its slot's bytes: 60 kept, 90 instead)
        self.assertEqual(await self.push(b"b" * 90, 1, SLOT), 2)
        # (a push under way reserves its bytes)
        coming, answer = await self.open(ask(PUSH, OTHER, size=10))
        self.assertEqual(answer, state("ready"))
        await self.answer(ask(PUSH, THIRD, size=1), state("busy", 0, sync.FULL_WAIT))
        await coming.data(b"c" * 10)
        self.assertEqual(await coming.message(), state("stored", 1))
        self.assertEqual(self.server.slots.held_bytes(), 100)

    async def test_slots_made_an_hour_are_bounded(self):
        await self.restart(per_hour=2)
        await self.push(b"a", slot=SLOT)
        self.monotonic.now += 1000
        await self.push(b"b", slot=OTHER)
        await self.answer(ask(PUSH, THIRD, size=1), state("busy", 0, 2600))
        # (a slot kept still takes its pushes, and is pulled)
        self.assertEqual(await self.push(b"aa", 1, SLOT), 2)
        self.monotonic.now += 2599
        await self.answer(ask(PUSH, THIRD, size=1), state("busy", 0, 1))
        self.monotonic.now += 1
        self.assertEqual(await self.push(b"c", slot=THIRD), 1)

    async def test_exchanges_at_once_are_bounded(self):
        await self.restart(at_once=2)
        first, answer = await self.open(ask(PUSH, SLOT, size=5))
        self.assertEqual(answer, state("ready"))
        second, answer = await self.open(ask(PUSH, OTHER, size=5))
        self.assertEqual(answer, state("ready"))
        for op in (PULL, PUSH, DROP):
            await self.answer(ask(op, THIRD, size=5 if op == PUSH else 0), state("busy", 0, sync.AT_ONCE_WAIT))
        for client in (first, second):
            await client.data(b"12345")
            self.assertEqual(await client.message(), state("stored", 1))
            await client.closed()
        await asyncio.sleep(0.05)
        self.assertEqual(await self.pull(slot=THIRD), ("none", 0, None))

    async def test_a_quiet_or_long_exchange_ends(self):
        await self.restart(quiet=0.2)
        client, answer = await self.open(ask(PUSH, size=5))
        self.assertEqual(answer, state("ready"))
        await self.refused(client)
        await self.restart(quiet=0.4, whole=0.5)
        loop = asyncio.get_running_loop()
        client, answer = await self.open(ask(PUSH, size=100))
        began = loop.time()
        self.assertEqual(answer, state("ready"))
        for _ in range(4):
            await asyncio.sleep(0.1)
            await client.data(b"x")
        await self.refused(client)
        self.assertLess(loop.time() - began, 0.75)
        self.assertEqual((self.server.slots.filling, self.server.slots.under_way), ({}, 0))
        self.assertFalse((self.dir / "cloud").exists())

    async def test_a_slot_outlives_a_restart(self):
        await self.push(b"kept")
        await self.restart()
        self.assertEqual(self.server.slots.index, {SLOT.hex(): 4})
        self.assertEqual(await self.pull(), ("found", 1, b"kept"))
        self.assertEqual(await self.push(b"kept again", 1), 2)
        # (and the day's totals with it, today's line taken up again)
        await self.restart()
        today = self.server.daily.snapshot()
        self.assertEqual((today["sync_pulls"], today["sync_pushes"], today["sync_slots"]), (1, 2, 1))

    async def test_a_slot_unused_for_its_days_goes(self):
        await self.restart(policy=Retention(cloud_days=3))
        await self.push(b"old", slot=SLOT)
        await self.push(b"used", slot=OTHER)
        # (leftovers of a write cut short: a blob without its meta, a temporary file)
        (self.dir / "cloud" / (THIRD.hex() + ".blob")).write_bytes(b"orphan")
        (self.dir / "cloud" / ".server-cut").write_bytes(b"half")
        self.now += 2 * DAY
        self.assertEqual(await self.pull(slot=OTHER), ("found", 1, b"used"))
        self.assertEqual(self.meta(OTHER)["used"], "2026-10-24")
        self.assertEqual(await self.server.retain(), {})
        self.assertEqual(sorted(path.name for path in (self.dir / "cloud").iterdir()),
                         sorted(slot.hex() + kind for slot in (SLOT, OTHER) for kind in (".blob", ".json")))
        # (a slot unused for its days is no slot, before the pass deletes it)
        self.now += DAY
        self.assertEqual(await self.pull(slot=SLOT), ("none", 0, None))
        self.assertEqual(await self.server.retain(), {"cloud": 1})
        self.assertEqual(sorted(path.name for path in (self.dir / "cloud").iterdir()),
                         [OTHER.hex() + ".blob", OTHER.hex() + ".json"])
        self.assertEqual(self.server.slots.index, {OTHER.hex(): 4})
        # (made anew by a push from 0)
        self.assertEqual(await self.push(b"new", slot=SLOT), 1)
        self.assertEqual(self.meta(SLOT)["created"], "2026-10-25")
        # (the days default to the contract's)
        self.assertEqual(Retention().cloud_days, SPEC["sync_days"])
        for wrong in (0, 3651, True):
            with self.assertRaises(ValueError):
                Retention(cloud_days=wrong)

    async def test_backups_leave_the_slots_out(self):
        await self.push(b"sealed")
        backups = self.dir.parent / (self.dir.name + "-backups")
        backups.mkdir()
        try:
            path = backup.archive(self.dir, backups, NOW)
            with tarfile.open(path) as tar:
                self.assertFalse([name for name in tar.getnames() if "cloud" in name])
            # (a restore neither brings a slot in nor takes one out)
            backup.restore(path, self.dir)
            self.assertEqual(await self.pull(), ("found", 1, b"sealed"))
        finally:
            for archive in backups.iterdir():
                archive.unlink()
            backups.rmdir()

    async def test_forget_a_slot(self):
        await self.push(b"sealed")
        result = admin.perform("forget", self.dir, cloud=SLOT.hex().upper())
        self.assertEqual(result, {"action": "forget", "cloud": SLOT.hex(), "slot": True})
        # (gone at once for the running server)
        self.assertEqual(await self.pull(), ("none", 0, None))
        self.assertEqual(admin.perform("forget", self.dir, cloud=SLOT.hex())["slot"], False)
        for wrong in ({"action": "forget", "cloud": "12"}, {"action": "forget", "cloud": "g" * 32},
                      {"action": "ban", "cloud": SLOT.hex()}):
            with self.assertRaises(ValueError):
                admin.perform(wrong["action"], self.dir, cloud=wrong["cloud"])
        # (the command line's --admin forget --cloud)
        await self.push(b"again", slot=OTHER)
        done = subprocess.run([sys.executable, str(ROOT / "server/main.py"), "--state-dir", str(self.dir),
                               "--admin", "forget", "--cloud", OTHER.hex()],
                              capture_output=True, text=True, timeout=20, check=True)
        self.assertEqual(json.loads(done.stdout)["slot"], True)
        self.assertEqual(await self.pull(slot=OTHER), ("none", 0, None))
        done = subprocess.run([sys.executable, str(ROOT / "server/admin.py"), "forget", "--state-dir", str(self.dir),
                               "--cloud", OTHER.hex()], capture_output=True, text=True, timeout=20, check=True)
        self.assertEqual(json.loads(done.stdout)["slot"], False)
        # (the count of slots catches up at the retention's pass)
        await self.server.retain()
        self.assertEqual(self.server.slots.kept(), 0)

    async def test_the_days_totals(self):
        await self.push(b"one")
        await self.push(b"two", 1)
        await self.answer(ask(PUSH, generation=1, size=3), state("conflict", 2))
        await self.pull(0)
        await self.pull(2)
        await self.answer(ask(PULL, auth=WRONG), state("denied"))
        await self.answer(ask(PULL, OTHER), state("none"))
        await self.push(b"other", slot=OTHER)
        today = self.server.daily.snapshot()
        self.assertEqual((today["sync_pulls"], today["sync_pushes"], today["sync_slots"]), (2, 3, 2))
        # (the day's end: written with the slots kept then)
        self.now += DAY
        ended = self.server.daily.rollover()
        self.assertEqual((ended["sync_pulls"], ended["sync_pushes"], ended["sync_slots"]), (2, 3, 2))
        self.server.daily.write(ended)
        line = json.loads((self.dir / "stats.jsonl").read_text().splitlines()[-1])
        self.assertEqual((line["sync_pulls"], line["sync_pushes"], line["sync_slots"]), (2, 3, 2))
        with self.assertRaises(ValueError):
            self.server.daily.count("sync_slots")

    async def test_a_visit_beside_a_sync(self):
        coming, answer = await self.open(ask(PUSH, size=4))
        self.assertEqual(answer, state("ready"))
        visitor = Client()
        self.clients.append(visitor)
        welcome = await visitor.open(self.port, "Lan")
        self.assertEqual(welcome[0], SPEC["welcome"])
        await coming.data(b"save")
        self.assertEqual(await coming.message(), state("stored", 1))
        self.assertEqual(len(self.server.players), 1)
        self.assertEqual(await self.pull(), ("found", 1, b"save"))


class SyncContractTests(unittest.TestCase):
    def test_sizes(self):
        self.assertEqual(protocol.SYNC.size, SPEC["sync_bytes"])
        self.assertEqual(protocol.SYNC_STATE.size, SPEC["sync_state_bytes"])
        self.assertEqual((len(SLOT), len(AUTH)), (SPEC["sync_slot_bytes"], SPEC["sync_auth_bytes"]))
        self.assertEqual(sync.CHUNK + 1, SPEC["max_message"])
        # (the bounds hold the largest save many times over)
        self.assertGreaterEqual(sync.MEGABYTES << 20, 1000 * SPEC["sync_max"])

    def test_the_bounds_are_checked(self):
        for wrong in ({"slots": 0}, {"megabytes": 0}, {"slots": True}, {"megabytes": "4096"}):
            with self.assertRaises(ValueError):
                Slots(None, **wrong)


if __name__ == "__main__":
    unittest.main()
