"""Crews on the shared dive (protocol v19, issue #589; docs/online/shared-dive.md,
Crews; shared/online/README.md, Protocol v19), modelled on teams.py.

A diver asks another diver of his room to crew up (CREW ask); the other
answers within ASK_WAIT seconds. A crew is up to party_most divers, the
asker's crew where he has one (any member may ask), else a new one of
the two. It lasts across layers until a member splits up (CREW split:
the others go on where two or more stay), leaves the server or the dive,
or ends his dive. Each member's game tells which fork's portal he stands
on (CREW way: depth, side and way; depth 0 none) and CREW_STATE formed
tells every member everyone's, so that each game opens the portal only
when all agree.

A crewmate's random battle starting (CREW battle, its index on the layer)
is offered to his crewmates in the same room (CREW_STATE offer); each
answers with CREW join within crew_join_ms. When every one offered has
joined, or the window ends, the server starts a party battle (party.py)
with the battle's owner as seat 0 and the joiners; where nobody joined,
the owner is told to fight alone (CREW_STATE alone). Refusals: NOTICE
notice_dive_out (not diving), 17 (that diver is not in this room, or that
offer is over), 16 (busy: an ask already out, or a party battle under
way) and notice_crew (the other in a crew already, the crew full, no
crew)."""

import time

from . import protocol, websocket

SPEC = protocol.SPEC
ASK_WAIT = 30           # seconds an ask waits for its answer
JOIN_WAIT = SPEC["crew_join_ms"] / 1000


class Crew:
    def __init__(self, founder):
        self.members = [founder]        # in the order they came
        self.ways = {}                  # member -> (depth, side, way) of the portal he stands on


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
        self.asked = {}         # asker id -> (asked id, monotonic time)
        self.offers = {}        # owner id -> Offer

    # -- the wire --

    def send(self, player_id, stage, other=0, value=0, crew=None):
        player = self.server.players.get(player_id)
        if not player or player.writer.is_closing():
            return
        members = []
        for member in (crew.members if crew else []):
            peer = self.server.players.get(member)
            members.append((member, peer.name if peer else "", crew.ways.get(member, (0, 0, 0))))
        player.writer.write(websocket.frame(protocol.crew_state(stage, other, value, members)))

    def tell(self, crew, other=0):
        for member in crew.members:
            self.send(member, SPEC["crew_formed"], other, 0, crew)

    # -- asks --

    async def handle(self, player, data, now=None):
        action, ref, value, way = protocol.read_crew(data)
        now = time.monotonic() if now is None else now
        if action == SPEC["crew_split"]:
            self.split(player.id)
            return
        dive = self.server.dive
        if not dive.room_of(player.id):
            await self.server.notice(player, SPEC["notice_dive_out"], "That takes a dive")
            return
        if action == SPEC["crew_ask"]:
            await self.ask(player, ref, now)
        elif action == SPEC["crew_answer"]:
            await self.answer(player, ref, value)
        elif action == SPEC["crew_way"]:
            await self.way(player, way)
        elif action == SPEC["crew_battle"]:
            await self.battle(player, value, now)
        else:
            await self.join(player, ref)

    async def ask(self, player, target_id, now):
        server, dive = self.server, self.server.dive
        target = server.players.get(target_id)
        if not target or target.id == player.id or dive.room_of(target.id) is not dive.room_of(player.id):
            await server.notice(player, 17, "That Navi is no longer here")
            return
        crew = self.of.get(player.id)
        if target.id in self.of or (crew and len(crew.members) >= SPEC["party_most"]):
            await server.notice(player, SPEC["notice_crew"], "No room in that crew")
            return
        if player.id in self.asked or any(t == target.id for t, _ in self.asked.values()):
            await server.notice(player, 16, "That Navi is busy")
            return
        self.asked[player.id] = (target.id, now)
        self.send(player.id, SPEC["crew_waiting"], target.id)
        self.send(target.id, SPEC["crew_asked"], player.id)

    async def answer(self, player, asker_id, yes):
        asked = self.asked.get(asker_id)
        if not asked or asked[0] != player.id:
            await self.server.notice(player, 17, "That Navi is no longer here")
            return
        del self.asked[asker_id]
        crew = self.of.get(asker_id)
        full = crew and len(crew.members) >= SPEC["party_most"]
        if not yes or asker_id not in self.server.dive.of or player.id in self.of or full:
            self.send(asker_id, SPEC["crew_split_up"], player.id)
            return
        if not crew:
            crew = self.of[asker_id] = Crew(asker_id)
        crew.members.append(player.id)
        self.of[player.id] = crew
        self.tell(crew, player.id)
        self.server.event("crew", members=list(crew.members))

    async def way(self, player, way):
        crew = self.of.get(player.id)
        if not crew:
            await self.server.notice(player, SPEC["notice_crew"], "No crew")
            return
        if way[0]:
            crew.ways[player.id] = way
        else:
            crew.ways.pop(player.id, None)
        self.tell(crew, player.id)

    def split(self, player_id, gone=False):
        """A diver out of his crew (a split, his leaving, his dive's end), and his asks taken back."""
        asked = self.asked.pop(player_id, None)
        if asked:
            self.send(asked[0], SPEC["crew_split_up"], player_id)
        for asker, (target, _) in list(self.asked.items()):
            if target == player_id:
                del self.asked[asker]
                self.send(asker, SPEC["crew_split_up"], player_id)
        self.offers.pop(player_id, None)
        for offer in self.offers.values():
            offer.invited.discard(player_id)
            if player_id in offer.joined:
                offer.joined.remove(player_id)
        crew = self.of.pop(player_id, None)
        if not crew:
            return
        crew.members.remove(player_id)
        crew.ways.pop(player_id, None)
        if not gone or player_id in self.server.players:
            self.send(player_id, SPEC["crew_split_up"], player_id)
        if len(crew.members) < 2:
            for member in crew.members:
                self.of.pop(member, None)
                self.send(member, SPEC["crew_split_up"], player_id)
            return
        self.tell(crew, player_id)

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
        invited = [m for m in crew.members if m != player.id and server.dive.room_of(m) is room
                   and not server.parties.fighting(m)]
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
        for offer in list(self.offers.values()):
            if now >= offer.until:
                self.close(offer)

    def expire(self, now):
        for asker, (target, since) in list(self.asked.items()):
            if now - since > ASK_WAIT:
                del self.asked[asker]
                self.send(asker, SPEC["crew_split_up"], target)
                self.send(target, SPEC["crew_split_up"], asker)
