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

Every region the engine writes past BN6's 8 MB in the core's ROM copy, and
in the RAM BN6 leaves alone, is a row of one file,
`src/emu/free_space.plan`: its name, its start and end, its zone, the files
that write it and the concept that explains it. Nothing else places a
region. `python3 tools/free_space.py --write` makes from it:

- `src/emu/free_space.h`: the `FREE_*` and `RAM_*` names the code uses (each
  region's start, and `NAME_END` where its row says `end`), and the plan as
  tables, `FREE_SPACE_ROM` and `FREE_SPACE_RAM` (X-macros), which
  `tests/test_core.c` and `src/emu/free_space_watch.c` read;
- the offsets, names and writers of the two tables of
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
moved from another game beside the other moved code. The plan's numbers:

<!-- tools/free_space.py writes this table from src/emu/free_space.plan -->
| Zone | Span | Holds | Regions | Headroom | Largest gap |
| --- | --- | --- | --- | --- | --- |
| `engine` | `+0x000000`-`+0x002F00` (11.8 KB) | the engine's own records, hooks and stubs, and BN6's small tables grown | 10 | 2.4 KB (20%) | 1.8 KB |
| `layers` | `+0x002F00`-`+0x080000` (500.2 KB) | layers and the town: their maps, rewritten as a layer or the town is entered; home's other places | 9 | 32.0 KB (6%) | 32.0 KB |
| `code-low` | `+0x080000`-`+0x150000` (832.0 KB) | other games' code, art and per-battle rooms that the code zone has no room for: fixed places, the same at every boot with the same ROMs | 14 | 0 B (0%) | 0 B |
| `words` | `+0x150000`-`+0x170000` (128.0 KB) | the director's and the PET's words: text archives, labels, lists | 12 | 18.8 KB (14%) | 17.0 KB |
| `world` | `+0x170000`-`+0x330000` (1792.0 KB) | other games' world, copied as their areas are entered: tiles, songs, overworld art, backgrounds | 6 | 124.0 KB (6%) | 64.0 KB |
| `code` | `+0x330000`-`+0x7A0000` (4544.0 KB) | other games' code homes and their per-battle rooms (issue #82): fixed places, the same at every boot with the same ROMs | 24 | 4.0 KB (0%) | 4.0 KB |
| `art` | `+0x7A0000`-`+0x800000` (384.0 KB) | art copied whole, a place each | 2 | 0 B (0%) | 0 B |
| `persistent` | `0x02010CE0`-`0x02011800` (2.8 KB) | persistent: only the power-on clear writes 0x02010CDC-0x020117FF (the battle sprite pool from its end) | 5 | 0 B (0%) | 0 B |
| `mode` | `0x02029A00`-`0x0202DA00` (16.0 KB) | a main mode's own while the map waits (BN6's shop, mode 0x2C, unpacks into 0x02029A00, 0x0202BA00 and 0x0202DA00 as it runs, and the map goes on after it): a moved screen's, run as a mode of its own from the map | 1 | 0 B (0%) | 0 B |
| `battle` | `0x0203FB50`-`0x02040000` (1.2 KB) | per battle: the battle's setup clears 0x0203CDA8-0x0203FFFF, and nothing else of BN6's reads or writes 0x0203FB50 on | 12 | 0 B (0%) | 0 B |
<!-- end of the table tools/free_space.py writes -->

Headroom is the zone's spans no region holds and its regions marked `free`
(`FREE_TAIL`, `RAM_BATTLE_SPARE`). The ROM copy has about 1 MB free of
its 8 MB since the layers' regions were sized by their largest (Moving
regions, below): most of it `code-low`'s one span, 768 KB, the room for
the moved code, art and per-battle rooms the `code` zone has none left
for, and the layers' own 96 KB. What a region holds is measured, not
its size: `CYBERWORLD_FREE_SPACE=.build/usage.txt` on any run (`build.py
shot` passes it into its container) writes, as the program ends, each
region's last byte that is not the copy's `0xFF` fill, and `python3
tools/free_space.py --usage .build/usage.txt` reports it beside the plan. A
layer's or a battle's room shows what that run's layer or battle needed,
not its worst case.

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
