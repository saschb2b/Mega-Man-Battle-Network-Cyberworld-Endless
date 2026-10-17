"""Trade decisions survive interruption without split or repeated commits."""

from concurrent.futures import ThreadPoolExecutor
from dataclasses import replace
from pathlib import Path
import sqlite3
import subprocess
import sys
import tempfile
from threading import Event
import unittest
from unittest.mock import patch

from test_server import ROOT
from cyberworld_server.trade_store import ABORTED, COMMITTED, OFFERED, Side, TradeError, TradeStore


class TradeStoreTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.path = Path(self.temporary.name)
        self.store = TradeStore(self.path)
        self.addCleanup(lambda: self.store.close())
        self.left = Side(b"a" * 16, b"A" * 32, 1, b"1" * 16, 0)
        self.right = Side(b"b" * 16, b"B" * 32, 2, b"2" * 16, 0)

    def create(self):
        return self.store.create(self.left, self.right)

    def commit(self, trade):
        self.store.prepare(trade.transaction, self.left.identity, trade.digest)
        return self.store.prepare(trade.transaction, self.right.identity, trade.digest)

    def restart(self):
        self.store.close()
        self.store = TradeStore(self.path)
        self.store.recover()

    def test_both_prepare_exact_terms_once_before_commit(self):
        trade = self.create()
        self.assertEqual(trade.state, OFFERED)
        self.assertEqual(len(trade.digest), 32)
        first = self.store.prepare(trade.transaction, self.left.identity, trade.digest)
        self.assertEqual((first.state, first.prepared), (OFFERED, 1))
        self.assertEqual(first, self.store.prepare(trade.transaction, self.left.identity, trade.digest))
        with self.assertRaises(TradeError):
            self.store.prepare(trade.transaction, self.right.identity, bytes(32))
        final = self.store.prepare(trade.transaction, self.right.identity, trade.digest)
        self.assertEqual((final.state, final.prepared, final.applied), (COMMITTED, 3, 0))
        self.assertEqual(final, self.store.prepare(trade.transaction, self.left.identity, trade.digest))
        self.assertEqual(final, self.store.cancel(trade.transaction, self.right.identity))
        self.assertEqual(final.digest, trade.digest)

    def test_cancel_at_each_precommit_boundary_never_commits_later(self):
        for preparations in (0, 1):
            with self.subTest(preparations=preparations):
                trade = self.create()
                if preparations:
                    self.store.prepare(trade.transaction, self.left.identity, trade.digest)
                self.store.cancel(trade.transaction, self.right.identity)
                for side in (self.left, self.right):
                    self.assertEqual(self.store.prepare(trade.transaction, side.identity, trade.digest).state, ABORTED)
                    self.assertIsNone(self.store.pending(side.identity))
                with self.assertRaises(TradeError):
                    self.store.acknowledge(trade.transaction, self.left.identity)

    def test_each_ack_releases_only_its_owner_and_old_checkpoint_stays_spent(self):
        trade = self.commit(self.create())
        first = self.store.acknowledge(trade.transaction, self.left.identity)
        self.assertEqual(first.applied, 1)
        self.assertEqual(first, self.store.acknowledge(trade.transaction, self.left.identity))
        self.assertIsNone(self.store.pending(self.left.identity))
        self.assertEqual(self.store.pending(self.right.identity).transaction, trade.transaction)
        self.store.acknowledge(trade.transaction, self.right.identity)
        self.restart()
        self.assertEqual(self.store.status(trade.transaction, self.left.identity).applied, 3)
        with self.assertRaisesRegex(TradeError, "already traded"):
            self.create()
        new_left = replace(self.left, checkpoint=b"C" * 32, chip=3, revision=1)
        new_right = replace(self.right, checkpoint=b"D" * 32, chip=4, revision=1)
        self.assertEqual(self.store.create(new_left, new_right).state, OFFERED)

    def test_restart_resolves_every_uncommitted_offer_but_preserves_commits(self):
        for preparations in (0, 1, 2):
            with self.subTest(preparations=preparations):
                trade = self.create()
                for side in (self.left, self.right)[:preparations]:
                    self.store.prepare(trade.transaction, side.identity, trade.digest)
                self.restart()
                expected = COMMITTED if preparations == 2 else ABORTED
                for side in (self.left, self.right):
                    self.assertEqual(self.store.status(trade.transaction, side.identity).state, expected)
                if expected == COMMITTED:
                    self.assertIsNotNone(self.store.pending(self.left.identity))

    def test_changed_checkpoint_or_guest_cannot_reuse_an_old_run_revision(self):
        self.left = replace(self.left, revision=7)
        trade = self.commit(self.create())
        for side in (self.left, self.right):
            self.store.acknowledge(trade.transaction, side.identity)
        fresh_right = replace(self.right, checkpoint=b"D" * 32, revision=1)
        for revision in (0, 6, 7):
            imported = replace(self.left, identity=b"x" * 16, checkpoint=b"X" * 32, revision=revision)
            with self.subTest(revision=revision), self.assertRaisesRegex(TradeError, "already traded"):
                self.store.create(imported, fresh_right)
        next_revision = replace(self.left, checkpoint=b"C" * 32, revision=8)
        self.assertEqual(self.store.create(next_revision, fresh_right).state, OFFERED)

    def test_copied_run_is_reserved_across_guest_identities(self):
        trade = self.create()
        copied = replace(self.left, identity=b"x" * 16, checkpoint=b"X" * 32, revision=1)
        third = Side(b"c" * 16, b"C" * 32, 3, b"3" * 16, 0)
        with self.assertRaisesRegex(TradeError, "another device"):
            self.store.create(copied, third)
        self.store.cancel(trade.transaction, self.left.identity)
        self.assertEqual(self.store.create(copied, third).state, OFFERED)

    def test_confirmation_binds_run_and_revision_as_well_as_checkpoint(self):
        trade = self.create()
        for side in (replace(self.left, run_id=b"x" * 16), replace(self.left, revision=1)):
            with self.subTest(side=side):
                self.assertNotEqual(trade.digest, replace(trade, left=side).digest)
        self.assertNotEqual(trade.digest, replace(trade, server=b"s" * 32).digest)

    def test_server_identity_persists_and_changed_identity_cannot_read_history(self):
        trade = self.create()
        self.restart()
        self.assertEqual(self.store.server_identity, trade.server)
        with self.assertRaisesRegex(TradeError, "server identity"):
            TradeStore(self.path, b"s" * 32)
        self.assertEqual(self.store.status(trade.transaction, self.left.identity).digest, trade.digest)
        fresh = TradeStore(self.path / "explicit", b"s" * 32)
        try:
            self.assertEqual(fresh.server_identity, b"s" * 32)
        finally:
            fresh.close()

    def test_unsupported_or_damaged_schema_is_preserved_and_refused(self):
        mutations = ("PRAGMA user_version=2", "PRAGMA user_version=0",
                     "ALTER TABLE spent ADD COLUMN ignored INTEGER", "DROP TABLE reservations")
        for index, mutation in enumerate(mutations):
            directory = self.path / str(index)
            original = TradeStore(directory)
            original.create(self.left, self.right)
            original.db.execute(mutation)
            before = list(original.db.iterdump())
            original.close()
            with self.subTest(mutation=mutation), self.assertRaisesRegex(TradeError, "schema"):
                TradeStore(directory)
            with sqlite3.connect(directory / "trades.sqlite3") as check:
                self.assertEqual(list(check.iterdump()), before)

    def test_recovery_refuses_damaged_history_before_releasing_any_run(self):
        mutations = ("DELETE FROM reservations", "DELETE FROM spent",
                     "UPDATE trades SET prepared=0", "UPDATE trades SET state=9",
                     "UPDATE trades SET applied=3", "UPDATE trades SET left_chip=0")
        for index, mutation in enumerate(mutations):
            store = TradeStore(self.path / str(index))
            try:
                trade = store.create(self.left, self.right)
                for side in (self.left, self.right):
                    store.prepare(trade.transaction, side.identity, trade.digest)
                store.db.execute(mutation)
                before = list(store.db.iterdump())
                with self.subTest(mutation=mutation), self.assertRaises(TradeError):
                    store.recover()
                self.assertEqual(list(store.db.iterdump()), before)
            finally:
                store.close()

    def test_no_participant_or_unknown_id_can_be_mistaken_for_abort(self):
        trade = self.create()
        for call in (self.store.status, self.store.cancel, self.store.acknowledge):
            with self.assertRaises(TradeError):
                call(trade.transaction, b"c" * 16)
            with self.assertRaisesRegex(TradeError, "Unknown"):
                call(b"x" * 16, self.left.identity)
        with self.assertRaises(TradeError):
            self.store.prepare(trade.transaction, b"c" * 16, trade.digest)
        self.assertEqual(self.store.status(trade.transaction, self.left.identity), trade)

    def test_invalid_claims_do_not_create_a_record(self):
        bad = [replace(self.left, identity=value) for value in (bytes(16), b"a" * 15, "a" * 16)]
        bad += [replace(self.left, checkpoint=value) for value in (bytes(32), b"A" * 31)]
        bad += [replace(self.left, run_id=value) for value in (bytes(16), b"1" * 15)]
        bad += [replace(self.left, revision=value) for value in (-1, 0xffffffff, True, "0")]
        bad += [replace(self.left, chip=chip) for chip in (-1, 0, 512, 27 << 9 | 1, 65536, True)]
        for side in bad:
            with self.subTest(side=side), self.assertRaises(TradeError):
                self.store.create(side, self.right)
        self.assertIsNone(self.store.pending(self.right.identity))
        with self.assertRaises(TradeError):
            self.store.create(self.left, replace(self.right, identity=self.left.identity))
        with self.assertRaises(TradeError):
            self.store.create(self.left, replace(self.right, chip=self.left.chip))

    def test_overlapping_offer_and_commit_cancel_race_has_one_decision(self):
        trade = self.create()
        with self.assertRaisesRegex(TradeError, "unresolved"):
            self.store.create(self.left, Side(b"c" * 16, b"C" * 32, 3, b"3" * 16, 0))
        self.store.prepare(trade.transaction, self.left.identity, trade.digest)
        with ThreadPoolExecutor(max_workers=2) as pool:
            futures = [pool.submit(self.store.prepare, trade.transaction, self.right.identity, trade.digest),
                       pool.submit(self.store.cancel, trade.transaction, self.left.identity)]
            decisions = [future.result() for future in futures]
        self.assertEqual(decisions[0].state, decisions[1].state)
        self.assertIn(decisions[0].state, (COMMITTED, ABORTED))

    def locked_read(self, other, first, second):
        """Hold the first read and make the other connection fail fast.

        Events establish the interleaving; SQLite's zero busy timeout proves
        the write lock already exists at the read, without scheduling sleeps.
        """
        read_done, resume = Event(), Event()
        original = other._read
        first_read = True

        def paused(*args, **kwargs):
            nonlocal first_read
            decision = original(*args, **kwargs)
            if first_read:
                first_read = False
                read_done.set()
                if not resume.wait(5):
                    raise RuntimeError("Test did not release the decision read")
            return decision

        self.store.db.execute("PRAGMA busy_timeout=0")
        with patch.object(other, "_read", paused), ThreadPoolExecutor(max_workers=1) as pool:
            running = pool.submit(first)
            try:
                self.assertTrue(read_done.wait(5), "First writer did not reach its read")
                with self.assertRaisesRegex(sqlite3.OperationalError, "locked"):
                    second()
            finally:
                resume.set()
            return running.result(timeout=5)

    def test_second_connection_cannot_commit_behind_cancel_read(self):
        trade = self.create()
        self.store.prepare(trade.transaction, self.left.identity, trade.digest)
        other = TradeStore(self.path)
        self.addCleanup(other.close)
        prepare = lambda: self.store.prepare(trade.transaction, self.right.identity, trade.digest)
        cancelled = self.locked_read(other, lambda: other.cancel(trade.transaction, self.left.identity), prepare)
        self.assertEqual(cancelled.state, ABORTED)
        self.assertEqual(prepare().state, ABORTED)
        self.assertEqual(self.store.db.execute("SELECT count(*) FROM spent").fetchone()[0], 0)

    def test_second_connection_cannot_overwrite_another_prepare_bit(self):
        trade = self.create()
        other = TradeStore(self.path)
        self.addCleanup(other.close)
        prepare = lambda: self.store.prepare(trade.transaction, self.right.identity, trade.digest)
        first = self.locked_read(other, lambda: other.prepare(trade.transaction, self.left.identity, trade.digest), prepare)
        self.assertEqual((first.state, first.prepared), (OFFERED, 1))
        final = prepare()
        self.assertEqual((final.state, final.prepared), (COMMITTED, 3))
        self.assertEqual(other.status(trade.transaction, self.left.identity), final)
        self.assertEqual(self.store.db.execute("SELECT count(*) FROM spent").fetchone()[0], 2)

    def test_failed_commit_rolls_back_decision_and_both_spent_entries(self):
        trade = self.create()
        self.store.prepare(trade.transaction, self.left.identity, trade.digest)
        self.store.db.execute("""CREATE TRIGGER fail_commit BEFORE INSERT ON spent
                               BEGIN SELECT RAISE(ABORT, 'injected storage failure'); END""")
        with self.assertRaises(sqlite3.DatabaseError):
            self.store.prepare(trade.transaction, self.right.identity, trade.digest)
        current = self.store.status(trade.transaction, self.left.identity)
        self.assertEqual((current.state, current.prepared), (OFFERED, 1))
        self.assertEqual(self.store.db.execute("SELECT count(*) FROM spent").fetchone()[0], 0)
        self.store.db.execute("DROP TRIGGER fail_commit")
        self.assertEqual(self.commit(trade).state, COMMITTED)

    def test_process_death_after_each_prepare_keeps_the_durable_decision(self):
        script = """
import os, sys
sys.path.insert(0, sys.argv[1])
from cyberworld_server.trade_store import Side, TradeStore
s=TradeStore(sys.argv[2])
t=s.create(Side(b'a'*16,b'A'*32,1,b'1'*16,0),Side(b'b'*16,b'B'*32,2,b'2'*16,0))
print(t.transaction.hex(), flush=True)
s.prepare(t.transaction,b'a'*16,t.digest)
if sys.argv[3]=='2': s.prepare(t.transaction,b'b'*16,t.digest)
os._exit(0)
"""
        for count in (1, 2):
            directory = self.path / str(count)
            result = subprocess.run([sys.executable, "-c", script, str(ROOT / "server"), str(directory), str(count)],
                                    check=True, text=True, capture_output=True)
            transaction = bytes.fromhex(result.stdout.strip())
            store = TradeStore(directory)
            try:
                store.recover()
                self.assertEqual(store.status(transaction, self.left.identity).state, COMMITTED if count == 2 else ABORTED)
            finally:
                store.close()


if __name__ == "__main__":
    unittest.main()
