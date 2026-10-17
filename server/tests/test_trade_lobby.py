"""Live offers cannot bypass the durable trade decision or carried revision."""

import asyncio
from dataclasses import replace
from pathlib import Path
import tempfile
import threading
import unittest

from test_server import ROOT
from cyberworld_server.trade_lobby import Carried, TradeLobby
from cyberworld_server.trade_store import ABORTED, COMMITTED, OFFERED, TradeError, TradeStore


class LobbyTests(unittest.IsolatedAsyncioTestCase):
    async def asyncSetUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.store = TradeStore(Path(self.temp.name))
        self.addCleanup(self.store.close)
        self.lobby = TradeLobby(self.store, 3)
        self.a, self.b, self.c = (bytes((value,)) * 16 for value in (1, 2, 3))
        self.ca = Carried(b"a" * 16, 0, b"A" * 32, False, False)
        self.cb = Carried(b"b" * 16, 0, b"B" * 32, True, False)
        for identity, name in ((self.a, "One"), (self.b, "Two"), (self.c, "Three")):
            await self.lobby.connect(identity, name, 1 if identity != self.c else 2)
        await self.lobby.ready(self.a, self.ca)
        await self.lobby.ready(self.b, self.cb)

    async def offer(self):
        await self.lobby.advertise(self.a, 1, 2)
        await self.lobby.advertise(self.b, 2, 1)
        return await self.lobby.request(self.a, self.b, 1, 2)

    async def commit(self):
        trade = await self.offer()
        await self.lobby.prepare(self.a, trade.transaction, trade.digest)
        return await self.lobby.prepare(self.b, trade.transaction, trade.digest)

    async def test_both_exact_offers_leave_board_until_resolved(self):
        trade = await self.offer()
        self.assertEqual(trade.state, OFFERED)
        self.assertEqual(await self.lobby.board(self.a), ())
        for call in (self.lobby.advertise(self.a, 1), self.lobby.ready(self.a, self.ca), self.lobby.move(self.a, 2)):
            with self.assertRaisesRegex(TradeError, "pending"):
                await call
        await self.lobby.cancel(self.b, trade.transaction)
        await self.lobby.advertise(self.a, 1)
        self.assertEqual((await self.lobby.board(self.b))[0].have, 1)
        self.assertEqual(await self.lobby.board(self.c), ())

    async def test_stale_board_selection_and_cross_channel_requests_are_refused(self):
        await self.lobby.advertise(self.a, 1, 2)
        await self.lobby.advertise(self.b, 2)
        await self.lobby.advertise(self.b, 3)
        with self.assertRaisesRegex(TradeError, "changed"):
            await self.lobby.request(self.a, self.b, 1, 2)
        await self.lobby.move(self.b, 2)
        self.assertEqual(await self.lobby.board(self.b), ())
        with self.assertRaisesRegex(TradeError, "channel"):
            await self.lobby.request(self.a, self.b, 1, 3)
        with self.assertRaises(TradeError):
            await self.lobby.request(self.a, self.a, 1, 1)

    async def test_disconnect_before_commit_records_abort_after_commit_preserves_it(self):
        trade = await self.offer()
        await self.lobby.prepare(self.a, trade.transaction, trade.digest)
        stopped = await self.lobby.disconnect(self.b)
        self.assertEqual(stopped.state, ABORTED)
        self.assertEqual((await self.lobby.status(self.a, trade.transaction)).state, ABORTED)
        self.assertIsNone(await self.lobby.connect(self.b, "Two", 1))
        await self.lobby.ready(self.b, self.cb)
        final = await self.commit()
        self.assertEqual((await self.lobby.disconnect(self.b)).state, COMMITTED)
        restored = await self.lobby.connect(self.b, "Two", 1)
        self.assertEqual(restored, final)
        self.assertEqual((await self.lobby.cancel(self.b, final.transaction)).state, COMMITTED)

    async def test_each_installed_revision_must_be_recaptured_and_old_ack_cannot_clear_it(self):
        final = await self.commit()
        for identity in (self.a, self.b):
            await self.lobby.acknowledge(identity, final.transaction)
            with self.assertRaisesRegex(TradeError, "checkpoint"):
                await self.lobby.advertise(identity, 3)
        new = replace(self.ca, checkpoint=b"N" * 32, revision=1)
        await self.lobby.ready(self.a, new)
        await self.lobby.advertise(self.a, 3)
        await self.lobby.acknowledge(self.a, final.transaction)
        self.assertEqual((await self.lobby.board(self.a))[0].carried, new)

    async def test_concurrent_requests_have_one_reserved_pair(self):
        await self.lobby.advertise(self.a, 1)
        await self.lobby.advertise(self.b, 2)
        results = await asyncio.gather(self.lobby.request(self.a, self.b, 1, 2),
                                       self.lobby.request(self.b, self.a, 2, 1), return_exceptions=True)
        self.assertEqual(sum(isinstance(value, TradeError) for value in results), 1)
        self.assertEqual(self.store.db.execute("SELECT count(*) FROM trades").fetchone()[0], 1)

    async def test_cancelled_create_settles_before_disconnect_or_other_callback(self):
        await self.lobby.advertise(self.a, 1)
        await self.lobby.advertise(self.b, 2)
        entered, release, finished = (threading.Event() for _ in range(3))
        create, written = self.store.create, []

        def delayed_create(*args):
            entered.set()
            try:
                if not release.wait(5):
                    raise TimeoutError("Test did not release create")
                written.append(create(*args))
                return written[-1]
            finally:
                finished.set()

        self.store.create = delayed_create
        request = asyncio.create_task(self.lobby.request(self.a, self.b, 1, 2))
        others = []
        try:
            self.assertTrue(await asyncio.to_thread(entered.wait, 2))
            request.cancel()
            await asyncio.sleep(0)
            request.cancel()  # A second transport/shutdown cancellation must not release the lock.
            others = [asyncio.create_task(self.lobby.board(self.b)),
                      asyncio.create_task(self.lobby.disconnect(self.a))]
            await asyncio.sleep(0)
            await asyncio.sleep(0)
            self.assertFalse(request.done())
            self.assertTrue(self.lobby.lock.locked())
            self.assertTrue(all(not task.done() for task in others))
        finally:
            release.set()
            outcomes = await asyncio.gather(request, *others, return_exceptions=True)
            self.assertTrue(await asyncio.to_thread(finished.wait, 2))
        self.assertIsInstance(outcomes[0], asyncio.CancelledError)
        self.assertEqual(outcomes[2].state, ABORTED)
        self.assertNotIn(self.a, self.lobby.players)
        self.assertEqual(self.store.status(written[0].transaction, self.b).state, ABORTED)
        self.assertIsNone(self.store.pending(self.a))
        self.assertIsNone(self.store.pending(self.b))

    async def test_all_star_and_malformed_claims_fail_before_reservation(self):
        star = replace(self.ca, all_star=True)
        await self.lobby.ready(self.a, star)
        with self.assertRaisesRegex(TradeError, "All \\*"):
            await self.lobby.advertise(self.a, 1)
        await self.lobby.advertise(self.a, 26 << 9 | 1)
        await self.lobby.advertise(self.b, 2)
        with self.assertRaisesRegex(TradeError, "different All"):
            await self.lobby.request(self.a, self.b, 26 << 9 | 1, 2)
        for have, want in ((0, 1), (True, 0), (-1, 0), (1, 1), (1, 512), (1, 65535)):
            with self.subTest(have=have, want=want), self.assertRaises(TradeError):
                await self.lobby.advertise(self.b, have, want)
        with self.assertRaises(TradeError):
            await self.lobby.ready(self.c, replace(self.ca, revision=-1))
        with self.assertRaisesRegex(TradeError, "checkpoint"):
            await self.lobby.advertise(self.c, 1)

    async def test_changing_checkpoint_withdraws_its_old_offer(self):
        await self.lobby.advertise(self.a, 1)
        await self.lobby.ready(self.a, replace(self.ca, checkpoint=b"N" * 32))
        self.assertFalse(await self.lobby.board(self.b))

    async def test_only_present_owner_can_resolve_history(self):
        trade = await self.offer()
        for action in (self.lobby.status, self.lobby.cancel, self.lobby.acknowledge):
            with self.assertRaises(TradeError):
                await action(self.c, trade.transaction)
        await self.lobby.disconnect(self.b)
        with self.assertRaises(TradeError):
            await self.lobby.status(self.b, trade.transaction)


if __name__ == "__main__":
    unittest.main()
