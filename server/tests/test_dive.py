"""The shared dive over real sockets (protocol v19, issue #589; v20, #714): rooms,
traces, the feed, DIVERS, crews, the doors' queues and party battles,
the Board's party marks and splits, moderation; no ROM needed."""

import asyncio
import json
from pathlib import Path
import struct
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "server"))
sys.path.insert(0, str(Path(__file__).resolve().parent))

from cyberworld_server import admin, crews, party, protocol, week
from cyberworld_server.app import Server
from cyberworld_server.arena import ENDED, LEAVE, OVER, START
from test_server import Client
from test_week import MONDAY, board

SPEC = protocol.SPEC
ENTER, LEAVE_DIVE, END, FOUND, PHRASE, WAVE, SPLIT = range(1, 8)


def traces(data):
    """DIVE_TRACES as (kind, [entry tuples])."""
    _, kind, count = protocol.TRACE_HEAD.unpack_from(data)
    shape = {SPEC["trace_marks"]: protocol.TRACE_MARK, SPEC["trace_phrases"]: protocol.TRACE_PHRASE,
             SPEC["trace_finds"]: protocol.TRACE_FIND, SPEC["trace_splits"]: protocol.TRACE_SPLIT,
             SPEC["trace_opened"]: struct.Struct("!B"), SPEC["trace_gone"]: struct.Struct("!I")}[kind]
    assert len(data) == 3 + count * shape.size and len(data) <= SPEC["max_message"]
    return kind, [shape.unpack_from(data, 3 + k * shape.size) for k in range(count)]


def arena_state(data):
    _, stage, battle, other, identity, side, delay, form, *seats = protocol.ARENA_STATE.unpack(data)
    return dict(stage=stage, battle=battle, other=other, side=side, delay=delay, form=form, seats=tuple(seats))


def crew_state(data):
    _, stage, other, value = protocol.CREW_STATE_HEAD.unpack_from(data)
    members = [protocol.CREW_MEMBER.unpack_from(data, protocol.CREW_STATE_HEAD.size + k * protocol.CREW_MEMBER.size)
               for k in range(SPEC["party_most"])]
    return dict(stage=stage, other=other, value=value, members=[m[0] for m in members if m[0]],
                vitals={m[0]: m[2:5] for m in members if m[0]}, where={m[0]: m[5] for m in members if m[0]})


def queue_state(data):
    _, stage, depth, side, way, place, count = protocol.QUEUE_STATE_HEAD.unpack_from(data)
    entries = [protocol.QUEUE_ENTRY.unpack_from(data, protocol.QUEUE_STATE_HEAD.size + k * protocol.QUEUE_ENTRY.size)
               for k in range(count)]
    return dict(stage=stage, door=(depth, side, way), place=place, members=[e[0] for e in entries],
                arena=[e[2] for e in entries])


def name(raw):
    return raw.rstrip(b"\0").decode()


async def until_true(condition, timeout=2.0):
    for _ in range(int(timeout * 100)):
        if condition():
            return
        await asyncio.sleep(0.01)
    raise AssertionError("Condition not reached")


class DiveTests(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        self.dir = tempfile.TemporaryDirectory()
        self.now = MONDAY + 3600
        self.clients = []
        # (a crew's waits short, so that walking together and apart takes a moment here)
        crews.FORM_WAIT, crews.SPLIT_WAIT, crews.AWAY_WAIT = 0.1, 0.3, 0.5
        await self.start()

    async def start(self):
        self.server = Server(seed=7, capacity=8, player_cap=32, state_dir=self.dir.name, clock=lambda: self.now)
        self.port = await self.server.start("127.0.0.1", 0)
        self.week, self.seed = self.server.weeks.current()

    async def asyncTearDown(self):
        for client in self.clients:
            await client.close()
        await self.server.close()
        self.dir.cleanup()
        crews.FORM_WAIT, crews.SPLIT_WAIT, crews.AWAY_WAIT, crews.VITALS_WAIT = (
            SPEC[k] / 1000 for k in ("crew_form_ms", "crew_split_ms", "crew_away_ms", "crew_vitals_ms"))
        party.SEAT_HOLD = 12.0

    async def restart(self):
        for client in self.clients:
            await client.close()
        self.clients = []
        await self.server.close()
        await self.start()

    async def connect(self, label, full=True, **identity):
        client = Client()
        self.clients.append(client)
        welcome = await client.open(self.port, label, key=label.encode().ljust(16, b"k"), bn5=full, **identity)
        client.id = struct.unpack("!I", welcome[5:9])[0]
        client.identity = welcome[14:30]
        return client

    async def until(self, client, kind, test=lambda data: True):
        for _ in range(200):
            data = await client.message(kind)
            if test(data):
                return data
        raise AssertionError("No such message")

    async def notice(self, client):
        for _ in range(2000):
            data = await client.receive()
            if data[0] == SPEC["notice"]:
                return struct.unpack("!H", data[1:3])[0]
        raise AssertionError("No NOTICE")

    async def enter(self, client, depth=1, side=0, way=0):
        await client.send(protocol.dive(ENTER, self.week, self.seed, depth, side, way))
        _, week_number, d, s, w, instance, count = protocol.DIVE_ROOM.unpack(await client.message(SPEC["dive_room"]))
        self.assertEqual((week_number, d, s, w), (self.week, depth, side, way))
        return instance, count

    async def peers(self, client, count):
        """The peers of the client's room or channel once there are `count`, as (session, flags)."""
        for _ in range(100):
            data = await client.message(SPEC["snapshot"])
            if data[1] == count:
                return [(protocol.PEER.unpack_from(data, 2 + k * 50)[0], data[2 + k * 50 + 49]) for k in range(count)]
        raise AssertionError("No such snapshot")

    # -- rooms --

    async def test_a_room_of_its_own_beside_the_square(self):
        a, b, square = await self.connect("Alpha"), await self.connect("Beta"), await self.connect("Gamma")
        self.assertEqual(await self.enter(a, 2, 0, 5), (1, 1))
        self.assertEqual(await self.enter(b, 2, 0, 5), (1, 2))
        # (the two see each other, each marked diving; the Square sees neither)
        seen = await self.peers(a, 1)
        self.assertEqual(seen[0][0], b.id)
        self.assertTrue(seen[0][1] & SPEC["peer_diving"])
        await self.peers(square, 0)
        await b.send(protocol.STATE.pack(SPEC["state"], 40, -12, 0, 3))
        for _ in range(60):
            data = await a.message(SPEC["snapshot"])
            if data[1] == 1 and protocol.PEER.unpack_from(data, 2)[2:6] == (40, -12, 0, 3):
                break
        else:
            self.fail("The room's positions never came")
        # (another layer's room is another room; a leave is a fresh WELCOME in the Square)
        await self.enter(b, 3, 0, 5)
        await self.peers(a, 0)
        await b.send(protocol.dive(LEAVE_DIVE))
        welcome = await b.message(SPEC["welcome"])
        self.assertLessEqual(struct.unpack("!H", welcome[12:14])[0], self.server.max_channels)
        await self.peers(square, 1)
        self.assertNotIn(b.id, self.server.dive.of)
        # (CHANNEL from a diver: out of the dive into that channel)
        await a.send(struct.pack("!BH", SPEC["channel"], 2))
        self.assertEqual(struct.unpack("!H", (await a.message(SPEC["welcome"]))[12:14])[0], 2)
        self.assertFalse(self.server.dive.of)

    async def test_a_full_room_opens_another_instance(self):
        clients = [await self.connect(f"Diver{k}") for k in range(10)]
        for k, client in enumerate(clients[:8]):
            self.assertEqual(await self.enter(client, 4), (1, k + 1))
        self.assertEqual(await self.enter(clients[8], 4), (2, 1))
        self.assertEqual(await self.enter(clients[9], 4), (2, 2))
        await self.peers(clients[8], 1)
        await clients[0].send(protocol.dive(LEAVE_DIVE))
        await clients[0].message(SPEC["welcome"])
        self.assertEqual(await self.enter(await self.connect("Late"), 4), (1, 8))

    async def test_who_may_dive_and_into_which_net(self):
        stamp = await self.connect("Stampless", full=False)
        await stamp.send(protocol.dive(ENTER, self.week, self.seed, 1, 0, 0))
        self.assertEqual(await self.notice(stamp), SPEC["notice_pass"])
        a = await self.connect("Alpha")
        await a.send(protocol.dive(ENTER, self.week - 2, self.server.weeks.seed(self.week - 2), 1, 0, 0))
        self.assertEqual(await self.notice(a), 23)
        await a.send(protocol.dive(ENTER, self.week, self.seed ^ 1, 1, 0, 0))
        self.assertEqual(await self.notice(a), 24)
        # (last week's net still opens, as its posts do)
        await a.send(protocol.dive(ENTER, self.week - 1, self.server.weeks.seed(self.week - 1), 1, 0, 0))
        self.assertEqual(protocol.DIVE_ROOM.unpack(await a.message(SPEC["dive_room"]))[1], self.week - 1)
        for message in (protocol.dive(FOUND, 1), protocol.crew(SPEC["crew_battle"], 0, 2), protocol.queue(SPEC["queue_leave"])):
            await stamp.send(message)
            self.assertEqual(await self.notice(stamp), SPEC["notice_dive_out"])
        # (a diver trades, teams up and watches nothing)
        for message in (protocol.OFFER.pack(SPEC["offer"], 5, 1, 0), protocol.TEAM.pack(SPEC["team"], 1, stamp.id, 0),
                        protocol.ARENA.pack(SPEC["arena"], 1, stamp.id, 20, 0)):
            await a.send(message)
            self.assertEqual(await self.notice(a), SPEC["notice_dive_in"])

    async def test_malformed_dives_close_the_connection(self):
        for bad in (protocol.dive(ENTER, self.week, self.seed, 11, 0, 0), protocol.dive(ENTER, self.week, self.seed, 1, 0, 0)[:-1],
                    bytes((SPEC["dive"], 9)), protocol.dive(PHRASE, 8, 0, 0, 0), protocol.dive(END, 3, 0, 0, 0, 0, 0, 0, 0),
                    protocol.dive(SPLIT, 1, 10, 12, 0, 1), protocol.dive(SPLIT, 1, 10, 3, 0, 5), protocol.dive(SPLIT, 1, 0, 3, 0, 1),
                    protocol.dive(PHRASE, 1, 9000, 0, 0), protocol.crew(7), protocol.crew(1, 5), protocol.crew(SPEC["crew_battle"], 0, 1, (2, 0, 0)),
                    protocol.crew(SPEC["crew_join"]),
                    protocol.queue(SPEC["queue_join"]), protocol.queue(5, (1, 0, 0)),
                    protocol.party(SPEC["party_hp"], 3, 10, 5), protocol.party(SPEC["party_damage"], 0, 1, 1)):
            client = await self.connect("Rogue")
            await client.send(bad)
            self.assertEqual((await client.message(SPEC["error"]))[:3], struct.pack("!BH", SPEC["error"], 1))
            self.clients.remove(client)
            await client.close()

    # -- traces --

    async def test_traces_are_kept_by_layer_and_read_back_after_a_restart(self):
        a, b = await self.connect("Alpha"), await self.connect("Beta")
        await self.enter(a, 3, 2, 7)
        await self.enter(b, 3, 2, 7)
        await a.send(protocol.dive(PHRASE, 4, 10, 20, 0))
        kind, entries = traces(await self.until(b, SPEC["dive_traces"], lambda d: d[1] == SPEC["trace_phrases"]))
        phrase_id = entries[0][0]
        self.assertEqual((name(entries[0][1]), entries[0][2:]), ("Alpha", (4, 10, 20, 0, 0)))
        await b.send(protocol.dive(WAVE, phrase_id))
        _, entries = traces(await self.until(a, SPEC["dive_traces"], lambda d: d[1] == SPEC["trace_phrases"] and traces(d)[1][0][-1]))
        self.assertEqual((entries[0][0], entries[0][-1]), (phrase_id, 1))
        # (his new phrase replaces his last: the old one gone for everyone)
        await a.send(protocol.dive(PHRASE, 2, 30, 20, 0))
        _, gone = traces(await self.until(b, SPEC["dive_traces"], lambda d: d[1] == SPEC["trace_gone"]))
        self.assertEqual(gone, [(phrase_id,)])
        await b.send(protocol.dive(WAVE, phrase_id))
        self.assertEqual(await self.notice(b), SPEC["notice_trace_gone"])
        # (a Mystery Data's first finder, and the opened set)
        await a.send(protocol.dive(FOUND, 6))
        _, opened = traces(await self.until(b, SPEC["dive_traces"], lambda d: d[1] == SPEC["trace_opened"]))
        self.assertEqual(opened, [(6,)])
        _, finds = traces(await self.until(b, SPEC["dive_traces"], lambda d: d[1] == SPEC["trace_finds"]))
        self.assertEqual([(i, name(n)) for i, n in finds], [(6, "Alpha")])
        await asyncio.sleep(1.1)        # (his burst, SOCIAL's, spent: a Wave, one at a phrase gone, then a second)
        await b.send(protocol.dive(FOUND, 6))
        await b.send(protocol.dive(FOUND, 2))
        _, finds = traces(await self.until(a, SPEC["dive_traces"], lambda d: d[1] == SPEC["trace_finds"] and d[3] == 2))
        self.assertEqual([(i, name(n)) for i, n in finds], [(2, "Beta")])
        # (a deletion: its mark, and its line in the feed)
        await asyncio.sleep(1.1)
        await b.send(protocol.dive(END, SPEC["dive_deleted"], -5, 6, 0, 0x91, 0, 0, 0))
        _, marks = traces(await self.until(a, SPEC["dive_traces"], lambda d: d[1] == SPEC["trace_marks"]))
        self.assertEqual([(name(m[0]), *m[1:]) for m in marks], [("Beta", -5, 6, 0, 0x91, SPEC["dive_deleted"])])
        await self.restart()
        c = await self.connect("Gamma")
        await c.send(protocol.dive(ENTER, self.week, self.seed, 3, 2, 7))
        got = {}
        await c.message(SPEC["dive_room"])
        for _ in range(4):
            kind, entries = traces(await c.message(SPEC["dive_traces"]))
            got[kind] = entries
        self.assertEqual([(name(m[0]), m[4]) for m in got[SPEC["trace_marks"]]], [("Beta", 0x91)])
        self.assertEqual([(p[0] != phrase_id, name(p[1]), p[2], p[-1]) for p in got[SPEC["trace_phrases"]]], [(True, "Alpha", 2, 0)])
        self.assertEqual(sorted((i, name(n)) for i, n in got[SPEC["trace_finds"]]), [(2, "Beta"), (6, "Alpha")])
        self.assertEqual(got[SPEC["trace_opened"]], [(2,), (6,)])
        # (another layer of the same week holds none of them)
        d = await self.connect("Delta")
        await self.enter(d, 3, 2, 8)
        await self.peers(d, 0)
        self.assertNotIn(SPEC["dive_traces"], [m[0] for m in [await d.receive() for _ in range(3)]])

    async def test_the_lists_are_capped_oldest_out(self):
        clients = [await self.connect(f"Phr{k}") for k in range(3)]
        for client in clients:
            await self.enter(client, 1)
        layer = None
        for k in range(SPEC["traces_most"] + 4):
            client = clients[k % 3]
            await client.send(protocol.dive(END, SPEC["dive_deleted"], k, 0, 0, 1, 0, 0, 0))
            await asyncio.sleep(0.35)
        layer = self.server.dive.traces.layers[(self.week, self.seed, 1, 0, 0)]
        self.assertEqual(len(layer.marks), SPEC["traces_most"])
        self.assertEqual(layer.marks[0]["x"], 4)
        # (the burst past: notice_dive_wait)
        for _ in range(4):
            await clients[0].send(protocol.dive(FOUND, 1))
        self.assertEqual(await self.notice(clients[0]), SPEC["notice_dive_wait"])

    async def test_splits_their_places_and_the_weeks_fastest(self):
        a, b, square = await self.connect("Alpha"), await self.connect("Beta"), await self.connect("Gamma")
        await self.enter(a, 3)
        await self.enter(b, 3)
        await a.send(protocol.dive(SPLIT, 0, 300, 9, 0, 1))
        _, index, party, tenths, place, count, best, best_name = protocol.DIVE_PLACE.unpack(await a.message(SPEC["dive_place"]))
        self.assertEqual((index, party, tenths, place, count, best, name(best_name)), (0, 1, 300, 1, 1, 300, "Alpha"))
        await b.send(protocol.dive(SPLIT, 0, 200, 11, 0, 1))
        place = protocol.DIVE_PLACE.unpack(await b.message(SPEC["dive_place"]))
        self.assertEqual(place[4:7], (1, 2, 200))
        await a.send(protocol.dive(SPLIT, 0, 250, 10, 0, 1))
        place = protocol.DIVE_PLACE.unpack(await a.message(SPEC["dive_place"]))
        self.assertEqual((place[4:7], name(place[7])), ((2, 2, 200), "Beta"))
        _, splits = traces(await self.until(a, SPEC["dive_traces"], lambda d: d[1] == SPEC["trace_splits"]))
        self.assertEqual([(s[0], s[1], s[2], name(s[5])) for s in splits], [(0, 1, 200, "Beta")])
        # (a guardian's bust in a party of two: the week's fastest of act 1 by party size, to everyone)
        await a.send(protocol.dive(SPLIT, 9, 410, 8, SPEC["split_guardian"], 2))
        data = await self.until(square, SPEC["week_splits"], lambda d: d[3:5] != bytes(2) or any(d[3:]))
        self.assertEqual(len(data), SPEC["week_splits_bytes"])
        entries = [protocol.SPLITS_ENTRY.unpack_from(data, 3 + k * 18) for k in range(16)]
        self.assertEqual((entries[1][0], name(entries[1][1])), (410, "Alpha"))
        self.assertEqual([e[0] for k, e in enumerate(entries) if k != 1], [0] * 15)
        # (WEEK_SPLITS comes with every WEEK_BOARD)
        late = await self.connect("Late")
        await late.message(SPEC["week_board"])
        self.assertEqual(protocol.SPLITS_ENTRY.unpack_from(await late.message(SPEC["week_splits"]), 3 + 18)[0], 410)

    # -- the feed, the flare and the pack --

    async def test_the_feed_the_flare_and_divers(self):
        a, b, c, last = await self.connect("Alpha"), await self.connect("Beta"), await self.connect("Gamma"), await self.connect("Old")
        square = await self.connect("Square")
        await self.enter(a, 3, 0, 4)
        await self.enter(b, 1, 0, 2)
        await self.enter(c, 4, 0, 4)
        await last.send(protocol.dive(ENTER, self.week - 1, self.server.weeks.seed(self.week - 1), 1, 0, 0))
        await last.message(SPEC["dive_room"])
        await a.send(protocol.queue(SPEC["queue_join"], (3, 0, 4)))
        flare = protocol.DIVE_FEED.unpack(await b.message(SPEC["dive_feed"]))
        self.assertEqual((flare[1], name(flare[2]), flare[3:6]), (SPEC["feed_flare"], "Alpha", (3, 0, 4)))
        # (DIVERS: four dive, the deepest first)
        data = await self.until(square, SPEC["divers"], lambda d: struct.unpack_from("!H", d, 1)[0] == 4)
        _, total, count = protocol.DIVERS_HEAD.unpack_from(data)
        entries = [protocol.DIVERS_ENTRY.unpack_from(data, 4 + 19 * k) for k in range(count)]
        self.assertEqual([(name(e[0]), e[1], e[2]) for e in entries][:2], [("Gamma", 4, 2), ("Alpha", 3, 1)])
        # (an end: to every diver of that week and the Square, not last week's)
        await c.send(protocol.dive(END, SPEC["dive_deleted"], 0, 0, 0, 0x1F, 0, 0, 0))
        for client in (square, a, b):
            feed = protocol.DIVE_FEED.unpack(await self.until(client, SPEC["dive_feed"], lambda d: d[1] == SPEC["feed_deleted"]))
            self.assertEqual((name(feed[2]), feed[3], feed[6]), ("Gamma", 4, 0x1F))
        # (C, behind no one and in act 2, had no flare; last week's diver no feed)
        for _ in range(30):
            data = await c.receive()
            self.assertFalse(data[0] == SPEC["dive_feed"] and data[1] == SPEC["feed_flare"])
        for _ in range(30):
            self.assertNotEqual((await last.receive())[0], SPEC["dive_feed"])
        await a.send(protocol.dive(END, SPEC["dive_won"], 0, 0, 0, 0, 0, 0, 0))
        self.assertEqual(await self.notice(a), 25)

    async def test_the_flare_on_the_door_s_own_layer(self):
        # (issue #752: a diver on the layer of the door is told someone waits there; one already in the queue is not,
        # nor the one who joined)
        a, b, c = await self.connect("Alpha"), await self.connect("Beta"), await self.connect("Gamma")
        for client in (a, b, c):
            await self.enter(client, 3, 0, 4)
        await b.send(protocol.queue(SPEC["queue_join"], (3, 0, 4)))
        await b.message(SPEC["queue_state"])
        await a.send(protocol.queue(SPEC["queue_join"], (3, 0, 4)))
        flare = protocol.DIVE_FEED.unpack(await self.until(c, SPEC["dive_feed"], lambda d: d[1] == SPEC["feed_flare"]
                                                           and name(protocol.DIVE_FEED.unpack(d)[2]) == "Alpha"))
        self.assertEqual(flare[3:6], (3, 0, 4))
        for client in (a, b):
            for _ in range(20):
                data = await client.receive()
                self.assertFalse(data[0] == SPEC["dive_feed"] and data[1] == SPEC["feed_flare"]
                                 and name(protocol.DIVE_FEED.unpack(data)[2]) == "Alpha")

    # -- crews --

    async def place(self, client, x, y=0):
        await client.send(protocol.STATE.pack(SPEC["state"], x, y, 0, 0))

    def crew_ids(self, client):
        crew = self.server.crews.of.get(client.id)
        return crew.members if crew else []

    async def settled(self, test, seconds=3):
        for _ in range(int(seconds / 0.02)):
            if test():
                return
            await asyncio.sleep(0.02)
        raise AssertionError("Never settled")

    async def test_crews_by_nearness(self):
        a, b, c, d, e = [await self.connect(n) for n in ("Alpha", "Beta", "Gamma", "Delta", "Echo")]
        for client in (a, b, c, d, e):
            await self.enter(client, 2)
        for client, x in ((a, 0), (b, 2000), (c, 4000), (d, 6000), (e, 8000)):
            await self.place(client, x)
        # (two passing each other are no crew: near for less than crew_form_ms)
        crews.FORM_WAIT = 0.6
        await self.place(b, 100)
        await asyncio.sleep(0.2)
        await self.place(b, 2000)
        await asyncio.sleep(0.8)
        self.assertFalse(self.server.crews.of)
        crews.FORM_WAIT = 0.1
        # (near: a crew, told to both, no asking)
        await self.place(b, 100)
        formed = crew_state(await self.until(a, SPEC["crew_state"], lambda m: crew_state(m)["members"] == [a.id, b.id]))
        # (v24: each member's HP and max HP as his DIVE vitals said, none sent yet, and his layer's depth)
        self.assertEqual((formed["stage"], formed["other"], formed["vitals"][b.id]), (SPEC["crew_formed"], b.id, (0, 0, 2)))
        await self.until(b, SPEC["crew_state"], lambda m: crew_state(m)["members"] == [a.id, b.id])
        # (past crew_near and within crew_far it holds: no flicker at the edge)
        for x in (180, 120, 190, 140, 185):
            await self.place(b, x)
            await asyncio.sleep(0.1)
        await asyncio.sleep(0.5)
        self.assertEqual(self.crew_ids(a), [a.id, b.id])
        # (a chain: Gamma near Beta, far from Alpha, is in it; then Delta near Gamma; four at most, Echo near them outside)
        await self.place(c, 300)
        await self.settled(lambda: self.crew_ids(a) == [a.id, b.id, c.id])
        await self.place(d, 420)
        await self.settled(lambda: self.crew_ids(a) == [a.id, b.id, c.id, d.id])
        await self.place(e, 360, 60)
        await asyncio.sleep(0.6)
        self.assertEqual(self.crew_ids(a), [a.id, b.id, c.id, d.id])
        self.assertNotIn(e.id, self.server.crews.of)
        # (Delta walks off: out after crew_split_ms, told so; Echo, near Gamma, takes the place)
        await self.place(d, 3000)
        await self.until(d, SPEC["crew_state"], lambda m: m[1] == SPEC["crew_split_up"])
        await self.settled(lambda: self.crew_ids(a) == [a.id, b.id, c.id, e.id])
        # (Gamma takes a portal: out after crew_away_ms unless the crew follows)
        await self.enter(c, 3)
        await asyncio.sleep(0.1)
        self.assertIn(c.id, self.crew_ids(a))
        await self.until(c, SPEC["crew_state"], lambda m: m[1] == SPEC["crew_split_up"])
        self.assertEqual(self.crew_ids(a), [a.id, b.id, e.id])
        # (an end of the dive and a session ended: out at once; a crew of one is none)
        await e.send(protocol.dive(END, SPEC["dive_deleted"], 0, 0, 0, 0, 0, 0, 0))
        await self.until(a, SPEC["crew_state"], lambda m: crew_state(m)["members"] == [a.id, b.id])
        await b.close()
        self.clients.remove(b)
        state = crew_state(await self.until(a, SPEC["crew_state"], lambda m: m[1] == SPEC["crew_split_up"]))
        self.assertEqual(state["members"], [])
        self.assertFalse(self.server.crews.of)

    async def test_crew_vitals(self):
        """v24, issue #717: a diver's HP and max HP (DIVE vitals) and his layer, told to his crew alone as they change,
        crew_vitals_ms apart at most; bounded."""
        a, b, c = [await self.connect(n) for n in ("Alpha", "Beta", "Gamma")]
        for client in (a, b, c):
            await self.enter(client, 2)
        for client, x in ((a, 0), (b, 50), (c, 4000)):
            await self.place(client, x)
        await self.settled(lambda: self.crew_ids(a) == [a.id, b.id])
        crews.VITALS_WAIT = 0.1
        vitals = SPEC["dive_vitals"]
        await b.send(protocol.dive(vitals, 120, 200))
        told = crew_state(await self.until(a, SPEC["crew_state"], lambda m: m[1] == SPEC["crew_vitals"]
                                           and crew_state(m)["vitals"][b.id] == (120, 200, 2)))
        self.assertEqual(told["members"], [a.id, b.id])
        await self.until(b, SPEC["crew_state"], lambda m: m[1] == SPEC["crew_vitals"])
        # (a layer on: his depth told; Gamma, in no crew, is told nothing)
        await self.enter(b, 3)
        await self.until(a, SPEC["crew_state"], lambda m: m[1] == SPEC["crew_vitals"] and crew_state(m)["vitals"][b.id] == (120, 200, 3))
        for _ in range(30):
            self.assertNotEqual((await c.receive())[0], SPEC["crew_state"])
        # (bounded: no max HP, HP past it, a max past 9999)
        for hp, most in ((0, 0), (201, 200), (100, 10000)):
            with self.assertRaises(protocol.ProtocolError):
                protocol.read_dive(protocol.dive(vitals, hp, most))
        self.assertEqual(protocol.read_dive(protocol.dive(vitals, 0, 200)), (vitals, (0, 200)))
        # (one taken every half crew_vitals_ms at most)
        later = self.server.crews.hp_at[b.id] + 10
        self.server.crews.vitals(b.id, 90, 200, later)
        self.server.crews.vitals(b.id, 80, 200, later + crews.VITALS_WAIT / 4)
        self.assertEqual(self.server.crews.hp[b.id], (90, 200))

    async def test_a_diver_gone_home(self):
        """v26, issue #756: a diver's game says where he is (DIVE where); at home between acts he is in no one's
        SNAPSHOT, forms no crew from the place he left and stays in his crew as it stood, his crew told where he is."""
        a, b, c = [await self.connect(n) for n in ("Alpha", "Beta", "Gamma")]
        for client in (a, b, c):
            await self.enter(client, 2)
        for client, x in ((a, 0), (b, 50), (c, 4000)):
            await self.place(client, x)
        await self.settled(lambda: self.crew_ids(a) == [a.id, b.id])
        self.assertEqual(sorted(s for s, _ in await self.peers(a, 2)), sorted([b.id, c.id]))
        crews.VITALS_WAIT = 0.1
        where = SPEC["dive_where"]
        await b.send(protocol.dive(where, SPEC["where_home"]))
        told = crew_state(await self.until(a, SPEC["crew_state"], lambda m: m[1] == SPEC["crew_vitals"]
                                           and crew_state(m)["where"][b.id] == SPEC["where_home"]))
        self.assertEqual(told["members"], [a.id, b.id])
        self.assertEqual([s for s, _ in await self.peers(a, 1)], [c.id])
        self.assertEqual([s for s, _ in await self.peers(c, 1)], [a.id])
        # (Gamma where Beta stood: no crew with one at home; Beta's crew kept past crew_away_ms)
        await self.place(c, 60)
        await asyncio.sleep(crews.AWAY_WAIT + 0.3)
        self.assertIn(b.id, self.crew_ids(a))
        self.assertFalse(any(sorted(pair) == sorted((b.id, c.id)) for pair in self.server.crews.near))
        # (in a battle: told so, still drawn)
        await self.place(c, 4000)
        await a.send(protocol.dive(where, SPEC["where_battle"]))
        await self.until(b, SPEC["crew_state"], lambda m: m[1] == SPEC["crew_vitals"] and crew_state(m)["where"][a.id] == SPEC["where_battle"])
        # (the next layer entered: on a layer again, drawn, his crew's away rule as ever)
        await self.enter(b, 3)
        await self.until(a, SPEC["crew_state"], lambda m: m[1] == SPEC["crew_vitals"] and crew_state(m)["where"][b.id] == SPEC["where_layer"])
        self.assertNotIn(b.id, self.server.crews.at)
        # (v29: busy, a chat or the PET open on his layer: told so, still drawn)
        await a.send(protocol.dive(where, SPEC["where_busy"]))
        await self.until(b, SPEC["crew_state"], lambda m: m[1] == SPEC["crew_vitals"] and crew_state(m)["where"][a.id] == SPEC["where_busy"])
        with self.assertRaises(protocol.ProtocolError):
            protocol.read_dive(protocol.dive(where, 4))

    async def test_a_full_crew_told_to_the_one_near(self):
        """v29, issue #758: a diver in no crew near a crew of four is told once that it is full (CREW_STATE full, that
        crew's members), so his game draws no "about to be" ring for it."""
        a, b, c, d, e = [await self.connect(n) for n in ("Alpha", "Beta", "Gamma", "Delta", "Echo")]
        for client in (a, b, c, d, e):
            await self.enter(client, 2)
        for k, client in enumerate((a, b, c, d)):
            await self.place(client, 20 * k)
        await self.place(e, 4000)
        await self.settled(lambda: len(self.crew_ids(a)) == 4)
        await self.place(e, 40)
        full = crew_state(await self.until(e, SPEC["crew_state"], lambda m: m[1] == SPEC["crew_full"]))
        self.assertEqual(sorted(full["members"]), sorted([a.id, b.id, c.id, d.id]))
        self.assertNotIn(e.id, self.server.crews.of)
        # (once: nothing more while he stands there)
        for _ in range(40):
            data = await e.receive()
            self.assertFalse(data[0] == SPEC["crew_state"] and data[1] == SPEC["crew_full"])

    async def test_crews_across_a_portal_and_two_crews_met(self):
        a, b, c, d = [await self.connect(n) for n in ("Alpha", "Beta", "Gamma", "Delta")]
        for client in (a, b, c, d):
            await self.enter(client, 2)
        for client, x in ((a, 0), (b, 50), (c, 3000), (d, 3050)):
            await self.place(client, x)
        await self.settled(lambda: self.crew_ids(a) == [a.id, b.id] and self.crew_ids(c) == [c.id, d.id])
        # (the crew through a portal, one after the other: still one on the next layer)
        crews.AWAY_WAIT = 1.0
        await self.enter(a, 3)
        await self.place(a, 10)
        await asyncio.sleep(0.4)
        await self.enter(b, 3)
        await self.place(b, 30)
        await asyncio.sleep(1.2)
        self.assertEqual(self.crew_ids(a), [a.id, b.id])
        # (two crews meet: one of four, the older taking the other's)
        for client, x in ((c, 60), (d, 90)):
            await self.enter(client, 3)
            await self.place(client, x)
        await self.settled(lambda: self.crew_ids(d) == [a.id, b.id, c.id, d.id])
        self.assertIs(self.server.crews.of[a.id], self.server.crews.of[d.id])

    async def crew_of(self, *clients):
        for k, client in enumerate(clients):
            await self.place(client, 20 * k)
        await self.settled(lambda: self.crew_ids(clients[0]) == [c.id for c in clients]
                           or sorted(self.crew_ids(clients[0])) == sorted(c.id for c in clients))

    async def test_a_battle_offered_to_the_crew(self):
        a, b, c = [await self.connect(n) for n in ("Alpha", "Beta", "Gamma")]
        for client in (a, b, c):
            await self.enter(client, 2)
        await self.crew_of(a, b, c)
        await a.send(protocol.crew(SPEC["crew_battle"], 0, 4))
        for mate in (b, c):
            offer = crew_state(await self.until(mate, SPEC["crew_state"], lambda m: m[1] == SPEC["crew_offer"]))
            self.assertEqual((offer["other"], offer["value"]), (a.id, 4))
        await b.send(protocol.crew(SPEC["crew_join"], a.id))
        await c.send(protocol.crew(SPEC["crew_join"], a.id))
        starts = [arena_state(await self.until(x, SPEC["arena_state"])) for x in (a, b, c)]
        self.assertEqual([s["side"] for s in starts], [0, 1, 2])
        self.assertTrue(all(s["stage"] == START and s["form"] == SPEC["arena_party"] for s in starts))
        self.assertEqual(starts[1]["seats"], (a.id, b.id, c.id, 0))
        self.assertTrue(self.server.parties.fighting(c.id))
        # (fighting: a new offer refused; seat 0's over ends it for everyone)
        await a.send(protocol.crew(SPEC["crew_battle"], 0, 5))
        self.assertEqual(await self.notice(a), 16)
        await a.send(protocol.ARENA.pack(SPEC["arena"], OVER, starts[0]["battle"], 0, 0))
        for x in (b, c):
            self.assertEqual(arena_state(await self.until(x, SPEC["arena_state"]))["stage"], ENDED)
        # (nobody joins within the window: the owner alone)
        crews.JOIN_WAIT = 0.3
        try:
            await b.send(protocol.crew(SPEC["crew_battle"], 0, 6))
            alone = crew_state(await self.until(b, SPEC["crew_state"], lambda m: m[1] == SPEC["crew_alone"]))
            self.assertEqual(alone["value"], 6)
            await c.send(protocol.crew(SPEC["crew_join"], b.id))
            self.assertEqual(await self.notice(c), 17)
        finally:
            crews.JOIN_WAIT = SPEC["crew_join_ms"] / 1000
        # (v23: a game whose MegaMan is busy says so at once; the battle starts with the rest, no window waited)
        crews.JOIN_WAIT = 60
        try:
            await c.send(protocol.crew(SPEC["crew_battle"], 0, 7))
            for mate in (a, b):
                await self.until(mate, SPEC["crew_state"], lambda m: m[1] == SPEC["crew_offer"])
            await a.send(protocol.crew(SPEC["crew_busy"], c.id))
            await b.send(protocol.crew(SPEC["crew_join"], c.id))
            start = arena_state(await self.until(b, SPEC["arena_state"], lambda m: arena_state(m)["stage"] == START))
            self.assertEqual(start["seats"], (c.id, b.id, 0, 0))
            await c.send(protocol.ARENA.pack(SPEC["arena"], OVER, start["battle"], 0, 0))
            await self.until(b, SPEC["arena_state"], lambda m: arena_state(m)["stage"] == ENDED)
            # (every one busy: the owner alone at once; a busy for an offer over says nothing)
            await a.send(protocol.crew(SPEC["crew_battle"], 0, 8))
            for mate in (b, c):
                await self.until(mate, SPEC["crew_state"], lambda m: m[1] == SPEC["crew_offer"])
                await mate.send(protocol.crew(SPEC["crew_busy"], a.id))
            alone = crew_state(await self.until(a, SPEC["crew_state"], lambda m: m[1] == SPEC["crew_alone"]))
            self.assertEqual(alone["value"], 8)
            await b.send(protocol.crew(SPEC["crew_busy"], a.id))
            self.assertFalse(self.server.crews.offers)
        finally:
            crews.JOIN_WAIT = SPEC["crew_join_ms"] / 1000
        # (only crewmates in the owner's room are offered: none, the owner alone at once)
        await self.enter(b, 3)
        await self.enter(c, 3)
        await a.send(protocol.crew(SPEC["crew_battle"], 0, 1))
        self.assertEqual(crew_state(await self.until(a, SPEC["crew_state"], lambda m: m[1] == SPEC["crew_alone"]))["value"], 1)

    async def test_a_crews_battle_never_offered_without_one_field(self):
        a, b = await self.connect("Alpha"), await self.connect("Beta", no_arena=True)
        for client in (a, b):
            await self.enter(client, 2)
        await self.crew_of(a, b)
        # (Beta's system cannot run the one field: Alpha's battle is offered to nobody, and Beta's offers nothing)
        await a.send(protocol.crew(SPEC["crew_battle"], 0, 2))
        self.assertEqual(crew_state(await self.until(a, SPEC["crew_state"], lambda m: m[1] == SPEC["crew_alone"]))["value"], 2)
        await b.send(protocol.crew(SPEC["crew_battle"], 0, 3))
        self.assertEqual(crew_state(await self.until(b, SPEC["crew_state"], lambda m: m[1] == SPEC["crew_alone"]))["value"], 3)
        self.assertFalse(self.server.crews.offers)

    # -- the door's queue and the party's battle --

    async def test_a_doors_queue_and_its_party_battle(self):
        a, b, c, d, e = [await self.connect(n) for n in ("Alpha", "Beta", "Gamma", "Delta", "Echo")]
        for client in (a, b, c, d, e):
            await self.enter(client, 3, 0, 1)
        door = (3, 0, 1)
        await a.send(protocol.queue(SPEC["queue_join"], (3, 0, 2)))
        self.assertEqual(await self.notice(a), SPEC["notice_queue"])
        for client in (a, b, c, d):
            await client.send(protocol.queue(SPEC["queue_join"], door))
        state = queue_state(await self.until(d, SPEC["queue_state"], lambda m: m[6] == 4))
        self.assertEqual((state["members"], state["place"], state["door"]), ([a.id, b.id, c.id, d.id], 4, door))
        await e.send(protocol.queue(SPEC["queue_join"], door))
        self.assertEqual(await self.notice(e), SPEC["notice_queue"])
        # (the leader alone starts or passes; a pass puts him last)
        await b.send(protocol.queue(SPEC["queue_start"]))
        self.assertEqual(await self.notice(b), SPEC["notice_queue"])
        await a.send(protocol.queue(SPEC["queue_pass"]))
        state = queue_state(await self.until(b, SPEC["queue_state"], lambda m: queue_state(m)["members"][0] == b.id))
        self.assertEqual((state["members"], state["place"]), ([b.id, c.id, d.id, a.id], 1))
        await d.send(protocol.queue(SPEC["queue_leave"]))
        self.assertEqual(queue_state(await self.until(d, SPEC["queue_state"], lambda m: m[1] == SPEC["queue_out"]))["place"], 0)
        await b.send(protocol.queue(SPEC["queue_start"]))
        self.assertEqual(queue_state(await self.until(c, SPEC["queue_state"], lambda m: m[1] == SPEC["queue_started"]))["members"],
                         [b.id, c.id, a.id])
        starts = {x: arena_state(await self.until(x, SPEC["arena_state"])) for x in (b, c, a)}
        self.assertEqual([starts[x]["side"] for x in (b, c, a)], [0, 1, 2])
        self.assertEqual(starts[a]["seats"], (b.id, c.id, a.id, 0))
        self.assertEqual(starts[b]["other"], c.id)
        battle = starts[b]["battle"]
        # (a seat's keys to the other seats as ARENA_SEAT)
        keys = protocol.ARENA_INPUT.pack(SPEC["arena_input"], battle, 0, 2) + b"\x00\x01\x00\x02"
        await c.send(keys)
        for x in (a, b):
            self.assertEqual(await x.message(SPEC["arena_seat"]), bytes((SPEC["arena_seat"], 1)) + keys)
        # (a seat gone: the others told which, the battle goes on)
        await a.send(protocol.ARENA.pack(SPEC["arena"], LEAVE, battle, 0, 0))
        gone = arena_state(await self.until(b, SPEC["arena_state"]))
        self.assertEqual((gone["stage"], gone["side"], gone["other"]), (SPEC["arena_seat_gone"], 2, a.id))
        await c.send(keys)
        self.assertEqual(await b.message(SPEC["arena_seat"]), bytes((SPEC["arena_seat"], 1)) + keys)
        # (seat 0's report and over: ended for the rest, and the feed's guardian down by a party of three)
        status = protocol.ARENA_STATUS.pack(SPEC["arena_status"], battle, 300, 500, 0, 900, 0, 0, 3, 0, 1)
        await c.send(protocol.ARENA.pack(SPEC["arena"], OVER, battle, 0, 0))
        await b.send(status)
        await b.send(protocol.ARENA.pack(SPEC["arena"], OVER, battle, 0, 0))
        self.assertEqual(arena_state(await self.until(c, SPEC["arena_state"], lambda m: m[1] == ENDED))["battle"], battle)
        feed = protocol.DIVE_FEED.unpack(await self.until(e, SPEC["dive_feed"], lambda m: m[1] == SPEC["feed_guardian"]))
        self.assertEqual((name(feed[2]), feed[3], feed[7]), ("Beta", 3, 3))
        self.assertFalse(self.server.parties.battles or self.server.parties.seat_of)
        # (a leader alone starts alone)
        await e.send(protocol.queue(SPEC["queue_join"], door))
        await self.until(e, SPEC["queue_state"], lambda m: m[1] == SPEC["queue_waiting"])
        await e.send(protocol.queue(SPEC["queue_start"]))
        self.assertEqual(queue_state(await self.until(e, SPEC["queue_state"], lambda m: m[1] == SPEC["queue_alone"]))["place"], 1)

    async def test_linked_fighters_and_a_seat_dropped(self):
        a, b, c = await self.connect("Alpha"), await self.connect("Beta"), await self.connect("Gamma", no_arena=True)
        self.assertTrue(self.server.players[c.id].no_arena)
        for client in (a, b, c):
            await self.enter(client, 6, 0, 3)
            await client.send(protocol.queue(SPEC["queue_join"], (6, 0, 3)))
        state = queue_state(await self.until(a, SPEC["queue_state"], lambda m: m[6] == 3))
        self.assertEqual(state["arena"], [SPEC["queue_arena"], SPEC["queue_arena"], 0])
        await a.send(protocol.queue(SPEC["queue_start"]))
        starts = {x: arena_state(await self.until(x, SPEC["arena_state"])) for x in (a, b, c)}
        self.assertEqual([starts[x]["form"] for x in (a, b, c)], [SPEC["arena_party"], SPEC["arena_party"], SPEC["arena_linked"]])
        battle = starts[a]["battle"]
        # (a linked fighter's damage to seat 0 alone, seat 0's HP to the linked alone; a party seat's keys never to him)
        damage = protocol.party(SPEC["party_damage"], battle, 120, 80)
        await c.send(damage)
        self.assertEqual(await a.message(SPEC["arena_seat"]), bytes((SPEC["arena_seat"], 2)) + damage)
        hp = protocol.party(SPEC["party_hp"], battle, 880, 1000)
        await a.send(hp)
        self.assertEqual(await c.message(SPEC["party_hp"]), hp)
        await b.send(hp)
        await a.send(protocol.ARENA_INPUT.pack(SPEC["arena_input"], battle, 0, 1) + b"\x00\x08")
        await b.message(SPEC["arena_seat"])
        for _ in range(30):
            self.assertNotIn((await c.receive())[0], (SPEC["arena_seat"], SPEC["party_hp"]))
        # (a dropped link, its hold over: the seat gone for the others, the battle on; SNAPSHOT says busy)
        party.SEAT_HOLD = 0.3
        await b.drop()
        self.clients.remove(b)
        gone = arena_state(await self.until(a, SPEC["arena_state"]))
        self.assertEqual((gone["stage"], gone["side"]), (SPEC["arena_seat_gone"], 1))
        self.assertEqual(arena_state(await self.until(c, SPEC["arena_state"]))["stage"], SPEC["arena_seat_gone"])
        self.assertEqual(self.server.busy_with(c.id), SPEC["busy_battle"])
        await c.send(damage)
        await a.message(SPEC["arena_seat"])

    async def party_of(self, *labels, door=(5, 0, 2)):
        """A door's party battle of these divers: their clients and the battle's id."""
        clients = [await self.connect(label) for label in labels]
        for client in clients:
            await self.enter(client, *door)
            await client.send(protocol.queue(SPEC["queue_join"], door))
        await self.until(clients[0], SPEC["queue_state"], lambda m: m[6] == len(clients))
        await clients[0].send(protocol.queue(SPEC["queue_start"]))
        starts = [arena_state(await self.until(x, SPEC["arena_state"])) for x in clients]
        return clients, starts[0]["battle"]

    async def test_a_seat_held_through_a_dropped_link_and_taken_back(self):
        # (issue #180: a short drop ridden out, the others told nothing, the seat's keys relayed again once back)
        (a, b, c), battle = await self.party_of("Alpha", "Beta", "Gamma")
        await b.drop()
        self.clients.remove(b)
        await until_true(lambda: b.id in self.server.parties.battles[battle].away)
        keys = protocol.ARENA_INPUT.pack(SPEC["arena_input"], battle, 0, 1) + b"\x00\x01"
        await c.send(keys)
        await a.message(SPEC["arena_seat"])
        b = await self.connect("Beta", resume=True)
        self.assertNotIn(b.id, self.server.parties.battles[battle].away)
        await c.send(keys)
        self.assertEqual(await b.message(SPEC["arena_seat"]), bytes((SPEC["arena_seat"], 2)) + keys)
        await b.send(keys)
        self.assertEqual(await self.until(a, SPEC["arena_seat"], lambda m: m[1] == 1), bytes((SPEC["arena_seat"], 1)) + keys)
        for _ in range(30):
            message = await a.receive()
            self.assertFalse(message[0] == SPEC["arena_state"] and message[1] == SPEC["arena_seat_gone"])

    async def test_a_battle_over_while_a_seat_is_held_is_told_on_its_resume(self):
        (a, b), battle = await self.party_of("Alpha", "Beta")
        await b.drop()
        self.clients.remove(b)
        await until_true(lambda: b.id in self.server.parties.battles[battle].away)
        await a.send(protocol.ARENA.pack(SPEC["arena"], OVER, battle, 0, 0))
        await until_true(lambda: battle not in self.server.parties.battles)
        b = await self.connect("Beta", resume=True)
        gone = arena_state(await self.until(b, SPEC["arena_state"]))
        self.assertEqual((gone["stage"], gone["side"], gone["other"]), (SPEC["arena_seat_gone"], 0, a.id))
        self.assertEqual(arena_state(await self.until(b, SPEC["arena_state"]))["stage"], ENDED)
        self.assertFalse(self.server.parties.missed or self.server.parties.seat_of)

    async def test_a_seat_zero_without_the_arena_links_every_seat(self):
        a, b = await self.connect("Alpha", no_arena=True), await self.connect("Beta")
        for client in (a, b):
            await self.enter(client, 1)
            await client.send(protocol.queue(SPEC["queue_join"], (1, 0, 0)))
        await self.until(a, SPEC["queue_state"], lambda m: m[6] == 2)
        await a.send(protocol.queue(SPEC["queue_start"]))
        self.assertEqual([arena_state(await self.until(x, SPEC["arena_state"]))["form"] for x in (a, b)],
                         [SPEC["arena_linked"], SPEC["arena_linked"]])

    # -- the Board --

    async def test_week_post_carries_the_party_acts(self):
        a, b = await self.connect("Alpha"), await self.connect("Beta")
        await a.send(week.post(self.week, self.seed, 7, 3, False, 40, 9, party=0b101))
        number, place, mine, top, _ = board(await self.until(a, SPEC["week_board"], lambda m: board(m)[1] == 1))
        self.assertEqual(mine[3], SPEC["week_party"])
        await b.send(week.post(self.week, self.seed, 4, 2, False, 40, 9))
        _, _, mine, top, _ = board(await self.until(b, SPEC["week_board"], lambda m: board(m)[1] == 2))
        self.assertEqual((mine[3], top[0][3]), (0, SPEC["week_party"]))
        for bad in (week.post(self.week, self.seed, 4, 2, False, 40, 9, party=0b100),
                    week.post(self.week, self.seed, 10, 4, True, 40, 9, party=0x10)):
            await b.send(bad)
            self.assertEqual(await self.notice(b), 25)
        await asyncio.gather(*self.server.writes)       # (the appends in their threads, done)
        lines = [json.loads(line) for line in (Path(self.dir.name) / "weeks.jsonl").read_text().splitlines()]
        self.assertEqual([line.get("party") for line in lines], [5, 0])

    async def test_a_dives_end_reaches_the_board_without_a_post(self):
        """v27 (issue #759): the server posts a dive's end itself, its layer the room's; the game's own WEEK_POST
        after it changes nothing, a worse end never takes a better one's place, a better one does."""
        a, b = await self.connect("Alpha"), await self.connect("Beta")
        await self.enter(a, 4)
        await a.send(protocol.dive(END, SPEC["dive_deleted"], 0, 0, 0, 0, 33, SPEC["week_pulled"], 1))
        _, _, _, top, _ = board(await self.until(b, SPEC["week_board"], lambda m: board(m)[1] == 0 and len(board(m)[3]) == 1))
        self.assertEqual((top[0][1], top[0][2], top[0][3], top[0][4]), (4, 2, SPEC["week_pulled"] | SPEC["week_party"], 33))
        self.assertEqual(self.server.daily.today["dives"], 1)
        # (the game's own post of that dive: the same entry, no second line, no second dive counted)
        await a.send(week.post(self.week, self.seed, 4, 2, False, 33, 9, party=1, pulled=True))
        await self.until(a, SPEC["week_board"])
        self.assertEqual(self.server.daily.today["dives"], 1)
        # (a worse end after it, layer 2: the best stays; a better, layer 5: it takes the place)
        await self.enter(a, 2)
        await a.send(protocol.dive(END, SPEC["dive_deleted"], 0, 0, 0, 0, 10, 0, 0))
        await self.enter(a, 5)
        await a.send(protocol.dive(END, SPEC["dive_deleted"], 0, 0, 0, 0, 50, 0, 0))
        _, _, _, top, _ = board(await self.until(b, SPEC["week_board"], lambda m: board(m)[3] and board(m)[3][0][1] == 5))
        self.assertEqual((top[0][1], top[0][3], top[0][4], len(top)), (5, 0, 50, 1))
        # (a won end only on the last layer, an act fought in a party past the dive's: nothing posted)
        await self.enter(a, 2)
        await a.send(protocol.dive(END, SPEC["dive_deleted"], 0, 0, 0, 0, 1, 0, 0b100))
        await asyncio.gather(*self.server.writes)
        lines = [json.loads(line) for line in (Path(self.dir.name) / "weeks.jsonl").read_text().splitlines()]
        self.assertEqual([(line["depth"], line["minutes"], line.get("pulled")) for line in lines], [(4, 33, True), (5, 50, False)])

    async def test_week_post_carries_the_crews_pull_back(self):
        a, b = await self.connect("Alpha"), await self.connect("Beta")
        await a.send(week.post(self.week, self.seed, 7, 3, False, 40, 9, pulled=True))
        _, _, mine, _, _ = board(await self.until(a, SPEC["week_board"], lambda m: board(m)[1] == 1))
        self.assertEqual(mine[3], SPEC["week_pulled"])
        # (ranked with the rest: a deeper dive pulled back above a shallower one that was not)
        await b.send(week.post(self.week, self.seed, 4, 2, False, 30, 9))
        _, _, mine, top, _ = board(await self.until(b, SPEC["week_board"], lambda m: board(m)[1] == 2))
        self.assertEqual((top[0][3], top[1][3], mine[3]), (SPEC["week_pulled"], 0, 0))
        bad = week.post(self.week, self.seed, 4, 2, False, 40, 9)
        await b.send(bad[:10] + bytes([8]) + bad[11:])
        self.assertEqual(await self.notice(b), 25)
        await asyncio.gather(*self.server.writes)
        lines = [json.loads(line) for line in (Path(self.dir.name) / "weeks.jsonl").read_text().splitlines()]
        self.assertEqual([line.get("pulled") for line in lines], [True, False])

    # -- moderation --

    async def test_a_moderators_hide_takes_the_traces_too(self):
        a, b = await self.connect("Alpha"), await self.connect("Beta")
        await self.enter(a, 2)
        await self.enter(b, 2)
        await a.send(protocol.dive(PHRASE, 1, 0, 0, 0))
        await a.send(protocol.dive(FOUND, 3))
        await a.send(protocol.dive(END, SPEC["dive_deleted"], 1, 1, 0, 2, 0, 0, 0))
        await b.send(protocol.dive(PHRASE, 3, 5, 5, 0))
        await self.until(a, SPEC["dive_traces"], lambda d: d[1] == SPEC["trace_phrases"] and b"Beta" in d)
        await asyncio.gather(*self.server.writes)       # (the traces' appends in their threads, done)
        result = admin.perform("hide", self.dir.name, a.identity.hex(), "test", week=self.week)
        # (his phrase and his mark gone, his find become the opened glow alone)
        self.assertEqual((result["week"], result["traces"]), (self.week, 2))
        self.server.store.reload()
        await b.send(protocol.dive(LEAVE_DIVE))
        await b.message(SPEC["welcome"])
        await b.send(protocol.dive(ENTER, self.week, self.seed, 2, 0, 0))
        await b.message(SPEC["dive_room"])
        got = {}
        for _ in range(2):
            kind, entries = traces(await b.message(SPEC["dive_traces"]))
            got[kind] = entries
        self.assertEqual([name(p[1]) for p in got[SPEC["trace_phrases"]]], ["Beta"])
        self.assertEqual(got[SPEC["trace_opened"]], [(3,)])
        await a.send(protocol.dive(PHRASE, 1, 0, 0, 0))
        self.assertEqual(await self.notice(a), 26)
        lines = [json.loads(line) for line in (Path(self.dir.name) / "dives.jsonl").read_text().splitlines()]
        self.assertEqual(sorted((line["kind"], line["name"]) for line in lines), [("open", ""), ("phrase", "Beta")])
        result = admin.perform("forget", self.dir.name, b.identity.hex())
        self.assertEqual(result["dives"], 1)


if __name__ == "__main__":
    unittest.main()
