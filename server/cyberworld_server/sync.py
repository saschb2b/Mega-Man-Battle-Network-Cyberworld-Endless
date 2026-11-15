"""Saves kept in step (shared/online/README.md, Saves kept in step;
docs/engine/saves.md, Saves kept in step).

A connection to /presence whose first message is SYNC is one exchange and
no visit, as TRANSFER's is (transfer.py): no player, no place, no channel,
no identity. A slot, named by a 16-byte id and opened by a 32-byte auth,
both the game's, holds one player's saves as a blob the game sealed and the
server never reads, with its generation, a count raised at each push. A
pull hands the blob over where the game's generation is not the slot's; a
push replaces it where the game's is (a push from 0 makes a slot the server
has not got, its auth the slot's from then on); a drop deletes the slot.

The slots live on disk in the state directory's `cloud/`: `SLOT.blob`, the
blob, and `SLOT.json`, its meta (the auth's SHA-256, never the auth; the
generation; the blob's size; the UTC days it was made and last used), each
written whole (a temporary file, fsync, a rename: identity.write_text)
under the state directory's lock, the blob before the meta, and a slot
removed meta first, so a meta always has its blob. Each exchange reads its
slot's meta from disk, so a slot `forget` removed from another process is
gone at once; the server keeps the slots' sizes in memory for its bounds,
read as it starts and again after each retention pass. A slot unused for
`days` days is no slot to any exchange and is deleted by the retention's
pass (retention.py, `cloud_days`). Backups leave `cloud/` out (backup.py).

The server keeps MOST slots and MOST_BYTES bytes at most (a push's bytes
reserved from its READY), makes PER_HOUR new slots an hour at most (the
hour's makings counted at their READY), takes AT_ONCE exchanges at once and
one push a slot; past any of these it answers BUSY with the seconds to
wait. It sits behind a proxy and never sees a peer's address, so none of
these is a peer's. A frame QUIET seconds in coming (or going) or an
exchange past WHOLE seconds ends it with an ERROR."""

import asyncio
import collections
import hashlib
import hmac
import json
import math
import os
from pathlib import Path
import re
import struct
import time

from . import protocol, websocket
from .identity import state_lock, write_text
from .protocol import ProtocolError, SPEC

FOLDER = "cloud"                # in the state directory
MOST = 20000                    # slots kept at most: server.ini's cloud_slots
MEGABYTES = 4096                # their blobs' bytes at most, in MiB: cloud_megabytes
LIMITS = {"cloud_slots": (1, 1_000_000), "cloud_megabytes": (1, 1 << 20)}
PER_HOUR = 60                   # new slots an hour
AT_ONCE = 16                    # exchanges under way at once
QUIET = 15.0                    # seconds a frame may be in coming or going
WHOLE = 120.0                   # seconds an exchange may take
HOUR = 3600
DAY = 24 * 3600
FULL_WAIT = HOUR                # BUSY's seconds while it keeps as many slots or bytes as it will (the retention's next pass)
AT_ONCE_WAIT = 10               # ... while as many exchanges are under way as it takes, or another push fills the slot
CHUNK = SPEC["max_message"] - 1     # TRANSFER_DATA's bytes at most
SLOT_NAME = re.compile(r"[0-9a-f]{32}")
META_BYTES = 1024               # a meta's file at most
STATUS = {name: SPEC["sync_" + name] for name in ("same", "found", "none", "denied", "ready", "stored",
                                                   "conflict", "busy", "off", "dropped")}


# ---- the days ----

def day_of(now):
    return time.strftime("%Y-%m-%d", time.gmtime(now))


def first_day(now, days):
    """The oldest last-used day a slot may have and still be kept: a slot
    used on day D is kept through day D + days - 1."""
    return day_of(now - (days - 1) * DAY)


# ---- the files (each run in a thread) ----

def folder(directory):
    return Path(directory) / FOLDER


def slot_id(text):
    """A slot's id as `--admin forget --cloud` takes it: 32 hexadecimal characters."""
    if not isinstance(text, str) or not SLOT_NAME.fullmatch(text.lower()):
        raise ValueError("A slot is 32 hexadecimal characters")
    return text.lower()


def auth_hash(auth):
    """What the server keeps of a slot's auth: its SHA-256, in hex."""
    return hashlib.sha256(auth).hexdigest()


def checked(meta):
    """A meta read back: its own fields, each of its kind, or None."""
    if not isinstance(meta, dict):
        return None
    auth, generation, size = meta.get("auth"), meta.get("generation"), meta.get("size")
    days = [meta.get("created"), meta.get("used")]
    if not (isinstance(auth, str) and re.fullmatch(r"[0-9a-f]{64}", auth)
            and all(isinstance(n, int) and not isinstance(n, bool) for n in (generation, size))
            and 1 <= generation <= 0xffffffff and 1 <= size <= SPEC["sync_max"]
            and all(isinstance(day, str) and len(day) == 10 for day in days)):
        return None
    return {"auth": auth, "generation": generation, "size": size, "created": days[0], "used": days[1]}


def read_meta(directory, slot):
    """A slot's meta from disk, or None where it has none or one that will not load."""
    try:
        with (folder(directory) / (slot + ".json")).open("r", encoding="utf-8") as stream:
            return checked(json.loads(stream.read(META_BYTES + 1)))
    except FileNotFoundError:
        return None
    except (OSError, ValueError):
        return None


def live(meta, first):
    """The meta of a slot still kept, None for one unused since before `first`."""
    return meta if meta is not None and meta["used"] >= first else None


def write_meta(directory, slot, meta):
    write_text(folder(directory) / (slot + ".json"), json.dumps(meta, separators=(",", ":"), sort_keys=True) + "\n")


def remove(directory, slot):
    """A slot's files deleted, the meta first: a meta always has its blob."""
    (folder(directory) / (slot + ".json")).unlink(missing_ok=True)
    (folder(directory) / (slot + ".blob")).unlink(missing_ok=True)


def scan(directory):
    """The slots on disk and their blobs' sizes: {slot: size}."""
    try:
        names = os.listdir(folder(directory)) if directory else []
    except FileNotFoundError:
        return {}
    found = {}
    for name in names:
        slot, dot, kind = name.partition(".")
        if kind == "json" and SLOT_NAME.fullmatch(slot):
            meta = read_meta(directory, slot)
            if meta:
                found[slot] = meta["size"]
    return found


def prune(directory, now, days):
    """The retention's pass (retention.py): the slots unused for `days`
    days deleted, and with them what a write cut short left (a meta that
    will not load, a blob without its meta, a temporary file). How many
    slots went."""
    cloud = folder(directory) if directory else None
    if not cloud or not cloud.is_dir():
        return 0
    first, gone = first_day(now, days), 0
    with state_lock(directory):
        names = os.listdir(cloud)
        metas = {name[:-5] for name in names if name.endswith(".json") and SLOT_NAME.fullmatch(name[:-5])}
        for slot in metas:
            if live(read_meta(directory, slot), first) is None:
                remove(directory, slot)
                gone += 1
        for name in names:
            slot, _, kind = name.partition(".")
            if name.startswith(".server-") or (kind == "blob" and slot not in metas):
                (cloud / name).unlink(missing_ok=True)
    return gone


def forget(directory, slot):
    """`--admin forget --cloud SLOT`: the slot deleted; whether it was there."""
    with state_lock(directory):
        there = (folder(directory) / (slot + ".json")).exists() or (folder(directory) / (slot + ".blob")).exists()
        remove(directory, slot)
    return there


def same_auth(meta, auth):
    return hmac.compare_digest(meta["auth"], auth)


def pull_disk(directory, slot, auth, generation, today, first):
    """A pull, under the lock: (status, meta, blob); the last-used day
    written where it was another."""
    with state_lock(directory):
        meta = live(read_meta(directory, slot), first)
        if meta is None:
            return STATUS["none"], None, None
        if not same_auth(meta, auth):
            return STATUS["denied"], None, None
        if meta["generation"] == generation:
            blob = None
        else:
            try:
                blob = (folder(directory) / (slot + ".blob")).read_bytes()
            except FileNotFoundError:
                return STATUS["none"], None, None
        if meta["used"] != today:
            meta = dict(meta, used=today)
            write_meta(directory, slot, meta)
        return (STATUS["same"] if blob is None else STATUS["found"]), meta, blob


def store_disk(directory, slot, auth, generation, blob, today, first):
    """A push's blob kept, under the lock, where the slot is still at the
    generation it was made from (a push from 0: where there is none):
    (status, meta)."""
    with state_lock(directory):
        meta = live(read_meta(directory, slot), first)
        if meta is None:
            if generation:
                return STATUS["none"], None
            meta = {"auth": auth, "generation": 0, "size": 0, "created": today, "used": today}
        elif not same_auth(meta, auth):
            return STATUS["denied"], None
        elif meta["generation"] != generation:
            return STATUS["conflict"], meta
        folder(directory).mkdir(mode=0o700, exist_ok=True)
        write_text(folder(directory) / (slot + ".blob"), bytes(blob))
        meta = dict(meta, generation=meta["generation"] % 0xffffffff + 1, size=len(blob), used=today)
        write_meta(directory, slot, meta)
        return STATUS["stored"], meta


def drop_disk(directory, slot, auth, first):
    with state_lock(directory):
        meta = live(read_meta(directory, slot), first)
        if meta is None:
            return STATUS["none"]
        if not same_auth(meta, auth):
            return STATUS["denied"]
        remove(directory, slot)
        return STATUS["dropped"]


# ---- the exchange ----

class Slots:
    def __init__(self, directory=None, on=True, count=None, clock=time.time, monotonic=time.monotonic,
                 slots=MOST, megabytes=MEGABYTES, days=SPEC["sync_days"], per_hour=PER_HOUR, at_once=AT_ONCE,
                 quiet=QUIET, whole=WHOLE):
        for name, value in (("cloud_slots", slots), ("cloud_megabytes", megabytes)):
            low, high = LIMITS[name]
            if isinstance(value, bool) or not isinstance(value, int) or not low <= value <= high:
                raise ValueError(f"{name} must be {low} to {high}")
        self.directory = Path(directory) if directory else None
        # (no state directory, no slots: a server started without one keeps none)
        self.on = bool(on) and self.directory is not None
        self.count = count or (lambda field: None)     # the daily totals' (stats.py)
        self.clock, self.monotonic = clock, monotonic
        self.most, self.most_bytes, self.days = slots, megabytes << 20, days
        self.per_hour, self.at_once, self.quiet, self.whole = per_hour, at_once, quiet, whole
        self.index = scan(self.directory)       # slot -> its blob's size
        self.filling = {}       # slot -> (size, makes): the pushes under way, their bytes reserved, whether each makes a slot
        self.made = collections.deque()     # the monotonic times of this hour's makings
        self.under_way = 0
        self.writes = set()     # the disk's work in threads, for shutdown

    # ---- what is kept ----

    def kept(self):
        """The slots kept now: the daily totals' sync_slots."""
        return len(self.index)

    def rescan(self, found):
        """The slots on disk as a retention pass left them (scan's)."""
        self.index = found

    def held_bytes(self):
        return sum(self.index.values()) + sum(size for size, _ in self.filling.values())

    def days_now(self):
        now = self.clock()
        return day_of(now), first_day(now, self.days)

    def making(self):
        """A slot made, from the hour's: 0, or the seconds until one may be."""
        now = self.monotonic()
        while self.made and self.made[0] <= now - HOUR:
            self.made.popleft()
        if len(self.made) < self.per_hour:
            return 0
        return max(1, math.ceil(self.made[0] + HOUR - now))

    async def disk(self, work, *args):
        """`work` in a thread, which a cancelled exchange leaves to finish:
        the server's close waits for it (app.py)."""
        task = asyncio.ensure_future(asyncio.to_thread(work, *args))
        self.writes.add(task)
        task.add_done_callback(self.writes.discard)
        return await asyncio.shield(task)

    async def exchange(self, reader, writer, first):
        """One connection's exchange, its first message `first`; a
        ProtocolError ends it with an ERROR (app.py's handle)."""
        if not self.on:
            return await self.end(writer, STATUS["off"])
        op, slot, auth, generation, size = protocol.read_sync(first)
        if self.under_way >= self.at_once:
            return await self.end(writer, STATUS["busy"], 0, AT_ONCE_WAIT)
        self.under_way += 1
        try:
            deadline = asyncio.get_running_loop().time() + self.whole
            slot, auth = slot.hex(), auth_hash(auth)
            if op == SPEC["sync_pull"]:
                await self.pull(writer, slot, auth, generation, deadline)
            elif op == SPEC["sync_push"]:
                await self.push(reader, writer, slot, auth, generation, size, deadline)
            else:
                status = await self.disk(drop_disk, self.directory, slot, auth, self.days_now()[1])
                if status in (STATUS["dropped"], STATUS["none"]):
                    self.index.pop(slot, None)
                await self.end(writer, status)
        finally:
            self.under_way -= 1

    async def pull(self, writer, slot, auth, generation, deadline):
        status, meta, blob = await self.disk(pull_disk, self.directory, slot, auth, generation, *self.days_now())
        if meta is None:
            if status == STATUS["none"]:
                self.index.pop(slot, None)
            return await self.end(writer, status)
        self.index[slot] = meta["size"]
        self.count("sync_pulls")
        if blob is None:
            return await self.end(writer, status, meta["generation"])
        await self.send(writer, protocol.sync_state(status, meta["generation"], len(blob)), deadline)
        for n, start in enumerate(range(0, len(blob), CHUNK)):
            await self.send(writer, bytes((SPEC["transfer_data"],)) + blob[start:start + CHUNK], deadline)
            if n % 32 == 31:
                # (a socket that takes all at once never makes drain wait:
                # the Square's tick goes on between every 16 KB)
                await asyncio.sleep(0)
        await websocket.send(writer, struct.pack("!H", 1000), 8)

    async def push(self, reader, writer, slot, auth, generation, size, deadline):
        today, first = self.days_now()
        meta = live(await self.disk(read_meta, self.directory, slot), first)
        if meta is not None and not same_auth(meta, auth):
            return await self.end(writer, STATUS["denied"])
        if meta is None and generation:
            self.index.pop(slot, None)
            return await self.end(writer, STATUS["none"])
        if meta is not None and meta["generation"] != generation:
            return await self.end(writer, STATUS["conflict"], meta["generation"])
        wait = self.room(slot, meta, size)
        if wait:
            return await self.end(writer, STATUS["busy"], 0, wait)
        self.filling[slot] = (size, meta is None)
        if meta is None:
            self.made.append(self.monotonic())
        try:
            await self.send(writer, protocol.sync_state(STATUS["ready"], generation), deadline)
            blob = bytearray()
            while len(blob) < size:
                message = await self.next(reader, writer, deadline)
                if message is None:
                    return
                if message[0] != SPEC["transfer_data"] or len(message) < 2:
                    raise ProtocolError("Expected TRANSFER_DATA")
                if len(message) - 1 > size - len(blob):
                    raise ProtocolError("TRANSFER_DATA past its size")
                blob += memoryview(message)[1:]
            status, meta = await self.disk(store_disk, self.directory, slot, auth, generation, blob, *self.days_now())
        finally:
            del self.filling[slot]
        if status == STATUS["stored"]:
            self.index[slot] = meta["size"]
            self.count("sync_pushes")
        elif status == STATUS["none"]:
            self.index.pop(slot, None)
        await self.end(writer, status, meta["generation"] if meta else 0)

    def room(self, slot, meta, size):
        """Whether the server takes this push now: 0, or BUSY's seconds."""
        if slot in self.filling:
            return AT_ONCE_WAIT
        if meta is None and slot not in self.index:
            making = sum(makes and other not in self.index for other, (_, makes) in self.filling.items())
            if len(self.index) + making >= self.most:
                return FULL_WAIT
        if self.held_bytes() + size - (meta["size"] if meta else 0) > self.most_bytes:
            return FULL_WAIT
        return self.making() if meta is None else 0

    # ---- the socket ----

    async def within(self, waiting, deadline):
        """`waiting` within QUIET seconds and the exchange's deadline."""
        loop = asyncio.get_running_loop()
        try:
            async with asyncio.timeout_at(min(loop.time() + self.quiet, deadline)):
                return await waiting
        except TimeoutError as exc:
            raise ProtocolError("Sync timed out") from exc

    async def next(self, reader, writer, deadline):
        """The client's next binary message, pings answered; None where it closed."""
        while True:
            opcode, data = await self.within(websocket.receive(reader), deadline)
            if opcode == 8:
                await self.send(writer, struct.pack("!H", 1000), deadline, 8)
                return None
            if opcode == 9:
                await self.send(writer, data, deadline, 10)
            elif opcode == 2:
                if not data:
                    raise ProtocolError("Unknown or out-of-order message")
                return data

    async def send(self, writer, data, deadline, opcode=2):
        writer.write(websocket.frame(data, opcode))
        await self.within(writer.drain(), deadline)

    async def end(self, writer, status, generation=0, size=0):
        """The exchange's last word, and the connection closed (app.py's handle)."""
        await websocket.send(writer, protocol.sync_state(status, generation, size))
        await websocket.send(writer, struct.pack("!H", 1000), 8)
