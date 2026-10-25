---
type: Code Map
title: "Developer tools inside the game"
description: "What `src/dev/` holds: `--dev` and its switches, the atlas, tour and pacing report, headless input and pictures (`capture.c`), and the tools run in the game's place (`--economy`, `--xdiff`, `--xrecords`)."
tags: [code]
generated: { by: claude-code/unrecorded, at: 2026-10-23T00:30:46Z }
---

# Developer tools inside the game

| Path | Contents |
| --- | --- |
| `src/dev/` | Developer tools inside the game: `--dev` (`devtools.c`), the atlas, the tour, the pacing report, headless runs' scripted input and pictures (`capture.c`: `--input`, `--taps`, `--shot`, remote play), the tools run in the game's place (`tools.c`: `--atlas`, `--pacing`, `--economy` through `economy_report.c`: what a run hands out by act, [Economy](../game/economy/overview.md), `--render-song`, `--sheet` through `sheets.c`, `--xdiff` through `xdiff.c`: a moved virus family, or MegaMan, against its own game's battle, issue #82; `--xrecords` through `xrecords_report.c`: BN5's records in BN6's engine, `--mp2k-trace` through `mp2k_trace.c`: the engine's MP2K frame by frame in BN6's layout, for `tools/mp2k_verify.py`) |
