# Phase 0: serial cable and carried MegaMan

This is a bounded technical experiment for issue #120, measured on the
Linux development host on 15 October 2026. It is not a playable Arena.
The implementation is `tools/online_link_probe.c`; it uses the pinned
mGBA 0.10.5 API already built by `docker/mgba.sh`. No Tango source was
used. The runtime and state format are mGBA's, not a new link protocol.

## Serial experiment

```sh
python3 build.py online-link
```

The tool creates two mGBA cores with a tiny original test ROM made in
memory. Each core has its own video buffer, RAM, SRAM and serial node.
`GBASIOLockstep` connects their `GBASIO` multiplayer drivers. The
cooperative scheduler accounts for cycle credits, waits and signals.
The test starts 256 transfers, changing both participants' sent words
each time, and checks all four received words per transfer: 1,024
comparisons. It also checks the ports' player IDs and busy status.

Observed: **256 transfers, 1,024 words checked, passed**, about 2.4–2.8 ms
for the synthetic serial workload. This is API evidence, not game
performance. `NDEBUG` is required when compiling the probe because mGBA's
Release library changes lockstep structure layout with that define.

**Decision:** mGBA's multiplayer driver is usable without a second game
engine or GPL code. Its in-process lockstep callbacks are not a network
transport: they assume shared state and synchronous cycle accounting.
The presence server intentionally does not pretend to relay this object.

**Unanswered gates:** driving both real games through Practice NetBattle,
separating the serial participants into two processes, representing
transfers and timing over the wire, then injecting 0/50/100/150 ms delay
and detecting desync. No such battle or delay result is claimed here.
These remain required before choosing an Arena implementation.

## State and carried data

The same tool tests `online_snapshot_read` with a synthetic layout:
folder entries, the base and extended Pack, NaviCust records/grid,
HP/upgrades and caller-supplied Souls; invalid or misaligned RAM pointers
must fail. It then saves a core, overwrites **every byte** of EWRAM and
IWRAM, loads the state and compares all **294,912 bytes**. A separate
complete-state check mutates SRAM and verifies its restoration too.

With a local checkpoint created by the two-client proof:

```sh
python3 build.py online-link --rom /path/to/your/bn6g.gba \
  --state .build/online-proof/run-EXAMPLE/Alpha/savedata/run.state
```

Both files are mounted read-only. The tool reads the carried record,
performs the RAM mutation/restore, and compares the carried record again.
It does not execute that checkpoint with an uninstalled map or advance
the player's run. The data and states never enter the server or Git.

Observed on a real proof checkpoint:

| Measurement | Result |
| --- | ---: |
| Read-only carried record | 6,664 bytes |
| Folder entries | 30 |
| Raw mGBA state | 397,312 bytes |
| Eight raw rollback states | 3,178,496 bytes |
| Raw save, mean of 200 | 0.0113 ms |
| Raw load, mean of 200 | 0.0151 ms |
| Full state with SRAM/RTC | 430,144 bytes |
| SRAM verified after mutation | 32,768 bytes |
| Full state load, mean of 200 | 0.1648 ms |
| EWRAM/IWRAM and carried record equality | passed |

These timings describe this host and this checkpoint only. They do not
measure full rollback resimulation, a browser, the 3DS, or any handheld.
The raw core state does not include all application-side state, patches,
network queues or the separate SRAM payload. A rollback ring must keep
those coherent too; raw-state size alone is not a memory budget.

**Decision:** use an explicit carried record for inspecting a visit;
retain the ordinary run checkpoint as the authoritative recovery point.
Souls remain caller-supplied from the run-side record, and the reader is
not a network serialization format. The Phase 0 Square reads a boot
loadout only. Applying a current run's loadout to a visit and returning
through CONTINUE, including crash recovery, is still an integration gate.

The two-client proof separately establishes that visiting, leaving and
losing the server do not modify any of either client's 12 checkpoint
files. That file-integrity test is distinct from this in-memory
mutation/restore test.
