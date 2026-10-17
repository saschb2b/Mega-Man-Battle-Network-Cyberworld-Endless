# Staged checkpoint chip swap: acceptance before implementation

Status: proposed acceptance for review. No swap routine is implemented by
this unit. The accepted inventory reader is a prerequisite, not its oracle.

## Pinned target and first unit

Use the player's pristine Gregar USA ROM, SHA-1
`89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6`, and the repository's mGBA 0.10.5
static build. Record SHA-256 for the library, probe, comparator, source
state and every staged result. Start from the same local map-side boot
state used by the [inventory proof](trade-inventory-evidence.md), or pin
and identify a new state before comparison. Input ROM/state files remain
read-only. All dumps, saves and generated states remain local.

First scope: two ordinary BN6 runs, no All * helper and no BN5 records.
Each player sends one spare chip in its exact native code and receives
one different chip. Prepare a private emulator state only; do not send
PREPARED, replace `savedata`, touch the Square's core, or claim that a
journal is durable. The caller later supplies returned state bytes to
the journal's staged-replacement API.

## Independent oracle

Prefer the original **Network → Chip Trade over a local cable pair**.
The existing `tools/online_link_bn6.c` interactive harness can drive two
pristine cores with shared cable scheduling. Keep its accepted Practice
script and comparator unchanged; add a separate trade observer/driver.

The controlled fixture gives each side spare native chips using local
fixture data, with different IDs/codes and the incoming chip initially
absent. Names, graphics and chip metadata come from the player's ROM.
Record all fixture writes. The original trade then determines the
resulting Pack counts, ownership-check bytes and Library flags. A
formula that subtracts/adds quantities is not an expected-output oracle.

The locator leads are the original Comm menu and chip give/take paths,
using the sources in [SOURCES.md](../SOURCES.md). The Network trade path
and its Library/ownership effects are **unverified** for this new unit.
Locate their original completion state and observed persistent fields
before claiming success, and record each new location in ROM_DATA.md.
Neither a screenshot that resembles completion nor a count changed by
the fixture itself proves a completed original exchange.

Budget the initial native Network experiment to three distinct attempts
(at most ten minutes of investigation each), retaining each observation.
If the original exchange cannot complete, record the first unreached
state and stop this approach as INCONCLUSIVE. A separately reviewed
fallback may call the original chip acquisition/removal routines on
controlled copies, but it must observe those routines' actual effects;
do not silently replace the Network oracle with our patch algorithm.

## Required observations

1. **Native control:** two fresh local-cable exchanges from identical
   states and input scripts agree on all observed quantities, ownership
   bytes, Library flags, complete equipped folders and protected loadout
   fields. Both original peers reach a verified successful completion
   and return from the trade. A stalled or failed peer is INCONCLUSIVE.
2. **Basic swap:** the prepared state for each side matches the original
   result for every Pack ID/code, the complete ownership-mark table and
   complete Library flag range. The exact outgoing and incoming code
   quantities differ by one; other counts agree with the original and
   remain unchanged. Equip copies cannot fund the exchange.
3. **Protected state:** all folders, HP, NaviCust board/grid, buster
   levels, Regular/Tag chips, Crosses, Souls, Beast, zenny/BugFrags,
   position, map and RNG remain equal to the source checkpoint in the
   candidate. The candidate does not advance an emulated frame.
4. **Whole-state boundary:** compare the candidate's complete serialized
   mGBA core state before/after. Allowed differences are only the exact
   Pack/ownership/Library fields observed in the native oracle, mapped
   through named BN6 fields and mGBA's pinned state layout. No broad RAM,
   menu, scratch, CPU or checksum mask is permitted. If serialization
   introduces a changing field, locate it and review its narrow reason
   before adding a rule. Native UI/cable execution advances CPU/menu
   state; that transient execution is not reproduced by staging.
5. **Reload:** write the candidate's returned bytes to a local staged
   state, restore them in a fresh private core, and re-read every field
   above. Open the original folder editor after reload and compare its
   displayed Pack quantities with the original trade's editor. Source
   ROM/state hashes and the live Square's state must remain unchanged.
6. **Failure closure:** stale prepared input, no spare, unsupported code,
   missing recipient capability, mismatched All * rules, full incoming
   count, malformed pointers, and identical outgoing/incoming entries
   must produce no staged state and no change to the source/core. These
   refusal checks use independent synthetic cases; they do not establish
   original Network trade semantics.

No decision about the profile's persistent Library is implicit here.
The original's in-core Library result is established first. If staging
also needs `profile.sav` to record the received chip, that project-specific
update gets a separate acceptance and remains an optional replacement
owned by the journal, not an incidental write by the emulator routine.

## Comparator and faults before the patch

Land the comparator and required native fixtures in a reviewed commit
before the patch routine. It checks target/fixture identity, original
completion on both peers and deterministic controls before judging a
candidate. Missing/truncated evidence, a missing native menu or an
unreached mutation path is INCONCLUSIVE, never PASS.

Deliberately break one behavior at a time: wrong outgoing code slot,
wrong incoming code slot, two copies removed, a copy created without a
give, omit an ownership or Library change that the native oracle actually
made, alter an equipped entry, alter one otherwise protected state byte,
return a stale saved-state buffer, and touch the live Square core. Every
reached fault must FAIL. If an original effect is unchanged for a chosen
chip, choose a new-chip fixture that reaches it rather than pretending
its omission was detected.

The acceptance is complete only after the ordinary BN6 pair passes,
every required fault fails, every used location has a re-verification
note, and ROM-free comparator/refusal tests join CI. BN5 records and All *
normalization are later running-project cases with their own reviewed
scope; ordinary native BN6 does not prove either behavior. Filesystem
flush, journal recovery, replay prevention and server decisions remain
the journal/coordinator's separate acceptance.
