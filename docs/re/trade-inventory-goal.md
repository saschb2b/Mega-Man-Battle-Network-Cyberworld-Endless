# Trade inventory reader: acceptance before implementation

Status: acceptance recorded; original-game experiment and runtime reader
not yet implemented. The immutable validator's synthetic tests are a
separate, completed check and cannot pass this acceptance.

## Pinned target and goal

Target: the player's Mega Man Battle Network 6: Cybeast Gregar (USA)
cartridge image, SHA-1 `89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6`, checked
against `src/core/rom.c`. Harness: the repository's mGBA 0.10.5 Release
build from `docker/mgba.sh`; no ROM file is changed. A fresh boot state or
local state used to skip the introductory dialogue must be identified by
hash in the local report. ROMs, states, RAM dumps and disassembly remain
outside git.

For the **prepared trade inventory**, the runtime reader's spare counts
must match **the pristine game's own folder editor's Pack quantities**
over the scenarios below. Compare every canonical chip/code count after
the editor has finished removing folder-owned copies. There are no
allowed count differences. The reader itself must change no emulated
memory, RNG, save file or checkpoint metadata.

The original editor is the oracle. A matching implementation of our
own subtraction formula is not an oracle. Disassembly is a locator and
explanation; observed native behavior decides which folders count.

## Scenarios

| Spec | Controlled state/input | Required observation |
| --- | --- | --- |
| native-control | From the same local start, identical native input twice with no reader | All observed Pack, folder and menu-state traces agree by frame |
| first-folder | Initial first folder, with a spare in an existing code and a spare in another code | Reader's map-side spares equal the native editor's counts for each code |
| second-folder | A distinct second editable folder, including copies shared with the first | Determine whether its entries are included in the map's counts and removed by the editor; reader agrees exactly |
| extra-folder | Borrowed ExtraFolder populated with distinct IDs, then selected | Determine which quantities the native editor counts as owned; borrowed copies must never become tradeable |
| active-folder | Select every available folder in turn without changing its contents | Switching the active folder cannot create a tradeable owned copy |
| count-boundaries | Zero spares, one spare, several equipped copies of one code, count 99 | Exact nonnegative native spare count and correct receive-capacity decision |
| observer-control | Run identical inputs with the reader disabled and enabled | Full EWRAM and IWRAM agree at every sampled frame; file hashes unchanged |
| capture-lifetime | Change live Pack data after preparing a snapshot | The prepared copy and its eligibility results remain unchanged |

The harness may use named RAM fields to construct a controlled test
state, then let the original menu perform the observed transition. It
must record those fixture changes and refuse a state that never reaches
the intended menu. A missing folder or unavailable menu is INCONCLUSIVE,
not a reason to reduce the scenario to a one-folder assertion.

BN5 availability, canonical registry IDs and All * code normalization are
the project's own behavior. Test those separately against the running
project after the native folder accounting is proved; do not label a
synthetic rule check as original BN6 evidence. The first runtime reader
must refuse any unsupported folder state instead of guessing.

## Comparator and mutations

Keep the comparator and this acceptance in reviewed commits separate
from the reader. It compares all observed counts without masks and
reports the first different chip/code, expected count and actual count.
The local report records target and fixture hashes, inputs, frame
numbers, native menu states, verdicts and untested paths.

Deliberately break the reader in each of these ways; every relevant
scenario must fail: include equipped copies as spares; ignore the second
owned folder; subtract the borrowed ExtraFolder; collapse different codes
of one name; retain a pointer to live Pack memory; and write one observed
RAM byte during capture. A mutation with no scenario reaching its faulty
path is INCONCLUSIVE and leaves the acceptance incomplete.

## Non-goals and boundary

This unit prepares immutable local inventory only. It does not send the
Pack, offer a trade, alter a run, write a save or acknowledge PREPARED to
the server. No original menu routine or disassembly is redistributed.
The next unit is the durable local transaction described in
[ONLINE_TRADE.md](../ONLINE_TRADE.md), after this reader's oracle proof.
