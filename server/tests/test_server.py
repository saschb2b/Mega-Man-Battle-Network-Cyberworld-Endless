"""Real socket tests of the server and the wire contract; no ROM needed."""

import asyncio
import base64
import hashlib
from pathlib import Path
import re
import struct
import sys
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "server"))

from cyberworld_server import protocol, websocket
from cyberworld_server.app import Server


def masked(data, opcode=2):
    mask = b"test"
    header = bytes((0x80 | opcode, 0x80 | len(data))) if len(data) < 126 else struct.pack("!BBH", 0x80 | opcode, 254, len(data))
    return header + mask + bytes(value ^ mask[i % 4] for i, value in enumerate(data))


class Client:
    async def open(self, port, name="Lan", protocols=None, **identity):
        self.reader, self.writer = await asyncio.open_connection("127.0.0.1", port)
        key = base64.b64encode(b"0123456789abcdef").decode()
        self.writer.write(("GET /presence HTTP/1.1\r\nHost: localhost\r\nUpgrade: websocket\r\n"
                           "Connection: Upgrade\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Key: " + key +
                           "\r\nSec-WebSocket-Protocol: " + (protocols or protocol.SPEC["subprotocol"]) + "\r\n\r\n").encode())
        response = await self.reader.readuntil(b"\r\n\r\n")
        assert response.startswith(b"HTTP/1.1 101")
        accept = base64.b64encode(hashlib.sha1((key + websocket.GUID).encode()).digest())
        assert accept in response
        self.protocol = next(line.split(":", 1)[1].strip() for line in response.decode().split("\r\n")
                             if line.lower().startswith("sec-websocket-protocol:"))
        await self.send(protocol.hello(name, **identity))
        return await self.receive()

    async def send(self, data):
        self.writer.write(masked(data))
        await self.writer.drain()

    async def receive(self):
        first, size = await asyncio.wait_for(self.reader.readexactly(2), 2)
        assert first in (0x82, 0x88, 0x8a) and not size & 0x80
        if size == 126:
            size = struct.unpack("!H", await self.reader.readexactly(2))[0]
        return await self.reader.readexactly(size)

    async def message(self, kind):
        for _ in range(100):
            message = await self.receive()
            if message and message[0] == kind:
                return message
        raise AssertionError("Message not received")

    async def close(self):
        """Leaving as the game does at the pad: BYE (protocol v10), then the
        socket closed; the place is freed at once."""
        if not self.writer.is_closing():
            try:
                self.writer.write(masked(bytes((protocol.SPEC["bye"],))))
                await self.writer.drain()
            except ConnectionError:
                pass
        await self.drop()

    async def drop(self):
        """The link gone without a word: the server holds the place (holds.py)."""
        self.writer.close()
        try:
            await self.writer.wait_closed()
        except ConnectionError:
            pass


class ServerTests(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        self.server = Server(seed=123456, capacity=2, player_cap=2)
        self.port = await self.server.start("127.0.0.1", 0)
        self.clients = []

    async def asyncTearDown(self):
        for client in self.clients:
            await client.close()
        await self.server.close()

    async def connect(self, name="Lan", **identity):
        client = Client()
        self.clients.append(client)
        return client, await client.open(self.port, name, **identity)

    async def test_two_clients_receive_movement_and_disconnect(self):
        first, welcome = await self.connect()
        second, other = await self.connect("MegaMan")
        self.assertEqual(welcome[0], protocol.SPEC["welcome"])
        self.assertEqual(welcome[1:5], other[1:5])
        self.assertNotEqual(welcome[5:9], other[5:9])
        await first.send(protocol.STATE.pack(protocol.SPEC["state"], 123, -234, 12, 7))
        # A ticker snapshot can already be queued before STATE is handled.
        # Observe the actual update instead of assuming host scheduling.
        for _ in range(30):
            message = await second.message(protocol.SPEC["snapshot"])
            if message[1] == 1 and protocol.PEER.unpack(message[2:])[2:6] == (123, -234, 12, 7):
                break
        else:
            self.fail("Peer movement was not delivered")
        self.assertEqual(message[1], 1)
        peer = protocol.PEER.unpack(message[2:])
        self.assertEqual(peer[0], struct.unpack("!I", welcome[5:9])[0])
        self.assertEqual(peer[1], welcome[14:30])
        self.assertEqual(peer[2:6], (123, -234, 12, 7))
        self.assertEqual(peer[6].rstrip(b"\0"), b"Lan")
        self.assertEqual(peer[7], 0)        # (its ROMs besides BN6's: none)
        await first.close()
        self.clients.remove(first)
        for _ in range(20):
            message = await second.message(protocol.SPEC["snapshot"])
            if message == bytes((protocol.SPEC["snapshot"], 0)):
                break
        self.assertEqual(message, bytes((protocol.SPEC["snapshot"], 0)))
        self.assertEqual(len(self.server.players), 1)

    async def test_rejects_incompatible_identity(self):
        # (v10: a game newer than the server told apart from an older one, so it says where the newer one is)
        for identity, code in ((dict(version=protocol.SPEC["version"] - 1), 2), (dict(version=protocol.SPEC["version"] + 1), 17),
                               (dict(square_make=protocol.SPEC["square_make"] + 1), 17), (dict(square_make=0), 3),
                               (dict(square_hash=protocol.SPEC["square_hash"] ^ 1), 3), (dict(rom_code="BAD!"), 4),
                               (dict(rom_sha1="00" * 20), 4), (dict(bn5=True, bn5_code="BRKP"), 15),
                               (dict(bn5=True, bn5_sha1="00" * 20), 15)):
            client, response = await self.connect(**identity)
            self.assertEqual(response[:3], struct.pack("!BH", protocol.SPEC["error"], code))
            await client.close()
            self.clients.remove(client)
        self.assertFalse(self.server.players)

    async def test_capacity_and_name_validation(self):
        _, response = await self.connect("bad<script>")
        self.assertEqual(response[:3], bytes((protocol.SPEC["error"], 0, 5)))
        await self.connect("One")
        await self.connect("Two")
        _, response = await self.connect("Three")
        self.assertEqual(response[:3], bytes((protocol.SPEC["error"], 0, 7)))
        self.assertEqual(len(self.server.players), 2)

    async def test_ping_round_trip_and_invalid_position(self):
        client, _ = await self.connect()
        await client.send(bytes((protocol.SPEC["ping"],)) + b"1234")
        self.assertEqual(await client.message(protocol.SPEC["pong"]), bytes((protocol.SPEC["pong"],)) + b"1234")
        await client.send(protocol.STATE.pack(protocol.SPEC["state"], 8193, 0, 0, 0))
        response = await client.message(protocol.SPEC["error"])
        self.assertEqual(response[:3], bytes((protocol.SPEC["error"], 0, 6)))

    async def test_rejects_unmasked_and_oversized_frames(self):
        for frame in (b"\x82\x01x", b"\x82\xfe\x02\x01", b"\x02\x80test", b"\x82\xff"):
            client, _ = await self.connect()
            client.writer.write(frame)
            await client.writer.drain()
            response = await client.message(protocol.SPEC["error"])
            self.assertEqual(response[0], protocol.SPEC["error"])
            await client.close()
            self.clients.remove(client)
            await asyncio.sleep(0.01)

    async def test_rate_limit(self):
        client, _ = await self.connect()
        client.writer.write(masked(bytes((protocol.SPEC["ping"],)) + b"0000") * 100)
        await client.writer.drain()
        response = await client.message(protocol.SPEC["error"])
        self.assertEqual(response[:3], bytes((protocol.SPEC["error"], 0, 8)))

    async def test_server_shutdown_closes_idle_players(self):
        client, _ = await self.connect()
        await asyncio.wait_for(self.server.close(), 1)
        await asyncio.wait_for(client.reader.read(), 1)
        self.assertTrue(client.reader.at_eof())
        self.assertFalse(self.server.players)

    async def test_origin_is_rejected_before_join(self):
        reader, writer = await asyncio.open_connection("127.0.0.1", self.port)
        writer.write(b"GET /presence HTTP/1.1\r\nHost: localhost\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n"
                     b"Sec-WebSocket-Version: 13\r\nSec-WebSocket-Key: MDEyMzQ1Njc4OWFiY2RlZg==\r\n"
                     b"Sec-WebSocket-Protocol: " + protocol.SPEC["subprotocol"].encode() + b"\r\nOrigin: https://untrusted.invalid\r\n\r\n")
        await writer.drain()
        self.assertTrue((await reader.read()).startswith(b"HTTP/1.1 400"))
        self.assertFalse(self.server.players)
        writer.close()
        await writer.wait_closed()


class ContractTests(unittest.TestCase):
    def test_shared_constants_and_game_generation_match(self):
        header = (ROOT / "shared/online/protocol.h").read_text()
        for name in ("hello", "welcome", "state", "snapshot", "error", "ping", "pong",
                     "max_players", "name_bytes", "max_message", "coordinate_limit", "profile", "inspect", "card",
                     "social", "event", "channel", "notice", "identity_bytes", "board_cells", "phrase_count",
                     "emote_count", "profile_bytes", "peer_bytes", "welcome_bytes", "trade", "trade_state", "arena",
                     "arena_state", "arena_loadout", "arena_input", "arena_hash", "chip_last", "code_last", "trade_bytes",
                     "trade_state_bytes", "arena_bytes", "arena_state_bytes", "arena_loadout_max", "arena_input_max", "bn6_chip_last", "hello_bytes", "roms_bn5", "roms_full", "notice_pass",
                     "stands", "arena_status", "watch", "watch_state", "watch_relay", "watch_keys", "stands_entry_bytes",
                     "stands_max", "arena_status_bytes", "watch_bytes", "watch_state_bytes", "watch_keys_head", "watch_keys_max",
                     "watch_most", "bye", "hello_resume", "error_maintenance", "error_server_older", "maintenance_bytes",
                     "busy_shift", "busy_battle", "busy_watch", "busy_trade", "link_shift", "link_good_ms", "link_fair_ms",
                     "ping_bytes", "arena_seen", "pending_unseen", "team", "team_state", "team_bytes", "team_state_bytes",
                     "notice_team", "arena_seat", "seats", "team_relay", "team_side_by_side"):
            value = int(re.search(r"#define ONLINE_" + name.upper() + r"\s+(\d+)", header)[1])
            self.assertEqual(value, protocol.SPEC[name])
        self.assertIn(f'#define ONLINE_PROTOCOL_VERSION {protocol.SPEC["version"]}', header)
        plan = (ROOT / "src/online/square_plan.h").read_text()
        self.assertEqual(int(re.search(r"#define SQUARE_MAKE (\d+)", plan)[1]), protocol.SPEC["square_make"])
        self.assertEqual(int(re.search(r"#define SQUARE_HASH (0x\w+)u", plan)[1], 16), protocol.SPEC["square_hash"])
        self.assertEqual(protocol.TRADE.size, protocol.SPEC["trade_bytes"])
        self.assertEqual(protocol.TRADE_STATE.size, protocol.SPEC["trade_state_bytes"])
        self.assertEqual(protocol.ARENA.size, protocol.SPEC["arena_bytes"])
        self.assertEqual(protocol.ARENA_STATE.size, protocol.SPEC["arena_state_bytes"])
        self.assertEqual(protocol.PROFILE.size, protocol.SPEC["profile_bytes"])
        self.assertEqual(protocol.PEER.size, protocol.SPEC["peer_bytes"])
        self.assertEqual(protocol.WELCOME.size, protocol.SPEC["welcome_bytes"])
        self.assertEqual(protocol.HELLO.size, protocol.SPEC["hello_bytes"])
        # (the stands, protocol v7)
        self.assertEqual(protocol.STANDS_ENTRY.size, protocol.SPEC["stands_entry_bytes"])
        self.assertEqual(protocol.ARENA_STATUS.size, protocol.SPEC["arena_status_bytes"])
        self.assertEqual(protocol.WATCH.size, protocol.SPEC["watch_bytes"])
        self.assertEqual(protocol.WATCH_STATE.size, protocol.SPEC["watch_state_bytes"])
        self.assertEqual(protocol.TEAM_STATE.size, protocol.SPEC["team_state_bytes"])
        # (v15: a 2v2's ARENA_SEAT around the largest relayed message)
        self.assertLessEqual(2 + 7 + protocol.SPEC["arena_loadout_max"], protocol.SPEC["max_message"])
        self.assertEqual(protocol.WATCH_KEYS.size, protocol.SPEC["watch_keys_head"])
        self.assertLessEqual(2 + protocol.SPEC["stands_max"] * protocol.STANDS_ENTRY.size, protocol.SPEC["max_message"])
        self.assertLessEqual(protocol.WATCH_KEYS.size + 2 * protocol.SPEC["watch_keys_max"], protocol.SPEC["max_message"])
        self.assertLessEqual(2 + 7 + protocol.SPEC["arena_loadout_max"], protocol.SPEC["max_message"])
        self.assertIn(f'#define ONLINE_SUBPROTOCOL "{protocol.SPEC["subprotocol"]}"', header)
        # (protocol v9: the pairs' rules gone with mixed pairs, the full set a session's ROMs)
        for gone in ("format_bn6", "format_full"):
            self.assertNotIn(gone, protocol.SPEC)
            self.assertNotIn("ONLINE_" + gone.upper(), header)
        # (every bit of the full set one a HELLO can carry: read_hello learns a new game's before roms_full names it)
        self.assertEqual(protocol.SPEC["roms_full"] & ~protocol.SPEC["roms_bn5"], 0)
        self.assertTrue(protocol.full_set(protocol.SPEC["roms_full"]))
        self.assertFalse(protocol.full_set(0))
        # (BN5's identity as the game checks its ROM: src/core/rom.c's xlayouts, web/play/app.js)
        rom = (ROOT / "src/core/rom.c").read_text()
        self.assertIn(f'"{protocol.SPEC["bn5_sha1"]}", "{protocol.SPEC["bn5_code"]}"', rom)
        # (BN6's blank records, where BN5's chips are installed, as src/emu/bn6_chips.h names them)
        chips = (ROOT / "src/emu/bn6_chips.h").read_text()
        blank = re.search(r"#define BN6_CHIP_BLANK\(id\)\s+(.*)", chips)[1]
        for low, high in protocol.SPEC["bn6_blank"]:
            self.assertIn(str(low), blank)
            self.assertIn(str(high), blank)

    def test_invalid_payloads(self):
        for data in (b"", b"\x03", protocol.STATE.pack(3, 0, 0, 0, 8)):
            with self.assertRaises(protocol.ProtocolError):
                protocol.read_state(data)
        with self.assertRaises(protocol.ProtocolError):
            protocol.read_hello(protocol.hello("Lan") + b"trailing")


if __name__ == "__main__":
    unittest.main()
