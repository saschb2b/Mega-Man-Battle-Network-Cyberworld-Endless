---
type: Tool
title: "The chip recipe generator"
description: "`tools/xchiprecipe.py` derives the code of BN5's chips from both ROMs and the symbol map, writing `src/emu/xchips_bn5.c`, `xchips_moved.c` and the Program Advances' `xchips_pas.c`, with `--check` and a `--report` of every unit; `tools/xchiprecipe_bn4.py` derives BN4's units into `src/emu/xchips_bn4.c`."
tags: [devtools]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-11T13:10:00Z }
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
of BN6's before them never let form, each its reason), and BN6's own
recipes whose parts a run never hands out, their parts the registry's
chips of BN5's same ids (`BN6_PA_BORROWED`: CrosOver from Django's V1,
SP and DS, [BN6 Gregar's own gaps](../multirom/bn6-gaps.md)). A twin is also held to read the same data:
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

## The BN4 path

```bash
python3 tools/xchiprecipe_bn4.py           # write src/emu/xchips_bn4.c (its UNITS, each whole) and .build/xchiprecipe/bn4_report.md
python3 tools/xchiprecipe_bn4.py --check   # write nothing: fail where src/emu/xchips_bn4.c is not what it would write
python3 tools/xchiprecipe_bn4.py --report --unit support:0x19:ShotStar   # derive more units for the report alone
```

BN4's chips are derived by a tool of their own, `tools/xchiprecipe_bn4.py`,
beside this one rather than inside it: `tools/xchiprecipe.py` rests on
BN5's symbol map, its twins and its numbering throughout, and BN4 has
none of those, so BN5's three files stay what they were byte for byte
(`--check`). The BN4 tool stands on the parts that moved BN4's families
and Navis ([The recipe generator](recipe-generator.md);
`tools/xrecipe_bn4.py`'s walk, pairs and blocks,
`tools/xnavirecipe_bn4_objects.py`'s moved fourth kind,
`tools/bn4_twins.py`'s pairs and audits), and writes what the same
installer takes (`XChipUnit`, `src/emu/xchips_code.c`).

- **A unit.** BN4 runs a chip by a state of MegaMan's. Its support state
  (family 0x0C, `BN4_SUPPORT_RUN`) runs support chips and Navi chips
  from one table by subfamily (`BN4_CHIP_ROUTINES`, 0x4E routines). A
  unit is one of them: a spawner of a fourth-kind object, held to the
  sixteen instructions of BN6's support routines (`0x080E8886` the
  control, read each run), in the region of the type it spawns. That
  region is the unit's first block, walked from the type's routine and
  from the spawner. A routine of another shape, or of another family, is
  a decision: the misc family (BN4's state 0x20) is not read yet.
- **A misc unit.** BN4's misc state (family 0x20, `BN4_MISC_RUN`) is
  BN6's family 0x1C instruction for instruction, each with a table of
  routines by subfamily (`BN4_MISC_ROUTINES` 0x12, BN6's 7). A unit of
  kind `misc` is one of BN4's routines there: copied as a block of its
  own where it is a routine by itself (Repair's), its number 0x80 + the
  subfamily in BN6's misc routines grown (`XC4_MISC`). One that lies in
  a third-kind type's code moves that type with it (Counter's, in its
  hit's). `IMMS` holds the
  immediates of a unit that name a place of BN4's engine BN6 keeps
  elsewhere, each read by hand in both games and checked against the
  instruction (the battle state's obstacles, +0x68 and +0xA0).
- **An attack unit.** A chip of neither state names a state of MegaMan's
  of its own by its record's family (`BN4_PLAYER_STATES`: the spread
  chips' 0x1E). A unit of kind `attack` is that state whole, its routine
  to the next state's, every subfamily of it: its phases, its bytes by
  subfamily, the arm and the shot it spawns moved with it. BN6 runs a
  family with r7 the attack's variables, which BN4's state makes itself:
  the immediate that does is given BN6's place (his AI data's +0x70,
  BN6's +0xA0), and any other field of his AI data is a decision until
  both games' use of it is read (`MEGAMAN_AI`). Its number is one of
  BN6's grown attack families the chips of BN4 have for their own
  (`ATTACK_NUMBERS`, 0xD0 on, `XATTACK_BN4`), fixed as the unit is
  written; its chips' rows name it in `sub` (0xD0 or over is a family,
  the record's subfamily BN4's own), a chip of the family another
  session moves waits (`ROWS_WAIT`), and its Program Advances stay
  `XREG_PA`. MegaMan's animation, each art's own number, and a place of
  BN4's NaviStats are hand-read immediates (`IMMS`). A kinds' table
  whose rows name an effect and a sound by a byte (`ROW_TABLES`: the
  spread shot's) is copied with the unit, each row of a kind its chips
  play given BN6's id of the effect (of the same look, or moved) and a
  slot a byte can name for the sound (`FAMILY_SLOTS`); the arm's kinds'
  table keeps a row's sprite list beside its number.
- **Its calls.** Each call out is paired with BN6's twin: the project's
  pairs (the families', the Navis' and the tool's own `TWINS`, each with
  its audit where the bodies differ), else a BN6 routine of the same
  body. The five generic states of a support chip's object, which every
  such type's tables name, are BN6's own at the same places of its
  types' tables. A call into another object type's code whose twin the
  search found by its body alone is a decision until its type is
  compared to its depth (`TYPE_TWINS`), the census's lesson ([BN4's chips
  in a run](../multirom/bn4-chips.md), The six "on BN6's code" are not).
- **Its objects.** A third- or fourth-kind type it spawns that BN6 has
  no twin of is moved with it, its region a block, its spawn through the
  second-level dispatch at a number of the chips' own: the fourth kind's
  from 0x91 (past BN5's 145 types, below the families' and Navis' 0xE0),
  the third kind's from a list of numbers no recipe names (`TYPES3`: the
  few past BN5's 204 first, then 79 of BN5's own numbers that no BN5
  track moved or paired, found by a scan of every recipe's rows). No other recipe may name a
  number it takes (`numbers_clear`).
- **Its copies.** A kinds' table an object reads out of its region
  (`TABLES`: the star's) is a block of the unit, each spawned kind's
  sprite copied (list 3 from 0x70 up), its
  byte in the copy set. An effect BN6 draws otherwise is moved to an id
  of the chips' (0xCF down to 0xB0) with its sprite (list 5's 0xEF down,
  list 4's 0x9F down). A sprite of list 1, the viruses' list, takes a
  number from 0x7F down to 0x40 (MokoRus's Moko the first). A type spawned through a wrapper of its code is
  moved as one spawned outright (`moved3`).
- **Its numbers.** The element's place (BN4's +0x17, BN6's +0x0E), the
  collision data's offsets, a panel's type and side after `getPanel`, a
  sprite load given the number of BN4's sprite's copy (every sprite a
  unit draws is copied, as the families' are), an effect's id by its look, a
  sound BN6 has no song of given its slot of BN6's grown song table
  (`SOUND_SLOTS`, from 0x1DA up, the same numbers as
  `src/audio/xsong.c`'s; `src/emu/song_table.h`) and played through a
  shim of the unit's home that names it, a number past a byte; one
  without a slot is a decision, the next slot its answer.
- **Where it goes.** `FREE_BN4_CHIPS` (`src/emu/xchips_bn4.h`): 96 homes
  (veneers, then shims), then code, then art. Its entry is the spawner's copy at
  BN6's grown support routines' 0x80 + BN4's subfamily (BN6's own are
  0-0x29, BN5's from 0x2A). `xchips_bn4_code_install` stages it where
  BN4's ROM is read; the stubs and the two grown tables are staged by
  whichever game's units install first.
- **Its chips.** The registry's rows of the unit's chips must be
  `XREG_MOVED` with the unit's number their subfamily
  (`tools/xreg_bn4.py` `MOVED`), and a dark chip's row among the
  registry's dark rows (`xreg4_dark_ids`, which the price's hook reads:
  [DarkChips](../game/meta/darkchips.md)): the generator refuses to
  write otherwise. Such a row's record takes BN6's support
  family and that subfamily, its parameters, lock-out and power BN4's
  own (`xrec_from_bn4`).

Proved by a chip's specs against BN4's own battle ([The
differential](differential.md), A BN4 chip's spec); a mutation of a
unit's code names BN4's address, which the runner pokes at its copy.
A unit of several subfamilies (`UNITS`' tuple: WhitWeb's 8, 9 and 0xA)
is one block with a routine each: the first is the unit's entry, each
other a row of the units' table that shares its code (`XChipUnit.share`),
its entry alone.

`UNITS` holds PanlSht (support 0x26), ShotStar (0x19), Repair (misc
0x0B), Counter (misc 0), WhitWeb (misc 8, 9, 0xA), Boomer (misc 2) and
MokoRus (misc 1), SidBmbo (misc 5) and Spreads (attack 0x1E); FlmLine
is next of the attack families.

A misc unit BN4's run waits for carries the frames (`misc_wait` reads
BN4's run at `MISC_WAIT4`: the subfamily it compares and the count it
sets; SidBmbo's 0x28), written as the unit's row's last number. The
installer then puts a wrapper at the end of the unit's home
(`xchips_code.c` `wait_stage`, 0x40 bytes of the project's own Thumb) as
the table's entry: BN6's misc run calls it every frame of the wait, it
calls the unit's routine the first time and counts as BN4's run does,
going on into BN6's run where the attack ends. An effect an earlier
unit moved is left to that unit's rows; the later unit's code names its
id by an immediate alone.

Several sessions add units side by side, so what a unit is given follows
one rule. `UNITS` is append-only, one unit a line: a unit's home, its
dispatch numbers, its effects' ids and its sprites' list numbers follow
its place there, so the branch merged first keeps its places, the
other's units go after and the file is written again, never joined by
hand. A sound's slot is the one number given out by name (`SOUND_SLOTS`,
`XSOUNDS`, the differential's sound rows), so each session takes its
slots from a range of its own, listed above `SOUND_SLOTS`. A mutation
pokes BN4's address or a registry record, never a home's address, which
moves with the units before it.

An immediate that names a meaning the engines keep in different bits is
an `IMMS` row like an offset's: MokoRus's hit's bits, struck and pushed
back, are 0x21 in BN4 and 0x61 in BN6 (`BN4_STRUCK_PUSHED`,
`BN6_STRUCK_PUSHED`), found by the differential at the hit's frame, the
struck virus going on with its attack in BN6's.
