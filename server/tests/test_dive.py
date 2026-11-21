"""The shared dive over real sockets (protocol v19, issue #589): rooms,
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

from cyberworld_server import admin, crews, protocol, week
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
                ways={m[0]: m[2:] for m in members if m[0]})


def queue_state(data):
    _, stage, depth, side, way, place, count = protocol.QUEUE_STATE_HEAD.unpack_from(data)
    entries = [protocol.QUEUE_ENTRY.unpack_from(data, protocol.QUEUE_STATE_HEAD.size + k * protocol.QUEUE_ENTRY.size)
               for k in range(count)]
    return dict(stage=stage, door=(depth, side, way), place=place, members=[e[0] for e in entries],
                arena=[e[2] for e in entries])


def name(raw):
    return raw.rstrip(b"\0").decode()


class DiveTests(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        self.dir = tempfile.TemporaryDirectory()
        self.now = MONDAY + 3600
        self.clients = []
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
        for message in (protocol.dive(FOUND, 1), protocol.crew(SPEC["crew_way"], 0, 0, (2, 0, 0)), protocol.queue(SPEC["queue_leave"])):
            await stamp.send(message)
            self.assertEqual(await self.notice(stamp), SPEC["notice_dive_out"])
        # (a diver trades, teams up and watches nothing)
        for message in (protocol.OFFER.pack(SPEC["offer"], 5, 1, 0), protocol.TEAM.pack(SPEC["team"], 1, stamp.id, 0),
                        protocol.ARENA.pack(SPEC["arena"], 1, stamp.id, 20, 0)):
            await a.send(message)
            self.assertEqual(await self.notice(a), SPEC["notice_dive_in"])

    async def test_malformed_dives_close_the_connection(self):
        for bad in (protocol.dive(ENTER, self.week, self.seed, 11, 0, 0), protocol.dive(ENTER, self.week, self.seed, 1, 0, 0)[:-1],
                    bytes((SPEC["dive"], 9)), protocol.dive(PHRASE, 8, 0, 0, 0), protocol.dive(END, 3, 0, 0, 0, 0),
                    protocol.dive(SPLIT, 1, 10, 12, 0, 1), protocol.dive(SPLIT, 1, 10, 3, 0, 5), protocol.dive(SPLIT, 1, 0, 3, 0, 1),
                    protocol.dive(PHRASE, 1, 9000, 0, 0), protocol.crew(7), protocol.crew(SPEC["crew_ask"]),
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
        await b.send(protocol.dive(END, SPEC["dive_deleted"], -5, 6, 0, 0x91))
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
            await client.send(protocol.dive(END, SPEC["dive_deleted"], k, 0, 0, 1))
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
        await c.send(protocol.dive(END, SPEC["dive_deleted"], 0, 0, 0, 0x1F))
        for client in (square, a, b):
            feed = protocol.DIVE_FEED.unpack(await self.until(client, SPEC["dive_feed"], lambda d: d[1] == SPEC["feed_deleted"]))
            self.assertEqual((name(feed[2]), feed[3], feed[6]), ("Gamma", 4, 0x1F))
        # (C, behind no one and in act 2, had no flare; last week's diver no feed)
        for _ in range(30):
            data = await c.receive()
            self.assertFalse(data[0] == SPEC["dive_feed"] and data[1] == SPEC["feed_flare"])
        for _ in range(30):
            self.assertNotEqual((await last.receive())[0], SPEC["dive_feed"])
        await a.send(protocol.dive(END, SPEC["dive_won"], 0, 0, 0, 0))
        self.assertEqual(await self.notice(a), 25)

    # -- crews --

    async def test_crews_ask_answer_ways_and_split(self):
        a, b, c, d, e = [await self.connect(n) for n in ("Alpha", "Beta", "Gamma", "Delta", "Echo")]
        for client in (a, b, c, d, e):
            await self.enter(client, 2)
        await a.send(protocol.crew(SPEC["crew_ask"], b.id))
        self.assertEqual(crew_state(await b.message(SPEC["crew_state"]))["stage"], SPEC["crew_asked"])
        self.assertEqual(crew_state(await a.message(SPEC["crew_state"]))["stage"], SPEC["crew_waiting"])
        await b.send(protocol.crew(SPEC["crew_answer"], a.id, 1))
        formed = crew_state(await a.message(SPEC["crew_state"]))
        self.assertEqual((formed["stage"], formed["members"]), (SPEC["crew_formed"], [a.id, b.id]))
        # (any member asks; four at most)
        for asker, other in ((b, c), (a, d)):
            await asker.send(protocol.crew(SPEC["crew_ask"], other.id))
            await self.until(other, SPEC["crew_state"], lambda m: m[1] == SPEC["crew_asked"])
            await other.send(protocol.crew(SPEC["crew_answer"], asker.id, 1))
        state = crew_state(await self.until(a, SPEC["crew_state"], lambda m: crew_state(m)["members"] == [a.id, b.id, c.id, d.id]))
        self.assertEqual(state["stage"], SPEC["crew_formed"])
        await a.send(protocol.crew(SPEC["crew_ask"], e.id))
        self.assertEqual(await self.notice(a), SPEC["notice_crew"])
        # (a way: everyone's told everyone's)
        await c.send(protocol.crew(SPEC["crew_way"], 0, 0, (3, 1, 9)))
        state = crew_state(await self.until(d, SPEC["crew_state"], lambda m: crew_state(m)["ways"].get(c.id) == (3, 1, 9)))
        self.assertEqual(state["ways"][a.id], (0, 0, 0))
        # (a crew lasts across layers; a split takes one out, the rest go on)
        await self.enter(c, 3)
        await c.send(protocol.crew(SPEC["crew_split"]))
        self.assertEqual(crew_state(await self.until(c, SPEC["crew_state"], lambda m: m[1] == SPEC["crew_split_up"]))["stage"],
                         SPEC["crew_split_up"])
        state = crew_state(await self.until(a, SPEC["crew_state"], lambda m: c.id not in crew_state(m)["members"]))
        self.assertEqual(state["members"], [a.id, b.id, d.id])
        # (an ask outside the room, and a diver's end of his dive: out of the crew)
        await a.send(protocol.crew(SPEC["crew_ask"], c.id))
        self.assertEqual(await self.notice(a), 17)
        await d.send(protocol.dive(END, SPEC["dive_deleted"], 0, 0, 0, 0))
        await self.until(a, SPEC["crew_state"], lambda m: crew_state(m)["members"] == [a.id, b.id])
        await b.close()
        self.clients.remove(b)
        state = crew_state(await self.until(a, SPEC["crew_state"], lambda m: m[1] == SPEC["crew_split_up"]))
        self.assertEqual(state["members"], [])
        self.assertFalse(self.server.crews.of)

    async def crew_of(self, *clients):
        lead = clients[0]
        for other in clients[1:]:
            await lead.send(protocol.crew(SPEC["crew_ask"], other.id))
            await self.until(other, SPEC["crew_state"], lambda m: m[1] == SPEC["crew_asked"])
            await other.send(protocol.crew(SPEC["crew_answer"], lead.id, 1))
            await self.until(lead, SPEC["crew_state"], lambda m, o=other: o.id in crew_state(m)["members"])

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
        # (a dropped link: the seat gone for the others, the battle on; SNAPSHOT says busy)
        await b.drop()
        self.clients.remove(b)
        gone = arena_state(await self.until(a, SPEC["arena_state"]))
        self.assertEqual((gone["stage"], gone["side"]), (SPEC["arena_seat_gone"], 1))
        self.assertEqual(arena_state(await self.until(c, SPEC["arena_state"]))["stage"], SPEC["arena_seat_gone"])
        self.assertEqual(self.server.busy_with(c.id), SPEC["busy_battle"])
        await c.send(damage)
        await a.message(SPEC["arena_seat"])

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
        await a.send(protocol.dive(END, SPEC["dive_deleted"], 1, 1, 0, 2))
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
