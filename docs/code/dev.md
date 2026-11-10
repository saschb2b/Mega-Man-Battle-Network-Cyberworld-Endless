---
type: Code Map
title: "Developer tools inside the game"
description: "What `src/dev/` holds: `--dev` and its switches, the atlas, tour and pacing report, headless input and pictures (`capture.c`), and the tools run in the game's place (`--economy`, `--xdiff`, `--xrecords`)."
tags: [code]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-10T17:55:06Z }
---

# Developer tools inside the game

| Path | Contents |
| --- | --- |
| `src/dev/` | Developer tools inside the game: `--dev` (`devtools.c`), the atlas (the walks it counts on a layer's floor in `lineups.c`: [the generator](../world/level-design/generator.md), Navigation; its `sizes` in `layer_sizes.c`: every area's layers over 300 runs' seeds, their largest tile map and coordinate data against their regions, `build.py sizes`, [Free space](../engine/emulation/free-space.md)), the tour, the pacing report (the tournament's rounds in `pacing_bracket.c`, and in the economy report `economy_bracket.c`: [the tournament](../game/tournament.md)), headless runs' scripted input and pictures (`capture.c`: `--input`, `--taps`, `--shot`, remote play), a battle of one Navi swapped mid-fight for the next (`swap_dev.c`, the `swap` step: [A battle on demand](../devtools/battle-on-demand.md)), the tools run in the game's place (`tools.c`: `--atlas`, `--pacing`, `--economy` through `economy_report.c`: what a run hands out by act, [Economy](../game/economy/overview.md), `--render-song`, `--sheet` through `sheets.c`, `--xdiff` through `xdiff.c`: a moved virus family, or MegaMan, against its own game's battle, issue #82, and its `game=bn4` through `xdiff_bn4.c`: BN4 Red Sun's own battle alone on the guest core, its result and the HP left, its `game=bn4:trace` and `bn4:bn6` traced in BN6's units through `xdiff_bn4_trace.c` (its objects', collision data's, AI data's and panels' fields at BN6's places, and BN4's routines the hooks are placed on: [the differential](../devtools/differential.md), The BN4 trace); `--xrecords` through `xrecords_report.c`: BN5's records in BN6's engine, `--mp2k-trace` through `mp2k_trace.c`: the engine's MP2K frame by frame in BN6's layout, for `tools/mp2k_verify.py`, and `--xsong-pack`: another game's song as `src/audio/xsong.c` copies it into BN6, for the same tool's `xN/` songs, [The engine's sound against the game's](../devtools/sound-check.md)) `xdiff_ncp.c`: the differential's `ncp5` and `ncp6`, a NaviCust program placed on each game's own board and compiled by the game before the battle ([The differential](../devtools/differential.md), A program on each game's NaviCust). |
