"""Phase 1 rooms, public cards, private identity, and local moderation."""

import asyncio
import json
from pathlib import Path
import struct
import tempfile
import time
import unittest

from test_server import Client, protocol
from cyberworld_server import admin, protocol
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
        self.assertEqual(len(welcome), protocol.SPEC["welcome_bytes"])
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

    @staticmethod
    def social(kind, choice, target=0):
        return struct.pack("!BBBI", protocol.SPEC["social"], kind, choice, target)

    async def test_quick_phrases_and_emotes_are_bounded_and_rate_limited(self):
        first, welcome = await self.connect()
        second, _ = await self.connect("Two")
        await first.send(self.social(0, 7))
        event = await second.message(protocol.SPEC["event"])
        self.assertEqual(len(event), protocol.SPEC["event_bytes"])
        self.assertEqual(event[:7], bytes((protocol.SPEC["event"],)) + welcome[5:9] + b"\x00\x07")
        self.assertEqual(event[7:12], bytes(5))   # (aimed at nobody, no flags)
        # (a burst of social_burst, then one back a second: v14, issue #176)
        for _ in range(protocol.SPEC["social_burst"] - 1):
            await first.send(self.social(1, 5))
        await first.send(self.social(0, 0))
        self.assertEqual((await first.message(protocol.SPEC["notice"]))[1:3], b"\x00\x0c")
        await first.send(self.social(0, 8))
        self.assertEqual((await first.message(protocol.SPEC["error"]))[0], protocol.SPEC["error"])

    async def test_every_emote_id_and_only_warm_ones_aimed(self):
        first, welcome = await self.connect()
        second, second_welcome = await self.connect("Two")
        for choice in range(protocol.SPEC["emote_count"]):
            self.server.players[struct.unpack("!I", welcome[5:9])[0]].social_tokens = 3
            await first.send(self.social(1, choice))
            self.assertEqual((await second.message(protocol.SPEC["event"]))[6], choice)
        # (Laugh, id 2, is never aimed: it could taunt)
        await first.send(self.social(1, 2, struct.unpack("!I", second_welcome[5:9])[0]))
        self.assertEqual((await first.message(protocol.SPEC["error"]))[0], protocol.SPEC["error"])

    async def test_beat_lies_ahead_on_the_servers_step(self):
        first, _ = await self.connect()
        second, _ = await self.connect("Two")
        await first.send(self.social(1, 1))
        event = await second.message(protocol.SPEC["event"])
        beat = struct.unpack("!H", event[12:14])[0]
        self.assertGreaterEqual(beat, protocol.SPEC["beat_lead_ms"] - 5)
        self.assertLessEqual(beat, protocol.SPEC["beat_lead_ms"] + protocol.SPEC["beat_ms"] + 5)
        # (the beat itself is a whole step of the server's clock)
        rules = self.server.social_rules
        for now in (10.0, 10.123, 10.249, 11.5):
            ahead = rules.beat(1, now)
            self.assertEqual(round(now * 1000 + ahead) % protocol.SPEC["beat_ms"], 0)
            self.assertGreaterEqual(ahead, protocol.SPEC["beat_lead_ms"])

    async def test_wave_answered_and_the_cap_on_one_target(self):
        first, welcome = await self.connect()
        second, second_welcome = await self.connect("Two")
        one, two = (struct.unpack("!I", w[5:9])[0] for w in (welcome, second_welcome))
        # a wave aimed at Two: both see it aimed
        await first.send(self.social(1, 0, two))
        event = await second.message(protocol.SPEC["event"])
        self.assertEqual(struct.unpack("!I", event[7:11])[0], two)
        self.assertEqual(event[11], 0)
        await first.message(protocol.SPEC["event"])
        # Two waves back within five seconds: answered, to both
        await second.send(self.social(1, 0, one))
        answer = await first.message(protocol.SPEC["event"])
        self.assertEqual(struct.unpack("!I", answer[1:5])[0], two)
        self.assertEqual(answer[11], protocol.SPEC["event_answered"])
        await second.message(protocol.SPEC["event"])
        # unanswered, One's aimed waves past the cap are shown to him alone
        for k in range(protocol.SPEC["aim_cap"] + 1):
            self.server.players[one].social_tokens = 3
            await first.send(self.social(1, 5, two))
            self.assertEqual((await first.message(protocol.SPEC["event"]))[6], 5)
        for k in range(protocol.SPEC["aim_cap"]):
            await second.message(protocol.SPEC["event"])
        await first.send(self.social(0, 1))
        self.assertEqual((await second.message(protocol.SPEC["event"]))[5:7], b"\x00\x01")
        await first.message(protocol.SPEC["event"])
        # an answer long after counts for nothing
        self.server.social_rules.aims[(one, two)] = [time.monotonic() - 10]
        await second.send(self.social(1, 0, one))
        late = await first.message(protocol.SPEC["event"])
        self.assertEqual((struct.unpack("!I", late[7:11])[0], late[11]), (one, 0))
        # aimed at nobody there: to all, aimed at none
        self.server.players[two].social_tokens = 3
        await second.send(self.social(1, 1, 0xdead))
        self.assertEqual(struct.unpack("!I", (await first.message(protocol.SPEC["event"]))[7:11])[0], 0)

    async def test_admin_mute_kick_ban_and_persistence(self):
        key = b"m" * 16
        first, welcome = await self.connect(key=key)
        identity = welcome[14:30].hex()
        admin.perform("mute", self.directory, identity, "Test moderation")
        await self.server.moderation()
        await first.send(self.social(0, 0))
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
            self.assertEqual(len(snapshot), 2 + 7 * protocol.SPEC["peer_bytes"])
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
