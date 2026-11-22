"""Regressions: a bad moderation file, idle sockets, housekeeping on the loop,
a reconnect with the same profile, and kicks that never expired."""

import asyncio
import json
from pathlib import Path
import struct
import tempfile
import threading
import time
import unittest

from test_server import Client, protocol
from cyberworld_server import admin
from cyberworld_server.app import Server
from cyberworld_server.identity import read_json, write_json


async def until(condition, timeout=2.0):
    deadline = time.monotonic() + timeout
    while not condition():
        if time.monotonic() > deadline:
            raise AssertionError("Condition not reached")
        await asyncio.sleep(0.01)


class HardeningTests(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix="cyberworld-hardening-")
        self.directory = Path(self.temporary.name)
        self.server = None
        self.clients = []
        self.sockets = []

    async def asyncTearDown(self):
        for client in self.clients:
            client.writer.close()
        for _, writer in self.sockets:
            writer.close()
        if self.server:
            await self.server.close()
        self.temporary.cleanup()

    async def start(self, **options):
        options = dict(dict(seed=7, capacity=2, player_cap=4, state_dir=self.directory), **options)
        self.server = Server(**options)
        self.port = await self.server.start("127.0.0.1", 0)
        return self.server

    async def connect(self, name="Lan", **options):
        client = Client()
        self.clients.append(client)
        return client, await client.open(self.port, name, **options)

    async def idle(self):
        connection = await asyncio.open_connection("127.0.0.1", self.port)
        self.sockets.append(connection)
        return connection

    async def test_bad_moderation_file_keeps_the_last_rules_and_admits(self):
        server = await self.start()
        banned = b"b" * 16
        admin.perform("ban", self.directory, server.store.identity(banned).hex())
        await server.moderation()
        (self.directory / "moderation.json").write_text("{not json", encoding="utf-8")
        with self.assertLogs("cyberworld.server", "ERROR") as logs:
            _, welcome = await self.connect("Fine")
            self.assertEqual(welcome[0], protocol.SPEC["welcome"])
            _, refused = await self.connect("Banned", key=banned)
            self.assertEqual(refused[:3], bytes((protocol.SPEC["error"], 0, 10)))
            await server.moderation()
            await server.moderation()
            _, again = await self.connect("Again")
            self.assertEqual(again[0], protocol.SPEC["welcome"])
        self.assertEqual(sum("moderation.json not loaded" in line for line in logs.output), 1)
        # The operator mends the file by hand (the admin CLI will not rewrite
        # a file it cannot read); the mended rules are read again.
        write_json(self.directory / "moderation.json", {"bans": {}, "mutes": {}, "kicks": {}})
        _, admitted = await self.connect("Banned", key=banned)
        self.assertEqual(admitted[0], protocol.SPEC["welcome"])

    def test_bad_moderation_file_at_startup_refuses_the_start(self):
        (self.directory / "moderation.json").write_text("[]", encoding="utf-8")
        with self.assertRaises(ValueError):
            Server(state_dir=self.directory)

    async def test_idle_sockets_are_dropped_and_hold_no_place(self):
        server = await self.start(player_cap=2, pending_cap=4, handshake_timeout=0.3)
        flood = [await self.idle() for _ in range(20)]
        started = time.monotonic()
        for reader, _ in flood:
            try:
                self.assertEqual(await asyncio.wait_for(reader.read(), 2), b"")
            except ConnectionResetError:
                pass
        self.assertLess(time.monotonic() - started, 1.5)
        await until(lambda: not server.pending)
        # An unfinished handshake is not a player: both places stay open.
        await self.idle()
        await until(lambda: len(server.pending) == 1)
        _, one = await self.connect("One")
        _, two = await self.connect("Two")
        self.assertEqual((one[0], two[0]), (protocol.SPEC["welcome"],) * 2)
        _, full = await self.connect("Three")
        self.assertEqual(full[:3], bytes((protocol.SPEC["error"], 0, 7)))

    async def test_roster_written_only_when_changed_and_off_the_loop(self):
        server = await self.start()
        server.housekeeping = 0.02
        writes = []
        original = server.store.roster

        def recorded(entries):
            writes.append((threading.get_ident(), [dict(entry) for entry in entries]))
            original(entries)
        server.store.roster = recorded
        loop_thread = threading.get_ident()
        client, _ = await self.connect("One")
        await until(lambda: writes)
        await asyncio.sleep(0.2)
        self.assertEqual(len(writes), 1)
        self.assertNotEqual(writes[0][0], loop_thread)
        self.assertEqual([entry["name"] for entry in writes[0][1]], ["One"])
        await client.send(struct.pack("!BH", protocol.SPEC["channel"], 2))
        await client.message(protocol.SPEC["welcome"])
        await until(lambda: len(writes) == 2)
        await asyncio.sleep(0.2)
        self.assertEqual(len(writes), 2)
        self.assertEqual(writes[1][1][0]["channel"], 2)
        self.assertEqual(read_json(self.directory / "players.json", {})["players"][0]["channel"], 2)

    async def test_moderation_read_again_only_when_its_file_changed(self):
        server = await self.start()
        server.housekeeping = 0.02
        _, welcome = await self.connect("One")
        loads = server.store.loads
        await asyncio.sleep(0.2)
        self.assertEqual(server.store.loads, loads)
        admin.perform("mute", self.directory, welcome[14:30].hex())
        await until(lambda: server.store.contains("mutes", welcome[14:30]))
        await asyncio.sleep(0.2)
        self.assertEqual(server.store.loads, loads + 1)

    async def test_reconnect_replaces_the_old_session(self):
        server = await self.start(capacity=2, player_cap=2)
        key = b"r" * 16
        old, first = await self.connect("Lan", key=key)
        await old.send(struct.pack("!BH", protocol.SPEC["channel"], 1))
        await self.connect("Other")
        # A full square still takes the profile back: it hands its place on.
        new, second = await self.connect("Lan", key=key)
        self.assertEqual(second[0], protocol.SPEC["welcome"])
        self.assertEqual(second[14:30], first[14:30])
        self.assertNotEqual(second[5:9], first[5:9])
        self.assertEqual(second[12:14], first[12:14])
        error = await old.message(protocol.SPEC["error"])
        self.assertEqual(error[1:3], b"\x00\x0c")
        self.assertIn(b"connected again", error)
        close = await old.receive()
        self.assertEqual(struct.unpack("!H", close[:2])[0], 1000)
        self.assertIn(b"connected again", close[2:])
        self.assertEqual(await asyncio.wait_for(old.reader.read(), 2), b"")
        self.assertEqual(len(server.players), 2)
        self.assertNotIn(struct.unpack("!I", first[5:9])[0], server.players)
        await asyncio.sleep(0.05)
        self.assertEqual(len(server.players), 2)
        await new.send(bytes((protocol.SPEC["ping"],)) + b"1234")
        self.assertEqual(await new.message(protocol.SPEC["pong"]), bytes((protocol.SPEC["pong"],)) + b"1234")

    async def test_kicks_expire_and_are_pruned(self):
        server = await self.start()
        key = b"k" * 16
        identity = server.store.identity(key).hex()
        with self.assertRaises(ValueError):
            admin.perform("kick", self.directory, identity, minutes=0)
        with self.assertRaises(ValueError):
            admin.perform("ban", self.directory, identity, minutes=5)
        before = time.time()
        result = admin.perform("kick", self.directory, identity, minutes=3)
        self.assertAlmostEqual(result["expires"], before + 180, delta=5)
        _, refused = await self.connect(key=key)
        self.assertEqual(refused[:3], bytes((protocol.SPEC["error"], 0, 14)))
        self.assertIn(b"try again in 3 min", refused)
        # Time passes: the kick ends by itself, and the next edit prunes it.
        rules = read_json(self.directory / "moderation.json", {})
        rules["kicks"][identity]["expires"] = time.time() - 1
        # A kick written before kicks expired lasts the default ten minutes.
        older = "0" * 32
        rules["kicks"][older] = {"time": time.time() - 3600, "reason": ""}
        write_json(self.directory / "moderation.json", rules)
        _, welcome = await self.connect(key=key)
        self.assertEqual(welcome[0], protocol.SPEC["welcome"])
        self.assertEqual(admin.perform("list", self.directory)["moderation"]["kicks"], {})
        admin.perform("mute", self.directory, "1" * 32)
        self.assertEqual(json.loads((self.directory / "moderation.json").read_text())["kicks"], {})
        # Bans stay until lifted.
        admin.perform("ban", self.directory, identity)
        rules = read_json(self.directory / "moderation.json", {})
        self.assertNotIn("expires", rules["bans"][identity])


if __name__ == "__main__":
    unittest.main()
