# Arena unit 3: native folder, chip use and controlled map entry

Status: proposed extension of unit 3 in [arena-next-goal.md](arena-next-goal.md).
The previous starter and local replay contracts stay unchanged. This unit
has no accepted installer or network result yet.

## Native control discovered first

On the pinned Gregar ROM and pre-director boot state from the accepted
frame/replay evidence, a private original-game probe calls `BN6_GIVE_CHIPS`
with chip ID 1, native code 0 (A), count 1. It resumes the interrupted
map caller and uses controller input to open the native folder editor.
Nine Down taps select zero-based slot 9, MiniBomb B. A, Right, a 40-frame
wait for the side slide, and A exchange it for the spare Cannon A. B
closes the editor, then B closes each remaining menu. The original map
now holds Cannon A in slot 9; its other 29 packed folder entries remain
unchanged. Native quantities report three Cannon A and two Cannon B.
The supplied ROM and source state are never written.

A discovery run from this map state reached Practice through the original
Comm menu. Its first hand contained Cannon A at slot 0. Selecting that
slot for player 0, choosing OK for both players, waiting 200 frames,
then pressing A for player 0 produced native hand ID 1 / power 40,
advanced the hand's consumed index from 0 to 1, and reduced the opposing
MegaMan from 100 HP to 60 HP. Subsequent charged buster releases completed
the fight. An earlier A pressed after only 120 waiting frames occurred
before the battle timer advanced: it did not consume the chip and is
retained as a negative lead, not counted as chip-use evidence.

These are discovery observations. Fresh repeated controls with the
reviewed observer and script must reproduce them before implementation.
The numeric discovery state is local only; its SHA-256 is
`4975e5f78e1fe3375ed8aad42c31ffd91eb58e0d6f86dcaad857d78751c7c5fe`.
Whole-game pictures can illustrate the control; they are not its oracle.

## Independent initialization and explicit RNG control

The oracle starts from the map fixture produced by the native acquisition
and editor path. The subject starts from the independently opened pinned
pre-director boot state, using its own cores and memory-only flash. Its
installer may copy only the explicitly supported `OnlineLoadout` fields;
it cannot load the oracle's map state, completed battle state, work RAM,
stack, timing or cable state. The native preparation code and the installer
are separate paths.

Before either path receives the first Comm input, require both cores to
be on the original map (`BN6_MODE_GAME`), with no battle entered.
The harness writes the same two fixed words, once only, to the named
`BN6_RNG_PRIMARY` and `BN6_RNG_SECONDARY` locations. The values are the
native discovery fixture's unmodified map observations: 2622147170 and
2129837214 respectively. Record the original values, resulting values,
mode, absolute core frame and source state identity on each side. No
later RNG following, reseeding or ignored RNG difference is allowed.
This explicit harness control removes menu-duration randomness from the
installer comparison. It does not establish arbitrary-run RNG continuity.
The native control must also pass when this same-value write is omitted.

The native input sequence selects the recorded Cannon A only on player 0,
then advances both cursors to OK. After the observed 200-frame wait it
uses that chip; six charged buster releases and a neutral tail must return
both players to the original result/Comm menu. Pin the complete sequence
and its lengths from the fresh repeated native control before the subject
is built. Do not retime the script to repair a subject divergence.

## Observation boundary and comparator

Keep the two original player cores distinct from the two network-worker
replicas. Observe each player core at every native video-frame boundary,
using mGBA's `videoFrameStarted` callback. The callback only queues numeric
observations; it never mutates the emulated game or writes the log. Write
queued records after stepping. Require a contiguous, duplicate-free frame
sequence for each core, and two controls that agree exactly at every
observed frame. Absolute source frame counts are recorded; battle comparison
uses each core's ordinal frame starting at the first ready Custom state.
Both paths must reach this same state within the pinned entry script;
missing or different Custom readiness is INCONCLUSIVE setup, not a pass.
There is no last-observation-per-timer reduction.

Compare all recorded fields from ready Custom through the same complete
result/menu boundary: battle phase/state/timer/result, side, both objects'
identity/alliance/HP/panels and `BN6_T1_CHIP`, Custom mode/cursor/hand size,
picked count and all pick slots, the entire 30-word battle deck, six hand
IDs/powers/bonuses and consumed index, both RNG words, and input. Do not
mask differences in those fields. Raw state, full flash and numeric cable
hashes still compare exactly between the independent worker replicas;
raw whole-machine equality between different map initialization paths is
not this subsystem contract. Every field uses an existing `bn6.h` name;
new interpretations require a ledger note in `ROM_DATA.md`.

Capture the complete production `online_snapshot_read` record before
entry. The supported profile must explicitly validate every field:
BN6-only native folder/pack, HP/base/max, buster stats, Regular/Tag,
limits, board/grid, Cross/Beast and run-side Souls. Unsupported features
must be rejected, never replaced by a starter default. The bounded fixture
exercises its nondefault folder and acquisition; unchanged default stats
are compared but do not establish every possible stat value or program.
Original source checkpoint and complete captured snapshot hashes must be
unchanged after normal completion, input corruption and disconnect.

The comparator must independently demonstrate actual selection, transfer
into the battle hand, chip consumption and 40-point HP loss before the
first buster input, and then a complete result on both cores. It must
reject missing/duplicate frames, wrong chip/code/slot/power/HP, missing
selection or consumption, changed RNG, changed source/checkpoint or
carried record, and fallback to the original starter folder. A no-chip
native control must fail the selected-chip predicate. Same script and
native logical comparison are required at 0/50/100/150 ms measured one-way
delivery, alongside complete replica and delivered-input evidence.

The comparator, mutation fixtures, native observer and its source/build
pins require separate review before the installer or delayed subject runs.
Use at most eight non-improving original-versus-subject attempts. Record
first divergence and unreached paths; do not broaden comparison masks or
reseed at a later point to obtain a pass. Production visits, ongoing run
save protection, prediction/rollback, migrated BN5 chips, Souls and device
qualification remain outside this bounded scenario.
