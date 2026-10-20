"""The week's net (issue #127): every Monday 00:00 UTC a seed of this
server's own, and the Board's dives on it.

The seed is the first four bytes of HMAC-SHA256(the server's identity
secret, "week" and the week's number), so each server has its own week
and keeps it across restarts. The week's number counts from the Monday
`week_epoch` seconds past 1970 (5 January 1970), a week a step.

A game posts a dive that ended (WEEK_POST): the week, its seed, the
layer reached, its act, won or deleted, the minutes it took and a
summary hash. The server checks only that it is plausible: the week is
this one or the last, the seed is that week's, the layer and act are
within the short net's structure, a win only on its last layer. It keeps
one entry per identity and week, the best, and appends each improvement
to its state folder's `weeks.jsonl`, read back at the start. Entries are
claims ("reported" on the Board): a modified game can post any layer,
and nothing is given for one. Moderators hide an entry (`--admin hide`):
moderation.json's `hidden` list, its week named, which also refuses that
identity's posts for that week.

The Board is every game's to read (a WEEK_BOARD after every WELCOME);
posting a dive takes the full set (protocol v9, protocol.full_set), a
post from a game without it refused with NOTICE notice_pass."""

import asyncio
import hashlib
import hmac
import json
import logging
import os
import struct
import time

from . import protocol, websocket

LOG = logging.getLogger("cyberworld.server")
SPEC = protocol.SPEC
WEEK = 7 * 24 * 3600
POST = struct.Struct("!BHIHBBHI")
ENTRY = struct.Struct("!16sHBBH")
BOARD_HEAD = struct.Struct("!BHBBB")
WON = 1
MAX_MINUTES = 0xFFFF


def week_of(now):
    """The week's number at `now` (seconds since 1970, UTC)."""
    return int((now - SPEC["week_epoch"]) // WEEK)


def week_start(week):
    return SPEC["week_epoch"] + week * WEEK


def act_of(depth):
    """The short net's act of a layer: three a act, the Nest the fourth."""
    return min((depth - 1) // 3 + 1, (SPEC["week_layers"] - 1) // 3 + 1)


def better(a, b):
    """Whether entry `a` ranks above `b`: deeper, then won, then quicker, then earlier."""
    return (a["depth"], a["won"], -a["minutes"], -a["time"]) > (b["depth"], b["won"], -b["minutes"], -b["time"])


def read_post(data):
    if len(data) != SPEC["week_post_bytes"] or data[0] != SPEC["week_post"]:
        raise protocol.ProtocolError("Malformed WEEK_POST")
    _, week, seed, depth, act, flags, minutes, summary = POST.unpack(data)
    return dict(week=week, seed=seed, depth=depth, act=act, won=flags & WON, flags=flags,
                minutes=minutes, hash=summary)


def post(week, seed, depth, act, won, minutes, summary):
    return POST.pack(SPEC["week_post"], week, seed, depth, act, WON if won else 0, minutes, summary)


class Weeks:
    def __init__(self, server, clock=time.time):
        self.server = server
        self.clock = clock
        self.best = {}          # (week, identity hex) -> entry
        self.write_task = None
        self.load()

    @property
    def path(self):
        directory = self.server.store.directory
        return os.path.join(directory, "weeks.jsonl") if directory else None

    def load(self):
        """The kept entries, from weeks.jsonl (the weeks before the last
        are let go at the next post)."""
        if not self.path or not os.path.exists(self.path):
            return
        with open(self.path, encoding="utf-8") as log:
            for line in log:
                try:
                    entry = json.loads(line)
                    key = (int(entry["week"]), str(entry["identity"]))
                except (ValueError, KeyError, TypeError):
                    continue
                if key not in self.best or better(entry, self.best[key]):
                    self.best[key] = entry

    def seed(self, week):
        """The week's seed: this server's own, never 0 (a game's "none")."""
        digest = hmac.new(self.server.store.secret, b"week" + struct.pack("!I", week), hashlib.sha256).digest()
        return struct.unpack("!I", digest[:4])[0] or 1

    def current(self):
        week = week_of(self.clock())
        return week, self.seed(week)

    def hidden(self, identity, week):
        entry = self.server.store.rules.get("hidden", {}).get(identity)
        return bool(entry) and entry.get("week") == week

    def ranked(self, week):
        entries = [e for (w, who), e in self.best.items() if w == week and not self.hidden(who, week)]
        ranked = []
        for entry in entries:
            at = next((k for k, other in enumerate(ranked) if better(entry, other)), len(ranked))
            ranked.insert(at, entry)
        return ranked

    def board_for(self, player):
        """WEEK_BOARD: this week's top eight, the recipient's own best and its
        place, last week's top three."""
        week = week_of(self.clock())
        this, last = self.ranked(week), self.ranked(week - 1)
        mine = next(((k + 1, e) for k, e in enumerate(this) if e["identity"] == player.identity.hex()), (0, None))
        top, past = this[:SPEC["week_top"]], last[:SPEC["week_last_top"]]
        head = BOARD_HEAD.pack(SPEC["week_board"], week, len(top), len(past), min(mine[0], 255))
        return head + self.entry(mine[1]) + b"".join(self.entry(e) for e in top + past)

    @staticmethod
    def entry(e):
        if not e:
            return bytes(ENTRY.size)
        return ENTRY.pack(e["name"].encode("ascii")[:16], e["depth"], e["act"], WON if e["won"] else 0, e["minutes"])

    async def send(self, player):
        await websocket.send(player.writer, self.board_for(player))

    def broadcast(self):
        for player in list(self.server.players.values()):
            if not player.writer.is_closing():
                player.writer.write(websocket.frame(self.board_for(player)))

    def refusal(self, entry, identity):
        """Why a post is not plausible, as a NOTICE (code, words); None where it is."""
        week = week_of(self.clock())
        if entry["week"] not in (week, week - 1):
            return 23, "That week's net is closed"
        if entry["seed"] != self.seed(entry["week"]):
            return 24, "Not that week's net"
        depth, act = entry["depth"], entry["act"]
        if not 1 <= depth <= SPEC["week_layers"] or act != act_of(depth) or entry["flags"] & ~WON \
                or (entry["won"] and depth != SPEC["week_layers"]) or not entry["hash"]:
            return 25, "That dive does not add up"
        if self.hidden(identity, entry["week"]):
            return 26, "A moderator took that week's post down"
        return None

    async def handle(self, player, data):
        entry = read_post(data)
        if await self.server.without_pass(player):
            return
        identity = player.identity.hex()
        refused = self.refusal(entry, identity)
        if refused:
            await self.server.notice(player, *refused)
            return
        entry.update(identity=identity, name=player.name, time=self.clock())
        key = (entry["week"], identity)
        if key in self.best and not better(entry, self.best[key]):
            await self.send(player)
            return
        self.best[key] = entry
        self.prune()
        self.server.event("week_post", id=player.id, week=entry["week"], depth=entry["depth"], won=entry["won"])
        if self.path:
            self.write_task = asyncio.create_task(asyncio.to_thread(self.write, dict(entry, flags=None)))
        self.broadcast()

    def prune(self):
        week = week_of(self.clock())
        for key in [key for key in self.best if key[0] < week - 1]:
            del self.best[key]

    def write(self, entry):
        entry.pop("flags", None)
        with open(self.path, "a", encoding="utf-8") as log:
            log.write(json.dumps(entry, separators=(",", ":"), sort_keys=True) + "\n")
