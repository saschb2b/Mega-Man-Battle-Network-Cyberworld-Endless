"""MegaMan's look on the wire (protocol v30, issue #854): LOOK bounded and
kept, every SNAPSHOT showing a peer carrying it. No ROM needed."""

import re
import struct
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "server"))
sys.path.insert(0, str(Path(__file__).resolve().parent))

from cyberworld_server import protocol
from cyberworld_server.app import Server
from test_server import Client

SPEC = protocol.SPEC
NONE = SPEC["look_none"]


def look(palettes, colours):
    return bytes((SPEC["look"], *palettes, *colours))


class LookTests(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        self.dir = tempfile.TemporaryDirectory()
        self.server = Server(seed=7, capacity=8, player_cap=16, state_dir=self.dir.name)
        self.port = await self.server.start("127.0.0.1", 0)
        self.clients = []

    async def asyncTearDown(self):
        for client in self.clients:
            await client.close()
        await self.server.close()
        self.dir.cleanup()

    async def connect(self, name):
        client = Client()
        self.clients.append(client)
        welcome = await client.open(self.port, name)
        return client, struct.unpack("!I", welcome[5:9])[0]

    async def peer_look(self, client, peer_id, want):
        """The look `client`'s SNAPSHOTs show `peer_id` in, once it is `want`."""
        for _ in range(60):
            data = await client.message(SPEC["snapshot"])
            self.assertEqual(len(data), 2 + data[1] * SPEC["peer_bytes"])
            for k in range(data[1]):
                peer = protocol.PEER.unpack_from(data, 2 + k * protocol.PEER.size)
                if peer[0] == peer_id and peer[-1] == want:
                    return
        raise AssertionError(f"No look {want.hex()}")

    async def test_a_look_reaches_the_others_and_changes(self):
        a, a_id = await self.connect("Alpha")
        b, _ = await self.connect("Beta")
        # (none sent: every channel as BN6 draws it)
        await self.peer_look(b, a_id, bytes((NONE,)) * 6)
        await a.send(look((3, 5, NONE), (2, 15, NONE)))
        await self.peer_look(b, a_id, bytes((3, 5, NONE, 2, 15, NONE)))
        await a.send(look((0, 0, 11), (0, 1, 7)))
        await self.peer_look(b, a_id, bytes((0, 0, 11, 0, 1, 7)))

    async def refused(self, data, code):
        a, _ = await self.connect("Alpha")
        await a.send(data)
        self.assertEqual((await a.message(SPEC["error"]))[:3], struct.pack("!BH", SPEC["error"], code))

    async def test_a_palette_past_the_last_is_refused(self):
        await self.refused(look((SPEC["look_palettes"], 0, 0), (0, 0, 0)), 6)

    async def test_a_colour_past_the_last_is_refused(self):
        await self.refused(look((0, 0, 0), (0, SPEC["look_colours"], 0)), 6)

    async def test_a_channel_half_bn6s_is_refused(self):
        await self.refused(look((NONE, 0, 0), (4, 0, 0)), 6)

    async def test_a_look_of_another_size_is_refused(self):
        await self.refused(look((0, 0, 0), (0, 0, 0)) + b"\0", 1)


class LookContractTests(unittest.TestCase):
    def test_the_bounds_are_the_games(self):
        """The server's own numbers, held to the game's model of a look
        (src/core/looks.h), which it never imports."""
        header = (ROOT / "src/core/looks.h").read_text()
        for name in ("channels", "palettes", "colours", "none"):
            value = int(re.search(r"#define LOOK_" + name.upper() + r"\s+(0x[0-9A-Fa-f]+|\d+)", header)[1], 0)
            self.assertEqual(value, SPEC["look_" + name], name)
        self.assertEqual(len(protocol.LOOK_NONE), SPEC["look_bytes"] - 1)
        self.assertEqual(SPEC["look_bytes"], 1 + 2 * SPEC["look_channels"])
        self.assertEqual(protocol.PEER.size, SPEC["peer_bytes"])
        self.assertLessEqual(2 + (SPEC["max_players"] - 1) * SPEC["peer_bytes"], SPEC["max_message"])


if __name__ == "__main__":
    unittest.main()
