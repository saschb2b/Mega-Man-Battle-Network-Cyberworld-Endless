"""Durable, immutable 1:1 trade decisions, independent of sockets and ROMs.

Only numeric chip entries and opaque identities/checkpoint digests are stored.
The client must durably reserve its checkpoint before calling prepare. This
module is not yet connected to the public presence protocol.
"""

from contextlib import contextmanager
from dataclasses import dataclass
import hashlib
from pathlib import Path
import secrets
import sqlite3
import struct
import threading

OFFERED, COMMITTED, ABORTED = 0, 1, 2

_SCHEMA = (
    """CREATE TABLE trades (
        id BLOB PRIMARY KEY, left_id BLOB NOT NULL, left_checkpoint BLOB NOT NULL,
        left_chip INTEGER NOT NULL, left_run BLOB NOT NULL, left_revision INTEGER NOT NULL,
        right_id BLOB NOT NULL, right_checkpoint BLOB NOT NULL,
        right_chip INTEGER NOT NULL, right_run BLOB NOT NULL, right_revision INTEGER NOT NULL,
        state INTEGER NOT NULL DEFAULT 0,
        prepared INTEGER NOT NULL DEFAULT 0, applied INTEGER NOT NULL DEFAULT 0
    )""",
    """CREATE TABLE reservations (
        identity BLOB PRIMARY KEY, run_id BLOB UNIQUE NOT NULL, trade_id BLOB NOT NULL
    )""",
    """CREATE TABLE spent (
        run_id BLOB NOT NULL, revision INTEGER NOT NULL, trade_id BLOB NOT NULL,
        PRIMARY KEY (run_id, revision)
    )""",
)


class TradeError(ValueError):
    """An invalid or unavailable trade; no progression has changed."""


@dataclass(frozen=True)
class Side:
    identity: bytes
    checkpoint: bytes
    chip: int
    run_id: bytes
    revision: int

    def validate(self):
        if not isinstance(self.identity, bytes) or len(self.identity) != 16 or not any(self.identity):
            raise TradeError("Invalid participant identity")
        if not isinstance(self.checkpoint, bytes) or len(self.checkpoint) != 32 or not any(self.checkpoint):
            raise TradeError("Invalid checkpoint digest")
        if not isinstance(self.run_id, bytes) or len(self.run_id) != 16 or not any(self.run_id):
            raise TradeError("Invalid run identity")
        if type(self.revision) is not int or not 0 <= self.revision < 0xffffffff:
            raise TradeError("Invalid trade revision")
        if type(self.chip) is not int or not 0 < self.chip <= 0xffff:
            raise TradeError("Invalid chip entry")
        if not 1 <= self.chip & 0x1ff <= 511 or self.chip >> 9 > 26:
            raise TradeError("Invalid chip ID or code")


@dataclass(frozen=True)
class Decision:
    transaction: bytes
    left: Side
    right: Side
    state: int
    prepared: int
    applied: int

    @property
    def digest(self):
        data = b"Cyberworld trade 1\0" + self.transaction
        for side in (self.left, self.right):
            data += side.identity + side.run_id + struct.pack("!I", side.revision)
            data += side.checkpoint + struct.pack("!H", side.chip)
        return hashlib.sha256(data).digest()

    def participant(self, identity):
        if identity == self.left.identity:
            return 1
        if identity == self.right.identity:
            return 2
        raise TradeError("Trade is not owned by this participant")


class TradeStore:
    """One database per server state directory. Call off the frame/event loop.

    FULL synchronous transactions make the decision durable before a caller
    can notify either participant. One unresolved trade reserves each identity.
    A committed run revision cannot be offered again, even after acknowledgement.
    No automatic history deletion: unknown is never interpreted as aborted.
    """

    def __init__(self, directory):
        root = Path(directory)
        root.mkdir(parents=True, exist_ok=True, mode=0o700)
        self.lock = threading.RLock()
        self.db = sqlite3.connect(root / "trades.sqlite3", check_same_thread=False, isolation_level=None)
        try:
            self.db.execute("PRAGMA journal_mode=WAL")
            self.db.execute("PRAGMA synchronous=FULL")
            self._initialize()
        except BaseException:
            self.db.close()
            raise

    def _initialize(self):
        with self._transaction():
            version = self.db.execute("PRAGMA user_version").fetchone()[0]
            schema = self.db.execute("SELECT sql FROM sqlite_master WHERE sql IS NOT NULL").fetchall()
            if version == 0 and not schema:
                for statement in _SCHEMA:
                    self.db.execute(statement)
                self.db.execute("PRAGMA user_version=1")
            elif version != 1 or {" ".join(row[0].split()) for row in schema} != {
                    " ".join(statement.split()) for statement in _SCHEMA}:
                raise TradeError("Unsupported trade history schema; keep the existing database")

    def close(self):
        with self.lock:
            self.db.close()

    @contextmanager
    def _transaction(self):
        # Acquire SQLite's writer reservation BEFORE reading a decision.
        # A Python lock alone cannot protect against a second connection.
        with self.lock:
            self.db.execute("BEGIN IMMEDIATE")
            try:
                yield
                self.db.commit()
            except BaseException:
                self.db.rollback()
                raise

    def _read(self, transaction, identity=None):
        if not isinstance(transaction, bytes) or len(transaction) != 16:
            raise TradeError("Invalid transaction ID")
        row = self.db.execute("SELECT * FROM trades WHERE id=?", (transaction,)).fetchone()
        if row is None:
            raise TradeError("Unknown transaction; restore the server's decision history")
        decision = Decision(row[0], Side(*row[1:6]), Side(*row[6:11]), *row[11:])
        decision.left.validate()
        decision.right.validate()
        if (type(decision.state) is not int or decision.state not in (OFFERED, COMMITTED, ABORTED) or
                type(decision.prepared) is not int or decision.prepared not in range(4) or
                type(decision.applied) is not int or decision.applied not in range(4) or
                (decision.state == COMMITTED and decision.prepared != 3) or
                (decision.state != COMMITTED and decision.prepared == 3) or
                (decision.state != COMMITTED and decision.applied) or
                decision.left.identity == decision.right.identity or decision.left.run_id == decision.right.run_id or
                decision.left.chip == decision.right.chip):
            raise TradeError("Damaged trade decision; restore the server's decision history")
        if identity is not None:
            decision.participant(identity)
        return decision

    def status(self, transaction, identity):
        with self.lock:
            return self._read(transaction, identity)

    def pending(self, identity):
        with self.lock:
            row = self.db.execute("SELECT trade_id FROM reservations WHERE identity=?", (identity,)).fetchone()
            return self._read(row[0], identity) if row else None

    def create(self, left, right):
        left.validate()
        right.validate()
        if left.identity == right.identity or left.run_id == right.run_id or left.chip == right.chip:
            raise TradeError("Trade needs two players and two different entries")
        with self._transaction():
            for side in (left, right):
                if self.pending(side.identity):
                    raise TradeError("Participant has an unresolved trade")
                if self.db.execute("SELECT 1 FROM reservations WHERE run_id=?", (side.run_id,)).fetchone():
                    raise TradeError("Run has an unresolved trade on another device")
                if self.db.execute("SELECT 1 FROM spent WHERE run_id=? AND revision>=?",
                                   (side.run_id, side.revision)).fetchone():
                    raise TradeError("Run revision was already traded; reconcile before offering again")
            transaction = secrets.token_bytes(16)
            self.db.execute("INSERT INTO trades VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, 0, 0, 0)",
                            (transaction, left.identity, left.checkpoint, left.chip, left.run_id, left.revision,
                             right.identity, right.checkpoint, right.chip, right.run_id, right.revision))
            self.db.executemany("INSERT INTO reservations VALUES (?, ?, ?)",
                                [(side.identity, side.run_id, transaction) for side in (left, right)])
            return self._read(transaction)

    def prepare(self, transaction, identity, digest):
        with self._transaction():
            decision = self._read(transaction, identity)
            if not isinstance(digest, bytes) or not secrets.compare_digest(digest, decision.digest):
                raise TradeError("Confirmation does not match the immutable trade")
            if decision.state != OFFERED:
                return decision
            prepared = decision.prepared | decision.participant(identity)
            state = COMMITTED if prepared == 3 else OFFERED
            self.db.execute("UPDATE trades SET prepared=?, state=? WHERE id=?", (prepared, state, transaction))
            if state == COMMITTED:
                self.db.executemany("INSERT INTO spent VALUES (?, ?, ?)",
                                    [(side.run_id, side.revision, transaction)
                                     for side in (decision.left, decision.right)])
            return self._read(transaction)

    def cancel(self, transaction, identity):
        with self._transaction():
            decision = self._read(transaction, identity)
            if decision.state == OFFERED:
                self.db.execute("UPDATE trades SET state=? WHERE id=?", (ABORTED, transaction))
                self.db.execute("DELETE FROM reservations WHERE trade_id=?", (transaction,))
            return self._read(transaction)

    def acknowledge(self, transaction, identity):
        with self._transaction():
            decision = self._read(transaction, identity)
            if decision.state != COMMITTED:
                raise TradeError("Only a committed trade can be acknowledged")
            applied = decision.applied | decision.participant(identity)
            self.db.execute("UPDATE trades SET applied=? WHERE id=?", (applied, transaction))
            self.db.execute("DELETE FROM reservations WHERE trade_id=? AND identity=?", (transaction, identity))
            return self._read(transaction)

    def recover(self):
        """On exclusive server startup, abort offers but retain every commit.

        This is an explicit lifecycle action, never run by a status reader or
        an admin opening the database. Clients query the persisted decision.
        """
        with self._transaction():
            self._check_history()
            self.db.execute("UPDATE trades SET state=? WHERE state=?", (ABORTED, OFFERED))
            self.db.execute("DELETE FROM reservations WHERE trade_id IN (SELECT id FROM trades WHERE state=?)",
                            (ABORTED,))

    def _check_history(self):
        """Refuse incomplete history instead of releasing a possibly spent run."""
        reservations, spent = set(), set()
        identities, runs, revisions = set(), set(), set()
        for (transaction,) in self.db.execute("SELECT id FROM trades").fetchall():
            decision = self._read(transaction)
            for bit, side in ((1, decision.left), (2, decision.right)):
                if decision.state == OFFERED or (decision.state == COMMITTED and not decision.applied & bit):
                    if side.identity in identities or side.run_id in runs:
                        raise TradeError("Conflicting trade reservations; restore the decision history")
                    identities.add(side.identity)
                    runs.add(side.run_id)
                    reservations.add((side.identity, side.run_id, transaction))
                if decision.state == COMMITTED:
                    revision = (side.run_id, side.revision)
                    if revision in revisions:
                        raise TradeError("Repeated spent revision; restore the decision history")
                    revisions.add(revision)
                    spent.add((*revision, transaction))
        if (reservations != set(self.db.execute("SELECT * FROM reservations")) or
                spent != set(self.db.execute("SELECT * FROM spent"))):
            raise TradeError("Incomplete trade history; restore the server's decision history")
