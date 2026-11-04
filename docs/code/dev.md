---
type: Code Map
title: "Developer tools inside the game"
description: "What `src/dev/` holds: `--dev` and its switches, the atlas, tour and pacing report, headless input and pictures (`capture.c`), and the tools run in the game's place (`--economy`, `--xdiff`, `--xrecords`)."
tags: [code]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-04T01:15:00Z }
---

# Developer tools inside the game

| Path | Contents |
| --- | --- |
| `src/dev/` | Developer tools inside the game: `--dev` (`devtools.c`), the atlas (the walks it counts on a layer's floor in `lineups.c`: [the generator](../world/level-design/generator.md), Navigation), the tour, the pacing report, headless runs' scripted input and pictures (`capture.c`: `--input`, `--taps`, `--shot`, remote play), the tools run in the game's place (`tools.c`: `--atlas`, `--pacing`, `--economy` through `economy_report.c`: what a run hands out by act, [Economy](../game/economy/overview.md), `--render-song`, `--sheet` through `sheets.c`, `--xdiff` through `xdiff.c`: a moved virus family, or MegaMan, against its own game's battle, issue #82, and its `game=bn4` through `xdiff_bn4.c`: BN4 Red Sun's own battle alone on the guest core, its result and the HP left, its `game=bn4:trace` and `bn4:bn6` traced in BN6's units through `xdiff_bn4_trace.c` (its objects', collision data's, AI data's and panels' fields at BN6's places: [the differential](../devtools/differential.md), The BN4 trace); `--xrecords` through `xrecords_report.c`: BN5's records in BN6's engine, `--mp2k-trace` through `mp2k_trace.c`: the engine's MP2K frame by frame in BN6's layout, for `tools/mp2k_verify.py`) |
