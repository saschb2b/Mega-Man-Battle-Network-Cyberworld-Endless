"""Team NetBattles (epic #167, protocol v12): teams of two and the relay; no ROM needed."""

import asyncio
from pathlib import Path
import struct
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "server"))
sys.path.insert(0, str(Path(__file__).resolve().parent))

from cyberworld_server import protocol, teams
from cyberworld_server.app import Server
from test_server import Client
from test_stands import arena, status, stands, watch

SPEC = protocol.SPEC
ARENA_START, ARENA_ENDED = 3, 4


def team(action, ref, value=0):
    return protocol.TEAM.pack(SPEC["team"], action, ref, value)


def team_state(data):
    """TEAM_STATE as a dict"""
    _, stage, partner, match, bout, *rest = protocol.TEAM_STATE.unpack(data)
    players, (left_us, left_them, number, result, hp) = rest[:4], rest[4:]
    return dict(stage=stage, partner=partner, match=match, bout=bout, players=players, left=(left_us, left_them),
                number=number, result=result, hp=hp)


class TeamTests(unittest.IsolatedAsyncioTestCase):
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
        client = Client()
        self.clients.append(client)
        welcome = await client.open(self.port, name, bn5=full)
        return client, struct.unpack("!I", welcome[5:9])[0]

    async def state(self, client, stage=None, number=0):
        for _ in range(100):
            got = team_state(await client.message(SPEC["team_state"]))
            if (stage is None or got["stage"] == stage) and got["number"] >= number:
                return got
        raise AssertionError("No such TEAM_STATE")

    async def started(self, client, bout):
        for _ in range(20):
            state = protocol.ARENA_STATE.unpack(await client.message(SPEC["arena_state"]))
            if state[1] == ARENA_START and state[2] == bout:
                return state
        raise AssertionError("No START")

    async def notice(self, client):
        return struct.unpack("!H", (await client.message(SPEC["notice"]))[1:3])[0]

    async def pair(self, captain, partner, partner_id):
        await captain[0].send(team(teams.PROPOSE, partner_id))
        self.assertEqual((await self.state(captain[0]))["stage"], teams.WAITING)
        asked = await self.state(partner[0])
        self.assertEqual((asked["stage"], asked["partner"]), (teams.ASKED, captain[1]))
        await partner[0].send(team(teams.ANSWER, captain[1], 1))
        formed = await self.state(captain[0], teams.FORMED)
        self.assertEqual(formed["partner"], partner_id)
        self.assertEqual((await self.state(partner[0], teams.FORMED))["partner"], captain[1])

    async def four(self):
        """Two teams: Alpha (captain) and Beta, Gamma (captain) and Delta; Alpha challenges Gamma, who takes it."""
        a, b, c, d = [await self.connect(n) for n in ("Alpha", "Beta", "Gamma", "Delta")]
        await self.pair(a, b, b[1])
        await self.pair(c, d, d[1])
        await a[0].send(arena(1, c[1], 30))
        challenge = protocol.ARENA_STATE.unpack(await c[0].message(SPEC["arena_state"]))
        await c[0].send(arena(2, challenge[2], 30))
        for who in (a, b, c, d):
            got = await self.state(who[0], teams.MATCH)
            self.assertEqual(got["number"], 0)
        return a, b, c, d

    async def bout(self, x, y, called=None):
        """A bout called: both fighters say they are ready, both STARTed; its call and x's side"""
        called = called or await self.state(x[0], teams.BOUT)
        bout = called["bout"]
        await x[0].send(arena(2, bout, 20))
        await asyncio.sleep(0.05)
        self.assertNotIn(bout, self.server.stands.books)          # (one ready: no START yet)
        await y[0].send(arena(2, bout, 20))
        sx, sy = await self.started(x[0], bout), await self.started(y[0], bout)
        self.assertEqual({sx[5], sy[5]}, {0, 1})
        return called, sx[5]

    async def test_a_team_is_asked_for_and_split(self):
        a, b = [await self.connect(n) for n in ("Alpha", "Beta")]
        await self.pair(a, b, b[1])
        c, _ = await self.connect("Gamma")
        await c.send(team(teams.PROPOSE, a[1]))
        self.assertEqual(await self.notice(c), SPEC["notice_team"])          # (Alpha is on a team)
        await c.send(arena(1, a[1], 20))
        self.assertEqual(await self.notice(c), SPEC["notice_team"])          # (a lone Navi against a team's)
        await a[0].send(arena(1, b[1], 20))
        self.assertEqual(await self.notice(a[0]), SPEC["notice_team"])       # (his own partner)
        await b[0].send(team(teams.SPLIT, 0))
        self.assertEqual((await self.state(a[0], teams.SPLIT_UP))["partner"], b[1])
        self.assertEqual((await self.state(b[0], teams.SPLIT_UP))["partner"], a[1])
        self.assertFalse(self.server.teams.of)

    async def test_a_team_split_before_the_yes_turns_the_challenge_down(self):
        a, b, c, d = [await self.connect(n) for n in ("Alpha", "Beta", "Gamma", "Delta")]
        await self.pair(a, b, b[1])
        await self.pair(c, d, d[1])
        await a[0].send(arena(1, c[1], 30))
        challenge = protocol.ARENA_STATE.unpack(await c[0].message(SPEC["arena_state"]))
        await d[0].send(team(teams.SPLIT, 0))
        await self.state(c[0], teams.SPLIT_UP)
        await c[0].send(arena(2, challenge[2], 30))
        ended = protocol.ARENA_STATE.unpack(await a[0].message(SPEC["arena_state"]))
        while ended[1] != ARENA_ENDED:
            ended = protocol.ARENA_STATE.unpack(await a[0].message(SPEC["arena_state"]))
        self.assertEqual(ended[2], challenge[2])
        self.assertFalse(self.server.teams.matches)

    async def test_a_no_and_a_lone_battle_as_ever(self):
        a, b = [await self.connect(n) for n in ("Alpha", "Beta")]
        await a[0].send(team(teams.PROPOSE, b[1]))
        await self.state(b[0], teams.ASKED)
        await b[0].send(team(teams.ANSWER, a[1], 0))
        self.assertEqual((await self.state(a[0], teams.SPLIT_UP))["partner"], b[1])
        await a[0].send(arena(1, b[1], 20))
        self.assertEqual(protocol.ARENA_STATE.unpack(await b[0].message(SPEC["arena_state"]))[1], 1)

    async def test_the_relay_from_the_blind_order_to_the_last_bout(self):
        a, b, c, d = await self.four()
        # (the order blind: Alpha's pick shows nothing until Gamma's is in)
        await a[0].send(team(teams.ORDER, b[1]))
        await asyncio.sleep(0.05)
        await c[0].send(team(teams.ORDER, c[1]))
        for who in (a, b, c, d):
            ordered = await self.state(who[0], teams.ORDERED)
            mine = (b[1], a[1]) if who in (a, b) else (c[1], d[1])
            theirs = (c[1], d[1]) if who in (a, b) else (b[1], a[1])
            self.assertEqual(tuple(ordered["players"]), mine + theirs)
            self.assertEqual(ordered["left"], (2, 2))
        # bout 1: Beta against Gamma; Alpha and Delta are told it is on, and watch it
        called, side = await self.bout(b, c)
        self.assertEqual((called["number"], called["hp"]), (1, 0))
        first, second = (b, c) if side == 0 else (c, b)
        for who in (a, d):
            fight = await self.state(who[0], teams.FIGHT)
            self.assertEqual(fight["bout"], called["bout"])
            await who[0].send(watch(1, fight["bout"]))
            self.assertEqual(protocol.WATCH_STATE.unpack(await who[0].message(SPEC["watch_state"]))[1], 1)
        entries = stands(await a[0].message(SPEC["stands"]))
        self.assertEqual(entries[0][-1], 1)                                  # (STANDS: bout 1 of a team match)
        # Beta wins with 40 HP left: Delta comes in against him, his HP carried
        await first[0].send(status(called["bout"], 40 if first is b else 0, 0 if first is b else 40, winner=1 if first is b else 2))
        await first[0].send(arena(4, called["bout"]))
        called2 = await self.state(b[0], teams.BOUT, 2)
        self.assertEqual((called2["number"], called2["hp"], called2["left"]), (2, 40, (2, 1)))
        self.assertEqual(tuple(called2["players"][:3:2]), (b[1], d[1]))
        _, side = await self.bout(b, d, called2)
        first = b if side == 0 else d
        # Delta wins: Alpha comes in against him
        await first[0].send(status(called2["bout"] + 0, 0 if first is b else 25, 25 if first is b else 0, winner=2 if first is b else 1))
        await first[0].send(arena(4, called2["bout"]))
        called3 = await self.state(a[0], teams.BOUT, 3)
        self.assertEqual((called3["number"], called3["hp"], called3["left"]), (3, 25, (1, 1)))
        _, side = await self.bout(a, d, called3)
        first = a if side == 0 else d
        await first[0].send(status(called3["bout"], 60 if first is a else 0, 0 if first is a else 60, winner=1 if first is a else 2))
        await first[0].send(arena(4, called3["bout"]))
        for who, result in ((a, teams.WON), (b, teams.WON), (c, teams.LOST), (d, teams.LOST)):
            over = await self.state(who[0], teams.OVER)
            self.assertEqual(over["result"], result)
        self.assertFalse(self.server.teams.matches)
        self.assertEqual(len(self.server.teams.of), 4)                       # (the teams stay, for another)

    async def test_a_drop_mid_match_breaks_it_off_for_both_teams(self):
        a, b, c, d = await self.four()
        await a[0].send(team(teams.ORDER, a[1]))
        await c[0].send(team(teams.ORDER, d[1]))
        called, _ = await self.bout(a, d)
        await d[0].close()
        state = protocol.ARENA_STATE.unpack(await a[0].message(SPEC["arena_state"]))
        self.assertEqual((state[1], state[2]), (ARENA_ENDED, called["bout"]))
        for who in (a, b, c):
            over = await self.state(who[0], teams.OVER)
            self.assertEqual(over["result"], teams.BROKE_OFF)
        self.assertEqual((await self.state(c[0], teams.SPLIT_UP))["partner"], d[1])
        self.assertFalse(self.server.teams.matches)
        self.assertFalse(self.server.arena.matches)

    async def test_an_order_not_picked_leads_the_captain_and_a_bout_needs_both_ready(self):
        a, b, c, d = await self.four()
        await a[0].send(team(teams.ORDER, b[1]))
        await asyncio.sleep(0.05)
        match = next(iter(self.server.teams.matches.values()))
        match.time -= teams.ORDER_WAIT + 1
        self.server.teams.expire(match.time + teams.ORDER_WAIT + 2)
        ordered = await self.state(c[0], teams.ORDERED)
        self.assertEqual(tuple(ordered["players"]), (c[1], d[1], b[1], a[1]))
        # (a bout's fighter busy with another challenge: refused, the bout goes on)
        other, _ = await self.connect("Epsilon")
        await other.send(arena(1, b[1], 20))
        self.assertEqual(await self.notice(other), 16)
        # (nor a trade with a player of a team match, waiting for his bout)
        await other.send(protocol.TRADE.pack(SPEC["trade"], 1, a[1], 1, 0, 0, 0))
        self.assertEqual(await self.notice(other), 16)
        # (both teams out in one bout: drawn)
        called, side = await self.bout(b, c)
        first = b if side == 0 else c
        await first[0].send(arena(4, called["bout"]))           # (no report: no winner, both out)
        called2 = await self.state(a[0], teams.BOUT, 2)
        self.assertEqual((called2["number"], called2["left"]), (2, (1, 1)))


if __name__ == "__main__":
    unittest.main()
