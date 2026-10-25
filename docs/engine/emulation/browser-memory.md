---
type: Engine Mechanism
title: "The browser's memory"
description: "The browser build's WebAssembly memory, from 64 MB at the title to about 133 MB in a run with BN5's ROM, measured in headless Chrome; some 27 MB less since the guest core left runs."
tags: [engine, emulation]
generated: { by: claude-code/unrecorded, at: 2026-10-12T21:51:29Z }
---

# The browser's memory

The browser build's WebAssembly memory starts at 64 MB
(`-sINITIAL_MEMORY=64MB`) and grows as the game asks, never shrinking;
what it reached is the page's high-water mark (`Module.HEAP8.byteLength`
in the page). Measured on 12 October 2026 in headless Chrome
(agent-browser), both ROMs given, the same saves loaded from a backup (a
run at the arena door of End Area's third layer, an older net's act, seed
3), each build served by `build.py serve`:

| | This build (issue #82's last round) | Before it (a9d98987, the guest core in runs) |
| --- | --- | --- |
| The title | 64 MB | 64 MB |
| A new game's first layer (Central Area) | 110.6 MB | (not measured) |
| CONTINUE on the older net's layer | 132.6 MB | 159.2 MB (the guest core booted with it) |
| A battle there (CanGards, BN6's engine) | 132.6 MB | 159.2 MB |

So a run with BN5's ROM keeps some 27 MB less: the guest core's ROM copy
(BN5's 8 MB padded to `EMU_ROM_SIZE`, which mGBA copies at its first
patch) and its own state are gone. The JavaScript heap stayed near 20 MB
in this build. Issue #68's measure, before BN5's battles came into BN6's
engine: 97 MB on a first layer with BN6 alone, 153 MB on BN5's first
layer with the guest booted, 197 MB at most.
