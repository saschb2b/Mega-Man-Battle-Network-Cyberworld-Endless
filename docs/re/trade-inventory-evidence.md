# Native trade inventory evidence

Verified 18 October 2026. This accepts the immutable inventory reader only;
checkpoint mutation, journal installation and live Trade Central remain separate units.

## Pins

| Input/build | Hash |
| --- | --- |
| Gregar USA ROM, SHA-1 | `89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6` |
| Local map-side boot state, SHA-256 | `4b921c9cf48c22085605714797373114de206a32b8bba821606dc348f73f95b6` |
| mGBA 0.10.5 static library, SHA-256 | `5b02195c77b74ffa6622952706025bc7e5a77bd5d14d9306e0bbc84ab789f0cd` |
| Compiled native probe, SHA-256 | `dd703f92f2b6c08f97ac81fd83ec2dc4351cad079f046acfc2c776e33173e454` |
| Reviewed comparator, SHA-256 | `39ef4f95f6b5685b4ff2f861bbb63c8bdb57245d65b101c0f7a8ad2b83e9bc57` |

The library is the repository’s `cyberworld-build` image (`docker/mgba.sh`).
The state is a local pristine `boot-3.state` generated before the run’s
custom map/chip installation; only its hash is recorded. Both input files
were mounted read-only and their before/after hashes agree. No saves are
opened or written by the probe. Per-run manifests also hash the fixture
source. ROMs, states, memory traces and generated mutation sources stay
under ignored local paths.

## Original oracle and results

The native editor supplies expected quantities after its own folder
accounting. The comparator performs no subtraction. For each case, two
independent native runs must agree before the captured reader is judged.
Every canonical chip ID/code pair is compared (512 × 27 bytes), including
zero values. Complete EWRAM and IWRAM are compared at identical sampled
frames, with no masks. Capture occurs between frames; the next frame
and each menu-input boundary are sampled. This is not a claim of tracing
every CPU instruction or every frame between those boundaries.

| Scenario | Frames | Full RAM samples per lane | Spare copies | Verdict |
| --- | ---: | ---: | ---: | --- |
| first-folder | 251 | 14 | 6 | PASS |
| second-folder | 251 | 14 | 6 | PASS |
| extra-folder | 251 | 14 | 6 | PASS |
| active-second | 571 | 38 | 6 | PASS |
| active-extra | 581 | 40 | 6 | PASS |
| count-boundaries | 251 | 14 | 102 | PASS |

The first folder is read from the player’s local state. Distinct second
and Extra fixtures copy it and replace its Cannon A entries with native
records 2 and 3 in their first legal codes. All three contain shared chip
copies. The original Extra slot is distinguished by its EQUIP-only menu;
the harness uses native EQUIP to select each active slot and native EDIT
to open the first folder. Names are copied from runtime memory solely to
initialize a valid local fixture. Three spare quantities (1, 2 and 3) and
a 99-total boundary are injected; every other equipped chip has zero
spares. The oracle removes all folder copies, including borrowed Extra.

The prepared object and its eligibility output remain byte-identical
after changing a live Pack quantity and before restoring that fixture
byte. The capture also refuses malformed pointers/counts/active slots,
unsupported equipped codes, editor/battle state, and non-checkpoints in
the independent ROM-free suite. BN5 availability and All * code rules
are tested there using invented records; this native control carries
neither BN5 records nor All * modifications.

## Required faults

| Deliberate break | Required case | Comparator result |
| --- | --- | --- |
| equipped-as-spares | first-folder | FAIL: counts: chip 1, code 0: expected 1, got 3 |
| omit-second-folder | second-folder | FAIL: counts: chip 1, code 1: expected 2, got 4 |
| omit-extra-folder | extra-folder | FAIL: counts: chip 1, code 1: expected 2, got 4 |
| collapse-codes | first-folder | FAIL: counts: chip 1, code 1: expected 2, got 0 |
| write-observed-ram | first-folder | FAIL: observer RAM: byte 557055: expected 0, got 1 |
| live-backed-snapshot | first-folder | FAIL: prepared snapshot lifetime: byte 72: expected 3, got 2 |

The first five faults compile altered reader source in `.build`; the
sixth deliberately refreshes the prepared copy after the live quantity
change, modelling a live-backed snapshot. Its prepared bytes and retained
counts diverge. No mutation switch exists in production code.

## Reproduce and remaining scope

```sh
python3 tools/online_trade_proof.py --state PATH/boot-3.state --mutations
```

The ROM defaults to `~/.cache/mmbn-ref/roms/bn6-gregar-us.gba`; use `--rom`
to choose the player’s verified copy. A boot state that cannot reach the
declared native menus is INCONCLUSIVE. Evidence and verdict JSON are
written under `.build/trade-proof`. The acceptance and comparator were
reviewed separately before this implementation (`41d06482`, `5ead5451`,
`88a3b544`).

An early Extra fixture lacked its runtime name and the original menu
never became usable. That was inconclusive, corrected in the harness,
and not counted as a pass. The current six specified native paths all
ran. Transplanted BN5 chips in a running checkpoint, actual All * native
editor behavior, storage durability, receipt replay and real trading are
not established by this reader proof.
