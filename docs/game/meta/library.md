---
type: Design
title: "The Library, a page per entry"
description: "The chip Library as a dex over the entries: the PET's Library row opens a screen of ours, BN6's own Library to the pixel, with pages for BN6, BN5 and BN4, each game's own Library in its own order and numbers, counting what a run can hand out; the owner's calls, what each page lists today, what the vaults and the title's marks count, and what is left out."
tags: [game, meta, pet, multirom]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-12T21:00:00Z }
sources:
  - id: bn6-rom
    resource: "Mega Man Battle Network 6: Cybeast Gregar (USA), SHA-1 89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6"
    title: "BN6's chip records: Library type, number and flags"
  - id: bn5-rom
    resource: "Mega Man Battle Network 5: Team Colonel (USA), SHA-1 5f472f78d8de2df01d5039e045c043cb40969a39"
    title: "BN5's chip records: class, order key (+0x1C), the other version's mark (+0x16)"
  - id: bn4-rom
    resource: "Mega Man Battle Network 4: Red Sun (USA), SHA-1 a97e96a7da03abd70f7953328e511b9fa29179f1"
    title: "BN4's chip records: class (+8), order key (+0x1C), the other version's mark (+0x16)"
  - id: mmkb
    resource: "https://megaman.miraheze.org/wiki/List_of_Mega_Man_Battle_Network_5_Battle_Chips"
    title: "MMKB's chip lists of BN5 and BN4, read 10 November 2026"
---

# The Library, a page per entry

The profile's Library ([Phase two](phase-two.md), 3) keeps one bit a chip
id, and since [the chip registry](../../multirom/chip-registry.md) a chip
id names a chip of BN6, of BN5 or of BN4. The owner asked for it to work
as a national dex over the entries: what was acquired, per game. This is
that, designed with the game-design skill and checked against
[the core loop](../core-loop.md) (a collection off the main path, trophies
and never power; side systems may grow, the owner's call of 26 October).

## The design

The dialectic is the run's own, **what you bring against what you find**:
the Library is the record of what was found, across runs. It feeds the
meta layer alone (meta as variety, never power), and its one rule is the
run's: **one chip per name**. A page per entry restates that rule rather
than adding one: Cannon is one chip, and it has a place in each game's
Library that has a Cannon.

- **One Library, a page per entry.** PET > Library opens the LIBRARY, a
  screen of ours that is BN6's own Library to the pixel wherever BN6's
  has the part: its card at its place and size (the cut corner, the hole,
  the grip, the contacts; a Mega's cyan, a Giga's pink), the picture, the
  element and the power on the dark, the description in the game's own
  three lines; the cyan panel and the navy field of seven rows; a row's
  number, icon, name and stars in BN6's own letters and cells; the class
  between the page arrows, the count's pill, the scroll bar's capsule, the
  cursor. Measured against a capture of BN6's screen on the same save,
  pixel for pixel: of the body's 32,640 pixels 435 differ, the
  count's digits and the band's letters (ours: BN6's are fonts of its ROM
  we have not located), the left arrow BN6's first tab lacks and the
  cursor's step.
- **The entries stand on the band**, where BN6's has nothing: a tag each
  (BN6, BN5, BN4), the one shown lit with its whole count.
- **BN6's Library's keys, all of them** (the owner's word, 10 November:
  the original's quality and what it offered). Measured on BN6's own
  screen: Left and Right turn its tabs, L and R move the list a screenful
  with the cursor on its row, START leaves as B does, SELECT and A do
  nothing; it has no sort and no filter. Ours keeps each: Left and Right
  turn the pages, an entry's three classes and then the next entry's (nine
  pages in a ring, as the Jukebox runs both games' sections in one row),
  L and R page the list, B or START leaves. A tap takes a row, a tab or an
  arrow.
- **A page is its game's own Library**: its standard chips, Megas and
  Gigas in that game's order, each with the number its game's Library
  gives it. A StdChip page lists every slot, a number never filled as
  `??`; the Mega and Giga pages list the filled alone, as BN6's tabs do.
- **A chip two games name fills its slot on each one's page** (the
  owner's call 1). The profile's bit is the run's chip's, so no save data
  is added: BN5's 001 Cannon and BN4's 001 Cannon are BN6's chip 1.
- **A page counts only what a run can hand out** (call 2). A chip that
  sits out until its code is moved is not listed and not counted; a chip
  cut from a release, a DarkChip and a Program Advance never are, nor a
  dark chip of BN4's (Hole, DrkLine, ElemDark, BugCurse and the rest,
  `xreg4_dark`) while no find, reward or flame hands one out. The
  numbers are the game's own, so a gap in them shows where chips are still
  to come, and every count shown can be reached. The count grows as chips
  come to play.
- **Without a game's ROM its page keeps its slots and its count**: the
  slots are numbers in the tree, not read from the ROM. A chip that game
  alone has reads as its placeholder (`BN5-12`) until the ROM is back.
- **A class whole lights its count** (call 3): the pill turns gold, and an
  entry whose three classes are whole turns its tag's count gold.
  The title keeps BN6's own STD, MEGA and GIGA COMP for BN6's classes
  ([The title's marks](title-marks.md)): no mark needs another game's ROM,
  and none goes out when a release adds chips to a page. Trophies, never
  keys.
- **On a second screen** the Library's panel follows the cursor as it
  followed BN6's: the chip as a card the size of the Custom screen's with
  its codes and its copies in the folder and the pack, and under it each
  entry's count and what the run added.

## The owner's calls (10 November 2026)

1. Shared-name chips tick every entry's page (over: only the page of the
   game they came from, which left BN4's page empty and no page looking
   like its game's Library).
2. Unplayable chips hidden, the numbers kept (over: shown as blanks and
   counted, a total nobody can reach today).
3. A completion mark on the page (over: a mark per entry on the title,
   new art of ours that would drop whenever a page grows).
4. The PET's Library row opens it, replacing BN6's own Library screen
   whole (and made as near BN6's as ours can be: its keys, its card, its
   rows), as Options took Comm's row (over: BN6's screen kept with ours
   behind its last tab, or behind KeyItem's LibCard). BN6's page is then
   our drawing in BN6's look, counted against what a run can hold: BN6's
   own screen counted 12/200 against chips no run gives. AGENTS.md's rule
   (never redraw a screen BN6 has) carries this one exception. What it
   costs is BN6's P.A. Memo tab, until a page of ours follows (Out of
   scope).

## What each page lists today

Measured from the three ROMs and the registry by `tools/library_pages.py
--report` (10 November 2026), and BN6's from its records as the game runs:

| Page | StdChip | MegaChip | GigaChip | All | Of them BN6's by name |
| --- | ---: | ---: | ---: | ---: | ---: |
| BN6 | 200 | 39 | 10 | 249 | all |
| BN5 (Team Colonel) | 180 of its 180 | 57 of its 60 | 4 of its 7 | 241 | 104 (85, 16, 3) |
| BN4 (Red Sun) | 88 of its 186 | 23: 17 of its 60, 6 of Blue Moon's | 4: 2 of its 5, 2 of Blue Moon's | 115 | 78 (61, 14, 3) |

The BN4 row is that day's, when none of BN4's own chips played. Each of
its own that plays is listed since (below), so its page holds more than
the row says; `--report` counts the table's slots (274 for BN4, its own
160 among them), not which of them play.

BN5's other 137 are the registry's own that play (95, 41, 1). BN4's other
36 are BN5's own of its name (26, 9, 1). BN4's own 160 chips (99, 56, 5)
have their slots in the table by the registry's rows (ids 527 on), and
those that play are listed (Invis the first; 98 with the Program Advances
at the 0.21.0 freeze, 12 November 2026), but for its dark chips, which no
run can reach yet: each other sits out until its code is moved, and joins
the page as it comes to play (since 12 November
2026 one held reaches the profile's Library, as BN5's own always did:
`chip_pool_older` takes BN4's rows; [BN4's chips in a
run](../../multirom/bn4-chips.md), unit 7); LifeAura's slot is
BN6's LifeAur's, one chip under two spellings (the owner's call) ([BN4's chips in a run](../../multirom/bn4-chips.md)),
towards 186, 79 and 9. The slots the
pages lack are the chips no run has an id for (Team ProtoMan's Navi chips,
the Gigas the registry leaves out) and those that sit out.

## The order and the numbers

**BN6**: its record's Library number (`BN6_CHIP_LIBRARY_NUMBER`, +0x15)
on its class's tab, over the chips a run can hold (`chip_pool_class`).

**BN5 and BN4 carry their Library order in the record's +0x1C**
(`BN5_CHIP_LIBRARY_ORDER`, `BN4_CHIP_LIBRARY_ORDER`; BN6's IDSortPos), and
no number at +0x15 (BN5's dark gate there, 0 to 2). Verified: read over
every record of both ROMs, the named folder chips' keys run without a
gap, class after class (BN5: 1 to 180 the standard chips, 181 to 261 the
Megas, 262 to 274 the Gigas; BN4: 1 to 186, 201 to 280, 281 to 290); and
the keys are the numbers MMKB's lists give ([Sources](../../research/sources.md)):
BN5's 5 Vulcan1, 8 Spreader, 9 MrkCan1, 12 Pulsar1, 15 TankCan1, 18
WideSht1, 21 ElcReel1, 70 AirHoc, BN4's 5 Vulcan1, 9 HeatShot, 12 Bubbler,
none of which is the chip's id (AirHoc is id 5).

A chip's number is its place in its class among the chips of the version
read (Team Colonel, Red Sun). The other version's chips are left out of
that count: its Megas by bit 0 of the record's +0x16 (its six Navis'
chips and two more, 20 in each game), its Gigas by their version mark
there ([Chips, folders and the Library](../../rom/chips.md), The Gigas'
version marks: bit 2 Team ProtoMan's and Red Sun's, bit 3 Team Colonel's
and Blue Moon's). That numbers Team Colonel's Megas 1 to 60, ending at
DjangoDS, and Red Sun's Gigas 1 to 5 from RedSun. Those of the other
version a run has a chip for follow the class's own, numbered on (Blue
Moon's ProtoMan is BN4's Mega 61).

Assumed, not verified in the games: that each version's Library numbers
its Megas and Gigas so (MMKB's pages were read word for word over the
standard chips alone); that BN5's Phoenix and DethPhnx, which carry
neither mark, both count among Team Colonel's Gigas (its four on the page
are then 1 BigHook, 2 HolyDrem, 4 CrossDiv, 5 BassAnly); and that BN4's
secret standard chips (151 on, RollAro1) continue its standard chips'
numbers, where the game may list them on a tab of their own. All of BN4's
secret chips sit out today.

The run's chip in a BN4 slot is the chip the engine pays for it
(`xchips_bn4_paid`: BN6's own of its name, else the registry's of BN5's,
else by its Navi's V1 and version); the table was checked against the
engine with the three ROMs read, 113 of 113 the same (before BN4's own
rows joined it, which the engine pays as the stand-in until they play).

## Where it lives

- `src/core/library_pages.c`: a page's slots, what plays (`library_plays`:
  BN6's by its pool, the registry's own over BN6's code or moved), the
  counts against the profile's bits. `library_pages_data.c` is BN5's and
  BN4's slots (the game's chip id, the run's, its number and class),
  written by `tools/library_pages.py` from the ROMs and the registry;
  after a registry change, run it again (`--check` says whether the file
  is what the ROMs give). The unit tests hold the tables to their rules
  and to the registry.
- `src/library/`: the screen (`library.c`), its picture (`library_draw.c`)
  and labels (`library_text.c`). `pet.c`'s hook on the PET's input takes A
  on the Library row, as on Comm and Save; the game holds under the
  screen as under the Jukebox ([The PET](../pet.md), Library).
- `--dev library=N` puts every Nth chip id in the profile's Library at the
  start (`devtools_veteran`), for its pictures: the screen itself reads no
  dev switch ([the layers](../../code/layers.md)).

## What counts the Library afterwards

- **Vaults and the LibCard** ([Phase two](phase-two.md), Gates): the whole
  Library as before, every chip a run can hold once, whatever pages it
  stands on (`meta_library_count(-1)`). A vault's lock is unchanged.
- **The title's marks**: BN6's three classes, as before.
- **The summary's Library row** and "New this run": every chip once.
- **The second screen's "Older Net" count** is gone: BN5's page is its
  count.
- **The game's own Library flags** are still given to each run's game
  (a Chip Trader reads them for a prize new to the Library).

## Out of scope

- **Program Advances.** BN6's P.A. Memo went with its Library screen, and
  the pages hold folder chips. A P.A. page per entry (the profile already
  keeps a formed Advance's bit, ids 320 on) with its recipe is the next
  piece.
- **DarkChips**, which BN5 and BN4 list on a tab of their own.
- **BN4's own chips on the page**: their slots are in the table, and
  each is listed as its code is moved (`library_plays` reads its row).
- **The count's digits and the band's letters as BN6's** (two fonts of
  its ROM not located yet; ours are the engine's heavy capitals), and the
  title's second screen.
