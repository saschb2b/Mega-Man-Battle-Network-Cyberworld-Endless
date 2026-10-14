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
    async def open(self, port, name="Lan", **identity):
        self.reader, self.writer = await asyncio.open_connection("127.0.0.1", port)
        key = base64.b64encode(b"0123456789abcdef").decode()
        self.writer.write(("GET /presence HTTP/1.1\r\nHost: localhost\r\nUpgrade: websocket\r\n"
                           "Connection: Upgrade\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Key: " + key +
                           "\r\nSec-WebSocket-Protocol: " + protocol.SPEC["subprotocol"] + "\r\n\r\n").encode())
        response = await self.reader.readuntil(b"\r\n\r\n")
        assert response.startswith(b"HTTP/1.1 101")
        accept = base64.b64encode(hashlib.sha1((key + websocket.GUID).encode()).digest())
        assert accept in response
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
        self.writer.close()
        await self.writer.wait_closed()


class ServerTests(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        self.server = Server(seed=123456, capacity=2)
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
        await asyncio.sleep(0.06)
        message = await second.message(protocol.SPEC["snapshot"])
        self.assertEqual(message[1], 1)
        peer = protocol.PEER.unpack(message[2:])
        self.assertEqual(peer[:5], (struct.unpack("!I", welcome[5:9])[0], 123, -234, 12, 7))
        self.assertEqual(peer[5].rstrip(b"\0"), b"Lan")
        await first.close()
        self.clients.remove(first)
        for _ in range(20):
            message = await second.message(protocol.SPEC["snapshot"])
            if message == bytes((protocol.SPEC["snapshot"], 0)):
                break
        self.assertEqual(message, bytes((protocol.SPEC["snapshot"], 0)))
        self.assertEqual(len(self.server.players), 1)

    async def test_rejects_incompatible_identity(self):
        for identity, code in ((dict(version=2), 2), (dict(layer_make=0), 3),
                               (dict(layer_hash=0), 3), (dict(rom_code="BAD!"), 4),
                               (dict(rom_sha1="00" * 20), 4)):
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
                     b"Sec-WebSocket-Protocol: cyberworld.presence.v1\r\nOrigin: https://untrusted.invalid\r\n\r\n")
        await writer.drain()
        self.assertTrue((await reader.read()).startswith(b"HTTP/1.1 400"))
        self.assertFalse(self.server.players)
        writer.close()
        await writer.wait_closed()


class ContractTests(unittest.TestCase):
    def test_shared_constants_and_game_generation_match(self):
        header = (ROOT / "shared/online/protocol.h").read_text()
        for name in ("hello", "welcome", "state", "snapshot", "error", "ping", "pong",
                     "max_players", "name_bytes", "max_message", "coordinate_limit"):
            value = int(re.search(r"#define ONLINE_" + name.upper() + r"\s+(\d+)", header)[1])
            self.assertEqual(value, protocol.SPEC[name])
        self.assertIn(f'#define ONLINE_PROTOCOL_VERSION {protocol.SPEC["version"]}', header)
        make = (ROOT / "src/net/layer_make.h").read_text()
        self.assertEqual(int(re.search(r"#define LAYER_MAKE (\d+)", make)[1]), protocol.SPEC["layer_make"])
        self.assertEqual(int(re.search(r"#define LAYER_MAKE_HASH (0x\w+)u", make)[1], 16), protocol.SPEC["layer_hash"])

    def test_invalid_payloads(self):
        for data in (b"", b"\x03", protocol.STATE.pack(3, 0, 0, 0, 8)):
            with self.assertRaises(protocol.ProtocolError):
                protocol.read_state(data)
        with self.assertRaises(protocol.ProtocolError):
            protocol.read_hello(protocol.hello("Lan") + b"trailing")


if __name__ == "__main__":
    unittest.main()
