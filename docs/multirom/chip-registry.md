---
type: Engine Mechanism
title: "The chip registry"
description: "BN5's 257 chips in BN6's chip tables grown to 512 (`src/core/xchips_registry.c`, `chips.c`): a chip both games name stays BN6's, the rest take free ids, with placeholders without BN5's ROM and the pack extended in EWRAM."
tags: [multirom, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-13T09:50:43Z }
---

# The chip registry

Issue #82's C1, the ground call 4 stands on: Team Colonel's chips in
BN6's own chip tables, so the pack, the folder, the Custom screen and
BN6's battles take them by their ids as they take BN6's, the same ids at
every boot, with BN5's ROM or without it. What is registered is the
finer calls' subset ([One net](one-net.md)). Those that play in BN6's battles
come into runs' finds, shops, traders and rewards since D1 (One net); the
chip step gives any one ([Dev tools](../devtools/overview.md)).

- **The registry** (`src/core/xchips_registry.c`, numbers only): 257 of
  BN5's chips, each with its id in BN6's tables and how it comes in. One
  chip per name: the 116 BN6 has by name are BN6's own (its id, record
  and code), and so are three its US release names otherwise (Attck+10
  and Attck+30, BN6's Atk+10 and Atk+30, which BN6's hand merges into the
  chip before them by their ids; PitHoky, BN6's PitHocky). The other 138
  take ids of their own: BN6's records no chip uses (203-220, 291-300,
  314 and 350-367: blank-named, named by none of its tables, docs/
  [The chip registry and BN5's chips' code](../rom/chips-code.md), The chip registry) and 411-511, past its 411 records.
  203-207 and 211-213 are the eight BN6's code already plays under other
  names (AirHoc over AirHocky's family; WideSht1-3 over WideSht's;
  Spreader over Spreadr1's; Geddon1-3 over Geddon's, which cracks, breaks
  or poisons by its first parameter as BN5's three do), 208-210 DrilArm1-3
  (C1 played them over BN6's DrilArm family; BN5's own drill since, moved:
  The chips' code), 214-220 the seven DarkChips, 291-294 the Megas (Z
  Saver among them: BN6's Z Saver is a pseudo-chip of its battles, which
  no folder holds, so BN5's has an id of its own, its code BN5's, moved),
  295-300 the first chips to be moved (Guard1-3, CrakOut, DublCrak,
  TripCrak: the three crack chips moved, [The chips' code](chips-code.md)), 314
  HolyDrem beside BN6's Gigas, 350-364 the fifteen
  Program Advances beside BN6's, 411-504 the rest, the Navi chips last
  (489-504; Colonel and Colonel SP are BN6's). 365-367 and 505-511 stay
  free. A row of its own keeps its codes, library type and MB: what its
  placeholder needs. The pairs were made by name from both ROMs, which
  their SHA-1s fix; the unit tests hold the table to its rules
  (`xreg_check`: every BN5 id once, every own id once and free, every
  BN6 id one of BN6's folder chips or Program Advances, every twin one of
  BN6's records). Of BN5's other chips of a name BN6 has, none is in the
  subset (Team ProtoMan's GunDelEX and Otenko, the Navi chips of Navis
  who guard no run, BN5's pseudo-chips).
- **The tables, one place to read them** (`src/core/chips.c`): BN6's
  records cannot grow where they are (other data follows them), so all
  512 are made anew from the player's ROMs: BN6's 411 as they are, the
  registry's at their ids, a blank record at every id past them; the
  names' and the descriptions' text archives for ids 0-511, BN6's own
  entries copied and the registry's in BN6's text codes (BN5's read
  through `xrom_text_bn6`, its descriptions' box commands kept); BN5's
  icons, pictures and palettes, a picture or palette two chips share
  copied once. The same bytes go into the core's copy (below) and are
  what the run's code reads: every reader of a chip's record, name,
  description or art (data.c's chip info, the chip pool, BN6's
  DarkChips' power, the guardians' rarity, the second screen's cards and
  icons, gfx.c's tiles, the All * helper's codes) takes them from
  `chips_record`, `chips_name`, `chips_desc` or `chips_bytes`, never
  from the player's file; what writes into the records in the core's
  copy (the All * helper, BN6's DarkChips) writes where the game reads
  them (`chips_table_bus`). 174560 bytes with BN5's ROM, 69632 without.
- **A record field by field** (`src/core/xchips_record.c`): BN5's
  record has BN6's layout, its numbers taken as they are (codes,
  elements, rarity, type, MB, parameters, lockout, power, gate) but four:
  its icon element (+6) as BN6's (BN5 numbers its Recovery, Invisible
  and the rest otherwise: measured over the 151 chips both games name
  alike), its lock-on type (+0x17, another meaning in BN5) and its family,
  subfamily and lock-on from its twin (the BN6 chip whose code plays it),
  its Library number and flags 0 (out of BN6's Library screen: the
  finer calls' Library), its ID sort key its own id, its ABC place after
  the BN6 chip whose name comes last before its own, no DarkChipID. A
  power BN5 computes (1001 and up, an index into BN5's own table, past
  BN6's) is shown as none. A chip whose code BN6 lacks keeps an inert
  family (BN6's blank records', 0x1C sub 3), no lock-on and no
  parameters. Its icon is drawn in BN6's icons' palette (BN5's outline is
  grey where BN6's is teal; its other colours the same or BN6's nearest).
- **Without BN5's ROM, placeholders**: the registry's chips keep their
  ids, codes, library type and MB, so a saved folder or pack never names
  garbage and the folder's rules (MB, Mega and Giga counts) still hold;
  each reads "BN5-<its BN5 number>", its card "Needs BN5 / ROM back / to
  battle", Null, BN6's blank art. BN5's ROM put in later (the ROMs
  screen) brings its chips in for theirs as a game is next entered: the
  image is made again and written over the placeholders'.
- **The install** (`src/emu/xchips_install.c`), at every start of the
  core before the boot, all or nothing (every word and halfword it
  changes read first and BN6's own or already its; its place in the free
  space 0xFF or already its): the image past `EMU_FREE` + `0x400000`
  ([Running the game's own logic](../engine/emulation/overview.md)), getChip's one literal pointed at the 512 records,
  the 23 words that name the four text archives pointed at the new ones,
  and BN6's pack routines split so ids 320-511 count in an extension of
  the pack in EWRAM BN6 leaves alone (`0x02010CE0`-`0x020115DF`, 12 bytes
  an id: cleared at power-on with the rest of EWRAM, held by every state
  and so by `run.state`): the count's and the total's lookups, the pack's
  totals, its list in the folder editor (to id 511) and its clear (the
  folder editor's exit clears the pack and gives its list back: a clear
  that left the extension counted it twice). BN6's support and Navi
  chips tested a pseudo-chip by its id (every id from 0x170): the test is
  kept to its own (0x170-0x19A), so a chip from 411 is none. Every write
  and its original bytes: [The chip registry and BN5's chips' code](../rom/chips-code.md), The chip registry.
- **Sitting out** (a hook at the end of the battle deck's deal, bn6f
  `sub_800A3E4`): the chips that sit out are taken out of the deck, the
  rest kept in their order (the Regular chip's and the TagChips' marks
  cleared where theirs went). They are the registry's chips whose code is
  not moved yet (pending; a moved chip, `XREG_MOVED`, plays where BN5's
  ROM is read, its code installed; none pending since issue #82's final
  engine round), BN5's DarkChips but where BN5's dark rule runs (the
  Souls installed: shelved, and dealt by the worried rule, Transplanted
  Souls, S7), its Program Advances (formed on the Custom screen from
  BN6's list grown with their recipes, never dealt, as BN6's own), and
  every one of BN5's where its ROM is not read. A run
  continued without BN5's ROM whose folder holds one of BN5's chips hears
  MegaMan say so once, as he stands free on the map ("Some of our chips
  came from the older Net...", `nets_words.c`).
- **The ids BN6's code tests**: every literal and immediate of a chip
  bound in Gregar (411, 0x170, 0x140, 0x13F, 0x13A) and every range test
  (221-280, the Navi chips) read in bn6f; the pack's bounds and the
  pseudo-chip test patched, the rest out of the registry's way or out of
  a run's reach (the link battle's copy of the pack), and the tables an id
  indexes (the ownership marks and their keys, the Library's flags) sized
  for nine bits by BN6 itself: [The chip registry and BN5's chips' code](../rom/chips-code.md), The chip registry.

Checked: the unit tests (the registry's rules, records made from made-up
BN5 records, placeholders, the ABC place, the icons' colours, the deck's
sit-out, the image's archives read back); in the game, WideSht1 L on
BN6's Custom screen with BN5's picture (L, Aqua, 70), fired at a Mettaur
(BN5's water wave over WideSht's family, the Mettaur deleted), MrkCan1
(id 411) in the PET's Chip Folder and in the folder editor's pack (its
count in the extension, the same after the editor is left and opened
again) with BN5's description, a folder of 29 MrkCan1 dealing a hand of
its one other chip, and a run saved with BN5's chips continued without BN5's ROM
(its placeholders in the folder, three sitting out of a battle, MegaMan's
words); the autopilot's seeds 1-12 the same, run log for run log, as
before the tables moved.

Not yet, each its later milestone's: CustSwrd's power (BN5's computed power 44,
past BN6's table) and the AntiFire, AntiWatr, AntiElec and AntiWood
traps (BN6's traps trigger by chip id and it has no elemental kinds);
BN5's DarkChips and its Program Advances came in issue #82's final
engine round (the dark rule with the Souls, the recipes in BN6's list:
the battle's Program Advance bits at `0x0203CA48` hold ids to 383 on
side 0, the link list, side 1's, BN6's own: [The chip registry and BN5's chips' code](../rom/chips-code.md), BN5's
Program Advances in BN6's lists); its Navi chips (M7: AntiNavi's
range, 221-280, and the battle's record of the last Navi chip at
`0x0203C960`); the Library (M9: BN6's Mega, Giga and P.A. tabs by their
own numbers; the profile's Library of 512 ids and the second screen's
came with D1, One net); the folder editor's pack list, 0xA000 bytes, holds
1280 chip-codes where ids to 511 could make 2044 (no run comes near); the
order stamps' clear at a NEW GAME covers BN6's 320 ids (the extension's
start at 0 from the power-on clear); the link battle's copy of the pack
(0xF00 bytes, no run reaches it).
