"""Authenticated socket adapter for the isolated trade coordinator.

Only the test service attaches this adapter today. Public presence v2 must
not send these extension messages before a compatible client is negotiated.
The presence owner supplies authenticated identities and calls disconnect
in its connection's finally block. Durable decisions precede every reply.
"""

import asyncio
from dataclasses import dataclass

from . import trade_protocol as wire, websocket
from .trade_lobby import TradeLobby
from .trade_store import TradeError


@dataclass(eq=False, frozen=True)
class TradeSession:
    identity: bytes
    writer: asyncio.StreamWriter


class TradeEndpoint:
    def __init__(self, store, capacity=256):
        self.lobby = TradeLobby(store, capacity)
        self.sessions = {}
        # Keep notifications in decision order, including when two peers
        # prepare concurrently. Slow clients have a bounded write deadline.
        self.lock = asyncio.Lock()

    def _current(self, session):
        if self.sessions.get(session.identity) is not session:
            raise TradeError("Trade connection is no longer current")

    def _room(self, channel, exclude=None):
        count = sum(p.channel == channel and p.identity != exclude for p in self.lobby.players.values())
        if count >= wire.SPEC["listings_max"]:
            raise TradeError("Trade channel is full")

    async def _send(self, session, data):
        try:
            if session.writer.is_closing():
                return False
            await websocket.send(session.writer, data)
            return True
        except (ConnectionError, asyncio.TimeoutError):
            session.writer.close()
            return False

    async def _decision(self, decision, only=None):
        payload = wire.decision_bytes(decision)
        recipients = (decision.left.identity, decision.right.identity) if only is None else (only,)
        await asyncio.gather(*(self._send(self.sessions[identity], payload)
                               for identity in recipients if identity in self.sessions))

    async def _boards(self, channels):
        deliveries = []
        for identity, session in tuple(self.sessions.items()):
            if self.lobby.players[identity].channel in channels:
                deliveries.append((session, wire.board(await self.lobby.board(identity))))
        await asyncio.gather(*(self._send(session, payload) for session, payload in deliveries))

    async def connect(self, identity, name, channel, writer):
        session = None
        try:
            async with self.lock:
                self._room(channel)
                pending = await self.lobby.connect(identity, name, channel)
                session = TradeSession(identity, writer)
                self.sessions[identity] = session
                if not await self._send(session, wire.caps(self.lobby.store.server_identity, True)):
                    raise ConnectionError("Trade capability delivery failed")
                if pending:
                    await self._decision(pending, only=identity)
                await self._send(session, wire.board(await self.lobby.board(identity)))
            return session
        except BaseException:
            if session is not None:
                await self.disconnect(session)
            raise

    async def message(self, session, data):
        async with self.lock:
            self._current(session)
            method, args = wire.read(data)
            result = await getattr(self.lobby, method)(session.identity, *args)
            if method == "board":
                await self._send(session, wire.board(result))
            elif result is not None:
                await self._decision(result, only=session.identity if method == "status" else None)
            if method in ("ready", "advertise", "request", "cancel", "acknowledge"):
                await self._boards({self.lobby.players[session.identity].channel})

    async def move(self, session, channel):
        async with self.lock:
            self._current(session)
            self._room(channel, session.identity)
            old = self.lobby.players[session.identity].channel
            await self.lobby.move(session.identity, channel)
            await self._boards({old, channel})

    async def _disconnect(self, session):
        async with self.lock:
            if self.sessions.get(session.identity) is not session:
                return
            channel = self.lobby.players[session.identity].channel
            del self.sessions[session.identity]
            pending = await self.lobby.disconnect(session.identity)
            if pending:
                await self._decision(pending)
            await self._boards({channel})

    async def disconnect(self, session):
        # Handler shutdown cannot cancel the cleanup halfway through a
        # database write and leave an offer reserved for a departed peer.
        cleanup = asyncio.create_task(self._disconnect(session))
        cancelled = False
        while not cleanup.done():
            try:
                await asyncio.shield(cleanup)
            except asyncio.CancelledError:
                cancelled = True
        cleanup.result()
        if cancelled:
            raise asyncio.CancelledError
