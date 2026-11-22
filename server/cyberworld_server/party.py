"""The guardians' doors' queues and the party battles of the shared dive
(protocol v19, issue #589; docs/online/shared-dive.md, The owner's
decisions; shared/online/README.md, Protocol v19).

A door's queue is kept by its layer (dive.py's room key: week, seed,
depth, side and way, every instance of it): a diver on that layer joins
it (QUEUE join, its depth, side and way his room's; NOTICE notice_queue
otherwise, or where it holds party_most already), and every diver of
the week behind him in the same act is flared (dive.py). The first in
it leads: he starts the battle when he likes (QUEUE start; anyone else's
is refused), anyone leaves when he likes (QUEUE leave), and a leader
whose game says he has been idle a minute passes the lead to the next
(QUEUE pass: he goes to the back). QUEUE_STATE tells every member the
queue as it stands, the leader first, each one's place; started where
the party battle began, out to one who left it, alone to a leader who
started with nobody beside him.

A party battle (a door's, or a crew's join-in battle, crews.py) is a
battle ID from the Arena's count and seats 0..N-1, seat 0 the leader (or
the battle's owner), each told with ARENA_STATE START: its seat, the input
delay from the seats' round trips as arena.py fits a pair's, the format
(`arena_party`, one field; `arena_linked` for a member whose system
cannot run the Arena, HELLO's hello_no_arena, and for every member where
seat 0's cannot) and the four seats' sessions. What a party seat sends
(ARENA_LOADOUT, ARENA_INPUT, ARENA_HASH) goes to the other party seats as
ARENA_SEAT, as a 2v2's does; a linked fighter's PARTY_DAMAGE goes to seat
0 as ARENA_SEAT, and seat 0's PARTY_HP to every linked fighter as sent.
Seat 0 reports with ARENA_STATUS and ends the battle with ARENA over
(everyone told ended); a seat's leave or drop is told to the others
(ARENA_STATE seat_gone, its side byte the seat) and the battle goes on
for the rest, each game finishing its own (the owner's call). A drop is
ridden out first (issue #180, docs/online/arena/dropped-link.md): a seat
whose diver's link dropped without a word is held SEAT_HOLD seconds, the
others' games waiting on his keys as on a slow link, and his resume
within them takes it back, what was said to him meanwhile said again;
past them it is gone as on a leave. A door's
battle over with seat 0's report naming the first side the winner is the
feed's "guardian down" with the party's size. The stands do not show
party battles yet."""

import time

from . import protocol, websocket
from .arena import ENDED, LEAVE, OVER, START, delay_for

SPEC = protocol.SPEC
# A seat held through its diver's dropped link (issue #180): the game fights
# on alone after 10 seconds down (src/online/party_play.c, HELD_MS), a
# resume within them takes the seat back; two more cover the server noticing
# the drop late
SEAT_HOLD = 12.0


class Queue:
    def __init__(self, room):
        self.key = room.key
        self.members = []       # player ids, the leader first


class Battle:
    def __init__(self, battle_id, seats, forms, key, guardian):
        self.id, self.seats, self.forms = battle_id, list(seats), forms
        self.key, self.guardian = key, guardian
        self.gone = set()       # seats left or dropped
        self.away = {}          # seats held through a dropped link: player id -> monotonic deadline
        self.status = None      # seat 0's last report (ARENA_STATUS)

    def here(self):
        return [p for p in self.seats if p not in self.gone]


class Parties:
    def __init__(self, server):
        self.server = server
        self.queues = {}        # layer key -> Queue
        self.in_queue = {}      # player id -> layer key
        self.battles = {}       # battle id -> Battle
        self.seat_of = {}       # player id -> battle id
        self.missed = {}        # player id -> what was said to his held seat while his link was down

    def fighting(self, player_id):
        return player_id in self.seat_of

    # -- the queue --

    def send_queue(self, queue, stage=None, only=None):
        stage = stage or SPEC["queue_waiting"]
        players = self.server.players
        entries = [(p, players[p].name if p in players else "", 0 if players.get(p) and players[p].no_arena
                    else SPEC["queue_arena"]) for p in queue.members]
        door = queue.key[2:]
        for place, member in enumerate(queue.members, 1):
            if only is None or member in only:
                self.tell(member, protocol.queue_state(stage, door, place, entries))

    def tell(self, player_id, message):
        player = self.server.players.get(player_id)
        if player and not player.writer.is_closing():
            player.writer.write(websocket.frame(message))
        elif player and self.away_in(player_id):
            self.missed.setdefault(player_id, []).append(message)

    def away_in(self, player_id):
        """His seat's battle where it is held through his dropped link, else None."""
        battle = self.battles.get(self.seat_of.get(player_id))
        return battle if battle and player_id in battle.away else None

    async def handle(self, player, data):
        action, door = protocol.read_queue(data)
        room = self.server.dive.room_of(player.id)
        if not room:
            await self.server.notice(player, SPEC["notice_dive_out"], "That takes a dive")
            return
        if action == SPEC["queue_join"]:
            await self.join(player, room, door)
        elif action == SPEC["queue_leave"]:
            self.leave(player.id)
        else:
            queue = self.queues.get(self.in_queue.get(player.id))
            if not queue or queue.members[0] != player.id:
                await self.server.notice(player, SPEC["notice_queue"], "Only the first at the door can")
                return
            if action == SPEC["queue_start"]:
                self.begin(queue)
            elif len(queue.members) > 1:
                queue.members.append(queue.members.pop(0))
                self.send_queue(queue)

    async def join(self, player, room, door):
        if door != room.key[2:]:
            await self.server.notice(player, SPEC["notice_queue"], "Not at that door")
            return
        if self.fighting(player.id):
            await self.server.notice(player, 16, "That Navi is busy")
            return
        queue = self.queues.get(room.key)
        if queue and player.id in queue.members:
            self.send_queue(queue, only=(player.id,))
            return
        if queue and len(queue.members) >= SPEC["party_most"]:
            await self.server.notice(player, SPEC["notice_queue"], "That door's queue is full")
            return
        self.leave(player.id)
        queue = self.queues.setdefault(room.key, Queue(room))
        queue.members.append(player.id)
        self.in_queue[player.id] = room.key
        self.send_queue(queue)
        self.server.dive.flare(player, room, queued=queue.members)

    def leave(self, player_id, told=True):
        key = self.in_queue.pop(player_id, None)
        queue = self.queues.get(key)
        if not queue:
            return
        queue.members.remove(player_id)
        if told:
            self.tell(player_id, protocol.queue_state(SPEC["queue_out"], key[2:], 0, []))
        if queue.members:
            self.send_queue(queue)
        else:
            del self.queues[key]

    def begin(self, queue):
        """The leader's start: the party battle, or the leader alone where nobody is beside him."""
        members = list(queue.members)
        for member in members:
            self.in_queue.pop(member, None)
        del self.queues[queue.key]
        if len(members) == 1:
            self.tell(members[0], protocol.queue_state(SPEC["queue_alone"], queue.key[2:], 1, []))
            return
        self.send_queue(queue, SPEC["queue_started"])
        room = self.server.dive.room_of(members[0])
        self.start(members, room, guardian=True, key=queue.key)

    # -- a party battle --

    def start(self, seats, room, guardian, key=None):
        players, arena = self.server.players, self.server.arena
        battle_id = arena.next
        arena.next = arena.next % 0xffffffff + 1
        host = not players[seats[0]].no_arena
        forms = {p: SPEC["arena_party"] if host and not players[p].no_arena else SPEC["arena_linked"] for p in seats}
        battle = self.battles[battle_id] = Battle(battle_id, seats, forms, key or room.key, guardian)
        for player_id in seats:
            self.seat_of[player_id] = battle_id
            self.leave(player_id, told=False)
            self.server.crews.offers.pop(player_id, None)
        delay = delay_for(max(players[p].round_trip for p in seats), 0)
        sessions = tuple(seats) + (0,) * (SPEC["seats"] - len(seats))
        for seat, player_id in enumerate(seats):
            other = seats[1] if seat == 0 else seats[0]
            message = protocol.arena_state(START, battle_id, other, players[other].identity, seat, delay,
                                           forms[player_id], sessions)
            self.tell(player_id, message)
        self.server.event("party", battle=battle_id, seats=list(seats), guardian=guardian)
        self.server.daily.count("battles_started")
        return battle

    def state(self, battle, player_id, stage, seat=0, other=0):
        peer = self.server.players.get(other)
        self.tell(player_id, protocol.arena_state(stage, battle.id, other, peer.identity if peer else bytes(16), seat, 0,
                                                  battle.forms.get(player_id, 0)))

    def action(self, player, action, battle_id):
        """ARENA leave or over for a party battle (arena.py): True where it was one's."""
        battle = self.battles.get(battle_id)
        if not battle:
            return False
        if player.id not in battle.here():
            return True
        if action == OVER and player.id == battle.seats[0]:
            self.end(battle)
        elif action == LEAVE:
            self.seat_gone(battle, player.id)
        return True

    def seat_gone(self, battle, player_id):
        battle.gone.add(player_id)
        self.seat_of.pop(player_id, None)
        seat = battle.seats.index(player_id)
        for other in battle.here():
            self.state(battle, other, SPEC["arena_seat_gone"], seat, player_id)
        if not battle.here():
            self.battles.pop(battle.id, None)

    def end(self, battle):
        # (a seat held through a drop hears every other seat gone on his resume, then the end: his game finishes its own)
        for player_id in battle.away:
            for seat, other in enumerate(battle.seats):
                if other != player_id:
                    self.state(battle, player_id, SPEC["arena_seat_gone"], seat, other)
        for player_id in battle.here():
            self.state(battle, player_id, ENDED)
        self.battles.pop(battle.id, None)
        for player_id in battle.here():
            self.seat_of.pop(player_id, None)
        self.server.daily.count("battles_finished")
        if battle.guardian and battle.status and battle.status[8] == 1:
            leader = self.server.players.get(battle.seats[0])
            _, _, depth, side, way = battle.key
            message = protocol.dive_feed(SPEC["feed_guardian"], leader.name if leader else "", depth, side, way,
                                         0, len(battle.seats))
            self.server.dive.week_and_square(battle.key[0], message)

    def status(self, player, data):
        """ARENA_STATUS of a party battle (seat 0's alone): True where it was one's."""
        battle_id, status = protocol.read_arena_status(data)
        battle = self.battles.get(battle_id)
        if not battle:
            return False
        if player.id == battle.seats[0]:
            battle.status = status
        return True

    def relay(self, player, data, battle_id):
        """A party seat's ARENA_LOADOUT, ARENA_INPUT or ARENA_HASH to the other party seats, as ARENA_SEAT."""
        battle = self.battles.get(battle_id)
        if not battle or player.id not in battle.here() or battle.forms[player.id] != SPEC["arena_party"]:
            return
        framed = websocket.frame(protocol.arena_seat(battle.seats.index(player.id), data))
        for other in battle.here():
            if other != player.id and battle.forms[other] == SPEC["arena_party"]:
                peer = self.server.players.get(other)
                if peer and not peer.writer.is_closing():
                    peer.writer.write(framed)

    def linked(self, player, data):
        """PARTY_DAMAGE (a linked fighter's, to seat 0 as ARENA_SEAT) and PARTY_HP (seat 0's, to the linked fighters)."""
        kind, battle_id, _, _ = protocol.read_party(data)
        battle = self.battles.get(battle_id)
        if not battle or player.id not in battle.here():
            return
        seat = battle.seats.index(player.id)
        if kind == SPEC["party_damage"] and seat and battle.forms[player.id] == SPEC["arena_linked"]:
            if battle.seats[0] in battle.here():
                self.tell(battle.seats[0], protocol.arena_seat(seat, data))
        elif kind == SPEC["party_hp"] and not seat:
            for other in battle.here()[1:]:
                if battle.forms[other] == SPEC["arena_linked"]:
                    self.tell(other, data)

    # -- leaving --

    def left_layer(self, player_id):
        """A diver gone on to another layer: his door's queue left."""
        self.leave(player_id)

    def left(self, player_id):
        """A diver out of the dive or the server: his queue left, his seat gone (held where his link dropped)."""
        self.leave(player_id, told=player_id in self.server.players)
        battle = self.battles.get(self.seat_of.get(player_id))
        if battle and player_id not in battle.away:
            self.seat_gone(battle, player_id)

    # -- a seat through a dropped link (issue #180) --

    def hold(self, player_id):
        """His link dropped: his seat in a party battle under way held SEAT_HOLD seconds."""
        battle = self.battles.get(self.seat_of.get(player_id))
        if battle and player_id in battle.here() and len(battle.here()) > 1:
            battle.away[player_id] = time.monotonic() + SEAT_HOLD
            self.server.event("party_held", id=player_id, battle=battle.id)

    def resumed(self, player):
        """His place taken back: his seat too, what was said to it meanwhile said again."""
        battle = self.away_in(player.id)
        if battle:
            del battle.away[player.id]
        for message in self.missed.pop(player.id, []):
            self.tell(player.id, message)

    def let_go(self, player_id):
        """His seat's hold over (its time, or his place freed): gone as on a leave."""
        self.missed.pop(player_id, None)
        battle = self.away_in(player_id)
        if battle:
            del battle.away[player_id]
            self.seat_gone(battle, player_id)

    def expire(self, now):
        for battle in list(self.battles.values()):
            for player_id, deadline in list(battle.away.items()):
                if now >= deadline:
                    self.server.event("party_let_go", id=player_id, battle=battle.id)
                    self.let_go(player_id)
