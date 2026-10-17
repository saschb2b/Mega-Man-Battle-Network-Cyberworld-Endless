"""Real sockets exercise the trade adapter without enabling public presence."""

import asyncio
import hashlib
from pathlib import Path
import struct
import tempfile
import threading
import unittest

from test_server import Client
from cyberworld_server import protocol
from cyberworld_server.app import Server
from cyberworld_server import trade_protocol as wire
from cyberworld_server.trade_endpoint import TradeEndpoint
from cyberworld_server.trade_store import ABORTED, COMMITTED, OFFERED, TradeError, TradeStore


class TradeTestServer(Server):
    """Only an explicit compatible subprotocol attaches the socket adapter."""

    def __init__(self, store, root):
        super().__init__(capacity=4, player_cap=8, state_dir=root, trade_endpoint=TradeEndpoint(store, 8))


class EndpointTests(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.clients = []
        await self.start()

    async def start(self):
        self.store = TradeStore(self.root)
        self.store.recover()
        self.server = TradeTestServer(self.store, self.root)
        self.port = await self.server.start("127.0.0.1", 0)

    async def stop(self):
        for client in self.clients:
            await client.close()
        self.clients.clear()
        await self.server.close()
        self.store.close()

    async def asyncTearDown(self):
        await self.stop()
        self.temporary.cleanup()

    async def connect(self, number, channel=1):
        client = Client()
        self.clients.append(client)
        welcome = await client.open(self.port, f"Navi{number}", key=bytes((number,)) * 16, channel=channel,
                                    protocols=wire.SUBPROTOCOL + "," + protocol.SPEC["subprotocol"])
        self.assertEqual(client.protocol, wire.SUBPROTOCOL)
        client.identity, client.number = welcome[14:30], number
        caps = await client.message(wire.SPEC["caps"])
        self.assertEqual(caps, wire.caps(self.store.server_identity, True))
        return client

    async def send(self, client, kind, payload=b""):
        await client.send(bytes((wire.SPEC[kind],)) + payload)

    async def ready(self, client, revision=0):
        await self.send(client, "ready", wire.READY.pack(bytes((client.number,)) * 16, revision,
                                                       bytes((client.number + 10,)) * 32, 0))

    async def advertise(self, client, have, want=0):
        await self.send(client, "offer", struct.pack("!HH", have, want))

    async def board(self, client, count):
        await self.send(client, "board_request")
        for _ in range(30):
            data = await client.message(wire.SPEC["board"])
            if data[1] == count:
                return data
        self.fail("Expected live board was not delivered")

    async def decision(self, client, state, prepared=None, applied=None):
        for _ in range(30):
            data = await client.message(wire.SPEC["decision"])
            if data[-3] == state and (prepared is None or data[-2] == prepared) and (applied is None or data[-1] == applied):
                return data
        self.fail("Expected durable decision was not delivered")

    async def offer(self):
        a, b = await self.connect(1), await self.connect(2)
        for client, chip in ((a, 1), (b, 2)):
            await self.ready(client)
            await self.advertise(client, chip, 3 - chip)
        board = await self.board(a, 2)
        self.assertEqual({wire.LISTING.unpack_from(board, 2 + index * wire.LISTING.size)[0]
                          for index in range(2)}, {a.identity, b.identity})
        await self.send(a, "request", struct.pack("!16sHH", b.identity, 1, 2))
        offered = await self.decision(a, OFFERED, 0)
        self.assertEqual(offered, await self.decision(b, OFFERED, 0))
        self.assertEqual((await self.board(a, 0))[1], 0)
        return a, b, offered

    async def commit(self):
        a, b, offered = await self.offer()
        transaction, digest = offered[33:49], hashlib.sha256(b"Cyberworld trade 1\0" + offered[1:191]).digest()
        await self.send(a, "prepare", transaction + digest)
        await self.decision(a, OFFERED, 1)
        await self.decision(b, OFFERED, 1)
        await self.send(b, "prepare", transaction + digest)
        committed = await self.decision(a, COMMITTED, 3)
        self.assertEqual(committed, await self.decision(b, COMMITTED, 3))
        return a, b, committed

    async def test_exact_terms_both_confirm_and_duplicate_messages_keep_one_commit(self):
        send = self.server.trades._send

        async def observed(session, data):
            if data[0] == wire.SPEC["decision"]:
                stored = self.store.status(data[33:49], session.identity)
                self.assertEqual(data, wire.decision_bytes(stored))
                if stored.state == COMMITTED:
                    self.assertEqual(self.store.db.execute("SELECT count(*) FROM spent").fetchone()[0], 2)
            return await send(session, data)

        self.server.trades._send = observed
        a, b, committed = await self.commit()
        transaction = committed[33:49]
        self.assertEqual(self.store.status(transaction, a.identity).state, COMMITTED)
        self.assertEqual(self.store.db.execute("SELECT count(*) FROM spent").fetchone()[0], 2)
        await self.send(a, "cancel", transaction)
        self.assertEqual(committed, await self.decision(a, COMMITTED))
        digest = hashlib.sha256(b"Cyberworld trade 1\0" + committed[1:191]).digest()
        await self.send(b, "prepare", transaction + digest)
        self.assertEqual(committed, await self.decision(b, COMMITTED))
        for client, mask in ((a, 1), (b, 3)):
            await self.send(client, "ack", transaction)
            await self.decision(client, COMMITTED, applied=mask)
        self.assertIsNone(self.store.pending(a.identity))
        self.assertIsNone(self.store.pending(b.identity))
        await self.advertise(a, 3)
        self.assertIn(b"checkpoint", await a.message(protocol.SPEC["notice"]))
        await self.ready(a, 1)
        await self.advertise(a, 3)
        await self.board(a, 1)
        await self.send(a, "ack", transaction)  # delayed old ACK cannot clear the new listing
        await self.decision(a, COMMITTED, applied=3)
        await self.board(a, 1)

    async def test_disconnect_after_one_prepare_aborts_and_stale_session_cannot_touch_rejoin(self):
        a, b, offered = await self.offer()
        transaction = offered[33:49]
        digest = hashlib.sha256(b"Cyberworld trade 1\0" + offered[1:191]).digest()
        await self.send(a, "prepare", transaction + digest)
        await self.decision(b, OFFERED, 1)
        old = self.server.trades.sessions[b.identity]
        await b.close()
        self.clients.remove(b)
        await self.decision(a, ABORTED)
        b = await self.connect(2)
        await self.server.trades.disconnect(old)
        self.assertIsNot(self.server.trades.sessions[b.identity], old)
        with self.assertRaisesRegex(TradeError, "current"):
            await self.server.trades.message(old, bytes((wire.SPEC["status"],)) + transaction)
        self.assertIsNone(self.store.pending(b.identity))

    async def test_restart_redelivers_commit_in_either_reconnect_order(self):
        for order in ((1, 2), (2, 1)):
            with self.subTest(order=order):
                a, b, committed = await self.commit()
                transaction, server_id = committed[33:49], self.store.server_identity
                await self.stop()
                await self.start()
                self.assertEqual(self.store.server_identity, server_id)
                for number in order:
                    client = await self.connect(number)
                    self.assertEqual(committed, await self.decision(client, COMMITTED))
                for client in self.clients:
                    await self.send(client, "ack", transaction)
                    await self.decision(client, COMMITTED, applied=1 if client.number == 1 and order[0] == 1 else
                                        2 if client.number == 2 and order[0] == 2 else 3)
                await self.stop()
                # The next subcase is an independent installed service.
                for path in self.root.glob("trades.sqlite3*"):
                    path.unlink()
                await self.start()

    async def test_channels_and_authenticated_participant_guard_private_decisions(self):
        a, b, offered = await self.offer()
        outsider = await self.connect(3, channel=2)
        await self.board(outsider, 0)
        await self.send(outsider, "status", offered[33:49])
        self.assertIn(b"participant", await outsider.message(protocol.SPEC["notice"]))
        await a.send(struct.pack("!BH", protocol.SPEC["channel"], 2))
        self.assertIn(b"pending", await a.message(protocol.SPEC["notice"]))
        await self.send(a, "prepare", offered[33:49] + b"x" * 32)
        self.assertIn(b"immutable", await a.message(protocol.SPEC["notice"]))
        self.assertEqual(self.store.status(offered[33:49], b.identity).prepared, 0)
        await self.send(a, "cancel", offered[33:49])
        await self.decision(b, ABORTED)
        await a.send(struct.pack("!BH", protocol.SPEC["channel"], 2))
        await a.message(protocol.SPEC["welcome"])
        await self.advertise(a, 1)
        await self.board(outsider, 1)
        await self.board(b, 0)

    async def test_cancelled_disconnect_finishes_durable_abort_before_releasing_session(self):
        a, b, offered = await self.offer()
        entered, release = threading.Event(), threading.Event()
        cancel = self.store.cancel

        def delayed(*args):
            entered.set()
            if not release.wait(5):
                raise TimeoutError("Cleanup test did not release database")
            return cancel(*args)

        self.store.cancel = delayed
        cleanup = asyncio.create_task(self.server.trades.disconnect(self.server.trades.sessions[a.identity]))
        try:
            self.assertTrue(await asyncio.to_thread(entered.wait, 2))
            cleanup.cancel()
            await asyncio.sleep(0)
            cleanup.cancel()
            await asyncio.sleep(0)
            self.assertFalse(cleanup.done())
        finally:
            release.set()
            result = await asyncio.gather(cleanup, return_exceptions=True)
        self.assertIsInstance(result[0], asyncio.CancelledError)
        await self.decision(b, ABORTED)
        self.assertIsNone(self.store.pending(a.identity))
        self.assertNotIn(a.identity, self.server.trades.lobby.players)

    async def test_lost_commit_delivery_is_recovered_without_cancelling_other_side(self):
        a, b, offered = await self.offer()
        transaction = offered[33:49]
        digest = hashlib.sha256(b"Cyberworld trade 1\0" + offered[1:191]).digest()
        send = self.server.trades._send

        async def dropped(session, data):
            if session.identity == a.identity and data[0] == wire.SPEC["decision"] and data[-3] == COMMITTED:
                session.writer.close()
                return False
            return await send(session, data)

        self.server.trades._send = dropped
        await self.send(a, "prepare", transaction + digest)
        await self.decision(b, OFFERED, 1)
        await self.send(b, "prepare", transaction + digest)
        committed = await self.decision(b, COMMITTED, 3)

        async def departed():
            while any(player.identity == a.identity for player in self.server.players.values()):
                await asyncio.sleep(0.001)

        await asyncio.wait_for(departed(), 2)
        self.server.trades._send = send
        rejoined = await self.connect(1)
        self.assertEqual(committed, await self.decision(rejoined, COMMITTED, 3))
        self.assertEqual(self.store.status(transaction, a.identity).state, COMMITTED)

    async def test_malformed_command_closes_sender_and_aborts_uncommitted_offer(self):
        a, b, offered = await self.offer()
        await self.send(a, "prepare", offered[33:49] + b"short")
        self.assertIn(b"Malformed", await a.message(protocol.SPEC["error"]))
        await self.decision(b, ABORTED)
        self.assertIsNone(self.store.pending(b.identity))

    async def test_slow_board_reader_does_not_delay_delivery_to_fast_peer(self):
        a, b = await self.connect(1), await self.connect(2)
        await self.ready(a)
        await self.advertise(a, 1)
        await self.board(b, 1)
        entered, release, delivered = asyncio.Event(), asyncio.Event(), asyncio.Event()
        send = self.server.trades._send

        async def backpressure(session, data):
            if data[0] == wire.SPEC["board"] and session.identity == a.identity:
                entered.set()
                await release.wait()
            result = await send(session, data)
            if data[0] == wire.SPEC["board"] and session.identity == b.identity:
                delivered.set()
            return result

        self.server.trades._send = backpressure
        update = asyncio.create_task(self.server.trades.message(self.server.trades.sessions[a.identity],
                                     bytes((wire.SPEC["offer"],)) + struct.pack("!HH", 3, 0)))
        try:
            await asyncio.wait_for(entered.wait(), 2)
            await asyncio.wait_for(delivered.wait(), 2)
            self.assertFalse(release.is_set())
        finally:
            release.set()
            await update
        self.server.trades._send = send

    async def test_unmodified_presence_keeps_trade_extension_disabled(self):
        server = Server()
        client = Client()
        try:
            port = await server.start("127.0.0.1", 0)
            await client.open(port, "Legacy")
            self.assertFalse(server.health()["rules"]["trading"])
            await self.send(client, "board_request")
            self.assertIn(b"Unknown", await client.message(protocol.SPEC["error"]))
        finally:
            await client.close()
            await server.close()

    async def test_legacy_client_on_trade_server_gets_only_presence(self):
        legacy = Client()
        self.clients.append(legacy)
        welcome = await legacy.open(self.port, "OldClient", key=b"o" * 16)
        self.assertEqual(legacy.protocol, protocol.SPEC["subprotocol"])
        player_id = struct.unpack("!I", welcome[5:9])[0]
        self.assertNotIn(player_id, self.server.trade_sessions)
        self.assertEqual((await legacy.receive())[0], protocol.SPEC["snapshot"])
        await self.send(legacy, "board_request")
        self.assertIn(b"Unknown", await legacy.message(protocol.SPEC["error"]))

    async def test_new_client_offer_remains_compatible_with_presence_only_server(self):
        server = Server()
        client = Client()
        try:
            port = await server.start("127.0.0.1", 0)
            await client.open(port, "Compatible", protocols=wire.SUBPROTOCOL + "," + protocol.SPEC["subprotocol"])
            self.assertEqual(client.protocol, protocol.SPEC["subprotocol"])
            self.assertEqual((await client.receive())[0], protocol.SPEC["snapshot"])
            self.assertFalse(server.trade_sessions)
        finally:
            await client.close()
            await server.close()

    async def test_disabling_new_trades_keeps_committed_recovery_available(self):
        _, _, committed = await self.commit()
        terms, transaction = committed[1:191], committed[33:49]
        await self.stop()
        await self.start()
        self.server.trades.enabled = False
        recovered = Client()
        self.clients.append(recovered)
        await recovered.open(self.port, "Navi1", key=b"\1" * 16, protocols=wire.SUBPROTOCOL)
        caps = await recovered.message(wire.SPEC["caps"])
        self.assertEqual(caps, wire.caps(self.store.server_identity, False))
        self.assertEqual((await recovered.message(wire.SPEC["decision"]))[1:191], terms)
        await self.send(recovered, "ready", wire.READY.pack(b"1" * 16, 0, b"A" * 32, 0))
        self.assertIn(b"disabled", await recovered.message(protocol.SPEC["notice"]))
        await self.send(recovered, "status", transaction)
        self.assertEqual((await recovered.message(wire.SPEC["decision"]))[-3:], bytes((COMMITTED, 3, 0)))
        await self.send(recovered, "ack", transaction)
        self.assertEqual((await recovered.message(wire.SPEC["decision"]))[-1], 1)
        self.assertFalse(self.server.health()["rules"]["trading"])


if __name__ == "__main__":
    unittest.main()
