---
type: Engine Mechanism
title: "Free space: zones, headroom and placing a region"
description: "The free space's one plan (`src/emu/free_space.plan`): its zones by kind and their headroom, `tools/free_space.py`'s header, tables and report, how a new region is placed, how an overrun is told, and a proposed move for more headroom."
tags: [engine, emulation, free-space]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-09T02:30:00Z }
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
| `engine` | `+0x000000`-`+0x002F00` (11.8 KB) | the engine's own records, hooks and stubs, and BN6's small tables grown | 6 | 7.5 KB (63%) | 7.0 KB |
| `layers` | `+0x002F00`-`+0x150000` (1332.2 KB) | layers and the town: their maps, rewritten as a layer or the town is entered | 8 | 0 B (0%) | 0 B |
| `words` | `+0x150000`-`+0x170000` (128.0 KB) | the director's and the PET's words: text archives, labels, lists | 11 | 21.8 KB (16%) | 20.0 KB |
| `world` | `+0x170000`-`+0x330000` (1792.0 KB) | other games' world, copied as their areas are entered: tiles, songs, overworld art, backgrounds; home's other places | 7 | 60.0 KB (3%) | 60.0 KB |
| `code` | `+0x330000`-`+0x7A0000` (4544.0 KB) | other games' code homes and their per-battle rooms (issue #82): fixed places, the same at every boot with the same ROMs | 25 | 254.9 KB (5%) | 192.0 KB |
| `art` | `+0x7A0000`-`+0x800000` (384.0 KB) | art copied whole, a place each | 1 | 24.0 KB (6%) | 24.0 KB |
| `persistent` | `0x02010CE0`-`0x02011800` (2.8 KB) | persistent: only the power-on clear writes 0x02010CDC-0x020117FF (the battle sprite pool from its end) | 5 | 0 B (0%) | 0 B |
| `mode` | `0x02029A00`-`0x0202DA00` (16.0 KB) | a main mode's own while the map waits (BN6's shop, mode 0x2C, unpacks into 0x02029A00, 0x0202BA00 and 0x0202DA00 as it runs, and the map goes on after it): a moved screen's, run as a mode of its own from the map | 1 | 0 B (0%) | 0 B |
| `battle` | `0x0203FB50`-`0x02040000` (1.2 KB) | per battle: the battle's setup clears 0x0203CDA8-0x0203FFFF, and nothing else of BN6's reads or writes 0x0203FB50 on | 12 | 4 B (0%) | 4 B |
<!-- end of the table tools/free_space.py writes -->

Headroom is the zone's spans no region holds and its regions marked `free`
(`FREE_TAIL`, `RAM_BATTLE_SPARE`). The ROM copy has about 360 KB free of
its 8 MB, most of it in `code`'s two gaps (`+0x4D0000`, 192 KB, and
`FREE_TAIL`, 63 KB): the zones were laid out as features came, and the
large regions were sized generously. What a region holds is measured, not
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

## Moving regions (a proposal, not applied)

A move changes ROM addresses that players' saved emulator states point
into (`run.state` holds pointers into the layers' and the installers'
regions), so it rides with a release that bumps `LAYER_MAKE`
(`src/net/layer_make.h`), the owner's to schedule. Moving a region is a
change of its row's numbers alone, and lint holds the rest.

The proposal, measured with `--usage` on the release's scripted runs
before it is applied: shrink the regions sized far past what they hold to
their worst case with a quarter to spare, and gather what they free into
one gap per zone at the zone's end, so that each kind's headroom is one
span a new region takes from the front:

| Zone | Now | Proposed | Why |
| --- | --- | --- | --- |
| `layers` | `FREE_TILE_MAP` 320 KB, `FREE_COORDS` 640 KB | each to its largest layer's, measured, with a quarter to spare | the largest gain: a layer's maps are bounded by `BN6_TILEMAP_MAX` |
| `world` | `FREE_INDOORS` (home's places) at its end | into `layers`, after `FREE_LAN_HP` | a home's maps are a layer's kind |
| `code` | `FREE_CHIP_PAS`, `FREE_CHIP_AI`, `FREE_SWAP_RETURN` (small stubs) between the Souls' art and `FREE_TAIL` | into `engine`'s 7 KB gap | hooks and stubs are the engine's kind, and `FREE_TAIL` joins the 192 KB gap's side |
| `code` | BN4's Navis' per-battle rooms (`FREE_BN4_NAVI_ART`, `FREE_BN4_NAVI_SPRITE`) between their code and the bracket | kept, with BN4's chips' code and data placed from `+0x4D0000` | BN4's chips take the zone's largest gap, before BN4's further Navis need theirs |
| `art` | `FREE_LIB_TEAM` 360 KB, 24 KB free | kept | a place each, sized by the teammates |

Until it is applied, new regions take the gaps `--fit` names in their
kind's zone, and the zones' table above says what is left.
