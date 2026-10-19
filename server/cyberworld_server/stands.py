"""The stands (issue #128): a NetBattle watched from the Arena's wing.

Every started battle is written down as it is relayed: both players'
MegaMen (ARENA_LOADOUT), their ready and installs' record, the pair's hash
every 60 frames (the cable's first's) and both players' keys from the
first frame. A player of the channel who asks to watch (WATCH) is sent the
battle's setup and then all of it, from the first frame, a little a tick
(a spectator who comes in late catches up as fast as his game runs it);
his game builds the same pair and runs it from the keys, behind the
players. Nothing a spectator sends reaches the battle, and a slow
spectator only falls behind: the players' relay never waits on him.

Every game of the channel, a game that cannot watch among them, is told
the channel's battles in STANDS: who fights whom, by which rules, how many
watch and cheer, and the report the cable's first game sends now and then
(ARENA_STATUS: each MegaMan's HP, the last chip each used and how many,
the winner). The two players hear nothing of the stands while they fight:
a phrase or emote of the channel's is held from them (app.Server.social),
and counted as a cheer for the report."""

import time

from . import protocol, websocket

WATCH_ON, WATCH_OVER = 1, 2                 # WATCH_STATE's stages
WAY_IN, FIGHTING, OVER, STOPPED = 1, 2, 3, 4  # a STANDS entry's
MOST_FRAMES = 72000      # frames of keys kept a side (20 minutes): past it a battle is no longer watched
BUDGET = 2048            # bytes a tick to one spectator, his output under PACE
PACE = 4096              # ... (the server drops a client whose output passes 8192)
STATUS_EVERY = 0.5       # seconds between two reports taken from a battle
STANDS_EVERY = 1.0       # seconds between two STANDS a channel, but for a battle begun or ended
KEPT = 60                # seconds a battle over is kept for its spectators still catching up
FIRST_MOST = 60          # a side's first keys come at its input delay's frame, a second at most (ARENA_DELAY_MAX)


class Book:
    """A started battle as the stands keep it."""

    def __init__(self, match, channel, names, delay):
        self.match, self.channel, self.names, self.delay = match, channel, names, delay
        self.loadout = [None, None]       # each side's ARENA_LOADOUT, as relayed
        self.record = [None, None]        # ... its ARENA_HASH of the installs' record
        self.ready = [None, None]         # ... and of its ready
        self.hashes = []                  # (frame, ARENA_HASH) of the cable's first, in order
        self.keys = [bytearray(), bytearray()]
        self.broken = False               # a gap in the keys, or past MOST_FRAMES: no longer watched
        self.status = (0, 0, 0, 0, 0, 0, 0, 0, 0)
        self.status_time = -1e9
        self.cheers = 0
        self.stage = WAY_IN
        self.over_time = None
        self.watchers = {}                # player id -> Cursor

    def frames(self, side):
        return len(self.keys[side]) // 2

    def entry(self):
        hp0, max0, hp1, max1, chip0, chip1, used0, used1, winner = self.status
        return (self.match.id, self.match.a, self.match.b, self.names[0].encode()[:16], self.names[1].encode()[:16],
                self.match.rules, self.stage, min(255, len(self.watchers)), min(255, self.cheers),
                hp0, max0, hp1, max1, chip0, chip1, used0, used1, winner)


class Cursor:
    """What one spectator has been sent of a battle."""

    def __init__(self):
        self.setup = False
        self.relayed = set()   # ("loadout"|"record"|"ready", side)
        self.keys = [0, 0]
        self.hashes = 0


class Stands:
    def __init__(self, server):
        self.server = server
        self.books = {}        # battle id -> Book, started and not yet let go
        self.watching = {}     # spectator id -> battle id
        self.last = {}         # channel -> the Book of its last battle over
        self.dirty = {}        # channel -> True where its STANDS is due (urgent) or False (when STANDS_EVERY allows)
        self.sent = {}         # channel -> the time of its last STANDS

    # -- the battles, from the Arena (arena.py) --

    def begin(self, match, delay):
        players = self.server.players
        a, b = players.get(match.a), players.get(match.b)
        if not a or not b:
            return
        self.books[match.id] = Book(match, a.channel, (a.name, b.name), delay)
        self.dirty[a.channel] = True

    def end(self, match):
        book = self.books.get(match.id)
        if not book or book.over_time is not None:
            return
        book.over_time = time.monotonic()
        book.stage = OVER if book.status[8] else STOPPED
        self.last[book.channel] = book
        self.dirty[book.channel] = True

    def record(self, match, player_id, data):
        """A message relayed between the two players, written down for the stands."""
        book = self.books.get(match.id)
        if not book or book.over_time is not None:
            return
        side = 0 if player_id == match.a else 1
        kind = data[0]
        if kind == protocol.SPEC["arena_loadout"]:
            book.loadout[side] = bytes(data)
        elif kind == protocol.SPEC["arena_hash"]:
            frame = protocol.ARENA_HASH.unpack(data)[2]
            if frame == 0xFFFFFFFE:
                book.record[side] = bytes(data)
            elif frame == 0xFFFFFFFF:
                book.ready[side] = bytes(data)
            elif side == 0 and (not book.hashes or frame > book.hashes[-1][0]) and len(book.hashes) < MOST_FRAMES // 60:
                book.hashes.append((frame, bytes(data)))
        elif kind == protocol.SPEC["arena_input"] and not book.broken:
            self.keys_in(book, side, data)

    def keys_in(self, book, side, data):
        _, _, first, count = protocol.ARENA_INPUT.unpack_from(data)
        known = book.frames(side)
        if not known and first <= FIRST_MOST:
            # (a side's first keys are its delay's frame's: the frames before are released keys on every game)
            book.keys[side] += bytes(2 * first)
            known = first
        if first > known or known + count > MOST_FRAMES:
            # (a frame missing, or a battle past what is kept: its spectators stop where they are)
            book.broken = True
            return
        skip = known - first
        if skip < count:
            book.keys[side] += data[protocol.ARENA_INPUT.size + 2 * skip:]
            if book.stage == WAY_IN:
                book.stage = FIGHTING

    def status(self, player, data):
        match_id, status = protocol.read_arena_status(data)
        book = self.books.get(match_id)
        if not book or book.over_time is not None or player.id != book.match.a:
            return
        now = time.monotonic()
        if now - book.status_time < STATUS_EVERY and not status[8]:
            return
        book.status, book.status_time = status, now
        self.dirty.setdefault(book.channel, False)

    def fighting(self, player_id):
        """A player of a battle under way, who hears nothing of the stands until it is over."""
        battle = self.server.arena.of.get(player_id)
        book = self.books.get(battle) if battle else None
        return bool(book and book.over_time is None)

    def cheer(self, player):
        """A phrase or emote of the channel's while its battles are on: a cheer for each."""
        if self.fighting(player.id):
            return
        for book in self.books.values():
            if book.channel == player.channel and book.over_time is None:
                book.cheers += 1
                self.dirty.setdefault(book.channel, False)

    # -- the spectators --

    async def watch(self, player, data):
        action, match_id = protocol.read_watch(data)
        if action == 2:
            self.stop(player.id)
            return
        book = self.books.get(match_id)
        if not self.server.arena_on:
            await self.server.notice(player, 18, "The Arena is closed on this server")
        elif not book or book.over_time is not None or book.broken or book.channel != player.channel:
            await self.server.notice(player, 17, "That battle is over")
        elif player.id in (book.match.a, book.match.b) or player.id in self.server.arena.of:
            await self.server.notice(player, 16, "That Navi is busy")
        elif book.match.rules == protocol.SPEC["format_full"] and not player.roms & protocol.SPEC["roms_bn5"]:
            await self.server.notice(player, 19, "Those rules need BN5 on both games")
        elif len(book.watchers) >= protocol.SPEC["watch_most"] and player.id not in book.watchers:
            await self.server.notice(player, 27, "The stands are full")
        else:
            self.stop(player.id)
            book.watchers[player.id] = Cursor()
            self.watching[player.id] = match_id
            self.dirty[book.channel] = True
            self.server.event("watch", id=player.id, match=match_id)

    def stop(self, player_id):
        book = self.books.get(self.watching.pop(player_id, 0))
        if book and book.watchers.pop(player_id, None) is not None:
            self.dirty.setdefault(book.channel, False)

    def busy(self, player_id):
        return player_id in self.watching

    def left(self, player):
        """A player gone from the channel (left, or changed channels)."""
        self.stop(player.id)

    # -- each tick: the spectators fed, the channels told --

    def pump(self, now):
        for book in list(self.books.values()):
            for player_id in list(book.watchers):
                player = self.server.players.get(player_id)
                if not player or player.writer.is_closing() or player.channel != book.channel:
                    self.stop(player_id)
                    continue
                self.feed(player, book, book.watchers[player_id])
            if book.over_time is not None and (not book.watchers or now - book.over_time > KEPT):
                for player_id in list(book.watchers):
                    self.stop(player_id)
                del self.books[book.match.id]
        wall = time.monotonic()
        for channel, urgent in list(self.dirty.items()):
            if urgent or wall - self.sent.get(channel, -1e9) >= STANDS_EVERY:
                del self.dirty[channel]
                self.sent[channel] = wall
                self.tell(channel)

    def feed(self, player, book, cursor):
        writer = player.writer
        out = []
        budget = BUDGET if writer.transport.get_write_buffer_size() < PACE else 0
        if not cursor.setup:
            cursor.setup = True
            out.append(protocol.watch_state(WATCH_ON, book.match.id, book.match.a, book.match.b, book.match.rules, book.delay))
        for name, kept in (("loadout", book.loadout), ("record", book.record), ("ready", book.ready)):
            for side in (0, 1):
                if kept[side] and (name, side) not in cursor.relayed:
                    cursor.relayed.add((name, side))
                    out.append(protocol.watch_relay(side, kept[side]))
        spent = sum(len(m) for m in out)
        most = protocol.SPEC["watch_keys_max"]
        while spent < budget:
            # (the side behind first, so both streams come in step; a hash once both sides' keys are past it)
            side = 0 if cursor.keys[0] <= cursor.keys[1] else 1
            if cursor.keys[side] >= book.frames(side):
                side ^= 1
            first = cursor.keys[side]
            count = min(most, book.frames(side) - first)
            if count <= 0:
                break
            out.append(protocol.watch_keys(book.match.id, side, first, bytes(book.keys[side][2 * first:2 * (first + count)])))
            cursor.keys[side] += count
            spent += len(out[-1])
            while cursor.hashes < len(book.hashes) and book.hashes[cursor.hashes][0] < min(cursor.keys):
                out.append(protocol.watch_relay(0, book.hashes[cursor.hashes][1]))
                cursor.hashes += 1
                spent += len(out[-1])
        done = book.over_time is not None and all(cursor.keys[s] >= book.frames(s) for s in (0, 1))
        if done or book.broken:
            while cursor.hashes < len(book.hashes):
                out.append(protocol.watch_relay(0, book.hashes[cursor.hashes][1]))
                cursor.hashes += 1
            out.append(protocol.watch_state(WATCH_OVER, book.match.id, book.match.a, book.match.b, book.match.rules, book.delay))
        for message in out:
            writer.write(websocket.frame(message))
        if done or book.broken:
            self.stop(player.id)

    def entries(self, channel):
        live = [book for book in self.books.values() if book.channel == channel and book.over_time is None]
        last = self.last.get(channel)
        entries = [book.entry() for book in live[:protocol.SPEC["stands_max"] - 1]]
        if last and last.match.id not in [entry[0] for entry in entries]:
            entries.append(last.entry())
        return entries

    def tell(self, channel):
        message = websocket.frame(protocol.stands(self.entries(channel)))
        for player in list(self.server.players.values()):
            # (the players of a battle under way are told once it is over)
            if player.channel == channel and not player.writer.is_closing() and not self.fighting(player.id):
                player.writer.write(message)

    def send_to(self, player):
        """STANDS for one player, as he comes into a channel."""
        if not player.writer.is_closing():
            player.writer.write(websocket.frame(protocol.stands(self.entries(player.channel))))
