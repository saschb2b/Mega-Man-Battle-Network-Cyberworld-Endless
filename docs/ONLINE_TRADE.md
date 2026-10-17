# Trade Central: inventory and transaction contract

Phase 2 of [epic #120](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/120)
adds a 1:1 swap between the Packs of two carried checkpoints. This
document separates the implemented local eligibility check from the
transaction and integration work required before players can trade.
The Square currently changes no run or save.

## Design

The dialectic remains **the run's integrity against the community's
generosity**. Trading is the social loop beside the run. Its economy is
a trader: it redistributes existing chips, creates no chips or currency,
and keeps each player's total chip count unchanged. A visible opportunity
cost and an honest confirmation are the relevant game-design patterns:
both chip cards and codes stay in view while each player confirms.

Only spare copies can leave the Pack. No folder, HP, NaviCust, upgrades,
Souls or checkpoint location changes. Owning several copies of one name
is permitted; the project's one-chip-per-name rule means that shared
BN5/BN6 names use the registry's canonical BN6 ID, not that a player may
own only one copy. The ordinary folder editor still enforces folder
limits after the run resumes.

All * is part of the run's code rules. As a conservative first contract,
All * runs trade only with other All * runs. Ordinary runs use each
chip's original legal codes; All * runs use `*` in Pack slot zero.
Neither side silently changes the received code. This prevents a helper
from supplying a code the recipient's run cannot use. This restriction
is a trade policy, not a claim about BN6's original Network menu.

One-for-one preserves quantity, but does not by itself prove equal
power. No class or rarity restriction has been imposed by this building
block; the epic's balance question remains a player-testing decision.

## Implemented: immutable local eligibility

`src/online/online_trade_inventory.[ch]` accepts a copied `OnlineLoadout`,
local chip metadata and a complete list of equipped entries. It performs
no emulator access, networking or writes. Its two public functions are:

- `online_trade_spares`: the exact canonical chip/code count less every
  equipped copy. Invalid, unavailable or over-limit quantities offer zero.
- `online_trade_check`: validates one outgoing spare and one incoming
  chip, a real checkpoint, matching All * rules and room under the
  recipient's 99-copy limit. An identical pair is refused as a no-op.

Metadata comes from local `chips_record`, `chips_bn5`, `chips_sits_out`
and the `xreg_*` accessors, never from a server's claim. Its
`canonical_id` must equal the Pack ID. The caller must exclude blank
records, placeholders, pseudo-chips, Program Advances and chips which
cannot play. A BN5-only record also requires the recipient's BN5 ROM.
The four codes supplied are the unmodified record's codes. A malformed
duplicate code is refused rather than choosing one of its count slots.

The complete equipped list is an explicit prerequisite. `OnlineLoadout`
currently snapshots only the first folder. The pure validator accepts
30 to 90 entries so an engine reader can supply every folder whose
copies the Pack counts; it subtracts matching copies across the whole
list. It cannot prove that a caller supplied all of them. Before wiring
the trade UI, verify the additional folders' ownership and borrowed
ExtraFolder behavior from the original game's code and in a running
checkpoint, then capture the correct complete list. No new engine reader
or assumption about those folders is introduced here.

That original-game verification follows the repository's
[reverse-engineering skill](../.claude/skills/reverse-engineering/SKILL.md):
pin Gregar by hash, land the observable acceptance and mutations before
the reader, then compare the pristine game's counted copies with the
reader. The synthetic tests below prove the pure contract, not those
original-game semantics.

Existing evidence: [ROM_DATA.md](ROM_DATA.md), “The second screen's shops
and traders”, records a real-game experiment in which the map's Pack
counts included the first folder's four Cannons, the editor removed
those copies, and closing it restored them. `director_folder.c` and
`second_read.c` follow that result. `xchips_registry.h` and
[MULTIROM.md](MULTIROM.md) define canonical names.
[META.md](META.md#all--a-fourth-helper-4-october-2026-issue-18) documents
the All * helper's code and Pack-slot changes. No ROM offset was added.

The ROM-free test uses invented metadata. It checks multiple equipped
folders, exact codes, underflow and overflow, BN5 absent/present,
unavailable records, aliases, malformed codes, All * peers, every u16
entry value, and byte-for-byte immutability of the input records:

```sh
make WERROR=1 build/host/test_online_trade_inventory
build/host/test_online_trade_inventory
```

## Implemented: durable server decisions

`server/cyberworld_server/trade_store.py` is a separately tested storage
component, not yet exposed through the presence protocol. SQLite FULL
synchronous transactions hold both sides of each immutable offer. Each
participant is identified by its server-specific public identity, opaque
run identity, monotonic trade revision, checkpoint SHA-256 and one numeric
chip/code entry. The confirmation
digest binds both sides, the persistent server identity and the randomly
generated transaction ID. Opening existing history with a changed server
identity is refused; a new database cannot silently answer for an old one.

Only the second distinct participant's valid PREPARED can commit. The
commit and both spent-revision records become durable together, before
any notification can be sent. Repeated preparations, cancellations and
acknowledgements return the same decision. A committed decision cannot
be aborted. Each identity and run remain reserved until that participant
acknowledges installation. Even after both acknowledgements, the spent
run revision and any older revision cannot fund another trade, including
a copy imported with a different checkpoint hash or guest identity. This
prevents replay by a conforming client, not deliberate forgery of run
identities or revisions by a modified offline client.

Every read/decision/write operation takes SQLite's writer reservation
before reading, so two database connections cannot race a stale cancel
against a commit. On exclusive service startup, recovery aborts open
offers and preserves committed decisions. It first checks decisions,
reservations and spent revisions for consistency; damaged history and
unsupported database schemas fail closed without releasing a run.
Unknown transaction IDs are
errors, never evidence that the other side did not receive a chip.
The decision history has no automatic deletion policy yet. It must be
backed up with the server identity key before enabling live trades.

The ROM-free suite exercises duplicate messages, mismatched terms,
unauthorized access, reservation conflicts, cancellation at each
precommit boundary, acknowledgement, copied runs and stale revision reuse,
corrupted history, incompatible schemas, injected storage failure and process death after either preparation. This proves
the coordinator's storage behavior; it does not prove a client wrote a
prepared checkpoint, or replace the client recovery tests below.

## Implemented: live offer coordination

`server/cyberworld_server/trade_lobby.py` holds at most one live have/want
listing per present player. Listings are scoped to their channel; both
selected entries must still match when a request reserves the pair.
Posting another checkpoint or moving channels withdraws the old listing.
All * flags are part of the immutable terms and must match. BN5
availability remains a local registry check; a server cannot prove the
contents of a player's ROM or Pack.

After reservation, both listings disappear until the decision resolves.
A disconnect asks the durable store to abort an offer that has not
committed; it cannot undo a commit. Reconnection returns an unresolved
committed decision. Each client must recapture its installed checkpoint
revision before advertising again. Replaying an old acknowledgement does
not erase that newer readiness. Database work runs off the presence event
loop. The module has no public socket entry point yet.

The confirmation digest is SHA-256 over `Cyberworld trade 1` followed by
a zero byte, the 32-byte server identity, the 16-byte transaction ID, then
each side in left/right order: public identity (16), run identity (16),
revision (4, network byte order), checkpoint SHA-256 (32), exact chip/code
entry (2, network byte order), and rules (1: bit 0 BN5, bit 1 All *).
State, prepared and applied masks are not terms and cannot change this
digest. This format is the common contract for the client journal and
upcoming wire codec. Persisted schemas are validated exactly; these
components have not yet been enabled in a released trade service.

## Required before a swap can modify a checkpoint

Eligibility is not a commit. The full transaction needs one immutable
offer and one durable decision, identified by a cryptographically random
transaction ID. Bind it to the server identity, both guest identities,
both run identities and checkpoint revisions, and the exact two
canonical chip/code pairs. Each participant has at most one unresolved
trade. Changing either offer clears both confirmations.

1. Both clients validate against their carried checkpoint. Each prepares
   a journal and a complete staged checkpoint containing the exact swap,
   flushes them, reads them back, then sends PREPARED for that offer.
   Preparing never replaces the playable checkpoint.
2. The server commits only after both confirmations and both durable
   preparations. Its single database transaction stores both
   participants and the immutable result before sending COMMITTED.
   Duplicate requests and retries return the same result. No timeout
   can turn a committed transaction into an abort.
3. Each client resolves the recorded decision and installs its staged
   checkpoint with an applied receipt in the same recoverable directory
   transaction. Only after durable installation does it acknowledge.
   The server retains the result until both acknowledgements and retains
   enough receipt history to refuse replay.
4. On startup, recovery runs before any CONTINUE, save writer, import,
   export or automatic copy. An unresolved PREPARED checkpoint remains
   reserved until the server says committed or aborted. An old server
   connection closing is not proof of an abort. After commit, a failed
   install must retry its staged data, never silently return the old
   chip to play.

“Both swapped, never one” means one irreversible server decision and
eventual installation on both devices. Two independent devices cannot
replace files simultaneously during a network partition. The UI must
show a pending recovery until the decision can be resolved; it must not
claim a completed swap while the local checkpoint is still old.

The transaction cannot use `save_write_blob` success as acknowledgement:
that may only mean queued, and `save_queue_wait` does not report an I/O
failure. A normal `save_checkpoint` independently queues many files.
Use a separate durable write/read-back path and stage the state with its
receipt, so a crash cannot leave a new Pack with an old receipt or vice
versa. The existing import's `savedata.old` is user-visible Undo and
must not become the trade transaction's rollback slot.

Directory and file durability must be tested per platform. The current
browser `platform_persist` schedules IndexedDB persistence without an
acknowledged completion; PREPARED and APPLIED must await a successful
persistence callback. The 3DS's generic rename removes an existing
destination first, and its fsync helper has no physical-storage
acknowledgement. A platform cannot advertise safe trading until its
transaction path and recovery have been exercised on its real storage.

The portable checkpoint needs its opaque run identity, monotonic trade
revision and applied receipts included in the `.cwsave` allowlist and
manifest validation. Guest secrets remain device-local. Import and Undo
must not replay an already settled receipt into a different checkpoint;
an old imported revision must be reconciled before another trade. Run
seeds are reproducible and are not unique transaction identities.

Protection against network retries and crashes is achievable. Preventing
an owner from deliberately cloning an offline save or modifying a client
is not proved by this protocol: the server never receives or verifies a
ROM, emulator state or full Pack. Documentation must keep that boundary
explicit.

## Integration and fault tests still required

Keep local inventory validation, the durable local journal, the UI and
text, and the server coordinator in separate modules. Only the frame
thread reads or changes emulator state; the network worker exchanges
copied records. Apply a swap to a restored checkpoint, not Square RAM.
Also restore the received chip's ownership mark and Library state using
the existing named fields and accessors. Do not change layer generation,
folder construction, `Run` layout or `RUN_MAGIC` for this feature.

The transaction suite must interrupt before and after every journal
write, flush, rename, confirmation, commit, notification, installation
and acknowledgement. Repeat with duplicate and reordered messages,
server restart, both reconnect orders, disk-full/write failure,
malformed receipts, unknown transaction IDs, stale checkpoint revisions,
import/Undo, changed server identities and delayed browser persistence.
Every recovered state must be old with no committed decision, pending
and unplayable, or exactly one applied swap for a committed decision.
An actual two-game proof must verify unchanged folders, HP, NaviCust,
powers, position and checkpoint metadata, and exactly the two intended
Pack count changes after reloading each saved state.
