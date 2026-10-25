---
type: Engine Mechanism
title: "The 3DS's memory"
description: "How the New 3DS's 96 MB heap holds BN6's ROM, the core's 16 MB copy and BN5's (`src/core/start_3ds.c`), mGBA's ROM buffer given back, one area's learned maps kept, measured by `tools/heap3ds.py`."
tags: [engine, emulation]
generated: { by: claude-code/unrecorded, at: 2026-10-13T00:52:19Z }
---

# The 3DS's memory

The New 3DS gives the game 124 MB. `src/core/start_3ds.c` splits it before
`main`: the heap takes all its area holds, 96 MB, and the linear heap the
rest, some 13 MB (the screens', the sound's, and the GBA's own RAM, which
mGBA's 3DS setup takes there). The heap is what runs short. What it holds
for a whole session:

| | The 3DS before issue #82's 3DS round | BN6 alone now | With BN5's ROM now |
| --- | --- | --- | --- |
| mGBA's ROM buffer | 16 MB | given back | given back |
| BN6's ROM (`R.data`, the player's bytes) | 8 MB | 8 MB | 8 MB |
| The core's copy (`EMU_ROM_SIZE`, patched) | 16 MB | 16 MB | 16 MB |
| BN5's ROM (`XR`) | | | 8 MB |
| All | 40 MB | 24 MB | 32 MB |

mGBA's 3DS setup (its `ctru-heap.c`, linked in with the core) takes its ROM
buffer before `main`, 16 MB as `docker/mgba.sh` builds it, for loaders of
its own (`mCorePreloadVF`, `mCoreLoadFile`) the game never calls: `emu.c`
hands the core its padded copy as a file in memory, which `GBALoadROM`
maps as it is for a 16 MB ROM and patches in place (mGBA 0.10.5's
`src/gba/gba.c` and `memory.c`'s `_pristineCow`). `start_3ds.c` gives the
buffer back at the start. BN5's ROM is then read whole, as on every
build, where the heap still has 80 MB free for it: its own 8 MB, the
core's copy still to come and 56 MB of play (a New 3DS has some 88 MB
free there); short of that it is left unread, the run is BN6's, and the
log and the PET's home say so.

What play takes was measured on a computer (`tools/heap3ds.py`,
[Dev tools](../../devtools/overview.md): the host build's allocations counted as the 3DS's heap
holds them, the core's copy kept, one past 96 MB refused). The installs
keep little of the heap (BN5's code and data, some 2 MB of its file read,
go into the core's copy, past the game's 8 MB). A layer's area learned
from its original maps takes up to 50 MB at once (a map, its mirror image
and its raised views, each a whole map's pixels twice), and what it
learned stays, 2 to 7 MB an area. Kept for every area a run had passed
through, those filled the heap in long runs: with the 3DS as it was, 20
of 24 long runs (seeds 1-24, the test autopilot kept alive, 120000
frames, 5 to 41 layers) had allocations refused, 544 in all (a map's
mirror image not learned, a book of its tiles lost, a cache left out).
So the 3DS keeps the learned maps of the layer's own area alone
(`src/map/netmap.c`), and an area a run comes back to is learned again,
the same (the endless net's acts once round: some 4 times a long run;
`log.txt` says "area N learned again"). Two areas kept refused
allocations in 17 of the 24 runs with BN5's ROM read.

| The same 24 long runs | The 3DS before | BN6 alone now | With BN5's ROM now |
| --- | --- | --- | --- |
| The heap's peak | 96 MB (full) | 87.7 MB | 95.8 MB |
| Runs with an allocation refused | 20 (544 refused) | 0 | 5 (6 refused: a speed cache of the tiles' learning each, its result the same) |
| Crashes | 0 | 0 | 0 |
| Areas learned again | 0 | 95 | 111 |

With BN5's ROM the heap is near full at a long run's worst: modelled 6 MB
short (`--cap 90`, what the 3DS's fragmentation might take), the same runs
with BN5 refused 332 allocations in 20 of the 24, 39 of them a book of an
area's tiles lost (its layers drawn from fewer of its original tiles),
none crashed; BN6 alone refused none. Two crashes the model found on the
way, an allocation refused where its result was used unchecked, are
mended: the pads found in a map's floor
(`tl_find_pads`) and the tile pairs its neighbours show (`seams_add`).
Other places in the map's learning still use an allocation unchecked
(`tests/lint/analyzer.txt`'s `decor.c`, `props.c`, `stairs.c`). The model
counts bytes, not the 3DS's fragmentation, so the device's own numbers are
`log.txt`'s: the heap after the ROMs are read ("3ds: BN5 read and checked
... heap N KB in use, M KB at most of 98304 KB") and after the installs
("3ds: the core started, the installs done"), and with `frame_log = on`
each second's line, whose "at most" is the heap's high-water mark (malloc's
trimming is off on the 3DS, so the space it took never shrinks).

BN5's ROM read in windows from the SD card instead, as a run needs it,
would keep its 8 MB out of the heap: its readers (the installs, the
dressing of a layer, its records and text, some 40 files over
`XR[].data`) read about 2 MB of it at the start (the installs, measured
page by page) and a layer's dressing 70 to 140 KB more. With the buffer
given back and one area's maps kept, the heap holds the file whole, and
every system reads BN5 alike. Where a New 3DS's own log shows BN5's long
runs short (the heap's "at most" at its size, `log.txt`), those windows
are the next room to make: some 6 MB.
