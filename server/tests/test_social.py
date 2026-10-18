"""Phase 1 rooms, public cards, private identity, and local moderation."""

import asyncio
import json
from pathlib import Path
import struct
import tempfile
import time
import unittest

from test_server import Client, protocol
from cyberworld_server import admin
from cyberworld_server.app import Server
from cyberworld_server.identity import IdentityStore, public_id


class SocialTests(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix="cyberworld-social-")
        self.directory = Path(self.temporary.name)
        self.server = Server(seed=42, capacity=2, player_cap=4, state_dir=self.directory)
        self.port = await self.server.start("127.0.0.1", 0)
        self.clients = []

    async def asyncTearDown(self):
        for client in self.clients:
            await client.close()
        await self.server.close()
        self.temporary.cleanup()

    async def connect(self, name="Lan", **options):
        client = Client()
        self.clients.append(client)
        return client, await client.open(self.port, name, **options)

    async def test_stable_private_identity_across_reconnects(self):
        key = bytes(range(16))
        first, welcome = await self.connect(key=key)
        identity = welcome[14:30]
        self.assertEqual(len(welcome), 32)
        self.assertNotEqual(identity, key)
        await first.close()
        self.clients.remove(first)
        await asyncio.sleep(0.02)
        _, again = await self.connect(key=key)
        self.assertEqual(again[14:30], identity)
        self.assertEqual(IdentityStore(self.directory).identity(key), identity)
        self.assertNotEqual(IdentityStore().identity(key), identity)
        self.assertNotIn(key.hex(), json.dumps(admin.perform("list", self.directory)))

    async def test_channels_partition_and_switch_without_disconnect(self):
        first, welcome = await self.connect("One")
        second, _ = await self.connect("Two")
        third, third_welcome = await self.connect("Three")
        self.assertEqual(struct.unpack("!H", welcome[12:14])[0], 1)
        self.assertEqual(struct.unpack("!H", third_welcome[12:14])[0], 2)
        self.assertEqual((await first.message(protocol.SPEC["snapshot"]))[1], 1)
        self.assertEqual((await third.message(protocol.SPEC["snapshot"]))[1], 0)
        await third.send(struct.pack("!BH", protocol.SPEC["channel"], 1))
        self.assertEqual((await third.message(protocol.SPEC["notice"]))[1:3], b"\x00\x0a")
        await second.send(struct.pack("!BH", protocol.SPEC["channel"], 2))
        changed = await second.message(protocol.SPEC["welcome"])
        self.assertEqual(struct.unpack("!H", changed[12:14])[0], 2)
        await asyncio.sleep(0.06)
        self.assertEqual((await third.message(protocol.SPEC["snapshot"]))[1], 1)

    async def test_inspect_only_public_profile_in_same_channel(self):
        first, welcome = await self.connect()
        second, _ = await self.connect("MegaMan")
        profile = protocol.PROFILE.pack(87, 600, 3, 4, 5, 63, 31, 123, 20, 200, 0xffff, bytes(range(7)) * 7)
        await first.send(bytes((protocol.SPEC["profile"],)) + profile)
        await second.send(bytes((protocol.SPEC["inspect"],)) + welcome[5:9])
        card = await second.message(protocol.SPEC["card"])
        self.assertEqual(card, bytes((protocol.SPEC["card"],)) + welcome[5:9] + welcome[14:30] + profile)
        # Rapid back/reopen must produce a fresh card response, not silently
        # strand the client waiting for its next card serial.
        request = bytes((protocol.SPEC["inspect"],)) + welcome[5:9]
        await second.send(request)
        await second.send(request)
        self.assertEqual(await second.message(protocol.SPEC["card"]), card)
        self.assertEqual(await second.message(protocol.SPEC["card"]), card)
        await first.send(struct.pack("!BH", protocol.SPEC["channel"], 2))
        await first.message(protocol.SPEC["welcome"])
        await second.send(bytes((protocol.SPEC["inspect"],)) + welcome[5:9])
        self.assertEqual((await second.message(protocol.SPEC["notice"]))[1:3], b"\x00\x0d")

    async def test_quick_phrases_and_emotes_are_bounded_and_rate_limited(self):
        first, welcome = await self.connect()
        second, _ = await self.connect("Two")
        await first.send(bytes((protocol.SPEC["social"], 0, 7)))
        event = await second.message(protocol.SPEC["event"])
        self.assertEqual(event, bytes((protocol.SPEC["event"],)) + welcome[5:9] + b"\x00\x07")
        await first.send(bytes((protocol.SPEC["social"], 1, 5)))
        await first.send(bytes((protocol.SPEC["social"], 0, 0)))
        self.assertEqual((await first.message(protocol.SPEC["notice"]))[1:3], b"\x00\x0c")
        await first.send(bytes((protocol.SPEC["social"], 0, 8)))
        self.assertEqual((await first.message(protocol.SPEC["error"]))[0], protocol.SPEC["error"])

    async def test_admin_mute_kick_ban_and_persistence(self):
        key = b"m" * 16
        first, welcome = await self.connect(key=key)
        identity = welcome[14:30].hex()
        admin.perform("mute", self.directory, identity, "Test moderation")
        await self.server.moderation()
        await first.send(bytes((protocol.SPEC["social"], 0, 0)))
        self.assertEqual((await first.message(protocol.SPEC["notice"]))[1:3], b"\x00\x0b")
        admin.perform("unmute", self.directory, identity)
        admin.perform("kick", self.directory, identity)
        await self.server.moderation()
        self.assertEqual((await first.message(protocol.SPEC["error"]))[1:3], b"\x00\x0e")
        await asyncio.sleep(0.02)
        _, refused = await self.connect(key=key)
        self.assertEqual(refused[:3], bytes((protocol.SPEC["error"], 0, 14)))
        self.assertIn(b"try again in 10 min", refused)
        admin.perform("unkick", self.directory, identity)
        second, again = await self.connect(key=key)
        self.assertEqual(again[0], protocol.SPEC["welcome"])
        admin.perform("ban", self.directory, identity)
        await self.server.moderation()
        await second.message(protocol.SPEC["error"])
        _, response = await self.connect(key=key)
        self.assertEqual(response[:3], bytes((protocol.SPEC["error"], 0, 10)))
        self.assertTrue(IdentityStore(self.directory).contains("bans", bytes.fromhex(identity)))
        admin.perform("unban", self.directory, identity)
        await asyncio.sleep(0.02)
        _, allowed = await self.connect(key=key)
        self.assertEqual(allowed[0], protocol.SPEC["welcome"])

    async def test_health_contains_no_player_identity_or_key(self):
        _, welcome = await self.connect("PrivateName", key=b"z" * 16)
        reader, writer = await asyncio.open_connection("127.0.0.1", self.port)
        writer.write(b"GET /health HTTP/1.1\r\nHost: localhost\r\n\r\n")
        await writer.drain()
        response = await reader.read()
        self.assertTrue(response.startswith(b"HTTP/1.1 200"))
        body = json.loads(response.split(b"\r\n\r\n", 1)[1])
        self.assertEqual(body["players"], 1)
        self.assertFalse(body["rules"]["free_text"])
        self.assertNotIn(b"PrivateName", response)
        self.assertNotIn(welcome[14:30].hex().encode(), response)
        writer.close()
        await writer.wait_closed()

    async def test_sixteen_clients_across_two_full_channels(self):
        await self.server.close()
        self.server = Server(capacity=8, player_cap=16)
        self.port = await self.server.start("127.0.0.1", 0)
        welcomes = []
        for index in range(16):
            _, welcome = await self.connect("Player" + str(index))
            welcomes.append(welcome)
        self.assertEqual([struct.unpack("!H", data[12:14])[0] for data in welcomes], [1] * 8 + [2] * 8)
        await asyncio.sleep(0.06)
        for client in self.clients:
            snapshot = await client.message(protocol.SPEC["snapshot"])
            while snapshot[1] != 7:
                snapshot = await client.message(protocol.SPEC["snapshot"])
            self.assertEqual(len(snapshot), 2 + 7 * 49)
        _, full = await self.connect("TooMany")
        self.assertEqual(full[:3], bytes((protocol.SPEC["error"], 0, 7)))


class ProfileTests(unittest.TestCase):
    def test_moderation_id_is_canonical(self):
        for invalid in ("0" * 31, " " + "0" * 30 + " ", "z" * 32):
            with self.assertRaises(ValueError):
                public_id(invalid)
        self.assertEqual(public_id("AB" * 16), "ab" * 16)

    def test_rejects_every_public_profile_boundary(self):
        base = [100, 100, 1, 1, 1, 0, 0, 0, 0, 0, 0, bytes(49)]
        for index, value in ((0, 101), (1, 0), (1, 10000), (2, 6), (3, 6), (4, 6),
                             (5, 64), (6, 32), (10, 65536), (11, bytes([7]) + bytes(48))):
            changed = list(base)
            changed[index] = value
            with self.subTest(index=index, value=value), self.assertRaises(protocol.ProtocolError):
                protocol.read_profile(bytes((protocol.SPEC["profile"],)) + protocol.PROFILE.pack(*changed))

    def test_zero_identity_and_invalid_emotes(self):
        with self.assertRaises(protocol.ProtocolError):
            protocol.read_hello(protocol.hello("Lan", key=bytes(16)))
        for data in (b"", b"\x0b\x02\x00", b"\x0b\x01\x06", b"\x0b\x00\x00x"):
            with self.assertRaises(protocol.ProtocolError):
                protocol.read_social(data)


if __name__ == "__main__":
    unittest.main()
