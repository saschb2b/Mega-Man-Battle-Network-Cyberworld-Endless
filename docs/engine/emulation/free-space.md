---
type: Engine Mechanism
title: "Free space: zones, headroom and placing a region"
description: "The free space's one plan (`src/emu/free_space.plan`): its zones by kind and their headroom, `tools/free_space.py`'s header, tables and report, how a new region is placed, how an overrun is told, and the move that sized the layers' regions by their largest and freed `code-low`."
tags: [engine, emulation, free-space]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-12T09:00:00Z }
sources:
  - id: plan
    resource: "src/emu/free_space.plan"
    title: "The free space's plan, every region's row"
  - id: tool
    resource: "tools/free_space.py"
    title: "The plan's header, tables, report and fit"
  - id: watch
    resource: "src/emu/free_space_watch.c"
    title: "The run-time watch of writes past a region"
---

# Free space: zones, headroom and placing a region

Every region the engine writes past BN6's 8 MB in the core's ROM copy, in
the work RAM BN6 leaves alone and in the cart's save RAM between BN6's own
blocks, is a row of one file,
`src/emu/free_space.plan`: its name, its start and end, its zone, the files
that write it and the concept that explains it. Nothing else places a
region. `python3 tools/free_space.py --write` makes from it:

- `src/emu/free_space.h`: the `FREE_*`, `RAM_*` and `SRAM_*` names the code
  uses (each region's start, and `NAME_END` where its row says `end`), and
  the plan as tables, `FREE_SPACE_ROM`, `FREE_SPACE_RAM` and
  `FREE_SPACE_SAVE` (X-macros), which
  `tests/test_core.c` and `src/emu/free_space_watch.c` read;
- the offsets, names and writers of the three tables of
  [Memory the engine writes](memory.md), a row a region and a row a gap,
  whose words stay that page's own (a new region's row starts with the
  plan's line);
- the zones' table below.

`build.py lint` runs `tools/free_space.py --check`, which fails where the
plan is not whole (regions out of order or over each other, outside their
zone, a part outside its parent, a writer that is no file of `src/`, a
concept that is not there) or where any of the three differs from what the
plan makes. `tests/test_core.c` checks the compiled tables the same way, so
a header edited by hand that still compiles is caught twice.

## The zones

A zone is a span of one kind, so that a new region goes where its kind
lives: what is rewritten as a layer is entered beside the layers, the code
moved from another game beside the other moved code. A zone's space is the
ROM copy (`rom`), the work RAM (`ram`) or the cart's save RAM (`save`:
what must travel with every state and is reached a byte at a time,
[Memory the engine writes](memory.md)). The plan's numbers:

<!-- tools/free_space.py writes this table from src/emu/free_space.plan -->
| Zone | Span | Holds | Regions | Headroom | Largest gap |
| --- | --- | --- | --- | --- | --- |
| `engine` | `+0x000000`-`+0x002F00` (11.8 KB) | the engine's own records, hooks and stubs, and BN6's small tables grown | 10 | 2.4 KB (20%) | 1.8 KB |
| `layers` | `+0x002F00`-`+0x080000` (500.2 KB) | layers and the town: their maps, rewritten as a layer or the town is entered; home's other places | 9 | 32.0 KB (6%) | 32.0 KB |
| `code-low` | `+0x080000`-`+0x150000` (832.0 KB) | other games' code, art and per-battle rooms that the code zone has no room for: fixed places, the same at every boot with the same ROMs | 14 | 0 B (0%) | 0 B |
| `words` | `+0x150000`-`+0x170000` (128.0 KB) | the director's and the PET's words: text archives, labels, lists | 12 | 18.8 KB (14%) | 17.0 KB |
| `world` | `+0x170000`-`+0x330000` (1792.0 KB) | other games' world, copied as their areas are entered: tiles, songs, overworld art, backgrounds | 8 | 555.0 KB (30%) | 232.0 KB |
| `code` | `+0x330000`-`+0x7A0000` (4544.0 KB) | other games' code homes and their per-battle rooms (issue #82): fixed places, the same at every boot with the same ROMs | 27 | 443.9 KB (9%) | 384.0 KB |
| `art` | `+0x7A0000`-`+0x800000` (384.0 KB) | art copied whole, a place each | 2 | 0 B (0%) | 0 B |
| `persistent` | `0x02010CE0`-`0x02011800` (2.8 KB) | persistent: only the power-on clear writes 0x02010CDC-0x020117FF (the battle sprite pool from its end) | 5 | 0 B (0%) | 0 B |
| `mode` | `0x02029A00`-`0x0202DA00` (16.0 KB) | a main mode's own while the map waits (BN6's shop, mode 0x2C, unpacks into 0x02029A00, 0x0202BA00 and 0x0202DA00 as it runs, and the map goes on after it): a moved screen's, run as a mode of its own from the map | 1 | 0 B (0%) | 0 B |
| `battle` | `0x0203FB50`-`0x02040000` (1.2 KB) | per battle: the battle's setup clears 0x0203CDA8-0x0203FFFF, and nothing else of BN6's reads or writes 0x0203FB50 on | 12 | 0 B (0%) | 0 B |
| `save` | `0x0E006810`-`0x0E007E00` (5.5 KB) | the cart's save RAM between BN6's save (0x0E000100-0x0E00680F) and its three small blocks (from 0x0E007E00): BN6's save library addresses none of it, and every state the engine writes holds it | 1 | 928 B (16%) | 496 B |
<!-- end of the table tools/free_space.py writes -->

Headroom is the zone's spans no region holds and its regions marked `free`
(`FREE_TAIL`, `RAM_BATTLE_SPARE`). The ROM copy has about 1.1 MB free of
its 8 MB since the measured pass of 13 November 2026 (below): `world`'s
555 KB, in spans of 232, 207 and 112 KB, `code`'s 444 KB, 384 KB of it
one span, and the layers' 32 KB; `code-low` and `art` are full. What a region holds
is measured, not its size: `CYBERWORLD_FREE_SPACE=.build/usage.txt` on any
run (`build.py shot` passes it into its container; `tools/xdiff.py` gives
each spec a file of its own, `PATH.SPEC`) writes, as the program ends,
each region's last byte that is not the copy's `0xFF` fill, and every span
the run wrote, 16 bytes a bit (`src/emu/free_space_watch.c`:
`emu.c`'s writes are the only way into the copy). `python3
tools/free_space.py --usage FILE...` reports them beside the plan, several
runs' files as one: a region's bytes written, the end of the last of
them, and its largest span no run wrote. The last byte alone reads a
region with a table at its end as full, and a room filled again per
battle or per layer shows there what the last one left; the spans show
what lies unwritten before the table, and the most any battle or layer of
the runs reached. A room's worst case is still its writer's to bound: the
runs show what they met.

## Placing a region

1. Ask for room: `python3 tools/free_space.py --fit SIZE --zone KIND`
   lists the gaps of that zone that hold SIZE bytes (aligned to `0x100`,
   `--align` to change), and without `--zone` every zone's. A region's
   size is its worst case (the largest Navi's sprite, the longest archive),
   with room to grow; a room rewritten per battle or per layer is sized by
   the largest thing it ever holds.
2. Add its row to the plan, in address order inside its zone: its own end,
   never "to the next one's start", so that a region grown later shows as
   over its neighbour instead of eating it. `end` in its flags where the
   code needs `NAME_END`; `in=PARENT` for a part laid out inside a region.
3. `python3 tools/free_space.py --write`, then give its row in
   [Memory the engine writes](memory.md) its words. `build.py lint` and
   `build.py test` hold the rest.
4. Its writer checks its own bound before it writes and tells on stderr,
   always and not under `CYBERWORLD_EMU_DEBUG` alone, when what it copies
   does not fit; an installer at the core's start writes inside
   `claims_begin`/`claims_end` (`src/emu/claims.h`).

`code-low` is the `code` zone's kind in a second span: a moved Navi's,
chip's or family's region that `code` has no room for takes its front
(`--fit SIZE --zone code-low`, or `--fit SIZE` alone). A BL from there
reaches 4 MB, `code`'s first regions (the families' room and homes, the
chips' records, the Souls) and not its Navis' or chips' code; a call
further goes through a veneer (`src/emu/thumb_reloc.h`), as calls into
BN6's own code already do from every zone.

Where no zone of its kind has room, the region does not go into another
kind's gap: the plan moves first (Moving regions, below), with the
release that bumps `LAYER_MAKE`.

## An overrun is told

Two checks watch the writes as they happen. `src/emu/free_space_watch.c`
sees every write of `emu.c`'s into the ROM copy's free space, whoever
writes it: one that is not inside a single region (it runs past its
region's end, into a gap or over the next) is told on stderr, the first
eight of a run, with the regions it runs from and into. The claims
(`src/emu/claims.c`) see the installers' writes at the core's start byte
by byte and tell where one writes over another's. Neither stops the write;
both make it loud.

What the review of the copiers found (November 2026), and what stands:

- Fallbacks that were silent: other games' battle backgrounds
  (`xbackdrop.c`) keep eight of the eleven areas' backgrounds that
  `rom.c` names (`XBGS_MAX`), the rest without a word, and a copy that
  finds no room was skipped silently; `mapslot_alloc` returned 0 without a
  word, and some callers write that 0 into BN6's tables. Both now tell on
  stderr; the backgrounds' count is left for its track.
- Writes with no bound: the town's coordinate data (`coords.c`) can run
  past `FREE_TOWN_COORDS` into `FREE_TOWN` for a large town; the Request
  BBS's archives (`request_bbs.c`) past `FREE_BBS`; the showcase's record
  (`showcase.c`) is written at `FREE_RECORDS_END`, outside its region. The
  watch above now tells each as it happens.

## Moving regions (applied, November 2026)

The proposal of 9 November 2026, applied the same day for the release
after 0.20.0, whose `LAYER_MAKE` (113) was bumped already: the regions sized far past what they hold shrunk to
their worst case, and what they freed gathered into one span. Moving a
region is a change of its row's numbers alone, and lint holds the rest.

The layers' sizes were measured first, with `build.py sizes` (the atlas's
`sizes`, `src/dev/layer_sizes.c`), on 9 November 2026: BN6's 19 areas,
BN5's six (`x0`-`x5`) and BN4's five (`x15`-`x19`), on runs of seeds
1-300, at every depth of the net's cycle and one past it (1-20: every size
of layout and every guardian's layer) and in each of the four side layers'
kinds at the largest size: 7,200 layers an area, 216,000 in all, every one
built; the nine areas between, BN5's dark layers' looks (`x6`-`x14`),
over seeds 1-30 alone that day, their largest coordinates 18,536 bytes. The coordinate data counts the most
warp pads a layer's coordinates take (784 trigger cells, 7,056 bytes as
literal blocks), which the layers built there have none of; a dark
layer's board and the Net Square's plaza, built otherwise, are not among
them, and are held by their writers' bounds alone:

| Region | Before | Measured largest | Now |
| --- | --- | --- | --- |
| `FREE_TILE_MAP` | 320 KB | 87,712 bytes (BN6's area 4, seed 260, depth 14) | 96 KB: the largest BN6's buffer takes (`BN6_TILEMAP_MAX`) as literal blocks, 92,164 bytes, held by a static assertion |
| `FREE_COORDS` | 640 KB | 24,240 bytes with the pads (BN6's area 5, seed 15, depth 20, its Glitch layer); BN5's largest 21,856, BN4's 20,200 | 32 KB (`0x8000`): the largest with a quarter to spare (30,300), rounded up to 4 KB |
| `FREE_TOWN_TILE_MAP` | 192 KB | (`town_plan` refuses a town past BN6's buffer) | 96 KB, the layers' bound |

What moved, each into its kind's zone:

| Region | From | To | Why |
| --- | --- | --- | --- |
| `FREE_COORDS` and the town's regions (`FREE_TOWN_TILE_MAP`, `FREE_TOWN_COORDS`, `FREE_TOWN`, `FREE_LAN_HP`) | `+0x60000`-`+0x150000` | `+0x28000`-`+0x68000` | packed after the shrunk tile map |
| `FREE_INDOORS` (home's other places) | the `world` zone's end, `+0x320000` | `layers`, after `FREE_LAN_HP` | a home's maps are a layer's kind |
| `FREE_CHIP_PAS`, `FREE_CHIP_AI`, `FREE_SWAP_RETURN` (small stubs) | between the Souls' art and `FREE_TAIL`, `+0x710000` | `engine`'s gap, `+0x1300` | hooks and stubs are the engine's kind; `FREE_TAIL` is `+0x710000`-`+0x711000` |

The room by zone:

| Zone | Headroom before | Headroom now |
| --- | --- | --- |
| `engine` | 7.5 KB | 6.4 KB (the stubs in) |
| `layers` | 0 B | 96 KB at its end, `+0x78000`-`+0x90000`: the next layer's or town's kind |
| `code-low` (new) | | 768 KB, `+0x90000`-`+0x150000`, one span: the `code` zone's kind |
| `world` | 60 KB | 124 KB (`FREE_INDOORS` out: 64 KB at its end) |
| `code` | 2.9 KB | 4 KB (`FREE_TAIL`) |

`code-low` is for what the `code` zone has no room for and the next
features' to take from its front: Team ProtoMan's five remaining Navis
(about `0x22000` bytes of battle sprites and their code), BN4's chips'
moved code ([BN4's chips](../../multirom/bn4-chips.md)) and BN4's further
late families (about `0x5000` bytes of art and `0x1000` of code each). None
is placed here: each takes its row with `--fit`.

A move changes ROM addresses that a saved emulator state's RAM can point
into. None of those pointers outlives a CONTINUE: every one re-enters its
map through BN6's own enter-map routine (`emu_warp`, a game call that runs
it whole before any frame of the state's: `layer_reenter` for a layer,
`home_resume` for Lan's HP, the town and home's places), which reads the
map's tables, scripts, tile map and coordinate data from the ROM copy at
their new places, the copy being built again before the state is loaded;
a run saved by 0.20.0 has another `LAYER_MAKE` and continues its layer
afresh besides. The stubs moved are pointed at only mid-battle (a hook's
return, the Program Advances' list on the Custom screen), and a checkpoint
is never taken in a battle (`boss_idle`). The rest of the plan, the `code`
zone's fixed places among it, did not move.

Every writer of a region shrunk checks its bound and tells on stderr where
what it writes does not fit, then writes nothing (`coords.c`, `netmap.c`,
`town.c`): a layer past its region is not built, as one past BN6's buffer
was not before. `build.py sizes` fails where the largest layer over the
300 runs' seeds outgrows its region; it is run again when generation
grows its layers (a `LAYER_MAKE` bump that makes them larger).

What one run used (a layer at depth 2 with BN5's and BN4's ROMs, 9
November 2026, `--usage`): the regions written once at the core's start
are near full, so the moved content's next families, Navis and chips need
the `code` zone's gaps, not their own regions' tails. `FREE_FAMILIES`,
`FREE_NAVI_ROOM`, `FREE_CHIP_PAS` and `FREE_BLOCKERS` read 100%,
`FREE_NAVI_SPRITES`, `FREE_NAVI_ART`, `FREE_SOULS`, `FREE_SOULS_ART` and
`FREE_RUSH` 97-99% (a region with a table at its end reads full however
much is free before it, so these are upper bounds); `FREE_CHIPS` used a
third, `FREE_SONGS` 42% and `FREE_XNAVIS` a third.

**12 November 2026: `code-low` takes the layers' headroom's back.** BN5's
LarkMan and GridMan, the Navis' seats 17-18, needed a home, a code room and
an art room each and a place for GridMan's battle sprite, and no zone of the
`code` kind had room (`--fit`: `code-low`'s 768 KB taken by Team ProtoMan's,
BN4's chips' and the shared art's third room). The zone boundary moved:
`layers` ends at `+0x80000`, its headroom 32 KB (`+0x78000`-`+0x80000`, of
the 96 KB no layer, town or home place used: the largest measured above
leave it whole), and `code-low` begins there, its front `FREE_NEBULA_NAVIS`
(`+0x80000`-`+0x90000`, [Transplanted Navis](../../multirom/navis.md),
GridMan). No region moved: a row's numbers alone, lint holding the rest.

## The measured pass (13 November 2026)

`python3 tools/free_space.py --fit 0x10000` found no gap of 64 KB in any
zone: the layers' 32 KB was the largest, and the `world` zone's last, 60 KB,
was about to hold BN4's Souls' shared room
([BN4's Souls](../../multirom/bn4-souls.md)), with a town of BN5's
(60 KB) and the Souls' second room (45 KB) still to place. So every region
was measured against what it reserves, the ones sized far past what they
hold shrunk where that is proven, and what they gave up gathered.

**How it was measured.** A build that keeps every write into the free
space (The zones, above), on 13 November 2026, over:

- a start and a first layer with BN6's ROM alone, with BN5's, with BN4's
  and with both (what the installers write as a core starts is the same
  at every start with the same ROMs);
- the autopilot from `--net-biome 0`-`7` (BN6's areas, dressed as the
  other games' where their ROMs are read: two seeds at two depths with
  both ROMs, a third with BN6's alone) and from `x0`-`x19` (BN5's six, the
  dark layers' nine looks, BN4's five; `x20` on are no areas) with both
  ROMs, two seeds at two depths; five runs from depths 6 to 18;
- a guardian's layer under `--guardian 24`-`29` (BN5's) and `34`-`45`
  (BN4's): seventeen of the eighteen staged their guardian, his sprite
  and art in their rooms;
- the town from `--scene town` over seven seeds and Lan's HP from
  `--scene home`; two runs with every Soul held (`--dev souls=0xFFF`);
- 112 of the differential's specs, a file each: BN4's twelve Navis' and
  eighteen of BN5's, every Soul's, BN4's Navi chips' and a sample of BN4's and
  BN5's chips, Program Advances and families, each with its recorded
  verdict;
- and, for the songs, a build of that day alone that copied every theme
  and every listed sound (`XSOUNDS`, 85) at once: 472,944 bytes with both
  ROMs, which no run reaches (a sound is copied as its track first plays
  it, or packed into its track's own room).

The starts and 60 of the areas' runs ran on the plan as it was, 64 runs;
the rest, 53 runs and the specs, on the plan as it is now, which they
checked at once. No run of the new plan wrote past a region
(`free_space_watch.c` tells each such write; none was told).

**What was sized far past what it holds**, and what became of it:

| Region | Reserved | Holds at most | How that is known | Now |
| --- | --- | --- | --- | --- |
| `FREE_CHIP_MOVED` (BN5's chips' moved code, tables and art) | 1,023 KB | its tables and homes 91,136 bytes, fixed places; the units' code 61,328 bytes of a room of 563,200; their art 304,608 of 393,216 | static: every block and sprite is at the place its recipe names (`xchips_bn5.c`), and `tests/test_core.c` checks each against the rooms' bounds | the code's room ends at `+0x640000` (`XC_CODE_END`, 169,984 bytes); the art is `FREE_CHIP_ART`, where it was; the 384 KB between them free. Nothing moved |
| `FREE_SONGS` | 768 KB | 472,944 bytes | every theme and listed sound copied at once (above); the runs' most 452,288 | 640 KB, its start where it was |
| `FREE_XAREA` | 192 KB | 59,876 bytes (BN4's Undernet's tile set and colours; its Park Area's 58,508, BN5's largest 35,648, a dark layer's look 28,460) | each of the twenty areas entered; and static: a tile set that fills the BGs' 64 KB of VRAM, as literal blocks, with 256 colours is 74,282 bytes, held by an assertion in `netmap.c` | 80 KB, its start where it was |
| `FREE_XNAVIS` | 256 KB | the areas' Navis and the map objects 85,472 bytes, copied once in a fixed order; a guardian's sprite and face 68,033 (KnightMan's) in its last `0x12000` | the same at every start with both ROMs; the guardians' runs | 184 KB: 114,688 for the first, a third to spare, and the guardian's `0x12000` |
| `FREE_BACKDROPS` | 256 KB | 44,240 bytes: eight backgrounds (`XBGS_MAX`) and the grown tables | copied once in the areas' order, the same at every start with both ROMs | 96 KB: room for the three areas' backgrounds the eight slots leave out |
| `FREE_MAP_BACKDROPS` | 192 KB | 88,976 bytes: every area's backdrop and animations | copied once, all of them, the first time one is needed | 128 KB |
| `FREE_PANELS` | 64 KB | 4,096 bytes | static: its layout's last part ends there (`xpanels_look.h`, `XP_PULL`) | 8 KB |

Each writer checks its bound before it writes and tells on stderr where
what it copies does not fit (`xsong.c`, `xnavi.c` and `netmap.c` did not
tell before; `xbackdrop.c` did).

**What moved.** In `world`: `FREE_XNAVIS`, `FREE_BACKDROPS`,
`FREE_MAP_BACKDROPS` and `FREE_FLAME`, packed behind the songs
(`+0x240000`-`+0x2A7000`), so that what the zone gave up is one span,
`+0x2A7000`-`+0x320000` (484 KB), and 112 KB after `FREE_XAREA`
(`+0x184000`-`+0x1A0000`). BN4's Souls' shared room, placed the same day
by its own change at `+0x2E1000`-`+0x2EC400` (45 KB, where the zone's
last gap was), lies inside that span and was left there, its spec naming
its address (`tests/xdiff/soul_thun.json`): the span is 232 KB before it
and 207 KB after. Moved to `+0x2A7000` by its track, it would leave
439 KB whole. `FREE_XAREA` and `FREE_SONGS` shrank from their
ends and stayed: a checkpoint's music player points into the songs, each
copied where it was in the session that saved it. In `code` nothing moved:
the chips' code and art are where their recipes put them, and the span
between them, `+0x640000`-`+0x6A0000` (384 KB), was never written;
`FREE_PANELS` gave up its tail, `+0x4C2000`-`+0x4D0000` (56 KB).

| Zone | Headroom before | Headroom now | Largest gap now |
| --- | --- | --- | --- |
| `world` | 64 KB (60 KB and 4 KB; 19 KB once the Souls' room took its 45) | 555 KB with the Souls' room in | 232 KB, `+0x2A7000`-`+0x2E1000`; then 207 KB, `+0x2EC400`-`+0x320000`, and 112 KB, `+0x184000`-`+0x1A0000` |
| `code` | 3.9 KB (`FREE_TAIL`) | 444 KB | 384 KB, `+0x640000`-`+0x6A0000` |
| the others | as they were: `layers` 32 KB, `words` 18.8 KB, `engine` 2.4 KB, `code-low` and `art` none | | |

`--fit 0x10000` now lists four places: `world`'s `+0x184000`, `+0x2A7000`
and `+0x2EC400`, `code`'s `+0x640000`. What is known to want room fits
with room over: the town's 60 KB and the Souls' second room of 45 KB
beside the first with `--fit`, or the second room, a room filled per
battle, into `code`'s span, its kind's. The owner's call for one shared
room ([BN4's Souls](../../multirom/bn4-souls.md), The shared room) was
made against a copy with no room for the twelve Souls' sprites, 438 KB
in all; that much is free now (439 KB around the room in `world`, 384 KB
in `code`), and the call is the owner's to keep or revisit.

**Saved states.** The regions moved are copied anew by each core, the
first time each is needed, and found again by a check of the copy
(`xnavi.c`, `xbackdrop.c`); a state's RAM points into them only on a map
or in a battle, and a CONTINUE re-enters its map through BN6's enter-map
routine (Moving regions, above) while a checkpoint is never taken in a
battle. Tried: three runs saved on a layer by the build before the move (in
BN5's ACDC Area, BN4's Park Area and a dark layer's look, `--net-biome
x0`, `x17` and `x8`), each continued from the title by the build before
it and by the build after it, on the autopilot for 5,000 frames: past
another game's Navi and a flame of darkness on the layer, onto the next
layer and into battles on the other game's background. The two builds'
pictures are the same byte for byte at each of seven frames from 400 to
5,190, and their run logs the same. `LAYER_MAKE` is not bumped for it:
it is 143, 18 past 0.21.0's 125, so a run saved by any released build
continues its layer afresh in any case.

**Sized past what they hold, and left** (each its track's to grow into,
or without a bound to prove):

- `FREE_CHIPS`: 376,544 bytes of 512 KB written, its last 147,744 never;
  the registry's art grows with every chip another game adds, and BN4's
  chips are in flight.
- `FREE_BN4_CHIPS`: its units' homes, code and art 38,960 bytes of
  320 KB so far, a span of 199,664 never written; four branches hold
  claims on its units ([claims](../../project/claims.md)).
- `FREE_LATE_FAMILIES` (52,096 of 192 KB) and `FREE_LATE_CODE` (5,360 of
  60 KB): BN4's late families, three moved so far.
- `FREE_BN4_NAVI_CODE` (70,464 of 128 KB), `FREE_BN4_NAVIS`' homes
  (51,264 of 64 KB): BN4's Navis' track.
- `FREE_SOULS_ART`: its first 60 KB never written (the Souls' sprites all
  fit their room's own art; ColonelSoul's button holds its last 4 KB);
  the Souls' track's, in flight.
- `FREE_TEAM_SPRITES` (221,808 of 272 KB): room for all six of Team
  ProtoMan's, by design.
- `FREE_CHIP_ART`: 304,608 of 384 KB, its tail 88,608; BN5's chips' next
  sprites'.
- `FREE_FAMILIES`: 44 KB between its parts never written; its layout is
  fixed offsets (`xvirus_recipe.h`), not a tail to give up.
- `FREE_TOWN_COORDS` (10,496 of 64 KB over seven towns), `FREE_INDOORS`,
  `FREE_LAN_HP`: the town's writer has no bound to prove one from (An
  overrun is told, above), and seven towns are no worst case.
- `FREE_PET_BODIES` (22,384 of 48 KB) and the other words: the lab's
  mails grow with every feature that writes one.
- `FREE_LIB_TEAM`: five places of `0x12000` (KnightMan's sprite and
  mugshot, 68,033 bytes), full by a static assertion; no run here met a
  liberation's team.

**Region by region** (bytes; "written" the bytes any run wrote, "to" the
end of the last of them from the region's start, "largest span never
written" inside it; a room filled per battle or layer shows the most the
runs met, not its worst case):

| Zone | Region | Reserved | Written | To | Largest span never written |
| --- | --- | --- | --- | --- | --- |
| `engine` | `FREE_WARP` | 256 | 32 | 32 | 224 |
| `engine` | `FREE_CALL_RETURN` | 128 | 0 | 2 | 128 |
| `engine` | `FREE_NO_MYSTERY` | 128 | 16 | 16 | 112 |
| `engine` | `FREE_RECORDS` | 256 | 96 | 192 | 64 |
| `engine` | `FREE_RUSH` | 576 | 560 | 560 | 16 |
| `engine` | `FREE_BLOCKERS` | 3,040 | 3,040 | 3,040 | 0 |
| `engine` | `FREE_CHIP_PAS` | 1,024 | 1,024 | 1,024 | 0 |
| `engine` | `FREE_CHIP_AI` | 64 | 16 | 16 | 48 |
| `engine` | `FREE_SWAP_RETURN` | 32 | 0 | 0 | 32 |
| `engine` | `FREE_NCP` | 4,096 | 4,064 | 4,064 | 32 |
| `layers` | `FREE_WARP_LIST` | 256 | 48 | 48 | 208 |
| `layers` | `FREE_LAYER` | 53,248 | 15,312 | 34,240 | 19,008 |
| `layers` | `FREE_TILE_MAP` | 98,304 | 75,616 | 75,616 | 22,688 |
| `layers` | `FREE_COORDS` | 32,768 | 12,400 | 12,400 | 20,368 |
| `layers` | `FREE_TOWN_TILE_MAP` | 98,304 | 52,416 | 52,416 | 45,888 |
| `layers` | `FREE_TOWN_COORDS` | 65,536 | 10,496 | 10,496 | 55,040 |
| `layers` | `FREE_TOWN` | 32,768 | 3,856 | 3,856 | 28,912 |
| `layers` | `FREE_LAN_HP` | 32,768 | 4,592 | 17,152 | 15,616 |
| `layers` | `FREE_INDOORS` | 65,536 | 5,504 | 34,880 | 30,656 |
| `code-low` | `FREE_NEBULA_HOMES` | 4,096 | 2,256 | 3,440 | 768 |
| `code-low` | `FREE_NEBULA_NAVI_CODE` | 24,576 | 15,136 | 21,952 | 4,896 |
| `code-low` | `FREE_NEBULA_NAVI_ART` | 2,048 | 0 | 0 | 2,048 |
| `code-low` | `FREE_NEBULA_SPRITES` | 34,816 | 29,632 | 29,632 | 5,184 |
| `code-low` | `FREE_TEAM_HOMES` | 12,288 | 6,256 | 11,760 | 2,048 |
| `code-low` | `FREE_TEAM_NAVI_CODE` | 61,440 | 32,336 | 59,056 | 10,240 |
| `code-low` | `FREE_TEAM_NAVI_ART` | 6,144 | 64 | 3,104 | 3,040 |
| `code-low` | `FREE_TEAM_SHARED` | 30,720 | 29,024 | 29,696 | 1,024 |
| `code-low` | `FREE_TEAM_SPRITES` | 278,528 | 216,512 | 221,808 | 56,720 |
| `code-low` | `FREE_BN4_CHIPS` | 327,680 | 38,960 | 327,680 | 199,664 |
| `code-low` | `FREE_SONG_TABLE` | 8,192 | 8,192 | 8,192 | 0 |
| `code-low` | `FREE_TEAM_SHARED_MORE` | 61,440 | 50,896 | 61,104 | 2,016 |
| `words` | `FREE_TALK` | 8,192 | 432 | 432 | 7,760 |
| `words` | `FREE_LABEL` | 4,096 | 2,096 | 2,096 | 2,000 |
| `words` | `FREE_PET_LABEL` | 4,096 | 2,096 | 2,096 | 2,000 |
| `words` | `FREE_PET_TEXT` | 4,096 | 2,896 | 2,896 | 1,200 |
| `words` | `FREE_PET_DESCS` | 12,288 | 6,960 | 6,960 | 5,328 |
| `words` | `FREE_PET_MAILS` | 8,192 | 2,256 | 2,256 | 5,936 |
| `words` | `FREE_PET_BODIES` | 49,152 | 22,400 | 22,400 | 26,752 |
| `words` | `FREE_PORTRAITS` | 6,400 | 3,360 | 4,960 | 1,440 |
| `words` | `FREE_TRADER_LIST` | 4,096 | 1,968 | 1,968 | 2,128 |
| `words` | `FREE_PET_OPTIONS` | 4,096 | 2,944 | 2,944 | 1,152 |
| `words` | `FREE_BBS` | 4,096 | 1,552 | 1,552 | 2,544 |
| `words` | `FREE_NCP_TEXT` | 3,072 | 2,320 | 2,448 | 624 |
| `world` | `FREE_XAREA` | 81,920 (was 196,608) | 59,888 | 59,888 | 22,032 |
| `world` | `FREE_SONGS` | 655,360 (was 786,432) | 452,288 | 452,288 | 203,072 |
| `world` | `FREE_XNAVIS` | 188,416 (was 262,144) | 153,536 | 182,736 | 29,200 |
| `world` | `FREE_BACKDROPS` | 98,304 (was 262,144) | 44,240 | 44,240 | 54,064 |
| `world` | `FREE_MAP_BACKDROPS` | 131,072 (was 196,608) | 88,976 | 88,976 | 42,096 |
| `world` | `FREE_FLAME` | 4,096 | 1,616 | 1,616 | 2,480 |
| `world` | `FREE_BN4_CHIP_SPRITE` | 61,440 | 60,992 | 60,992 | 448 |
| `code` | `FREE_VIRUS_ROOM` | 98,304 | 76,608 | 91,648 | 13,120 |
| `code` | `FREE_FAMILIES` | 753,664 | 504,272 | 753,664 | 44,208 |
| `code` | `FREE_CHIPS` | 524,288 | 376,544 | 376,544 | 147,744 |
| `code` | `FREE_SOULS` | 262,144 | 208,560 | 254,480 | 30,448 |
| `code` | `FREE_PANELS` | 8,192 (was 65,536) | 3,568 | 3,968 | 4,224 |
| `code` | `FREE_LATE_FAMILIES` | 196,608 | 46,416 | 52,096 | 144,512 |
| `code` | `FREE_NAVI_SPRITES` | 524,288 | 497,568 | 521,216 | 4,192 |
| `code` | `FREE_NAVI_ROOM` | 32,768 | 32,384 | 32,768 | 384 |
| `code` | `FREE_NAVI_HOMES` | 32,768 | 13,488 | 31,552 | 7,376 |
| `code` | `FREE_NAVI_CODE` | 131,072 | 46,048 | 111,056 | 20,016 |
| `code` | `FREE_NAVI_ART` | 327,680 | 220,576 | 325,632 | 19,264 |
| `code` | `FREE_ATTACK_FAMILIES` | 1,024 | 1,024 | 1,024 | 0 |
| `code` | `FREE_CHIP_MOVED` | 261,120 (was 1,047,552) | 88,080 | 152,464 | 108,656 |
| `code` | `FREE_CHIP_ART` | 393,216 (new) | 258,576 | 304,608 | 88,608 |
| `code` | `FREE_SOULS_ART` | 65,536 | 2,560 | 64,000 | 61,440 |
| `code` | `FREE_BN4_CHIP_STUBS` | 64 | 48 | 48 | 16 |
| `code` | `FREE_TAIL` | 4,032 | 0 | 0 | 4,032 |
| `code` | `FREE_LATE_CODE` | 61,440 | 5,360 | 5,360 | 56,080 |
| `code` | `FREE_BN4_NAVIS` | 65,536 | 13,728 | 51,264 | 17,200 |
| `code` | `FREE_BN4_NAVI_CODE` | 131,072 | 70,432 | 70,464 | 60,608 |
| `code` | `FREE_BN4_NAVI_ART` | 90,112 | 73,472 | 73,472 | 16,640 |
| `code` | `FREE_BN4_CHIP_ART` | 90,112 | 14,208 | 15,360 | 74,752 |
| `code` | `FREE_BN4_NAVI_SPRITE` | 65,536 | 60,992 | 60,992 | 4,544 |
| `code` | `FREE_BRACKET_FACES` | 16,384 | 13,440 | 13,536 | 2,848 |
| `code` | `FREE_BRACKET` | 65,536 | 44,720 | 63,392 | 6,064 |
| `art` | `FREE_LIB_TEAM` | 368,640 | 0 | 0 | 368,640 |
| `art` | `FREE_NAVI_SHARED_FOUR` | 24,576 | 14,416 | 17,520 | 7,056 |
