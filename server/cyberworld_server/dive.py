"""The shared dive's rooms, traces and feed (protocol v19, issue #589;
docs/online/shared-dive.md; shared/online/README.md, Protocol v19).

A diver's game says DIVE enter as MegaMan comes onto a layer of the
week's net: the week, its seed, the layer's depth, its side kind (0 the
main path, else the game's LAYER_* kind) and its way (the layer's area).
The session leaves its Square channel, as CHANNEL does (its trade, its
battle, its team and its watching end), and stands in a room of that
layer: an instance of at most the server's room capacity (eight), a new
one where those are full. A room is a channel of its own, numbered past
every Square channel (ROOMS), so STATE, SNAPSHOT, SOCIAL and INSPECT work
in it as in a Square channel and the Square never sees a diver; SNAPSHOT
marks each diver `peer_diving`. DIVE leave goes back to a Square channel
as CHANNEL 0 does (a fresh WELCOME); CHANNEL from a diver is a leave to
that channel. A diver trades, challenges, teams up and watches nothing
(NOTICE notice_dive_in). Only a session with the NetPass's DeepNet stamp
dives, the full set WEEK_POST takes (NOTICE notice_pass), and only into
this week's or the last's net, its seed that week's (NOTICE 23 and 24).

What a diver leaves on a layer (an end, a find, a phrase, a Wave, a
split) is kept by layer for the week (traces.py) and told to every diver
on that layer; entering a room sends the layer's traces. An end is also
the feed's (DIVE_FEED, to every diver of that week and every Square
session); a split is answered with its place (DIVE_PLACE), and a
guardian's that changes the week's fastest sends everyone WEEK_SPLITS.
How many dive and where goes to everyone as DIVERS, at most every
divers_ms, on a change, and after each WELCOME. Traces come at the
SOCIAL burst and refill (NOTICE notice_dive_wait past it); a muted
identity leaves no phrase (NOTICE 11); a hidden one none (NOTICE 26).

Crews (crews.py) and the doors' queues and party battles (party.py) go
by the rooms here."""

from . import protocol, websocket
from .traces import TRACE_LOG, Traces, act_of, compact, phrase_entry, split_entry
from .week import week_of

SPEC = protocol.SPEC
ROOMS = 1 << 16         # rooms' channel numbers start past every Square channel's (256 at most)


class Room:
    def __init__(self, key, instance, channel):
        self.key, self.instance, self.channel = key, instance, channel
        self.members = set()

    @property
    def week(self):
        return self.key[0]

    @property
    def depth(self):
        return self.key[2]


class Dive:
    def __init__(self, server):
        self.server = server
        self.rooms = {}             # channel -> Room
        self.of = {}                # player id -> Room
        self.next_channel = ROOMS
        self.traces = Traces()
        self.tokens = {}            # player id -> (tokens, monotonic time): the traces' burst
        self.divers_sent = None     # the last DIVERS, and when (monotonic)
        self.divers_time = -1e9
        self.load()

    # -- keeping --

    def first_kept(self):
        return week_of(self.server.weeks.clock()) - self.server.retention.board_weeks + 1

    def load(self):
        directory = self.server.store.directory
        if directory:
            for entry in compact(directory, self.first_kept())[0]:
                self.traces.apply(entry)

    def keep(self, room, player, kind, **fields):
        """A trace made: kept here, and appended to dives.jsonl. What it did (traces.Traces.apply)."""
        week, seed, depth, side, way = room.key
        line = dict(week=week, seed=seed, depth=depth, side=side, way=way, kind=kind,
                    identity=player.identity.hex(), name=player.name, **fields)
        done = self.traces.apply(line)
        if done is not None and self.server.store.directory:
            if kind == "phrase":
                line["id"] = done[0]["id"]
            self.server.append(TRACE_LOG, dict(line, time=self.server.weeks.clock()))
        return done

    def shown(self, week):
        hidden = self.server.weeks.hidden
        return lambda identity: not hidden(identity, week)

    def prune(self):
        first = self.first_kept()
        for key in [k for k in self.traces.layers if k[0] < first]:
            del self.traces.layers[key]
        for key in [k for k in self.traces.guardians if k[0] < first]:
            del self.traces.guardians[key]

    # -- who dives --

    def room_of(self, player_id):
        return self.of.get(player_id)

    def peers(self, room):
        """The divers on a room's layer, every instance of it."""
        return [p for r in self.rooms.values() if r.key == room.key for p in map(self.server.players.get, r.members) if p]

    def send(self, player, message):
        if player and not player.writer.is_closing():
            player.writer.write(websocket.frame(message))

    def to_layer(self, room, messages):
        for player in self.peers(room):
            for message in messages:
                self.send(player, message)

    def week_and_square(self, week, message):
        """Every diver of that week, and every session of the Square."""
        for player in list(self.server.players.values()):
            room = self.of.get(player.id)
            if not room and player.channel <= self.server.max_channels or room and room.week == week:
                self.send(player, message)

    # -- rooms --

    async def handle(self, player, data, now):
        action, fields = protocol.read_dive(data)
        if action == SPEC["dive_enter"]:
            await self.enter(player, *fields)
            return
        room = self.of.get(player.id)
        if not room:
            await self.server.notice(player, SPEC["notice_dive_out"], "That takes a dive")
            return
        if action == SPEC["dive_leave"]:
            await self.leave(player)
            return
        if action == SPEC["dive_vitals"]:
            self.server.crews.vitals(player.id, *fields, now)
            return
        if not await self.may_trace(player, room, action, now):
            return
        handler = {SPEC["dive_end"]: self.end, SPEC["dive_found"]: self.found, SPEC["dive_phrase"]: self.phrase,
                   SPEC["dive_wave"]: self.wave, SPEC["dive_split"]: self.split}[action]
        await handler(player, room, *fields)

    async def may_trace(self, player, room, action, now):
        server = self.server
        if action == SPEC["dive_phrase"] and server.store.contains("mutes", player.identity):
            await server.notice(player, 11, "This profile is muted")
            return False
        if server.weeks.hidden(player.identity.hex(), room.week):
            await server.notice(player, 26, "A moderator took that week's post down")
            return False
        burst, refill = SPEC["social_burst"], SPEC["social_refill_ms"] / 1000
        tokens, since = self.tokens.get(player.id, (burst, now))
        tokens = min(burst, tokens + (now - since) / refill)
        if tokens < 1:
            self.tokens[player.id] = (tokens, now)
            await server.notice(player, SPEC["notice_dive_wait"], "Please wait a moment")
            return False
        self.tokens[player.id] = (tokens - 1, now)
        return True

    async def enter(self, player, week, seed, depth, side, way):
        server = self.server
        if await server.without_pass(player):
            return
        this = week_of(server.weeks.clock())
        if week not in (this, this - 1):
            await server.notice(player, 23, "That week's net is closed")
            return
        if seed != server.weeks.seed(week):
            await server.notice(player, 24, "Not that week's net")
            return
        key = (week, seed, depth, side, way)
        current = self.of.get(player.id)
        if current and current.key != key:
            server.parties.left_layer(player.id)
            self.out_of_room(player)
        elif not current:
            # (out of the Square as CHANNEL leaves a channel: what was under way there ends)
            server.leave_channel(player)
        room = self.of.get(player.id) or self.place(player, key)
        self.send(player, protocol.dive_room(week, depth, side, way, room.instance, len(room.members)))
        for message in self.traces.entries(key, self.shown(week)):
            self.send(player, message)
        server.event("dive", id=player.id, week=week, depth=depth, side=side, way=way, instance=room.instance)

    def place(self, player, key):
        rooms = sorted((r for r in self.rooms.values() if r.key == key), key=lambda r: r.instance)
        room = next((r for r in rooms if len(r.members) < self.server.capacity), None)
        if not room:
            taken = {r.instance for r in rooms}
            instance = next(n for n in range(1, len(taken) + 2) if n not in taken)
            room = Room(key, instance, self.next_channel)
            self.next_channel = self.next_channel + 1 if self.next_channel < 0x7fffffff else ROOMS
            self.rooms[room.channel] = room
        room.members.add(player.id)
        self.of[player.id] = room
        player.channel = room.channel
        return room

    def out_of_room(self, player):
        room = self.of.pop(player.id, None)
        if not room:
            return
        room.members.discard(player.id)
        if not room.members:
            del self.rooms[room.channel]
        self.server.social_rules.left(player.id)

    async def leave(self, player, requested=0):
        """Back to the Square: a channel chosen as CHANNEL chooses one, the dive's crew, queue and battle left."""
        server = self.server
        channel = server.choose_channel(requested, player.id)
        if not channel:
            await server.notice(player, 10, "Channel is full or unavailable")
            return
        self.gone(player)
        player.channel = channel
        await server.arrive(player)
        server.event("dive_leave", id=player.id, channel=channel)

    def gone(self, player):
        """A diver out of the dive (back to the Square, or his session ended):
        his crew split, his queue and his party's battle left, his room left."""
        self.server.crews.split(player.id, gone=True)
        self.server.parties.left(player.id)
        self.out_of_room(player)
        self.tokens.pop(player.id, None)

    def dropped(self, player):
        """A diver's session ended (holds.py): out of the dive, and standing in no room while his place is held."""
        if player.id in self.of:
            # (his party seat held a while first: a short drop ridden out, issue #180)
            self.server.parties.hold(player.id)
            self.gone(player)
            player.channel = 0

    # -- traces --

    async def end(self, player, room, how, x, y, z, cause, minutes=0, flags=0, party=0):
        if how == SPEC["dive_won"] and room.depth != SPEC["week_layers"]:
            await self.server.notice(player, 25, "That dive does not add up")
            return
        mark = self.keep(room, player, "mark", x=x, y=y, z=z, cause=cause, how=how)
        entry = protocol.TRACE_MARK.pack(protocol.name16(mark["name"]), x, y, z, cause, how)
        self.to_layer(room, protocol.dive_traces(SPEC["trace_marks"], [entry]))
        kind = SPEC["feed_won"] if how == SPEC["dive_won"] else SPEC["feed_deleted"]
        _, _, depth, side, way = room.key
        self.week_and_square(room.week, protocol.dive_feed(kind, player.name, depth, side, way, cause))
        self.server.event("dive_end", id=player.id, depth=depth, how=how, cause=cause)
        # (the dive ended: its crew and its queue left; he stays on the layer until his game moves on)
        self.server.crews.split(player.id, gone=False)
        self.server.parties.left(player.id)
        # (v30: its end on the Board at once, the layer the server's own, no visit to the Square waited for; the
        # game's WEEK_POST after it confirms the same entry, week.py)
        await self.server.weeks.from_dive(player, room.week, room.key[1], depth, how == SPEC["dive_won"], minutes, flags, party)

    async def found(self, player, room, index):
        layer = self.traces.layers.get(room.key)
        if layer and index in layer.opened:
            return      # (opened this week already: nothing new to keep or tell)
        index, first = self.keep(room, player, "find", index=index)
        messages = protocol.dive_traces(SPEC["trace_opened"], [bytes((index,))])
        if first:
            messages += protocol.dive_traces(SPEC["trace_finds"], [protocol.TRACE_FIND.pack(index, protocol.name16(player.name))])
        self.to_layer(room, messages)

    async def phrase(self, player, room, phrase, x, y, z):
        entry, gone = self.keep(room, player, "phrase", phrase=phrase, x=x, y=y, z=z)
        messages = protocol.dive_traces(SPEC["trace_gone"], [pid.to_bytes(4, "big") for pid in gone])
        self.to_layer(room, messages + protocol.dive_traces(SPEC["trace_phrases"], [phrase_entry(entry)]))

    async def wave(self, player, room, phrase_id):
        layer = self.traces.layers.get(room.key)
        entry = layer.phrases.get(phrase_id) if layer else None
        if not entry or not self.shown(room.week)(entry["identity"]):
            await self.server.notice(player, SPEC["notice_trace_gone"], "That trace is gone")
            return
        entry = self.keep(room, player, "wave", id=phrase_id)
        self.to_layer(room, protocol.dive_traces(SPEC["trace_phrases"], [phrase_entry(entry)]))

    async def split(self, player, room, index, tenths, level, flags, party):
        guardian = bool(flags & SPEC["split_guardian"])
        shown = self.shown(room.week)
        before = self.traces.fastest(room.week, shown) if guardian else None
        self.keep(room, player, "split", index=index, party=party, tenths=tenths, level=level, guardian=guardian)
        place, count, (best, best_name) = self.traces.place(room.key, index, party, tenths, shown)
        self.send(player, protocol.dive_place(index, party, tenths, place, count, best, best_name))
        battle = self.traces.layers[room.key].splits.get((index, party), {})
        quickest = min((e for who, e in battle.items() if shown(who)), key=lambda e: e["tenths"], default=None)
        if quickest:
            self.to_layer(room, protocol.dive_traces(SPEC["trace_splits"], [split_entry(index, party, quickest)]))
        if guardian and self.traces.fastest(room.week, shown) != before and room.week == week_of(self.server.weeks.clock()):
            message = self.splits()
            for other in list(self.server.players.values()):
                self.send(other, message)

    def splits(self):
        """WEEK_SPLITS: this week's fastest bust of each act's guardian, alone and by party size."""
        week = week_of(self.server.weeks.clock())
        return protocol.week_splits(week, self.traces.fastest(week, self.shown(week)))

    # -- the flare and the pack --

    def flare(self, player, room):
        """A diver at a door's queue: divers of the week behind him in the same act told where he waits."""
        _, seed, depth, side, way = room.key
        message = protocol.dive_feed(SPEC["feed_flare"], player.name, depth, side, way)
        for other_id, other in list(self.of.items()):
            if other.week == room.week and other.key[1] == seed and other.depth < depth \
                    and act_of(other.depth) == act_of(depth):
                self.send(self.server.players.get(other_id), message)

    def divers(self):
        entries = []
        for player_id, room in self.of.items():
            player = self.server.players.get(player_id)
            if player:
                flags = SPEC["divers_fighting"] if self.server.parties.fighting(player_id) else 0
                entries.append((player.name, room.depth, act_of(room.depth), flags))
        entries.sort(key=lambda e: (-e[1], e[0]))
        return protocol.divers(len(self.of), entries)

    def send_divers(self, player):
        self.send(player, self.divers())

    def pump(self, now):
        """DIVERS to every session where it changed, at most every divers_ms."""
        if now - self.divers_time < SPEC["divers_ms"] / 1000:
            return
        message = self.divers()
        if message == self.divers_sent:
            return
        self.divers_sent, self.divers_time = message, now
        for player in list(self.server.players.values()):
            self.send(player, message)

    def peer_bits(self, player_id):
        return SPEC["peer_diving"] if player_id in self.of else 0
