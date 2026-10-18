# Stage a real project checkpoint: acceptance before implementation

Status: reviewed scope; original-code precondition reached and not met.
The ordinary project installs the chip registry even without BN5. Its
quantity lookup and relocated records need the separately reviewed
[registry equivalence unit](trade-project-registry-goal.md) before this
checkpoint acceptance can proceed. No candidate extension is accepted. The
[pristine routine-level stage](trade-stage-evidence.md) is accepted within
its envelope. Actual project saves are still refused. Nothing in this
unit enables live offers, sends PREPARED or consumes a remote decision.
The original full native cable Trade remains INCONCLUSIVE.

## Scope and identity

Start with two ordinary BN6-only runs created by the project's normal
NEW GAME and checkpoint path. Use different seeds and independently
created run identities. No All *, BN5, Souls or new format migration.
Pin the build, configuration, own Gregar ROM, mGBA library, complete
source `savedata` trees and their receipts before comparing a candidate.
The fixture may acquire spare native chips through the already verified
original Give routine, then use the normal checkpoint writer. Record
that fixture operation; never patch Pack/folders in the saved file to
manufacture expected trade effects.

`emu_init` loads a 16 MiB padded image before installers run. mGBA's
saved `romCrc32` therefore cannot be assumed to identify all later patches.
For each case, reconstruct the core by the normal run loader from the
same source checkpoint. Record the actual loaded image size, core CRC,
effective in-memory ROM SHA-256, installer configuration and layer seed.
Two fresh reconstructions must agree at the same observation boundary.
The CRC expected by staging must be derived from this verified context;
a caller-supplied arbitrary CRC or a blanket acceptance of patched CRCs
is not sufficient.

Verify the actual Take/Give entrypoints, transitive executed instructions
and native code lookup used by the oracle against the pristine target.
Collect and compare instruction addresses/bytes as the calls execute;
identify and pin required ROM data reads (including the selected chips'
code records). If an installer changes the executed routine or required
code data, stop this scope as INCONCLUSIVE and review that effect
separately. The full effective-ROM digest supplements these semantic
checks; a digest alone does not prove original behavior.

## Original and unchanged controls

The source checkpoint must come from `save_checkpoint`, with its queue
drained and no write failure. Capture its complete file bytes and
prepared inventory. Observe normal CONTINUE/reconstruction before any
Square transition; a saved file's bytes and the reconstructed live state
are distinct records and must not be silently interchanged.

Run the original Take-one/Give-one sequence in two independent restored
project contexts. Reach a controlled map-call boundary with a bounded
observer and prove the complete inventory at that boundary still equals
the immutable source checkpoint. Advancing to the boundary must never
supply expected protected-state bytes to the candidate. Record full
states before/after each original call, arguments, CPU/stack, return path
and all memory differences as in the accepted pristine observer. Missing
return, changed initial inventory, unexplained call effects or differing
controls are INCONCLUSIVE.

Keep two independent unchanged-source load/save controls using the same
normal reconstruction and serialization path as the candidate reload.
Pin their complete outputs before candidate implementation. Review every
source-to-control difference with the responsible normal loader or mGBA
operation; no blanket RAM, patched-byte, audio or engine-state mask.
The existing audio normalization explains only its already observed
fields; it does not justify an additional project difference by analogy.

Require a reciprocal pair using two different native code slots and at
least one incoming chip previously absent from both Pack and Library.
Choose real spare counts after all equipped folders are accounted for.
A second pair or a controlled replay must exercise last-spare removal and
incoming quantity 98 if reachable through the original acquisition path.
Unreached boundary fixtures remain explicit and cannot count as passes.

## Candidate and normal resume

Before extending the candidate, commit and independently review the
project-specific comparator and immutable context contract. Preserve the
pristine comparator, pins and controls as an independent earlier unit.
The adapter may return owned serialized bytes; production staging must
not instantiate another full core on memory-constrained platforms.

The candidate file must equal its exact source file outside only the
individual inventory fields observed in the project original controls.
The complete Pack, ownership table, Library and acquisition-order result
must equal the original routines. Every folder, HP, NaviCust, buster
level, Regular/Tag, power, position, map/RNG, CPU, flash and extdata byte
otherwise remains unchanged. Do not serialize a running Square core or
copy a fresh core's normalized bytes into the immutable checkpoint.

Fresh candidate reloads must match the two unchanged-source controls on
every protected serialized byte and the original result on all inventory
regions. Use normal CONTINUE, then open the original folder editor and
compare every exact-code spare count. Also observe that normal resume
preserves the received Library flag/ownership. The existing
`library_to_game` only adds profile flags; `library_from_game` merges
observed run flags into permanent progress at ordinary save/end paths.
Verify those actual paths for this received chip before deciding whether
a separate `profile.sav` replacement is needed. Any new profile mutation
needs its own narrow reviewed acceptance, never an incidental side effect
of checkpoint staging.

Source ROM and checkpoint directories are read-only inputs. Record their
complete hashes before/after. A separately captured live Square sentinel
must remain unchanged. No frame may run in the candidate operation.

## Isolated durable handoff

After the state comparison passes, hand each owned `run.state` result to
`online_trade_journal_prepare` in a separate disposable copy of its real
source directory. The terms bind the actual two identities, run IDs,
revisions and full source checkpoint hashes; use the existing terms hash
and authority boundary. Do not use the pristine fixture's dummy terms.

Before PREPARED, original playable files must remain unchanged and the
journal must certify the complete staged tree and revision-plus-one
receipt. Drive a matching decision only through a clearly labeled local
test fixture; this is not proof of a live server exchange. Recover after
PREPARED and after a recorded COMMITTED install interruption. Require
exactly one applied reciprocal swap, matching prepared bytes/receipt,
then repeated recovery/decision/ack without another quantity change.
All nonreplacement checkpoint files stay byte-identical except the
explicit applied receipt/transfer metadata owned by the journal.

This handoff reuses the separate journal's persistence/fault suite; it
adds real project state and resume evidence, not a replacement for every
storage fault test. Live protocol, confirmation UI and platform durability
remain separate integration requirements.

## Required refusal and fault checks

Retain all twelve reached pristine-stage faults in the project context.
Also reject changed effective ROM/configuration, wrong source checkpoint
or run revision, stale prepared inventory after a normal checkpoint,
unreviewed native instruction/code-record change, an arbitrary accepted
CRC, source reload controls missing/differing/from another source, and
profile/receipt corruption. Where a real project checkpoint cannot reach
an input case, use separately labeled synthetic refusal tests.

Keep full native/control/source pins fixed in the proof driver. Missing
evidence or an unreached original/native-editor path is INCONCLUSIVE;
a reached mismatch fails. Only after this reviewed unit passes may the
stage adapter become eligible for live journal integration.
