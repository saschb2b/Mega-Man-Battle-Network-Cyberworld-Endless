# Staged chip swap: original-routine oracle acceptance

Status: reviewed fallback acceptance. No patch routine is
implemented. The [native peer exchange](trade-stage-link-attempts.md)
remains INCONCLUSIVE and is a separate integration requirement.

## Scope and authority

Retain the target, file pins, immutable input, ordinary BN6-only scope,
whole candidate-state boundary, reload, refusal cases and mutation list
from [trade-stage-goal.md](trade-stage-goal.md). Change only the inventory
oracle: execute the original chip removal and acquisition routines in a
pristine private core, with the same chip/code/count arguments used by
the located original Trade completion call path. Label every resulting
claim **routine-level**, never a completed cable exchange.

The existing `BN6_GIVE_CHIPS` location is a locator lead for acquisition.
Locate its removal counterpart and the Trade completion caller in the
pinned Gregar ROM through the sources in SOURCES.md, verify their actual
call targets and argument decoding, and record every new used location
and re-verification step in ROM_DATA.md. Matching source or instructions
alone does not establish their side effects.

## Harness before candidate

1. Restore the pinned map-side state in two fresh private cores. Apply
   identical, recorded fixtures: valid equipped folders, spare outgoing
   native chip, incoming chip absent, deliberately unset incoming Library
   and invalid incoming ownership mark. Choose legal native codes from
   the ROM, including a fixture using a nonzero code slot. Pin each
   resulting source state. The map-side Pack includes equipped copies;
   calling removal is allowed only after the independent inventory
   reader has verified a spare.
2. Invoke each original routine through the emulated CPU. Preserve and
   record the entry PC, arguments, stack, return address and toolkit
   context; stop only at its verified return PC. An instruction/time
   limit, unexpected return, unknown memory write or missing callee is
   INCONCLUSIVE. Do not implement expected effects in the harness.
3. Observe removal and acquisition separately, then their take-one /
   give-one sequence. Record complete serialized state and flash before
   and after each call, all Pack bytes (including ordering stamps), the
   entire ownership table and Library flag range. Keep CPU/stack execution
   effects visible in the evidence; do not fold them into the candidate's
   allowed-difference set.
4. Require both independently restored oracle executions to agree on all
   inventory and protected fields and their return observations. A no-op
   entry/return control characterizes the calling harness. The fixtures
   must reach actual count, ownership and Library changes; an unchanged
   field cannot satisfy its mutation requirement.
5. The original Trade caller also restores folder accounting and saves.
   Do not invoke that wrapper from a map-state fixture: its menu-specific
   preconditions are not established. If the routine-level results leave
   a required inventory effect unexplained, stop this unit and review a
   separate caller-context experiment. Native save/menu/cable completion
   is explicitly outside this oracle's proven scope.

## Candidate comparison

Before implementing a candidate, land and independently review its
comparator. It requires two returned original controls, exact target and
fixture pins, readonly ROM/state hashes before/after, and a complete
observation manifest. Missing evidence is INCONCLUSIVE.

The candidate takes an immutable checkpoint buffer and its prepared
inventory, validated exchange terms and side. It returns owned bytes or
a refusal; it has no access to the live Square core and performs no file
installation. Its complete serialized state must equal the input except
for the **individual inventory fields actually changed by the original
calls**. The comparator enumerates those fields by named BN6 accessors
and the pinned mGBA serialized-state layout; it does not permit the whole
Pack, ownership table, Library block, CPU, stack or flash as a blanket
mask. Every other byte, including extdata, stays unchanged.

The reviewed addition after observing two original controls is the exact
incoming code's 16-bit acquisition stamp and the shared 16-bit next-stamp
counter (`BN6_PACK_ORDER`, `BN6_CHIP_ORDER_COUNTER`). Give copies that
counter into the incoming slot and then decrements it. These are individual
inventory fields, not an allowance for a larger Pack or metadata range.
Require separate omission mutations for each. A counter-zero fixture
must observe the original wrap behavior; a last-spare and incoming-98
fixture exercise the quantity boundaries within this ordinary BN6 scope.

Compare the complete Pack, ownership and Library result against the
original calls, not a subtraction formula. Verify all equipped entries
and protected gameplay fields against the source. Restore returned bytes
in a fresh private core and compare them with the separately reviewed
[unchanged-source reload controls](trade-stage-reload.md), without any
ignored audio bytes. Re-read inventory, then enter the original folder
editor and compare its spare counts with the original-routine result's
editor. The source files and a separately snapshotted live-core sentinel
must remain unchanged.

The original-goal mutations remain required, including wrong code slot,
extra removal/creation, missing observed ownership/Library effect, folder
or unrelated-byte corruption and stale return bytes. A live-core write
fault may be exercised only in the test harness against the sentinel;
the production candidate API must not receive a live core pointer.

First scope is ordinary native BN6 codes. All *, BN5 records, profile
Library persistence, journal durability and completed native peer trade
retain their separate acceptance requirements. A successful result here
must state that limitation in the evidence and canonical dashboard.
