---
type: Tool
title: "The chip recipe generator"
description: "`tools/xchiprecipe.py` derives the code of BN5's chips from both ROMs and the symbol map, writing `src/emu/xchips_bn5.c`, `xchips_moved.c` and the Program Advances' `xchips_pas.c`, with `--check` and a `--report` of every unit."
tags: [devtools]
generated: { by: claude-code/unrecorded, at: 2026-10-12T23:50:43Z }
---

# The chip recipe generator

```bash
python3 tools/xchiprecipe.py           # write src/emu/xchips_bn5.c (the units) and src/core/xchips_moved.c (their chips)
python3 tools/xchiprecipe.py --check   # write nothing: fail where the two files differ from what it would write
python3 tools/xchiprecipe.py --report  # every unit and candidate derived: .build/xchiprecipe/report.md
python3 tools/xchiprecipe.py --report --unit attack:0x4f:DrilArm support:0x31:Poltrgst   # more candidates
```

`tools/xchiprecipe.py` (issue #82's C3; [The chips' code](../multirom/chips-code.md), The chips'
code) derives the code of BN5's chips on the recipe generator's parts:
a unit is one of BN5's attack families (by the family a chip's record
names) or one of its support or Navi routines (by subfamily), walked
from the routine its table names with every routine and object it
reaches. It needs the symbol map (`build.py symbols --bn6f DIR`). On top
of the family deriver's rules: a twin of an object's routine must be the
same code to its depth (its states and substates too, a pair the project
audited as serving counting as the same) and load sprites of the same
bytes by its numbers (each game's sprite lists read), else BN5's is
copied; a sprite moved code loads that BN6 keeps another of is copied to
one of the chips' numbers of its grown list (lists 1-5, from
`0xFF` down; the families' from the bottom up; a sprite the moved objects
or the Navis' shared art place already, installed before the chips' code,
drawn at their number, one BN5 sprite one number), the load set to it (an
immediate, or the low byte of the packed word BN5's 0x0800F198 takes); a
table's word aimed at a twin whose body differs is a decision unless
audited (the symbol map's pair of BN5's 0x0800A2F0 was another routine);
a raw table's words that point into the unit's blocks move as a pool's
(CrakOut's crack patterns); an
object type BN6's twin would spawn BN6's own of, where BN6 runs other
code for it, is moved (MetaGel's thrower, whose gel's sprite is another
in BN6); `TYPE_AUDITS` names BN6's own objects every attack of both
games spawns (the hit area, the third kind's 3, and the effect, the
fourth kind's 0), `FORCE_COPY` and `FORCE_MOVED` the routines and types
the differential found reading BN6's data (the shockwave's kinds); a
pool load whose pool lies past a routine no walk reaches walks that
routine too (one block holds both); a dispatcher's state table counts
its far entries (the engine's own end state). Numbers are fixed by
BN5's: its attack family F is BN6's 0x50 + F, its support subfamily S
the grown support routines' 0x2A + S, its Navi subfamily S the Navi
routines' 0x1D + S; homes and code in `UNITS`' order, a new unit after
the last. `UNITS` are the ones written, each whole and its chips' specs
passing; `CANDIDATES` are derived for the report alone, each with what
it still needs. `src/core/xchips_moved.c` takes the registry's
`XREG_MOVED` rows of the written units, the Program Advances and the
DarkChips of `DARK_MOVED` whose code moved (their records take the
moved family; a Program Advance is formed, never dealt, and a DarkChip
comes by BN5's dark rule, the Souls'), and BN6's own chips of a name
whose code is BN5's, `SAME_MOVED` (DarkPlus: BN6's record of it takes
BN5's family and parameters where BN5's ROM is read): a unit's chips
must be `XREG_MOVED` (the generator refuses to write otherwise; a record
of a subfamily the unit plays none of, and `ROWS_WAIT`, none now,
excepted), and a chip whose unit is not written stays `XREG_PENDING`,
sitting out (none since issue #82's final engine round). It writes
`src/core/xchips_pas.c` too: BN5's recipes of the registry's Program
Advances (BN5's list `0x08028020`) in BN6's ids, for BN6's list grown
with them (`pa_rows`; `PA_UNFORMED` the recipes BN6's codes or a recipe
of BN6's before them never let form, each its reason). A twin is also held to read the same data:
the tables its literals name, where they are plain bytes in both ROMs,
must be the same (`same_data`: DrkSonic's sonic's times by kind its BN6
twin keeps otherwise).

What else a unit is given, each a table of the generator's with its why:
`HELPERS`, BN5's code several units call whose object one copy alone may
move (the buster arm, the shake), derived first and installed once
before the units at `XC_HELPERS`, every routine of theirs a unit calls
aimed at their copy; the muzzle flash is BN5's image object, moved once
for every track with the Navis' install (`tools/bn5_objects.py`): a
unit's call into its module aimed at that copy, its type spawned as the
moved objects' (`XV_TYPE_SHARED`), and each flash number the units give
it (by one instruction or `FLASH_NUMBERS`, read from each call site) one
`tools/bn5_objects.py` fixes, its sprite at a number of the chips' there
(`flash_kinds`: an unfixed one a decision); `STUBS`, routines aimed at the chips' own stubs
(a panel claim's spawners, proved inert: [The chips' code](../multirom/chips-code.md), The chips'
code); `COPY_HELPERS`, BN5's engine routines with no twin copied with a
unit (the swords' reach); `DATA_SIZES`, raw tables longer than the walk
can tell; `DEAD_CODE`, unreached bytes no word, call or branch names;
`TABLE_SPRITES`, `TABLE_EFFECTS` and `TABLE_PANELS`, an object's kind
table whose rows' sprites, effect ids or panel types are given BN6's or
the chips' numbers by their bytes (`XvByte`); `MEGAMAN_LOADS`, MegaMan's
own sprite loaded by a unit (each game's own); `SOUND_SLOTS`, the song
slot each copied sound takes (the shared allocations'; the chips' own
from 0x52 down; a sound compared with BN6's of its number by its music
player too), its immediates set to it and `src/audio/xsong.c`'s
`xsong_install_at` copying the song as the units install; effects whose
entry differs take BN6's of the same sprite, animation and palette or an
id of the chips' (0xD0-0xFF, from the top down), or the Navis' number
of the same BN5 effect where their install made one (one BN5 effect one
number); BN5's lava, sea and holy
set or compared are BN6's 13, 14 and 5. `HELPERS` holds the computed
powers' routines too (BN5's table of them `0x0800E728`: BN6's grown at
`XC_POWERS`, BN5's entry i at 0x80 + i), `UNIT_STUBS` calls one unit
aims at a stub (ElemRage's point 0x14 and HolyDrem's 0x1D answered
with BN5's numbers), `WORDS_TO` a literal of BN5's RAM a unit names that
BN6 keeps at a place of the chips' own (the SP powers' best deletion
times, `XC_SP_TIMES`, a table of none), `AFTERIMAGE_SPAWN` the
afterimage's spawner, whose packed word a unit hands it names a sprite
(DarkWide's wave's), `TABLE_ARGS` the routines a unit
hands a table of routines to (an obstacle's actions: walked whole),
`TABLE_SOUNDS` and `EFFECT_CALLS`, `EFFECT_RANGES`, `PANEL_LOOKUPS`,
`COLL_SUMS` and `POINTS` the numbers a unit gives by a table's row, an
engine routine's register, a run by kind, a lookup, a collision data's
offset and a point of MegaMan's sprite ([The chips' code](../multirom/chips-code.md), The chips'
code, has each), `SUB_TABLES` a family's routines by subfamily (the
subfamilies no moved row plays aimed at a stub, not walked),
`NAVI_SIX` the Navis' battle sprites a Navi chip draws (BN5's list 2's
n as BN6's `0x20` + n - 7, the Navis' one copy, which the unit's install
asks `xnavi_sprite_install` for: the unit's `navis`). Every object type
a written unit spawns through the dispatch must have its decision,
moved or twin, in `tools/bn5_twins.py`'s `TYPES` (one for every track):
the generator refuses to write where one is missing or another; a moved
type `TYPES` gives the Navis (the moved objects' or a Navi's copy,
installed before the chips' code: `src/emu/xinstall.c`) is spawned as
theirs (`XV_TYPE_SHARED`, none of its code copied: the Navi chips'
attacks run the Navis' own), and the families after them spawn the
types `TYPES` gives the chips the same way. A twin whose audit serves
BN5's own body (`copy`) is copied as the families' is; a number a unit
keeps is not audited by `KEPT` (the units are proven frame by frame, as
the Navis are); a gap between a unit's code and its pool keeps its
routines its own (`join_pool_gaps` off: the families' join zeroes them).
A call of a unit's into the code of a type it spawns as a Navi's copy
goes to that copy (`navi_modules`, read from `src/emu/xnavi_bn5.c`: the
Navi's block holding the type's routine, the routines its code reaches
there); `NAVI_AI_CALLS` the routines a Navi chip's object hands a Navi's
AI index (its companion begun and ended: BN6's index for BN5's Navi
there, 32 + n - 7; `NAVI_TRIPLES` the companion's triples whose third
table is the same as their second in both games, ColonelDS's object's
call with the third given the second's kind); `OWN_STATES` the CurActions an object writes into a
states table of its own, kept BN5's; `POWER_BASES` the bases a computed
power's routine subtracts from its record's power, BN6's records'
numbers; `ROWS_WAIT` the registry's chips a written unit plays that
still sit out, by BN5's id, each its reason (none since the Navi chips'
SP and DS play);
`FORCE_TWIN` a routine aimed at its BN6 twin though their tables differ
(the bomb's update, whose BN6 twin runs BN6's actions table, its status
states numbered as BN6's generic states write them).
