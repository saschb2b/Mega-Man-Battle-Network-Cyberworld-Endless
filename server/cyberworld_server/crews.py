"""Crews on the shared dive (protocol v20, issues #589 and #714;
docs/online/shared-dive.md, Crews by nearness; shared/online/README.md,
Protocol v20).

A crew is who dives together, by where they walk, never by asking: two
divers of one room (a layer's instance) whose places (STATE, sent since
each entered the room) stand within the near reach of each other
for crew_form_ms are a crew, or join the one either is in where it holds
fewer than party_most and he stands with its core; two crews that meet
become one where both fit in party_most, the older taking the other's.
The pairs near longest go first, so a fifth diver near a crew of four
stays outside it (the first there, not the nearest). A crew holds by
chains: its core is the largest part of its members, in one room, each
within the far reach of another of it (wider than the near, so a crew
at the edge does not form and split with every step). The reach is on
screen (issue #750, the owner's call of 22 November 2026): a box in the
picture's pixels, BN6's projection putting one who stands (dx, dy) from
MegaMan at dx + dy across and (dy - dx) / 2 down; crew_near_wide and
crew_near_high a crew forms within, crew_far_wide and crew_far_high it
holds within, both inside the 240 by 160 picture whichever of the two
looks (docs/online/shared-dive.md, On screen only); ties go to
the part holding the crew's first member. A member out of the core for
crew_split_ms in the core's room, or crew_away_ms elsewhere (taking a
portal before the others, home between acts, a place not yet sent in a
new room), is out of it; a crew of one is no crew. A member out of the
dive (back to the Square, his session ended, his dive's end) is out at
once. Every member is told the crew as it stands (CREW_STATE formed, the
diver who came or went its other) at each change; a diver no longer in
one is told split_up.

A crewmate's random battle starting (CREW battle, its index on the layer)
is offered to his crewmates in the same room (CREW_STATE offer), never to
or from a game that cannot run the one field (HELLO's no_arena); each game
answers at once, unasked (issue #715): CREW join where its MegaMan is free,
CREW busy (v23) where he is not (a chat, a shop, a guardian's door, his own
battle held), which takes him out of the offer. When every one offered has
answered, or crew_join_ms has passed (a game that never answers), the
server starts a party battle (party.py) with the battle's owner as seat 0
and the joiners; where nobody joined, the owner is told to fight alone
(CREW_STATE alone). Refusals: NOTICE
notice_dive_out (not diving), 17 (that offer is over), 16 (busy: a party
battle under way) and notice_crew (no crew)."""

import time
from itertools import combinations

from . import protocol, websocket

SPEC = protocol.SPEC
JOIN_WAIT = SPEC["crew_join_ms"] / 1000
NEAR = (SPEC["crew_near_wide"], SPEC["crew_near_high"])   # the picture's pixels: a crew forms within this
FAR = (SPEC["crew_far_wide"], SPEC["crew_far_high"])       # ... and holds within this
FORM_WAIT = SPEC["crew_form_ms"] / 1000     # seconds near before a crew forms (two passing are none)
SPLIT_WAIT = SPEC["crew_split_ms"] / 1000   # seconds out of reach in the room before a member is out
AWAY_WAIT = SPEC["crew_away_ms"] / 1000     # ... and elsewhere (a portal taken first, home)
VITALS_WAIT = SPEC["crew_vitals_ms"] / 1000  # a crew told its members' HP and layers at most so often


def on_screen(dx, dy, reach):
    """Whether one standing (dx, dy) world units from another stands within reach (wide, high) of him in the
    picture: BN6's projection, dx + dy across and (dy - dx) / 2 down (src/online/map_label.c)."""
    wide, high = reach
    return abs(dx + dy) <= wide and abs(dy - dx) <= 2 * high


class Crew:
    def __init__(self, members, now):
        self.members = list(members)    # in the order they came
        self.formed = now
        self.away = {}                  # member -> since when he stands out of the core


class Offer:
    def __init__(self, owner, index, invited, now):
        self.owner, self.index = owner, index
        self.invited = set(invited)
        self.joined = []
        self.until = now + JOIN_WAIT


class Crews:
    def __init__(self, server):
        self.server = server
        self.of = {}            # player id -> Crew
        self.offers = {}        # owner id -> Offer
        self.placed = {}        # player id -> the room his last STATE came in
        self.near = {}          # (id, id), the lower first -> since when they stand within NEAR
        self.hp = {}            # player id -> (hp, max_hp) as his DIVE vitals last said
        self.hp_at = {}         # ... and when it was taken
        self.told = {}          # id(crew) -> (what its last CREW_STATE said of each member, when)
        self.full_told = set()  # (player id, id(crew)): a diver in no crew told that crew near him is full (v29)
        self.at = {}            # player id -> where his game said he is (v26: where_home, where_battle; none on the layer)

    # -- the wire --

    def send(self, player_id, stage, other=0, value=0, crew=None):
        player = self.server.players.get(player_id)
        if not player or player.writer.is_closing():
            return
        members = []
        for member in (crew.members if crew else []):
            peer = self.server.players.get(member)
            members.append((member, peer.name if peer else "", self.vitals_of(member)))
        player.writer.write(websocket.frame(protocol.crew_state(stage, other, value, members)))

    def tell(self, crew, other=0, stage=None, now=None):
        self.told[id(crew)] = (self.said(crew), time.monotonic() if now is None else now)
        for member in crew.members:
            self.send(member, SPEC["crew_formed"] if stage is None else stage, other, 0, crew)

    # -- each one's HP and layer (v24, issue #717): information, told to his crew alone --

    def vitals_of(self, player_id):
        """(hp, max_hp, the depth of the layer he dives, where he is), 0 each where not known or not on a layer."""
        room = self.server.dive.room_of(player_id)
        hp, maximum = self.hp.get(player_id, (0, 0))
        return hp, maximum, room.depth if room else 0, self.at.get(player_id, SPEC["where_layer"])

    # -- where each one is (v26, issue #756) --

    def where_set(self, player_id, where):
        """A DIVE where (or a room entered: on a layer). A diver at home between acts is drawn on no layer (hidden),
        forms no crew from the place he left, and stays in his crew as it stood; his crew is told (CREW_STATE vitals)."""
        if where == SPEC["where_layer"]:
            self.at.pop(player_id, None)
        else:
            self.at[player_id] = where

    def home(self, player_id):
        return self.at.get(player_id) == SPEC["where_home"]

    def hidden(self):
        """The divers drawn on no layer: those at home."""
        return {player_id for player_id, where in self.at.items() if where == SPEC["where_home"]}

    def said(self, crew):
        return tuple((member, self.vitals_of(member)) for member in crew.members)

    def vitals(self, player_id, hp, maximum, now):
        """A DIVE vitals: kept, one taken every half crew_vitals_ms at most (a game sends one per crew_vitals_ms)."""
        if now - self.hp_at.get(player_id, -VITALS_WAIT) < VITALS_WAIT / 2:
            return
        self.hp[player_id] = (hp, maximum)
        self.hp_at[player_id] = now

    def retell(self, now):
        """Each tick: a crew whose members' HP or layers changed since it was last told is told again (CREW_STATE
        vitals), crew_vitals_ms apart at most."""
        for crew in {id(c): c for c in self.of.values()}.values():
            said, when = self.told.get(id(crew), ((), -VITALS_WAIT))
            if now - when >= VITALS_WAIT and self.said(crew) != said:
                self.tell(crew, 0, SPEC["crew_vitals"], now)

    async def handle(self, player, data, now=None):
        action, ref, value = protocol.read_crew(data)
        now = time.monotonic() if now is None else now
        if not self.server.dive.room_of(player.id):
            await self.server.notice(player, SPEC["notice_dive_out"], "That takes a dive")
            return
        if action == SPEC["crew_battle"]:
            await self.battle(player, value, now)
        elif action == SPEC["crew_busy"]:
            self.busy(player, ref)
        else:
            await self.join(player, ref)

    # -- by nearness --

    def moved(self, player):
        """A STATE: his place is the room's he stands in now."""
        room = self.server.dive.room_of(player.id)
        if room:
            self.placed[player.id] = room
        else:
            self.placed.pop(player.id, None)

    def where(self, player_id):
        """(room, x, y) where his place was sent in the room he stands in, else (room, None, None)."""
        room = self.server.dive.room_of(player_id)
        player = self.server.players.get(player_id)
        if not room or not player or self.placed.get(player_id) is not room or self.home(player_id):
            return room, None, None
        return room, player.position[0], player.position[1]

    def within(self, a, b, reach):
        room_a, xa, ya = self.where(a)
        room_b, xb, yb = self.where(b)
        if room_a is None or room_a is not room_b or xa is None or xb is None:
            return False
        return on_screen(xb - xa, yb - ya, reach)

    def settle(self, now):
        """Each tick: who stands near whom, every crew's core, the crews formed, grown and split."""
        rooms = {}
        for player_id, room in self.server.dive.of.items():
            rooms.setdefault(room.channel, []).append(player_id)
        near = {}
        for ids in rooms.values():
            for pair in combinations(sorted(ids), 2):
                if self.within(*pair, NEAR):
                    near[pair] = self.near.get(pair, now)
        self.near = near
        for crew in {id(c): c for c in self.of.values()}.values():
            self.hold(crew, now)
        for (a, b), since in sorted(near.items(), key=lambda item: (item[1], item[0])):
            if now - since >= FORM_WAIT:
                self.together(a, b, now)
        self.full(near)
        self.retell(now)
        self.told = {key: value for key, value in self.told.items() if key in {id(c) for c in self.of.values()}}

    def full(self, near):
        """A diver in no crew near a full crew is told so once (CREW_STATE full, v29): his game draws no "about to be"
        ring for a crew that cannot take him, and says it is full."""
        crews = {id(c): c for c in self.of.values()}
        for pair in near:
            for lone, other in (pair, pair[::-1]):
                crew = self.of.get(other)
                if lone in self.of or not crew or len(crew.members) < SPEC["party_most"] or (lone, id(crew)) in self.full_told:
                    continue
                self.full_told.add((lone, id(crew)))
                self.send(lone, SPEC["crew_full"], other, 0, crew)
        self.full_told = {(p, c) for p, c in self.full_told if c in crews and p not in self.of and p in self.server.dive.of}

    def core(self, crew):
        """The largest part of the crew within FAR of each other in one room (ties: the part with its first member)."""
        parts, seen = [], set()
        for start in crew.members:
            if start in seen or self.where(start)[1] is None:
                continue
            part, edge = [], [start]
            seen.add(start)
            while edge:
                member = edge.pop()
                part.append(member)
                for other in crew.members:
                    if other not in seen and self.within(member, other, FAR):
                        seen.add(other)
                        edge.append(other)
            parts.append(part)
        if not parts:
            return set(crew.members)        # (no place sent yet anywhere: nothing to judge)
        first = {m: k for k, m in enumerate(crew.members)}
        return set(max(parts, key=lambda part: (len(part), -min(first[m] for m in part))))

    def hold(self, crew, now):
        core = self.core(crew)
        room = self.server.dive.room_of(next(iter(core)))
        gone = []
        for member in crew.members:
            # (at home between acts: his crew kept as it stood, issue #756)
            if member in core or self.home(member):
                crew.away.pop(member, None)
                continue
            since = crew.away.setdefault(member, now)
            wait = SPLIT_WAIT if self.where(member)[0] is room and self.where(member)[1] is not None else AWAY_WAIT
            if now - since >= wait:
                gone.append(member)
        for member in gone:
            self.out(member)

    def together(self, a, b, now):
        crew_a, crew_b = self.of.get(a), self.of.get(b)
        most = SPEC["party_most"]
        if crew_a and crew_a is crew_b:
            return
        if crew_a and crew_b:
            # (two crews met: one where both fit, the older taking the other's)
            if a in crew_a.away or b in crew_b.away or len(crew_a.members) + len(crew_b.members) > most:
                return
            older, newer = (crew_a, crew_b) if crew_a.formed <= crew_b.formed else (crew_b, crew_a)
            for member in newer.members:
                older.members.append(member)
                self.of[member] = older
            self.tell(older, newer.members[0])
            self.server.event("crew", members=list(older.members))
            return
        if crew_a or crew_b:
            crew, joiner, mate = (crew_a, b, a) if crew_a else (crew_b, a, b)
            if mate in crew.away or len(crew.members) >= most:
                return
            crew.members.append(joiner)
            self.of[joiner] = crew
            self.tell(crew, joiner)
            self.server.event("crew", members=list(crew.members))
            return
        crew = Crew((a, b), now)
        self.of[a] = self.of[b] = crew
        self.tell(crew, b)
        self.server.event("crew", members=list(crew.members))

    def out(self, player_id):
        """A diver out of his crew: told so, the rest told the crew as it stands, a crew of one none."""
        crew = self.of.pop(player_id, None)
        if not crew:
            return
        crew.members.remove(player_id)
        crew.away.pop(player_id, None)
        self.send(player_id, SPEC["crew_split_up"], player_id)
        if len(crew.members) < 2:
            for member in crew.members:
                self.of.pop(member, None)
                self.send(member, SPEC["crew_split_up"], player_id)
            return
        self.tell(crew, player_id)
        self.server.event("crew", members=list(crew.members))

    def split(self, player_id, gone=False):
        """A diver out of the dive (back to the Square, his session ended, his dive's end): out of his crew at once."""
        self.offers.pop(player_id, None)
        for offer in self.offers.values():
            offer.invited.discard(player_id)
            if player_id in offer.joined:
                offer.joined.remove(player_id)
        self.placed.pop(player_id, None)
        self.hp.pop(player_id, None)
        self.hp_at.pop(player_id, None)
        self.at.pop(player_id, None)
        self.near = {pair: since for pair, since in self.near.items() if player_id not in pair}
        self.out(player_id)

    # -- a battle offered --

    async def battle(self, player, index, now):
        crew = self.of.get(player.id)
        if not crew:
            await self.server.notice(player, SPEC["notice_crew"], "No crew")
            return
        server = self.server
        if server.parties.fighting(player.id) or player.id in self.offers:
            await server.notice(player, 16, "That Navi is busy")
            return
        room = server.dive.room_of(player.id)
        # (a crew's battle is fought on one field: a game that cannot run it, HELLO's no_arena, is offered nothing
        # and offers nothing, docs/online/shared-dive.md)
        # (on screen only, issue #750: a crewmate offered it stands within the far reach of the owner now, each in the
        # other's picture; one walking off, in his crew_split_ms before he is out of it, is offered nothing)
        invited = [m for m in crew.members if m != player.id and server.dive.room_of(m) is room
                   and not server.parties.fighting(m) and not server.players[m].no_arena and self.within(player.id, m, FAR)]
        if player.no_arena:
            invited = []
        if not invited:
            self.send(player.id, SPEC["crew_alone"], player.id, index, crew)
            return
        self.offers[player.id] = Offer(player.id, index, invited, now)
        for member in invited:
            self.send(member, SPEC["crew_offer"], player.id, index, crew)

    async def join(self, player, owner_id):
        offer = self.offers.get(owner_id)
        if not offer or player.id not in offer.invited:
            await self.server.notice(player, 17, "That battle is over")
            return
        if player.id not in offer.joined:
            offer.joined.append(player.id)
        if set(offer.joined) >= offer.invited:
            self.close(offer)

    def busy(self, player, owner_id):
        """A game offered a battle whose MegaMan cannot go in: out of the offer, the battle started once the rest answered
        (a late one, the offer over, says nothing)."""
        offer = self.offers.get(owner_id)
        if not offer or player.id not in offer.invited:
            return
        offer.invited.discard(player.id)
        if player.id in offer.joined:
            offer.joined.remove(player.id)
        if set(offer.joined) >= offer.invited:
            self.close(offer)

    def close(self, offer):
        """An offer's window over: the party battle started with those who joined, or the owner alone."""
        self.offers.pop(offer.owner, None)
        room = self.server.dive.room_of(offer.owner)
        joined = [m for m in offer.joined if room and self.server.dive.room_of(m) is room
                  and not self.server.parties.fighting(m)]
        if not room:
            return
        if not joined:
            self.send(offer.owner, SPEC["crew_alone"], offer.owner, offer.index, self.of.get(offer.owner))
            return
        self.server.parties.start([offer.owner] + joined, room, guardian=False)

    def pump(self, now):
        self.settle(now)
        for offer in list(self.offers.values()):
            if now >= offer.until:
                self.close(offer)
