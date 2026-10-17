"""Live, bounded trade offers around the durable coordinator.

No sockets or ROM data here. The caller authenticates the presence identity,
and notifies both participants only after each awaited decision returns.
This component is not yet enabled by the public presence service.
"""

import asyncio
from dataclasses import dataclass, replace
import re

from .trade_store import OFFERED, Side, TradeError


@dataclass(frozen=True)
class Carried:
    run: bytes
    revision: int
    checkpoint: bytes
    bn5: bool
    all_star: bool

    def side(self, identity, chip):
        if type(self.bn5) is not bool or type(self.all_star) is not bool:
            raise TradeError("Invalid carried rules")
        side = Side(identity, self.checkpoint, chip, self.run, self.revision, int(self.bn5) | int(self.all_star) << 1)
        side.validate()
        return side


@dataclass(frozen=True)
class Offer:
    identity: bytes
    name: str
    channel: int
    carried: Carried | None = None
    have: int = 0
    want: int = 0


class TradeLobby:
    def __init__(self, store, capacity=256):
        if type(capacity) is not int or not 1 <= capacity <= 256:
            raise TradeError("Invalid trade lobby capacity")
        self.store, self.capacity = store, capacity
        self.players = {}
        self.lock = asyncio.Lock()

    async def _stored(self, method, *args):
        # SQLite flushing must not stall movement/pings on the event loop.
        return await asyncio.to_thread(getattr(self.store, method), *args)

    def _player(self, identity):
        player = self.players.get(identity)
        if player is None:
            raise TradeError("Player is no longer in the Square")
        return player

    async def connect(self, identity, name, channel):
        if (not isinstance(identity, bytes) or len(identity) != 16 or not any(identity) or
                not isinstance(name, str) or not re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9 _-]{0,15}", name) or
                type(channel) is not int or not 1 <= channel <= 256):
            raise TradeError("Invalid trade participant")
        async with self.lock:
            if identity in self.players or len(self.players) >= self.capacity:
                raise TradeError("Trade participant already connected or lobby full")
            pending = await self._stored("pending", identity)
            self.players[identity] = Offer(identity, name, channel)
            return pending

    async def ready(self, identity, carried):
        if not isinstance(carried, Carried):
            raise TradeError("A carried checkpoint is required")
        carried.side(identity, (26 << 9 | 1) if carried.all_star else 1)
        async with self.lock:
            player = self._player(identity)
            if await self._stored("pending", identity):
                raise TradeError("Resolve the pending trade before changing checkpoints")
            self.players[identity] = replace(player, carried=carried, have=0, want=0)

    async def advertise(self, identity, have, want=0):
        async with self.lock:
            player = self._player(identity)
            if not player.carried:
                raise TradeError("A carried checkpoint is required")
            if type(have) is not int or type(want) is not int or (not have and want):
                raise TradeError("Invalid have/want offer")
            for entry in (have, want):
                if entry:
                    player.carried.side(identity, entry)
                    if player.carried.all_star and entry >> 9 != 26:
                        raise TradeError("All * offers must keep their code")
            if have and have == want:
                raise TradeError("Offer would exchange the same entry")
            if await self._stored("pending", identity):
                raise TradeError("Resolve the pending trade before posting another offer")
            self.players[identity] = replace(player, have=have, want=want)

    async def board(self, identity):
        async with self.lock:
            channel = self._player(identity).channel
            return tuple(p for p in self.players.values() if p.have and p.channel == channel)

    async def request(self, identity, target, give, receive):
        """Bind a selection to both exact advertised entries, never a new offer."""
        async with self.lock:
            left, right = self._player(identity), self._player(target)
            if identity == target or left.channel != right.channel:
                raise TradeError("Choose another player in this channel")
            if not left.carried or not right.carried or not left.have or not right.have:
                raise TradeError("Both players need a live offer")
            if (left.have != give or right.have != receive or
                    (left.want and left.want != receive) or (right.want and right.want != give)):
                raise TradeError("The advertised offer changed or does not match")
            if left.carried.all_star != right.carried.all_star:
                raise TradeError("The runs use different All * rules")
            decision = await self._stored("create", left.carried.side(identity, give), right.carried.side(target, receive))
            self.players[identity] = replace(left, have=0, want=0)
            self.players[target] = replace(right, have=0, want=0)
            return decision

    async def prepare(self, identity, transaction, digest):
        async with self.lock:
            self._player(identity)
            return await self._stored("prepare", transaction, identity, digest)

    async def cancel(self, identity, transaction):
        async with self.lock:
            self._player(identity)
            return await self._stored("cancel", transaction, identity)

    async def status(self, identity, transaction):
        async with self.lock:
            self._player(identity)
            return await self._stored("status", transaction, identity)

    async def acknowledge(self, identity, transaction):
        async with self.lock:
            player = self._player(identity)
            decision = await self._stored("acknowledge", transaction, identity)
            # The caller must capture the installed revision before posting.
            # Replayed acknowledgements must not discard a newer readiness.
            side = decision.left if decision.left.identity == identity else decision.right
            if player.carried and (player.carried.run, player.carried.revision) == (side.run_id, side.revision):
                self.players[identity] = replace(player, carried=None, have=0, want=0)
            return decision

    async def move(self, identity, channel):
        if type(channel) is not int or not 1 <= channel <= 256:
            raise TradeError("Invalid channel")
        async with self.lock:
            player = self._player(identity)
            pending = await self._stored("pending", identity)
            if pending:
                raise TradeError("Resolve the pending trade before changing channels")
            self.players[identity] = replace(player, channel=channel, have=0, want=0)

    async def disconnect(self, identity):
        async with self.lock:
            self._player(identity)
            try:
                pending = await self._stored("pending", identity)
                if pending and pending.state == OFFERED:
                    pending = await self._stored("cancel", pending.transaction, identity)
                return pending
            finally:
                self.players.pop(identity, None)
