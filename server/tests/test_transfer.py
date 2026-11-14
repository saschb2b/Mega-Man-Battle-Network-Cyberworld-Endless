"""Saves sent by a code (shared/online/README.md, Saves sent by a code;
transfer.py): put, get and done over real sockets; no ROM needed."""

import asyncio
import base64
import os
from pathlib import Path
import struct
import sys
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "server"))
sys.path.insert(0, str(Path(__file__).resolve().parent))

from cyberworld_server import protocol, transfer
from cyberworld_server.app import Server
from cyberworld_server.transfer import Transfers
from test_server import Client, masked
from test_stats import get

SPEC = protocol.SPEC
LOOKUP = bytes(range(16))
OTHER = bytes(range(16, 32))


class Clock:
    """A monotonic clock the test moves."""

    def __init__(self):
        self.now = 1000.0

    def __call__(self):
        return self.now


def ask(op, lookup=LOOKUP, size=0, version=None):
    return protocol.TRANSFER.pack(SPEC["transfer"], SPEC["transfer_version"] if version is None else version,
                                  op, lookup, size)


def state(status, value=0):
    return protocol.transfer_state(status, value)


class TransferClient(Client):
    """A game sending or fetching a save: the upgrade, then TRANSFER first."""

    async def upgrade(self, port):
        self.reader, self.writer = await asyncio.open_connection("127.0.0.1", port)
        key = base64.b64encode(b"0123456789abcdef").decode()
        self.writer.write(("GET /presence HTTP/1.1\r\nHost: localhost\r\nUpgrade: websocket\r\n"
                           "Connection: Upgrade\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Key: " + key +
                           "\r\nSec-WebSocket-Protocol: " + SPEC["subprotocol"] + "\r\n\r\n").encode())
        response = await self.reader.readuntil(b"\r\n\r\n")
        assert response.startswith(b"HTTP/1.1 101")

    async def frame(self):
        first, size = await asyncio.wait_for(self.reader.readexactly(2), 5)
        assert not size & 0x80 and first & 0x80
        if size == 126:
            size = struct.unpack("!H", await self.reader.readexactly(2))[0]
        return first & 15, await self.reader.readexactly(size)

    async def message(self):
        opcode, data = await self.frame()
        assert opcode == 2, opcode
        return data

    async def closed(self, code=1000):
        """The server's close frame, then the end of the socket."""
        opcode, data = await self.frame()
        assert (opcode, data[:2]) == (8, struct.pack("!H", code)), (opcode, data)
        assert await asyncio.wait_for(self.reader.read(), 5) == b""

    async def data(self, blob, chunk=SPEC["max_message"] - 1):
        for start in range(0, len(blob), chunk):
            self.writer.write(masked(bytes((SPEC["transfer_data"],)) + blob[start:start + chunk]))
        await self.writer.drain()

    async def blob(self, size):
        """A get's DATA, `size` bytes of it, each at most max_message - 1."""
        got = bytearray()
        while len(got) < size:
            message = await self.message()
            assert message[0] == SPEC["transfer_data"] and 2 <= len(message) <= SPEC["max_message"], message[:3]
            got += message[1:]
        return bytes(got)


class TransferTests(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        self.clock = Clock()
        await self.start()

    async def start(self, on=True, **limits):
        self.server = Server(seed=123456, capacity=2, player_cap=2, transfers=on)
        self.server.transfers = Transfers(on, self.server.daily.count, clock=self.clock, **limits)
        self.port = await self.server.start("127.0.0.1", 0)
        self.clients = []

    async def asyncTearDown(self):
        await self.stop()

    async def stop(self):
        for client in self.clients:
            await client.drop()
        await self.server.close()

    async def restart(self, on=True, **limits):
        await self.stop()
        await self.start(on, **limits)

    async def open(self, first):
        """A connection whose first message is `first`; its answer."""
        client = TransferClient()
        self.clients.append(client)
        await client.upgrade(self.port)
        await client.send(first)
        return client, await client.message()

    async def put(self, blob, lookup=LOOKUP):
        client, answer = await self.open(ask(SPEC["transfer_put"], lookup, len(blob)))
        self.assertEqual(answer, state(SPEC["transfer_ready"]))
        await client.data(blob)
        self.assertEqual(await client.message(), state(SPEC["transfer_held"], SPEC["transfer_minutes"] * 60))
        await client.closed()

    async def fetch(self, lookup=LOOKUP, done=True):
        """A get: the blob (DONE sent after it where `done`), or None where none is held."""
        client, answer = await self.open(ask(SPEC["transfer_get"], lookup))
        if answer == state(SPEC["transfer_none"]):
            await client.closed()
            return None
        status, size = struct.unpack("!BI", answer[1:])
        self.assertEqual((answer[0], status), (SPEC["transfer_state"], SPEC["transfer_found"]))
        blob = await client.blob(size)
        if done:
            await client.send(ask(SPEC["transfer_done"], lookup))
            self.assertEqual(await client.message(), state(SPEC["transfer_removed"]))
            await client.closed()
        else:
            await client.drop()
        return blob

    async def refused(self, client, answer=None):
        """An exchange ended with an ERROR and a close."""
        answer = answer or await client.message()
        self.assertEqual(answer[0], SPEC["error"])
        await client.closed(1008)

    async def test_put_get_done(self):
        blob = os.urandom(1500)
        await self.put(blob)
        self.assertEqual(await self.fetch(), blob)
        self.assertIsNone(await self.fetch())
        self.assertEqual(self.server.transfers.held, {})
        today = (await get(self.port, "/stats"))[1]["today"]
        self.assertEqual((today["transfers_held"], today["transfers_taken"]), (1, 1))
        # (no visit: no player, nothing pending, nothing under way, no join counted)
        self.assertEqual((self.server.players, self.server.pending, today["joins"]), ({}, set(), 0))
        self.assertEqual(self.server.transfers.under_way, 0)
        self.assertTrue((await get(self.port, "/health"))[1]["rules"]["transfers"])

    async def test_the_largest_save_and_a_get_without_done(self):
        blob = os.urandom(SPEC["transfer_max"])
        await self.put(blob)
        # (a broken download: no DONE, so it stays to be tried again)
        self.assertEqual(await self.fetch(done=False), blob)
        await asyncio.sleep(0.05)
        self.assertEqual(await self.fetch(), blob)
        self.assertIsNone(await self.fetch())
        today = self.server.daily.snapshot()
        self.assertEqual((today["transfers_held"], today["transfers_taken"]), (1, 1))

    async def test_a_lookup_in_use_is_taken(self):
        await self.put(b"first")
        client, answer = await self.open(ask(SPEC["transfer_put"], LOOKUP, 5))
        self.assertEqual(answer, state(SPEC["transfer_taken"]))
        await client.closed()
        # (a put under way holds its lookup too)
        coming, answer = await self.open(ask(SPEC["transfer_put"], OTHER, 5))
        self.assertEqual(answer, state(SPEC["transfer_ready"]))
        client, answer = await self.open(ask(SPEC["transfer_put"], OTHER, 5))
        self.assertEqual(answer, state(SPEC["transfer_taken"]))
        await coming.data(b"later")
        self.assertEqual(await coming.message(), state(SPEC["transfer_held"], 900))
        self.assertEqual(await self.fetch(OTHER), b"later")
        self.assertEqual(await self.fetch(), b"first")

    async def test_malformed_exchanges_end_with_an_error(self):
        put = SPEC["transfer_put"]
        for first in (ask(put, size=0), ask(put, size=SPEC["transfer_max"] + 1), ask(put, size=5, version=2),
                      ask(9, size=0), ask(SPEC["transfer_get"], size=5), ask(SPEC["transfer_done"]),
                      ask(put, size=5)[:-1], bytes((SPEC["transfer"],))):
            client, answer = await self.open(first)
            await self.refused(client, answer)
        # (a DATA past the size, an empty one, and another message in a put)
        for data in (b"toolong", b"", None):
            client, answer = await self.open(ask(put, OTHER, 5))
            self.assertEqual(answer, state(SPEC["transfer_ready"]))
            if data is None:
                await client.send(ask(SPEC["transfer_get"]))
            else:
                await client.send(bytes((SPEC["transfer_data"],)) + data)
            await self.refused(client)
        self.assertEqual((self.server.transfers.held, self.server.transfers.coming), ({}, {}))
        # (after a get, DONE alone: not another lookup's, and no DATA)
        await self.put(b"saved")
        for wrong in (ask(SPEC["transfer_done"], OTHER), bytes((SPEC["transfer_data"], 1)), ask(SPEC["transfer_get"])):
            client, answer = await self.open(ask(SPEC["transfer_get"]))
            self.assertEqual(answer, state(SPEC["transfer_found"], 5))
            self.assertEqual(await client.blob(5), b"saved")
            await client.send(wrong)
            await self.refused(client)
        self.assertEqual(await self.fetch(), b"saved")
        self.assertEqual(self.server.transfers.under_way, 0)

    async def test_full_server_is_busy(self):
        await self.restart(most=4, most_bytes=100)
        lookups = [bytes(15) + bytes((n,)) for n in range(1, 5)]
        for n, lookup in enumerate(lookups):
            await self.put(bytes((n,)) * 10, lookup)
        busy = state(SPEC["transfer_busy"], transfer.FULL_WAIT)
        client, answer = await self.open(ask(SPEC["transfer_put"], OTHER, 1))
        self.assertEqual(answer, busy)       # (as many as it holds)
        await client.closed()
        # (a get still goes; those taken make room)
        self.assertEqual(await self.fetch(lookups[0]), bytes(10))
        self.assertEqual(await self.fetch(lookups[1]), bytes((1,)) * 10)
        client, answer = await self.open(ask(SPEC["transfer_put"], LOOKUP, 81))
        self.assertEqual(answer, busy)       # (the bytes: 20 held and 81 more past 100)
        await client.closed()
        # (a put under way reserves its bytes)
        coming, answer = await self.open(ask(SPEC["transfer_put"], LOOKUP, 70))
        self.assertEqual(answer, state(SPEC["transfer_ready"]))
        client, answer = await self.open(ask(SPEC["transfer_put"], OTHER, 11))
        self.assertEqual(answer, busy)
        await client.closed()
        await coming.data(b"d" * 70)
        self.assertEqual(await coming.message(), state(SPEC["transfer_held"], 900))
        await self.put(b"e" * 10, OTHER)
        self.assertEqual(self.server.transfers.held_bytes(), 100)

    async def test_exchanges_at_once_are_bounded(self):
        await self.restart(at_once=2)
        await self.put(b"held")
        first, answer = await self.open(ask(SPEC["transfer_put"], OTHER, 5))
        self.assertEqual(answer, state(SPEC["transfer_ready"]))
        second, answer = await self.open(ask(SPEC["transfer_get"]))
        self.assertEqual(answer, state(SPEC["transfer_found"], 4))
        for op in (SPEC["transfer_get"], SPEC["transfer_put"]):
            client, answer = await self.open(ask(op, bytes(16), 5 if op == SPEC["transfer_put"] else 0))
            self.assertEqual(answer, state(SPEC["transfer_busy"], transfer.AT_ONCE_WAIT))
            await client.closed()
        await first.data(b"12345")
        self.assertEqual(await first.message(), state(SPEC["transfer_held"], 900))
        await first.closed()
        await asyncio.sleep(0.05)
        self.assertEqual(await self.fetch(OTHER), b"12345")
        await second.drop()

    async def test_lookups_a_minute_are_bounded(self):
        await self.restart(per_minute=3)
        for _ in range(3):
            self.assertIsNone(await self.fetch())
        client, answer = await self.open(ask(SPEC["transfer_get"]))
        self.assertEqual(answer, state(SPEC["transfer_busy"], 20))
        await client.closed()
        # (a put asks about a lookup too: TAKEN would say it is held)
        client, answer = await self.open(ask(SPEC["transfer_put"], LOOKUP, 5))
        self.assertEqual(answer, state(SPEC["transfer_busy"], 20))
        await client.closed()
        self.clock.now += 15
        client, answer = await self.open(ask(SPEC["transfer_get"]))
        self.assertEqual(answer, state(SPEC["transfer_busy"], 5))
        await client.closed()
        self.clock.now += 5
        self.assertIsNone(await self.fetch())

    async def test_transfers_off(self):
        await self.restart(on=False)
        for first in (ask(SPEC["transfer_put"], size=5), ask(SPEC["transfer_get"]), ask(SPEC["transfer_done"]),
                      ask(SPEC["transfer_put"], size=5, version=2)):
            client, answer = await self.open(first)
            self.assertEqual(answer, state(SPEC["transfer_off"]))
            await client.closed()
        self.assertFalse((await get(self.port, "/health"))[1]["rules"]["transfers"])

    async def test_held_for_its_minutes(self):
        await self.put(b"saved")
        self.clock.now += SPEC["transfer_minutes"] * 60 - 1
        self.assertEqual(await self.fetch(done=False), b"saved")
        self.clock.now += 1
        self.assertIsNone(await self.fetch())
        # (and pruned in the server's housekeeping, unasked)
        await self.put(b"again", OTHER)
        self.clock.now += SPEC["transfer_minutes"] * 60
        self.server.housekeeping = 0.01
        for _ in range(100):
            if not self.server.transfers.held:
                break
            await asyncio.sleep(0.02)
        self.assertEqual(self.server.transfers.held, {})

    async def test_a_quiet_or_long_exchange_ends(self):
        await self.restart(quiet=0.2)
        client, answer = await self.open(ask(SPEC["transfer_put"], LOOKUP, 5))
        self.assertEqual(answer, state(SPEC["transfer_ready"]))
        await self.refused(client)
        # (a byte every tenth of a second is never quiet, but runs past the whole)
        await self.restart(quiet=0.4, whole=0.5)
        loop = asyncio.get_running_loop()
        client, answer = await self.open(ask(SPEC["transfer_put"], LOOKUP, 100))
        began = loop.time()
        self.assertEqual(answer, state(SPEC["transfer_ready"]))
        for _ in range(4):
            await asyncio.sleep(0.1)
            await client.data(b"x")
        await self.refused(client)
        self.assertLess(loop.time() - began, 0.75)
        self.assertEqual((self.server.transfers.held, self.server.transfers.coming), ({}, {}))
        self.assertEqual(self.server.transfers.under_way, 0)
        # (a get whose DONE never comes: the blob kept)
        await self.restart(quiet=0.2)
        await self.put(b"saved")
        client, answer = await self.open(ask(SPEC["transfer_get"]))
        self.assertEqual(await client.blob(5), b"saved")
        await self.refused(client)
        self.assertIn(LOOKUP, self.server.transfers.held)

    async def test_a_visit_beside_a_transfer(self):
        coming, answer = await self.open(ask(SPEC["transfer_put"], LOOKUP, 4))
        self.assertEqual(answer, state(SPEC["transfer_ready"]))
        visitor = Client()
        self.clients.append(visitor)
        welcome = await visitor.open(self.port, "Lan")
        self.assertEqual(welcome[0], SPEC["welcome"])
        await visitor.send(bytes((SPEC["ping"],)) + b"1234")
        self.assertEqual(await visitor.message(SPEC["pong"]), bytes((SPEC["pong"],)) + b"1234")
        await coming.data(b"save")
        self.assertEqual(await coming.message(), state(SPEC["transfer_held"], 900))
        self.assertEqual(len(self.server.players), 1)
        self.assertEqual(await self.fetch(), b"save")
        self.assertEqual(self.server.daily.snapshot()["joins"], 1)


class TransferContractTests(unittest.TestCase):
    def test_sizes(self):
        self.assertEqual(protocol.TRANSFER.size, SPEC["transfer_bytes"])
        self.assertEqual(protocol.TRANSFER_STATE.size, SPEC["transfer_state_bytes"])
        self.assertEqual(len(LOOKUP), SPEC["transfer_lookup_bytes"])
        self.assertEqual(transfer.CHUNK + 1, SPEC["max_message"])
        # (the bounds hold the largest save many times over)
        self.assertGreaterEqual(transfer.MOST_BYTES, 16 * SPEC["transfer_max"])


if __name__ == "__main__":
    unittest.main()
