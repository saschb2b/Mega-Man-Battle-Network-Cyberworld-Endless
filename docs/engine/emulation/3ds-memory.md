---
type: Engine Mechanism
title: "The 3DS's memory"
description: "How the New 3DS's 96 MB heap holds BN6's ROM, the core's 16 MB copy and one extra ROM, BN5's first (`src/core/start_3ds.c`), mGBA's ROM buffer given back, one area's learned maps kept, its maps' mirror images and raised views made in place, measured by `tools/heap3ds.py`: the budget per part and the margin a new feature is checked against."
tags: [engine, emulation]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-12T20:30:00Z }
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
from its original maps took up to 50 MB at once (a map, its mirror image
and its raised views, each a whole map's pixels twice; since 9 November
2026 the views are made in place, and the learning holds one map: [The
learning made in place](#the-learning-made-in-place)), and what it
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

## A third ROM: BN4 Red Sun

BN4 Red Sun's ROM ([Battle Network 4 in a run](../../multirom/bn4.md)) is
offered since 5 November 2026, its families and areas in runs. The 3DS
reads one extra ROM at most, BN5's before BN4's (`start_3ds.c`, by
`rom.h`'s `XROMS_MOST`, which the NetPass's rule reads too:
[A system with room for one game](../../online/netpass.md#a-system-with-room-for-one-game)), and leaves the second unread, said in `log.txt` ("a New
3DS holds 1 extra ROM beside BN6's, read already") and on the PET ("BN5
found; BN4 too, but this 3DS has no room for it"). Modelled on 4
November 2026 (`python3 tools/heap3ds.py --cases bn5,bn4,all --seeds
1-12`, 120000 frames a run, BN4's read for research, its one family,
Billy, installed, its areas held):

| The same 12 long runs | BN6 and BN5 | BN6 and BN4 | BN6, BN5 and BN4 |
| --- | --- | --- | --- |
| The heap's peak | 95.9 MB | 95.4 MB | 96.0 MB (full) |
| Runs with an allocation refused | 6 (18 refused: a speed cache of the tiles' learning each) | 0 | 9 (73 refused: maps' mirror images and raised views not learned, besides the caches) |
| Crashes | 0 | 0 | 0 |
| Areas learned again | 29 | 15 | 34 |

Modelled again on 5 November 2026 with BN4 as players have it, its six
families installed and its five areas dressing BN6's (`--seeds 1-6`):
BN6 and BN4 peaked at 95.8 MB with no allocation refused in its 6 runs
and no crash, 12 areas learned again; BN6 and BN5 had 9 refused in 4
(the tiles' speed cache, as before); all three read at once 50 in 5
(maps' mirror images), which a 3DS never does.

BN4's file costs the heap what BN5's does, 8 MB; with its areas dressed
its layers learn BN4's 256-colour maps, and the runs above still had
room. Both together leave a long run's play 8 MB short, and what is
refused then is not a cache but a layer's own look. A New 3DS with
BN5's and BN4's ROMs plays with BN5's; BN4's comes in where BN5's is not
there.

## The learning made in place

Measured on 9 November 2026, as the next version's content came in fast
(ten-bit chip ids, PR #318; BN5's Navis; Oran Isle, PR #321): every case
of `tools/heap3ds.py` over seeds 1-12 (120000 frames a run), dev as it
was and each branch beside it, seeds 1-6 where marked. Peak in MB;
allocations refused, in how many runs:

| Before the change | BN6 alone | With BN5 | With BN4 | With BN5 and BN4 |
| --- | --- | --- | --- | --- |
| dev (c3d14c50) | 87.4; none | 95.9; 24 in 9 | 94.8; 6 in 4 | 96.0; 137 in 11 |
| Ten-bit chip ids (PR #318) | 87.6; none (seeds 1-8) | 95.8; 36 in 6 (seeds 1-6) | - | 96.0; 71 in 6 (seeds 1-6) |
| BN5's Navis (their branch) | 87.4; none (seeds 1-8) | 95.7; 36 in 6 (seeds 1-6) | - | 96.0; 54 in 5 (seeds 1-6) |
| Oran Isle (PR #321) | 87.4; none (seeds 1-8) | 95.9; 10 in 5 (seeds 1-6) | - | 95.9; 71 in 6 (seeds 1-6) |

(-: not measured in the round's time box; 0.20.0's one run measured,
BN6 alone on seed 2, peaked at 75.7 MB as dev's did.) No run crashed. A
long run with BN5 refused something in 9 of 12 on dev itself, and in
every run of the branches: the heap stood within 1.2 MB of full in every
case but BN6 alone.

What was live at a long run's peak (dev with BN5, seed 1, 94.96 MB:
each block of 32 KB and more named where it was asked for, a scratch
copy of `tools/heap3ds.c`):

| What | MB |
| --- | --- |
| The core's ROM copy (kept for the core's life) | 16.0 |
| BN6's ROM and BN5's (`R.data`, `XR`) | 16.0 |
| Home's source, kept for the town's next visit (`townsrc_learn`: Central Town's map, its mirror image, their entries) | 13.8 |
| The layer's area's own map being learned (`decode_tiles`: its pixels as shown, its first layer, its front and colour planes) | 11.8 |
| A raised view of it (`area_src_raise`'s copy) | 11.8 |
| That view's mirror image (`area_src_mirror`'s copy) | 11.8 |
| The tiles' learning of the view (`tiles_learn`: its candidates, looks, kept pixels and seams) | 8.8 |
| The installs' log (`xlog_write`) | 2.1 |
| The rest: SDL's, mGBA's, the game's tables, every block under 32 KB | 3.1 |

Three of those were the same pixels: the map, a view of it moved down a
floor's height, and that view flipped. `learn_view` and `learn_map`
(`src/map/netmap_learn.c`) now make both views in place
(`area_src_mirror_in`, `area_src_raise_in` in `src/map/area_src.c`): the
map's own tiles and pixels are flipped or moved and put back after, the
view sharing them, a raised view keeping only the rows it moves past the
map's foot (its height times the map's width, 80 KB for a floor 8 pixels
up on a map 1024 wide), a mirror only its heights and rings (some tens of
KB). The towns' sources, which keep a map and its mirror at once, and the
atlas keep the copies. The learning holds one map now, not three; what it
learns is the same, byte for byte (`tests/test_core.c`'s
`test_area_views`, and the pictures below).

| After it | BN6 alone | With BN5 | With BN4 | With BN5 and BN4 |
| --- | --- | --- | --- | --- |
| dev with the change | 67.4; none | 75.8; none | 76.3; none | 85.8; none |
| BN5's Navis with the change | - | 77.8; none (seeds 1-6) | - | 85.8; none (seeds 1-6) |

Every case some 20 MB lower, nothing refused, no crash, and the areas
learned again as often as before (42 with BN5 over the 12 runs, 16 with
BN4).

### What a refusal costs

Every allocation refused in the runs above, before the change, was asked
for by the learning (the first eight of each run named, 245 in all): 134
by the mirror images' and raised views' copies (`copy_parts`), 94 by the
tiles' kept pixels and the pick's memo (`tiles_keep_begin`), 17 by
`tiles_learn`'s own candidates and panel states, whose refusal leaves a
book of nothing. On dev with BN5 alone, its 24 were the kept pixels' and
`tiles_learn`'s; the branches' and the `all` case's were mostly the
copies'. Forced to fail with `heap3ds.py --refuse`
(BN5, seed 1, 40000 frames to depth 12, a picture every 1000 frames, 39
in all, set beside the same run without the refusal; the first two rows
the change against dev, nothing refused):

| Refused | Refusals | Pictures the same | Crashes |
| --- | --- | --- | --- |
| Nothing: dev's build with the 3DS's rules against the change's (its peak 84.2 MB, then 60.8) | 0 | 39 of 39 | 0 |
| Nothing: the host build, every area kept, dev's against the change's (91.6 MB, then 70.9) | 0 | 39 of 39 | 0 |
| `tiles_keep_begin`: the tiles' kept pixels and the pick's memo | 106 | 39 of 39 | 0 |
| `area_src_mirror_in`: a mirror image's heights and rings | 15 | 0 of 39 | 0 |

The tiles' kept pixels and the pick's memo are caches of answers worked
out again without them: refused, the layer is the same and only slower,
benign. A mirror image or a raised view refused is not, nor a book's
candidates in `tiles_learn`: the area learns
from fewer of its original maps' views, and the layer drawn from them
differs from the first picture on (the run itself takes another course
from there). It also breaks a layer's being made again the same from its
seed: learned again with more room (a checkpoint's CONTINUE, or the area
learned again later), the same layer would be drawn otherwise. So the
"speed cache" refusals of 5 November were benign; the mirror images'
and the books' refusals were not, and are what the change removes: with
it, no run of any case refused anything.
Neither crashed: the one place a refusal in the learning still wrote
through its allocation unchecked, the counter's mirror's walls
(`props_mirror_walls`), now checks it.

## The budget and the margin rule

What a long run holds on the New 3DS, by part, since the change: the
budget a new feature is counted against.

| Part | MB | When |
| --- | --- | --- |
| The core's ROM copy | 16.0 | the whole session |
| BN6's ROM | 8.0 | the whole session |
| An extra ROM (BN5's, else BN4's) | 8.0 | the whole session, where read |
| Home's source (Central Town's map, its mirror image, their entries) | 13.8 | from the first visit home, the whole run |
| The layer's area's learned books | 2 to 7 | while its layers last |
| An area's learning: one of its maps at a time | up to 11.8 | while it learns (a layer's first making) |
| The tiles' learning of that map | up to 9 | the same |
| The installs' log, SDL, mGBA, the game's tables | about 5 | the whole session |

The margin rule: `python3 tools/heap3ds.py --cases bn6,bn5,bn4 --cap 86`
over seeds 1-12 refuses nothing, and no crash, so a long run's peak stays
10 MB under the heap: 6 MB for what the 3DS's fragmentation may take (the
`--cap 90` test above) and 4 MB for what the model does not count (the
device's `log.txt` "at most" against the model's peak). The `all` case,
both extra ROMs at once, is measured but not held to it: a 3DS reads one
(A third ROM, above). A run whose peak at 96 MB stays under 86 with
nothing refused makes the same allocations at 86, so the tables above
answer the rule as well. After the change the cases a 3DS plays peak at
about 76 to 78 MB: some 8 MB of room under the rule for what is still to
come (BN4's chips, Team ProtoMan's Navis, more families), each counted
in the table above as it lands. A
feature that keeps memory for a whole run (a ROM's table, a cache, a
source kept for its next use) adds its row to the table above in the
same change. Where it would break the rule, the next room is, in order:
home's source let go on the jack-in and learned again at the next visit
(13.8 MB, as the Oran Isle stop does for its own town, PR #321), and BN5's
ROM read in windows from the SD card (6 MB, above).
