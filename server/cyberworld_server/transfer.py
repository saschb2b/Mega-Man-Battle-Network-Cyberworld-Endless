"""Saves sent by a code (shared/online/README.md, Saves sent by a code;
docs/engine/saves.md, Sending by code).

A connection to /presence whose first message is TRANSFER rather than
HELLO is one exchange and no visit: no player, no place, no channel, no
identity. A put leaves a sealed blob under a 16-byte lookup; a get hands it
back, and the game's DONE on the same connection lets it go. Both are the
game's: the server can open neither the blob nor the code a player types
from what it holds.

Blobs are held in memory alone, never in a file, a log or a backup, for
`transfer_minutes` minutes or until taken. A get that found one and got no
DONE leaves it held until its time is up, so a broken download can be tried
again. The server holds MOST blobs and MOST_BYTES bytes at most (a put's
bytes reserved from its READY), takes AT_ONCE exchanges at once, and answers
PER_MINUTE lookups a minute, puts' and gets' alike, from a bucket of the
whole server's: it sits behind a proxy and never sees a peer's address, and
a put's TAKEN says whether a lookup is held as a get's NONE does. A frame
QUIET seconds in coming (or going) or an exchange past WHOLE seconds ends
it with an ERROR. Expired blobs are pruned as each exchange begins and in
the server's housekeeping (app.py's tick)."""

import asyncio
import dataclasses
import math
import struct
import time

from . import protocol, websocket
from .protocol import ProtocolError, SPEC

MOST = 128                  # blobs held at most, puts under way among them
MOST_BYTES = 64 << 20       # their bytes at most
AT_ONCE = 8                 # exchanges under way at once
PER_MINUTE = 30             # lookups a minute, the bucket's depth too
QUIET = 15.0                # seconds a frame may be in coming or going
WHOLE = 120.0               # seconds an exchange may take
FULL_WAIT = 60              # BUSY's seconds while the server holds as much as it will
AT_ONCE_WAIT = 10           # ... while as many exchanges are under way as it takes
CHUNK = SPEC["max_message"] - 1     # TRANSFER_DATA's bytes at most


@dataclasses.dataclass
class Blob:
    data: bytes
    expires: float          # the clock's


class Transfers:
    def __init__(self, on=True, count=None, clock=time.monotonic, minutes=SPEC["transfer_minutes"],
                 most=MOST, most_bytes=MOST_BYTES, at_once=AT_ONCE, per_minute=PER_MINUTE,
                 quiet=QUIET, whole=WHOLE):
        self.on = on
        self.count = count or (lambda field: None)     # the daily totals' (stats.py)
        self.clock = clock
        self.seconds = minutes * 60
        self.most, self.most_bytes, self.at_once = most, most_bytes, at_once
        self.per_minute, self.quiet, self.whole = per_minute, quiet, whole
        self.held = {}          # lookup -> Blob
        self.coming = {}        # lookup -> size: the puts under way, their lookups and bytes reserved
        self.under_way = 0
        self.tokens, self.filled = float(per_minute), clock()

    # ---- what is held ----

    def prune(self):
        """The blobs whose time is up let go: how many."""
        now = self.clock()
        gone = [lookup for lookup, blob in self.held.items() if blob.expires <= now]
        for lookup in gone:
            del self.held[lookup]
        return len(gone)

    def held_bytes(self):
        return sum(len(blob.data) for blob in self.held.values()) + sum(self.coming.values())

    def ask(self):
        """A lookup asked about, from the bucket: 0, or the seconds until one may be."""
        now = self.clock()
        self.tokens = min(self.per_minute, self.tokens + max(0.0, now - self.filled) * self.per_minute / 60)
        self.filled = now
        if self.tokens >= 1:
            self.tokens -= 1
            return 0
        return max(1, math.ceil((1 - self.tokens) * 60 / self.per_minute))

    # ---- the exchange ----

    async def exchange(self, reader, writer, first):
        """One connection's exchange, its first message `first`; a
        ProtocolError ends it with an ERROR (app.py's handle)."""
        if not self.on:
            return await self.end(writer, SPEC["transfer_off"])
        op, lookup, size = protocol.read_transfer(first)
        if op == SPEC["transfer_done"]:
            raise ProtocolError("TRANSFER done before a get")
        if self.under_way >= self.at_once:
            return await self.end(writer, SPEC["transfer_busy"], AT_ONCE_WAIT)
        self.under_way += 1
        try:
            deadline = asyncio.get_running_loop().time() + self.whole
            self.prune()
            wait = self.ask()
            if wait:
                return await self.end(writer, SPEC["transfer_busy"], wait)
            if op == SPEC["transfer_put"]:
                await self.put(reader, writer, lookup, size, deadline)
            else:
                await self.get(reader, writer, lookup, deadline)
        finally:
            self.under_way -= 1

    async def put(self, reader, writer, lookup, size, deadline):
        if lookup in self.held or lookup in self.coming:
            return await self.end(writer, SPEC["transfer_taken"])
        if len(self.held) + len(self.coming) >= self.most or self.held_bytes() + size > self.most_bytes:
            return await self.end(writer, SPEC["transfer_busy"], FULL_WAIT)
        self.coming[lookup] = size
        try:
            await self.send(writer, protocol.transfer_state(SPEC["transfer_ready"]), deadline)
            data = bytearray()
            while len(data) < size:
                message = await self.next(reader, writer, deadline)
                if message is None:
                    return
                if message[0] != SPEC["transfer_data"] or len(message) < 2:
                    raise ProtocolError("Expected TRANSFER_DATA")
                if len(message) - 1 > size - len(data):
                    raise ProtocolError("TRANSFER_DATA past its size")
                data += memoryview(message)[1:]
            self.held[lookup] = Blob(bytes(data), self.clock() + self.seconds)
        finally:
            del self.coming[lookup]
        self.count("transfers_held")
        await self.end(writer, SPEC["transfer_held"], self.seconds)

    async def get(self, reader, writer, lookup, deadline):
        blob = self.held.get(lookup)
        if blob is None:
            return await self.end(writer, SPEC["transfer_none"])
        await self.send(writer, protocol.transfer_state(SPEC["transfer_found"], len(blob.data)), deadline)
        for n, start in enumerate(range(0, len(blob.data), CHUNK)):
            await self.send(writer, bytes((SPEC["transfer_data"],)) + blob.data[start:start + CHUNK], deadline)
            if n % 32 == 31:
                # (a socket that takes all at once never makes drain wait:
                # the Square's tick goes on between every 16 KB)
                await asyncio.sleep(0)
        message = await self.next(reader, writer, deadline)
        if message is None:
            return
        op, done, _ = protocol.read_transfer(message)
        if op != SPEC["transfer_done"] or done != lookup:
            raise ProtocolError("Expected TRANSFER done")
        # (the blob this get found: one taken already, or gone and put
        # again under the same lookup, is not let go twice)
        if self.held.get(lookup) is blob:
            del self.held[lookup]
            self.count("transfers_taken")
        await self.end(writer, SPEC["transfer_removed"])

    # ---- the socket ----

    async def within(self, waiting, deadline):
        """`waiting` within QUIET seconds and the exchange's deadline."""
        loop = asyncio.get_running_loop()
        try:
            async with asyncio.timeout_at(min(loop.time() + self.quiet, deadline)):
                return await waiting
        except TimeoutError as exc:
            raise ProtocolError("Transfer timed out") from exc

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

    async def end(self, writer, status, value=0):
        """The exchange's last word, and the connection closed (app.py's handle)."""
        await websocket.send(writer, protocol.transfer_state(status, value))
        await websocket.send(writer, struct.pack("!H", 1000), 8)
