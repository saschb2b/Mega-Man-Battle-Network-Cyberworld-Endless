"""The stands (issue #128): a NetBattle watched, and its report; no ROM needed."""

import asyncio
from pathlib import Path
import struct
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "server"))
sys.path.insert(0, str(Path(__file__).resolve().parent))

from cyberworld_server import protocol
from cyberworld_server.app import Server
from cyberworld_server.arena import delay_for
from test_server import Client

SPEC = protocol.SPEC


def arena(action, ref, latency=0):
    return protocol.ARENA.pack(SPEC["arena"], action, ref, latency)


def keys(match, first, values):
    return protocol.ARENA_INPUT.pack(SPEC["arena_input"], match, first, len(values)) + struct.pack(f"!{len(values)}H", *values)


def hashed(match, frame, value):
    return protocol.ARENA_HASH.pack(SPEC["arena_hash"], match, frame, value)


def loadout(match, body):
    return protocol.ARENA_LOADOUT.pack(SPEC["arena_loadout"], match, len(body)) + body


def status(match, hp0, hp1, chip0=0, chip1=0, winner=0):
    return protocol.ARENA_STATUS.pack(SPEC["arena_status"], match, hp0, 100, hp1, 100, chip0, chip1, 1, 2, winner)


def watch(action, match):
    return protocol.WATCH.pack(SPEC["watch"], action, match)


def stands(data):
    size = protocol.STANDS_ENTRY.size
    assert len(data) == 2 + data[1] * size
    return [protocol.STANDS_ENTRY.unpack_from(data, 2 + k * size) for k in range(data[1])]


class StandsTests(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        self.dir = tempfile.TemporaryDirectory()
        self.server = Server(seed=7, capacity=8, player_cap=16, state_dir=self.dir.name, trading=True, arena=True)
        self.port = await self.server.start("127.0.0.1", 0)
        self.clients = []

    async def asyncTearDown(self):
        for client in self.clients:
            await client.close()
        await self.server.close()
        self.dir.cleanup()

    async def connect(self, name, full=True):
        """A game in the channel: with the full set (BN6's and BN5's ROMs), or BN6's alone."""
        client = Client()
        self.clients.append(client)
        welcome = await client.open(self.port, name, bn5=full)
        return client, struct.unpack("!I", welcome[5:9])[0]

    async def battle(self):
        a, a_id = await self.connect("Alpha")
        b, b_id = await self.connect("Beta")
        await a.send(arena(1, b_id, 40))
        match = protocol.ARENA_STATE.unpack(await a.message(SPEC["arena_state"]))[2]
        await b.message(SPEC["arena_state"])
        await b.send(arena(2, match, 40))
        await a.message(SPEC["arena_state"])
        await b.message(SPEC["arena_state"])
        return a, b, a_id, b_id, match

    async def stands_until(self, client, test):
        for _ in range(60):
            entries = stands(await client.message(SPEC["stands"]))
            if test(entries):
                return entries
        raise AssertionError("No such STANDS")

    async def receive_all(self, client, until, start=0):
        """Messages until `until` says one is the last: (relays, keys by side from frame `start`, states)."""
        relays, got, states = [], [[], []], []
        for _ in range(2000):
            message = await client.receive()
            if message[0] == SPEC["watch_relay"]:
                relays.append((message[1], message[2:]))
            elif message[0] == SPEC["watch_keys"]:
                _, match, side, first, count = protocol.WATCH_KEYS.unpack_from(message)
                self.assertEqual(len(message), protocol.WATCH_KEYS.size + 2 * count)
                self.assertLessEqual(count, SPEC["watch_keys_max"])
                self.assertEqual(first, start + len(got[side]))
                got[side] += struct.unpack(f"!{count}H", message[protocol.WATCH_KEYS.size:])
            elif message[0] == SPEC["watch_state"]:
                states.append(protocol.WATCH_STATE.unpack(message))
            if until(relays, got, states):
                return relays, got, states
        raise AssertionError("Not received")

    async def test_a_spectator_late_gets_the_setup_and_every_key_from_the_first_frame(self):
        a, b, a_id, b_id, match = await self.battle()
        c, c_id = await self.connect("Gamma")
        entries = await self.stands_until(c, lambda e: e and e[0][0] == match)
        self.assertEqual(entries[0][1:5], (a_id, b_id, b"Alpha".ljust(16, b"\0"), b"Beta".ljust(16, b"\0")))
        await a.send(loadout(match, b"\x01AAAA"))
        await b.send(loadout(match, b"\x01BBBB"))
        await a.send(hashed(match, 0xFFFFFFFE, 0))
        await a.send(hashed(match, 0xFFFFFFFF, 0xABCD))
        await b.send(hashed(match, 0xFFFFFFFF, 0xABCD))
        # (1,600 frames before the spectator comes, at the relay's pace: he catches up from the first)
        # (each side's first keys at the delay's frame, 8 here: the frames before released keys)
        for first in range(8, 1600, 8):
            await a.send(keys(match, first, [first + k for k in range(8)]))
            await b.send(keys(match, first, [first + k + 1 for k in range(8)]))
            if first % 120 == 56:
                await a.send(hashed(match, first + 3, first))
            if first % 120 == 16:       # (a lag note for the other's time sync, v11: never the stands')
                await a.send(hashed(match, 0xFFFFFFFD, 3))
            await asyncio.sleep(0.012)
        await a.send(keys(match, 0, [0] * 8))       # (a resent message: left as it was)
        await asyncio.sleep(0.2)
        await c.send(watch(1, match))
        relays, got, states = await self.receive_all(c, lambda r, g, s: len(g[0]) >= 1600 and len(g[1]) >= 1600)
        self.assertEqual(states[0][1:6], (1, match, a_id, b_id, delay_for(40, 40)))
        self.assertEqual(got[0], [0] * 8 + [k for k in range(8, 1600)])
        self.assertEqual(got[1], [0] * 8 + [k + 1 for k in range(8, 1600)])
        kept = [protocol.ARENA_HASH.unpack(data)[2] for side, data in relays[5:] if data[0] == SPEC["arena_hash"]]
        self.assertEqual(kept[:3], [59, 179, 299])
        sides = sorted((side, data[0]) for side, data in relays[:5])
        self.assertEqual(sides, [(0, SPEC["arena_loadout"]), (0, SPEC["arena_hash"]), (0, SPEC["arena_hash"]),
                                 (1, SPEC["arena_loadout"]), (1, SPEC["arena_hash"])])
        # (live from here: a battle's end, its report and the last keys)
        # (the cable's first ends it once it has run the other's keys: they came through the server first)
        await b.send(keys(match, 1600, [9] * 8))
        await asyncio.sleep(0.05)
        await a.send(keys(match, 1600, [7] * 8))
        await a.send(status(match, 40, 0, 1, 5, winner=1))
        await a.send(arena(4, match))
        relays, got, states = await self.receive_all(c, lambda r, g, s: s and s[-1][1] == 2, 1600)
        self.assertEqual((got[0], got[1]), ([7] * 8, [9] * 8))
        over = await self.stands_until(c, lambda e: e and e[-1][5] == 3)
        self.assertEqual(over[-1][8:], (40, 100, 0, 100, 1, 5, 1, 2, 1, 0))   # (the last: no team bout, v12)
        self.assertFalse(self.server.stands.watching)

    async def test_watching_needs_the_full_set_and_the_stands_are_every_games(self):
        a, b, a_id, b_id, match = await self.battle()
        c, _ = await self.connect("Gamma", full=False)
        entries = await self.stands_until(c, lambda e: e and e[0][0] == match)
        self.assertEqual((entries[0][1:3], entries[0][5]), ((a_id, b_id), 1))
        await c.send(watch(1, match))
        self.assertEqual(struct.unpack("!H", (await c.message(SPEC["notice"]))[1:3])[0], SPEC["notice_pass"])
        self.assertFalse(self.server.stands.watching)
        await c.send(watch(2, match))       # (a stop: never refused)
        await c.send(bytes((SPEC["ping"],)) + b"\0\0\0\1")
        for _ in range(100):
            message = await c.receive()
            self.assertNotEqual(message[0], SPEC["notice"])
            if message[0] == SPEC["pong"]:
                break
        d, _ = await self.connect("Delta")
        await d.send(watch(1, match))
        self.assertEqual(protocol.WATCH_STATE.unpack(await d.message(SPEC["watch_state"]))[1], 1)

    async def test_cheers_are_held_from_the_players_and_counted(self):
        a, b, a_id, b_id, match = await self.battle()
        c, c_id = await self.connect("Gamma")
        await c.send(watch(1, match))
        await c.message(SPEC["watch_state"])
        await c.send(bytes((SPEC["social"], 1, 1)) + bytes(4))
        self.assertEqual(struct.unpack("!I", (await c.message(SPEC["event"]))[1:5])[0], c_id)
        entries = await self.stands_until(c, lambda e: e and e[0][7] == 1)
        self.assertEqual(entries[0][6], 1)    # (one watching)
        # (the players heard nothing: their next messages are no EVENT)
        await a.send(arena(4, match))
        for client in (a, b):
            for _ in range(5):
                message = await client.receive()
                self.assertNotEqual(message[0], SPEC["event"])
                if message[0] == SPEC["stands"]:
                    break

    async def test_a_spectator_is_busy_and_a_battle_over_is_not_watched(self):
        a, b, a_id, b_id, match = await self.battle()
        c, c_id = await self.connect("Gamma")
        d, _ = await self.connect("Delta")
        await c.send(watch(1, match))
        await c.message(SPEC["watch_state"])
        await d.send(arena(1, c_id))
        self.assertEqual(struct.unpack("!H", (await d.message(SPEC["notice"]))[1:3])[0], 16)
        await a.send(watch(1, match))
        self.assertEqual(struct.unpack("!H", (await a.message(SPEC["notice"]))[1:3])[0], 16)
        await b.send(arena(3, match))
        await asyncio.sleep(0.2)
        await d.send(watch(1, match))
        self.assertEqual(struct.unpack("!H", (await d.message(SPEC["notice"]))[1:3])[0], 17)
        # (a report from the cable's second is not taken, nor a malformed one)
        await b.send(status(match, 1, 1))
        with self.assertRaises(protocol.ProtocolError):
            protocol.read_arena_status(status(match, 200, 1))


if __name__ == "__main__":
    unittest.main()
