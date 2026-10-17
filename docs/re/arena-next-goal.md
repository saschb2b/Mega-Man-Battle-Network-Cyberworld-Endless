# Arena next units: cable snapshots, then a carried chip

Status: proposed acceptance for review; neither unit is implemented or
accepted. The production Arena remains disabled. The completed input-delay
unit and its exact limits are in [ONLINE_FRAME.md](../ONLINE_FRAME.md).

## Order and evidence

Take the **local cable snapshot** first. It reuses the already controlled
starter Practice fight, the same original ROM and the same controller
script. It adds one mechanism: restore and replay a complete linked pair.
The current frame worker already hashes the core, flash and numeric cable
state, but has no restore operation. mGBA's core state alone does not
serialize the external lockstep driver or its scheduled cable event. A
successful single-core state round trip therefore cannot establish this
unit. Prediction and delayed correction come after the snapshot proof.

Then take a **BN6-only carried loadout with one selected native chip**.
The present accepted script selects no chips, and its original starts
from a pre-director boot state. `OnlineLoadout` currently reads a run's
folder, pack and stats; it does not install them into the two Practice
cores. The copied loadout needs an independent native setup oracle before
its battle is meaningful. Souls, moved BN5 chips and patched code add
separate mechanisms and remain outside this next scenario.

## Unit 2: complete linked-pair snapshot

For the pinned starter Practice fight, restore a snapshot of both cores
and the local cable, replay the same controller inputs, and require the
same serialized core bytes, entire flash bytes, numeric cable state and
logical battle observations as an uninterrupted original local pair at
every replayed simulation step. No masks or allowed differences.

Use the ROM, boot-state, mGBA library and existing original script hashes
recorded in [arena-frame-evidence.json](arena-frame-evidence.json). A new
observing harness must first reproduce the old original's logical trace,
then repeat its own uninterrupted full trace twice. Its full trace must
include both core-state lengths and SHA-256, both flash lengths and
SHA-256, explicit cable fields and both logical game samples. The subject
uses the same mGBA library but adds snapshot/restore. Logs contain hashes
and numeric observations; raw states and flash remain local only.

| Spec | Required evidence |
| --- | --- |
| observer-control | Two uninterrupted original runs agree in full trace; old original logical acceptance still passes |
| snapshot-noop | Taking and retaining snapshots without restoring changes no subsequent observation |
| restore-immediate | Each captured component agrees immediately after restoration, including cable event scheduledness and relative due time |
| replay-custom | Snapshot while both players are in Custom; advance, restore and replay 1, 4 and 8 simulation steps; every replayed step agrees with the uninterrupted original |
| replay-active | Repeat those horizons during a held charge, immediately before its release, and after damage, with active phase and controller evidence proving each path |
| replay-cable | At least one snapshot is taken with a non-idle serial transfer and a scheduled cable event; its entire replay agrees |
| repeat-restore | Restore the same active snapshot twice and obtain the same full replay twice |
| source-preservation | Source ROM and state hashes remain unchanged; every emulated flash device is memory-only |

Choose snapshot positions from the uninterrupted trace before running the
subject. Record each position's frame, phase, input and cable evidence.
If a required state never occurs at the available boundary, report it as
unreached/INCONCLUSIVE and improve the observer boundary in a separately
reviewed change. Do not replace it with a convenient idle snapshot.

The snapshot must include the per-pair script/frame bookkeeping and
credits/waiting state, each core, each flash device, the cable's receive
words and transfer state, and each node's event state. Pointer links and
callbacks are reconstructed against the live pair, never deserialized as
addresses. The manifest pins the observer, comparator, script, fixture,
library and subject. Record snapshot bytes and copy/restore/replay cost,
but do not turn a desktop measurement into a device qualification.

Before implementation, land a separately reviewed comparator and mutation
fixtures. Reject malformed or empty traces, wrong component lengths,
missing replay steps, shifted input ranges, wrong source hashes and any
changed core/flash/cable digest or field. Target-bound mutations must
omit cable restoration, omit savedata restoration after a real flash
change, corrupt a credit/receive word and shift a scheduled cable event.
Each must fail at an exercised point; a mutation whose field never changes
does not count. The unchanged fixture must pass. If obtaining a real flash
change requires a new setup path, track that path as INCONCLUSIVE until
it is independently controlled; do not claim flash rollback from a
constant flash image.

This proves a local complete-state snapshot, not prediction, input-queue
rollback, out-of-order corrections, Internet play, audio-output continuity
or production recovery. Use at most eight non-improving original-versus-
subject attempts before reporting the first remaining divergence.

## Unit 3: carried native chip scenario

Prepare a BN6-only fixture locally through the native game's own folder
and stat paths, with a non-default legal folder and one known native
damage chip selected in Custom and then used. Record the actual chip ID,
code, slot, native record power and input script only after observing the
original; no assumed chip identity or damage is an acceptance constant.
Keep the complete native fixture state local and hash it. A separate
carried snapshot is read from this fixture using the production reader.

The oracle begins from that complete native fixture. The subject begins
from its independently initialized core and installs only the bounded
carried fields; it must not load the oracle's completed battle state.
The installer and original native fixture preparation must be separate
code paths. First prove the captured folder, counts, HP, buster stats,
limits and Regular/Tag metadata agree with the original before entering
Practice. Reject unsupported loadout fields explicitly; never silently
replace them with defaults or claim the full production run is carried.

| Spec | Required evidence |
| --- | --- |
| carried-control | Two native-fixture original runs agree, and installing a captured unchanged baseline preserves the observed loadout and battle behavior |
| selected-chip | Custom selects the recorded native chip; hand and object observations prove that chip is used and consumed, with its original timing, HP changes and outcome |
| carried-delays | The same scenario completes at 0/50/100/150 ms with exact original logical comparisons and complete replica equality |
| return-preservation | Original source checkpoint and the full recorded carried snapshot remain unchanged after success, corruption and disconnect |

Broaden the original observer to include `BN6_CUSTOM_PICKED`,
`BN6_CUSTOM_PICKS`, `BN6_BATTLE_HAND` with its named hand fields,
`BN6_T1_CHIP`, both RNG words and the existing battle observations.
Each new claim must use the existing ledger in `ROM_DATA.md`; a new
field or interpretation needs a re-verification note there. Compare
corresponding player cores, and record every observation at each core's
own frame transition so repeated active timers do not hide chip use.

The separately reviewed comparator must reject wrong chip/code, slot,
power, HP, selection count, missing consumption, altered RNG observations,
source-checkpoint mutation and a subject that falls back to the starter
folder. The original must actually select and use the chip before this
scenario can pass. A fixture resembling a carried run does not establish
the production online visit/save guard; that integration stays unreached.
