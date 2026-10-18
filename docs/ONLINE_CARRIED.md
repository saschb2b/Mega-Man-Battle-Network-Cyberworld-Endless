# Arena: native carried-chip control

The **original-game control passes**. Unit 3 as a whole remains
**INCONCLUSIVE**: there is no carried-loadout installer or delayed carried
battle yet, and the production Arena is disabled. This checkpoint follows
the accepted [starter input-delay](ONLINE_FRAME.md) and
[local snapshot/replay](ONLINE_REPLAY.md) experiments.

```sh
python3 tools/online_carried_control.py \
  --rom /path/to/bn6-gregar-us.gba \
  --state /path/to/boot-3.state
```

The command requires the pinned Gregar USA ROM, pre-director boot state,
and mGBA 0.10.5 Release library used by the preceding experiments. It
builds the three original-only tools with warnings as errors, verifies
44 source pins and the resolved build recipe, and keeps all native states,
complete captured loadouts and detailed traces under `.build/online-carried/`.
Those local files must not be committed or uploaded. The committed
[numeric evidence](re/arena-carried-evidence.json) contains hashes and
observations only.

Two fresh preparations invoke the original `BN6_GIVE_CHIPS` with ID 1,
code A and count 1, then exchange MiniBomb B in zero-based folder slot 9
for that Cannon A through the native folder editor. Both cores in both
preparations reproduce the same complete map-state hash. The production
`online_snapshot_read` captures 6,664 bytes; only the folder and pack differ
from the starter record. Board/grid remain empty and stats remain native
defaults. The `.loadout` file is the reader's complete local numeric record,
so a future independent installer need not load the oracle's core state.

| Original control | Result |
| --- | --- |
| Independently prepared native map fixtures | Four identical state hashes |
| Repeated native fights and same-value RNG write omitted | Identical complete core/flash/cable trace |
| Individual native core-frame observations | 2,442 / 2,443, with no timer deduplication |
| Original existing runner against new observer | All 1,018 active battle timers agree |
| Cannon A selected by player 0 | Slot 0; ID 1, power 40 |
| Hand transfer, consumption, damage | Custom-relative frames 223, 366, 387 |
| Opponent HP before any buster input | 100 → 60 |
| Final outcome | Results 1 / 2; winner 100 HP, opponent 0 HP |
| Native input with the chip selection omitted | Rejected specifically for missing selection |
| ROM-free comparator mutation guards | Nine tests pass |

The observer queues each core's numeric state at mGBA's native video-frame
boundary and writes it after stepping. It compares Custom selection,
the full deck and battle hand, both objects and both RNG words. A single
explicit RNG initialization at the map boundary uses the native fixture's
existing values; omitting that write changes neither the logical trace nor
the full state trace. The reviewed [boundary contract](re/arena-carried-boundary.md)
forbids later reseeding or ignored differences.

The next step is an independently initialized private pair that installs
only supported captured fields, then compares against these frozen native
controls. It still needs a no-op installer control, unsupported-profile
rejection, full original comparison, the 0/50/100/150 ms matrix and source
preservation after success and faults. Nondefault stats/programs, BN5 chips,
Souls, production visit/save integration, prediction and device performance
are unreached. The completed local replay unit does not establish them.
