---
type: Design
title: "BN4's chips in a run"
description: "Phase 4 of BN4 Red Sun in a run, designed and scoped, its pairings (unit 0), BN5's first batch and ten-bit chip ids (unit 1) built, an id past 511 whole to 602: the census of BN4's 388 chip records against BN6's and BN5's registry (191 new to a run, 6 on BN6's code, the rest in 66 units of BN4's code), chip ids past 511 by ten bits for BN4's chips and BN5's alike (BN5's ShadeMan and BlizzardMan chips at 505-510, 511 held), the design (BN4's own chips found on BN4's ground, by its own records), the online rule, the build plan in units with their proofs, and the owner's calls."
tags: [multirom, bn4, chips, design]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-10T18:30:00Z }
sources:
  - id: bn4-rom
    resource: "Mega Man Battle Network 4: Red Sun (USA), B4WE, SHA-1 a97e96a7da03abd70f7953328e511b9fa29179f1"
    title: "The owner's BN4 Red Sun ROM, read as docs/rom/bn4.md says"
  - id: bn6-rom
    resource: "Mega Man Battle Network 6: Cybeast Gregar (USA), SHA-1 89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6"
    title: "BN6 Gregar's chip records, attack families and code, the pairs' index of its routine bodies"
  - id: registry
    resource: "src/core/xchips_registry.c at dev 3a4309e1"
    title: "BN5's chips as the registry holds them"
---

# BN4's chips in a run

Phase 4 of [Battle Network 4 in a run](bn4.md): BN4 Red Sun's own chips
in BN6's tables, designed and scoped so building can start. Built so far:
the pairings (unit 0) and BN5's first batch in the free ids (Past 511). The census is read from both ROMs (scratch reads, not kept;
How the census was made, below), the design reasoned with the
game-design skill and checked against [the core loop](../game/core-loop.md),
the scoping by the reverse-engineering skill's Locate and Understand.
What BN5's chips took is the road: [the chip registry](chip-registry.md)
and [the chips' code](chips-code.md).

## Where things stand

**Unit 3, under way** (10 November 2026; [The differential](../devtools/differential.md),
A BN4 chip's spec): BN4's own battle takes a folder and plays a chip
from it (`folder4`, each chip through `BN4_CHIP_GATE`; `cust4`, its
Custom screens' keys; `karma4`, MegaMan's karma), proved by
`bn4_chip_cannon_control` and `bn4_chip_hole_control`, each with a twin
broken on purpose. The comparator has a MegaMan mode for BN4
(`"subject": "megaman"`: a difference of his or the world's fails the
spec), with his numbers read as BN6's: his action by the families both
games pair, the chip's id by the registry's rows and the spec's folders,
his attack's subfamily by the two records, and what BN6 alone keeps (its
count of chips held, its attack's entry number, the frame its support
run holds him after the chip's routine) each explained in
`tests/xdiff/legit.json`'s `bn4` section from both games' code. **The first
chip passes:** `bn4_chip_areagrab`, BN4's AreaGrab against BN6's over
300 frames, three mutations caught (the pairing by name, unit 0's,
proved for it). It took MegaMan's own clock (BN6's support run holds
him a frame longer, until a hit puts him back on the battle's), the
grab's sounds paired, BN6's sound as a support chip's name shows, and
BN6's panel's side copies read as its code writes them (+4 the field's
start's, +7 the side each frame). Two specs record where a chip parts:
`bn4_chip_cannon` (t33: BN6's Cannon is its own code) and
`bn4_chip_timebomb` (t32: the records apart; on BN6's code its placement
at t34). Open: the generator's BN4 path (`tools/xchiprecipe.py` reading
BN4), which the moved units need.

**The six "on BN6's code" are not** (10 November 2026, read in both ROMs
and run): the census compared each routine's body and stopped at a twin,
without its callees. Hole and DrkLine make BN4's Hole panel, a panel
BN6 has none of, and are dark chips (below); TimeBomb's object is BN6's
by body but places its bomb by BN4's own routine (`0x080E4DE4`, 55
instructions, against BN6's `0x080E45D0`, 14); LifeAura's and BlakBarr's
barrier reads its kind through a table of BN4's where BN6 takes the
record's parameter (BN4 `0x080E5784`, BN6 `0x080E4E3C`), as every
barrier and panel chip both games name holds another parameter in each
(HolyPanl 6 and 4, Barrier 0 and 1, Barr100 1 and 5). Unit 4 as planned
falls away: each of the six is a unit of unit 5, its code BN4's moved,
or a record on BN6's code with its parameters mapped, whichever its
spec proves. Ball, RollSP and RollDS are not yet read this deep.

**BN4's light and dark chips** (verified in the ROM, 10 November 2026):
a chip's record holds its alignment at +0x15 (`BN4_CHIP_ALIGN`: 0 any,
1 light, 2 dark). As a chip is used (`BN4_CHIP_USE_TEST`, `0x0800D5D6`)
a dark chip plays only where MegaMan's karma (`BN4_NAVI_KARMA`) is under
470 and a light chip only at 470 or over; otherwise the dud plays (a
puff, family 0x20's subfamily 4). 63 records are light (the SP Navi
chips, HolyPanl, Snctuary, the barriers) and 31 dark (Static, Hole,
DrkLine, Muramasa, Anubis, ElemDark, BlakWing, the DS Navi chips, four
past 300). A run takes no karma (The DarkChips, below), so each plays
for any MegaMan; the differential's BN4 battle is given the karma its
chip asks (`karma4`). What Hole's panel is for is not yet read (the
players' account, that a DS Navi chip needs one on the field, is a lead
for unit 6).

**Unit 2, BN4's registry rows, built** (10 November 2026; The build plan):
194 rows, written by `tools/xreg_bn4.py` from the three ROMs into
`src/core/xchips_registry_bn4.c`, a table of their own beside BN5's
(`XChipReg4`, `xreg4_of`, `xreg4_at`), so that what walks BN5's rows meets
none of them. The tool mirrors the engine's pairing (`pair_bn4`) on the
names in BN6's codes: 175 folder chips have no chip of the run's (the
census's 172 less ShadeMan's SP and DS, which BN5's rows pay since, plus
SerchMan V1, CannMode, SwrdMode, FinalGun and BugCurse, the names BN5 has
unregistered; BugCharg none, ProtoMan's DS none) and 19 Program Advances.
The ids, fixed for good: **527-602**, the 76 with a pack entry today, go
to the chips that play first in this plan's order (Hole, DrkLine,
TimeBomb, LifeAura, BlakBarr, Ball, RollSP and RollDS; the Navi chips of
GutsMan, ThunMan, WindMan, FireMan, SerchMan, AquaMan and WoodMan; the
fifteen of the units under 300 bytes; then SidBmbo, WhitWeb, MokoRus,
Binder, SandRing, Boomer, BugChain, the spread chips, FlmLine, SeekBom,
DrkVulcn and BlakWeap); **605-623** to the Program Advances (the engine
keeps the bit of one formed from 603, 32 of them); **635-733** to the
other 99 folder chips, in BN4's order. Every row is `XREG_PENDING` (it
sits out: its code is not in BN6's engine) or `XREG_PA`. Each record is
made from BN4's in BN6's layout (`xrec_from_bn4`: its MB, kind and class
from +6 to +8; its kind as BN6's icon element, BN4 numbering its kinds as
BN5 its icon elements; a computed power unshown; inert until its code
plays), its name and description from BN4's archives in BN6's codes
(BN4's punctuation, 0x40 to 0x57, is BN6's 0x58 on, measured over the 64
descriptions it shares word for word with a chip of the run; its box's
lines set in BN6's), its icon in BN6's icons' palette, its picture and
palette copied, 123 KB of art in all. Without BN4's ROM each is a
placeholder that keeps its codes, class and MB ("BN4-82", "Needs BN4 ROM
back to battle"). Seen: BN4's GutsMan and Hole in BN6's folder editor
with BN4's pictures, descriptions and codes (the chip step, `chip 535 G
15`), and Hole's placeholder with BN4's ROM away. Unit tests hold the
rows' rules, a record made from a made-up BN4 record, the placeholder and
the text codes. A reward of one still pays the stand-in's zenny, as it
sits out. Not in this unit: an id past 602 has no pack entry (RAM to be
found: a second survey with canaries found the folder editor, the Library
and the NaviCust writing most of the stretches that looked free, two left
unproved), so a chip from 635 cannot be held yet.

**Unit 1, ten-bit chip ids, live; ids past 511 in part** (below, The build plan): the
census of BN6's code is read and kept by `tools/chip_ids.py` ([The chip
registry and BN5's chips' code](../rom/chips-code.md), Ten-bit chip
ids): of 625 shapes 128 are code, 77 of them to patch (116 halfwords,
`--table`), 2 a reward's, 6 the link battle's, 37 no chip, the folder
editor's 5 item-key shifts kept (its items, id << 7 | code, hold ids to
511 whatever an entry's form) and 1 the link battle's real-time chip
open. Built: every reader and writer of a folder entry in the engine's
C through `src/core/chip_entry.h` (nine bits still, one number to
change), its converter from nine bits unit-tested, the installer of the
census's table and the preset folders (`src/emu/xchips_wide.c`, inert at
nine bits), the profile's Library to 1023, the wire's chips to 1023
(protocol v16). Tried at ten bits, not shipped: the autopilot's seeds 1-3
the same run log and pictures as at nine, the folder editor the same
picture for picture, every differential spec its nine-bit verdict.

**Ten bits are live** since: `CHIP_ID_BITS` 10, `RUN_MAGIC` "CWED" (a
run of "CWEB" or "CWEC" converted once, [Portable saves](../engine/saves.md)),
the image of 1024 records and four archives each of names and
descriptions, the engine's tables sized by `CHIPS_IDS` and `XREG_IDS`
(1024), the names' and descriptions' routines over four archives, the
ownership marks hooked for ids from 512 (owned without a mark), the link
battle's real-time chip settled (capped at 0x139 before its mask). An id
past 511 plays: `--dev alias=600` makes chip 600 a copy of Cannon's
record, name and description, and the chip step `chip 600 A 30` puts it
in the folder; the Custom screen deals and draws it (Cannon A 40),
picked and fired it deletes a Mettaur; the run saved so (`run.sav`
"CWED") and CONTINUEd deals it again. Proved against nine bits: the
autopilot's seeds 1-12 from fresh data folders the same run log (66
battles) and the same pictures at frames 5000, 10000 and 14990; every
differential spec its nine-bit verdict (421 PASS on dev merged, the two
recorded FAILs and two smoke INCONCLUSIVEs as before), 246 of 246
mutations caught; the server's contract tests take a trade and an offer
of 600 and 1023. The New 3DS's heap modelled (`tools/heap3ds.py`, seeds
1-3): BN6 alone 85.9 MB, none refused; with BN4 94.7 MB, none refused;
with BN5 95.8 MB, a few of the tiles' learning caches refused as on 4
November (docs/engine/emulation/3ds-memory.md), the image 64 KB larger.

**An id past 511 is whole, to 602** (built 9 November 2026; [The chip
registry and BN5's chips' code](../rom/chips-code.md), The pack past 511).
The pack: a survey of the work RAM ([The RAM survey](../devtools/ram-survey.md),
54 runs) found no 6 KB BN6 leaves alone, nor 1 KB outside the persistent
zone, which is full; but the extension's entries of ids 320-410 are
Program Advances' and BN6's pseudo-chips', which no pack counts, so ids
512-602 take them (`bn6_pack_entry`; a canary over them changed by the
power-on clear alone). 91 ids: BN5's remaining chips and BN4's first
units. The folder editor's items are id << 6 | code (35 halfwords), every
reader of a name or a description takes one of four archives (six tables'
literals, two readers rewritten, the chat box's buffer command hooked), a
chip from 512 given enters the profile's Library by an event, where BN6
would set another block's flag, and a reward names one in a wide form
(kind 2 or 3 with bit 13, `reward_chip`) that a hook at BN6's split reads:
no table, no reserved id (511 stays out of the registry's reach all the
same). Proved with chip 600 (`--dev alias=600`, `reward=600`): given,
listed and swapped in the folder editor, counted again as it is left,
named on the results screen and in a Mystery Data's chat (another
archive's text with the chat's hook out), the autopilot's seeds 1-12 the
base build's run logs and pictures.

Not yet: a pack entry for the ids from 603 (BN4's later units: RAM to be
found, or its single-code chips, Navi chips and Gigas, packed closer), a
NetBattle's loadout past 511 (unit 8), and BN5's chips' and Souls' moved
code read for entries it splits itself (the differential passes at ten
bits).

A BN4 virus's reward chip pays the run's chip it is (`xchips_bn4_paid`,
unit 0, built 9 November 2026), by three rules in turn, each one chip per
name as the registry's:

1. **By name, BN6's own, whatever the class**: BN6's folder chip of its
   name (BN4's names in BN6's codes first: its dash 0x40, plus 0x46 and
   dot 0x4E are BN6's 0x98, 0x9E and 0xA6, `rom.c`'s `bn4_code`). 78
   chips: the 65 paid before, 3 more now that the punctuation reads
   (M-Cannon, Atk+10, Navi+20), and the 10 BN4 calls Mega where BN6's are
   standard (Muramasa, Guardian, Anubis, Atk+30, BugFix, DblPoint,
   Snctuary, FullCust, NeoVari, SuprVulc).
2. **By name, the registry's own id** (BN5's ROM read): BN5's chip of its
   name where the registry gives it an id of its own and it is no Program
   Advance (no reward names one). 34 chips: the 26 moved (Z Saver,
   NumberMan's V1 and ShadeMan's among them), the 7 playing over BN6's
   code, a DarkChip (DrkLance).
3. **By Navi and version**: BN4's SP and DS (its record's first parameter
   3 and 4, as BN5's; spelt with letters, "ProtoMSP", where BN6 and BN5
   mark the version) whose V1, one or two ids before with its family and
   subfamily, paired: the registry's row of BN5's id as many after the
   V1's, its record of that version; else BN6's own chip of the V1's
   family and subfamily named with the SP mark. 5 chips: NumberMan's SP
   and DS, ShadeMan's SP and DS (the registry's), ProtoMan's SP (BN6's).
   ProtoMan's DS pays none: BN6's Navi chips are V1, EX and SP. Roll's SP
   and DS wait for theirs (unit 4).

A BN4 pseudo-chip or Program Advance pairs none; BN4's Giga FinalGun
neither, BN6's of the name being a pseudo-chip (it waits for an id of its
own). One the run lacks, or that sits out, pays 200 zenny as before (the
owner's call 9 of [Battle Network 4 in a run](bn4.md)). With BN5's ROM,
117 of BN4's chips pay a chip, 52 more than the 65 before; without it 79
(a scratch mirror of the rules over the three ROMs). The eight moved families'
reward rows (Billy, Spikey, Shrimpy, Walla, Elmperor, CirKill, Mettaur,
Melody) name 276 chip entries, of 26 chips, all BN4's own but Guard1-3:
none paid a chip before; with BN5's ROM Mettaur's 38 Guard entries now
do (a scratch tally of the ROM's rows, numbers only). Mettaur is moved for
Roll's summon alone, no visitor: the six visitors' rows name only BN4's
own chips, so a player meets the pairings as BN4's chips come (unit 2 on)
and as more of its families visit. A tournament round
against a BN4 Navi pays a BN6 Navi chip of the round's band as a
stand-in ([the tournament](../game/tournament.md), call 7). BN4's own
chips have their ids since unit 2 (above); none plays yet.

## The census

### BN4's 388 records against a run's chips

Ids 1-388 of `BN4_CHIP_RECORDS`, paired by name (each game's text in
BN6's letters, punctuation mapped: BN4 spells the dash and the plus with
other codes than BN6 and BN5), then by Navi and version where the games
spell a Navi chip otherwise:

| Records | How many | What a run does with them today |
| --- | --- | --- |
| Not chips: no codes or no name (copies of LifeAura, Bass and MegaSoul, blanks) | 21 | nothing |
| BN4's pseudo-chips (class 3): the Souls' sixteen, DarkNeo's boss records, BN4's training tools | 30 | nothing; the Souls come by their own track ([BN4's Souls](bn4-souls.md)) |
| BN6's by name and class | 88 | paid as BN6's own |
| BN6's by name, another class: ten BN6 standard chips BN4 calls Mega (Muramasa, Guardian, Anubis, Atk+30, BugFix, DblPoint, Snctuary, FullCust, NeoVari, SuprVulc), Z Saver and the Giga FinalGun (BN6's of both names are pseudo-chips) | 12 | 200 zenny: `pair_bn4` refuses a pair whose library type differs |
| By Navi and version: ProtoMan SP and DS (BN6's), NumberMan SP and DS (the registry's) | 4 | 200 zenny: BN4 spells "SP" and "DS", BN6 and BN5 a glyph |
| The registry's by name (BN5's): 25 moved, 7 over BN6's code, 3 Program Advances, 1 DarkChip | 36 | 200 zenny (`pair_bn4` pairs BN6's own alone) |
| BN5 has the name, unregistered: SearchMan and ShadeMan V1, CannMode, SwrdMode, BugCurse, BugCharg | 6 | 200 zenny |
| **New to a run** | **191** | 200 zenny |

The 191 by kind: 100 standard chips, 12 Megas, 4 Gigas (RedSun,
SignlRed, BlakBarr, BlueMoon), 51 Navi chips, 5 DarkChips (DrkCanon,
DarkBomb, DrkVulcn, DrkSpred, DrkStage) and 19 Program Advances.

The Navi chips: BN4 holds 60 (20 Navis, V1, SP and DS each). Roll V1,
ProtoMan's three and NumberMan's three are a run's already; SearchMan V1
and ShadeMan V1 have BN5's names but no registry row; the other 51 are
the SP and DS of Roll, SearchMan and ShadeMan and all three of GutsMan,
WindMan, FireMan, ThunderMan, MetalMan, JunkMan, AquaMan, WoodMan,
TopMan, BurnerMan, ColdMan, SparkMan, LaserMan, KendoMan and VideoMan.
Eight of those Navis are moved and in the tournament (GutsMan,
ThunderMan, WindMan, FireMan, SearchMan, AquaMan, Roll and WoodMan:
[BN4's Navis in BN6's engine](bn4-navis.md)): 23 chips.

The Program Advances: BN4's list (`BN4_PA_LIST`, [BN4 Red Sun's
tables](../rom/bn4.md), Its Program Advances) holds 37 recipes of 30
Program Advances. Eight are BN6's own (LifeSrd, PoisPhar, BodyGrd,
GigaCan1-3, TimeBom+, H-Burst), three the registry's (SuprSpr1-3), 19
new: HeatSprd and BubSprd (three of the spread chips in a row of codes),
FlmCros1-3 (FlmLine), PitRng1-3 (Boomer1-3), BstFang1-3 (TwnFng),
PitHoky1-3 (AirHoc1-3), MagShok1-3 (MagBolt), P.Driver (GunSol1, 2 and
3) and DarkNeo (DrkLine, BugChain and Bass or BassAnly). Every one is
formed from chips of the 191 but DarkNeo's Bass.

### Their rarity, codes and MB

BN4's records are BN6's layout but for +6 to +8 (its MB, its kind and its
class where BN6 keeps its icon element, its library type and its MB:
`BN4_CHIP_MB`, `BN4_CHIP_KIND`, `BN4_CHIP_CLASS`). Their numbers are on
BN6's scale:

| | BN4's new standard chips (100) | BN6's standard chips (199) |
| --- | --- | --- |
| Rarity 0 / 1 / 2 / 3 / 4 | 15% / 30% / 41% / 13% / 1% | 20% / 29% / 35% / 12% / 4% |
| Codes: one / three / four | 2 / 62 / 36 | 31 / 116 / 52 |
| With the \* code | 38 | (BN6's own counts in `economy.c`'s reports) |
| MB, median | 28 | 30 |

The Megas are rarity 3 and 4 (BN6's 2 to 4), MB 38 to 80; the Gigas
rarity 4, one code each; the Navi chips one code each, V1 rarity 3, SP
and DS 4, MB 32 to 88; the DarkChips rarity 4, MB 99, four codes with
\*. Nothing needs rescaling: a BN4 chip's rarity is a tier as a BN6
chip's is.

### Their code: BN6's, or BN4's moved

A chip runs by its record's family (+0xB) and subfamily (+0xC). BN4's
family is a state of MegaMan's (`BN4_PLAYER_STATES` by its number),
BN6's an attack family (its table by the number less 0x10), so the
numbers differ; the 88 chips both name pair 16 of BN4's families with
BN6's (BN4's 0x09 bombs BN6's 0x12, 0x0A swords 0x13, 0x0B cannons 0x14,
0x0C support 0x15 and the Navi chips 0x1B, 0x1D recovery 0x20, 0x1F
vulcans 0x17, 0x20 the misc family 0x1C, and nine more of one chip
each). BN4's 0x0C runs its support chips and its Navi chips from one
subfamily table, where BN6 splits them. A new chip whose family and
subfamily a paired chip shares runs on BN6's code with its own record;
for the rest, every routine its dispatch reaches and every object it
spawns (`BN4_SPAWN_1`, `_3`, `_4` with the type in r0) was looked up by
body in the pairs' index of BN6's routines (`tools/xrecipe_pairs.py`),
the walk stopped at a twin.

- **On BN6's code, 6 chips by the census, none by the differential**
  (Where things stand: the walk compared bodies and not callees; kept as
  the census wrote it): Hole and DrkLine (BN4's 0x0C/0x0B is
  BN6's panel changer, support subfamily 5, HolyPanl's and Snctuary's,
  the panel by the record's parameters); TimeBomb (0x0C/0x11,
  TimeBom+'s, BN6's subfamily 10); LifeAura and BlakBarr (0x0C/0x15,
  the barrier's, BN6's subfamily 4: whether BN6's barrier draws their
  auras is the differential's to say); Ball (the bomb family's
  subfamily 4, its routine BN6's by body). RollSP and RollDS run BN6's
  Roll (Navi family 0x1B, 0) with a computed power (1001 and 1023,
  indices into BN4's own table of computed powers, which moves as
  BN5's did: [the chips' code](chips-code.md), Computed powers).
- **BN4's code moved, the rest:** 134 chips in 48 units by family and
  subfamily, the 19 Program Advances with their parts' units, and 51
  Navi chips in 18 units, one a Navi.
- **Already a run's, a pairing away:** 16 (the twelve of another class,
  the four by Navi): no new id, no code.

The code a unit moves, beyond an engine base of 340 routines (13.6 KB)
that four or more units reach and that is ported or paired once, as
BN4's families' 71 routines were (an upper bound: a routine is counted
moved where its body has no twin, and the generator's audits pair more):

| Units | Chips | Code to move | Which |
| --- | --- | --- | --- |
| 8 | 15 | under 300 B each, 1.0 KB | Marking, Repair, Invis, PanlSht1-3, DrkStage, FirePlus/ThunPlus/AquaPowr/WoodPowr, Counter1-3, ShotStar |
| 21 | 62 | 300 to 600 B, 9.8 KB | SidBmbo1-3, WhitWeb1-3 (three units), MokoRus1-3, Binder1-3, SandRing, Boomer1-3, BugChain, the spread chips (HeatShot to BublSide, DrkSpred, HeatSprd, BubSprd), FlmLine1-3 with FlmCros1-3, SeekBom1-3, TwnFng1-3 with BstFang1-3, PropBom1-3, MagBolt1-3 with MagShok1-3, PitRng1-3, Thunder1-3, DrkVulcn, BlakWeap, DrkCanon, GutPnch1-3 |
| 13 | 43 | 600 B to 1 KB, 9.9 KB | PopUp, GunSol1-3 and GunSolEX, DarkBomb, ElemFlar to ElemDark, CircGun1-3, BlueMoon, HawkCut1-3, Meteors1-3, Ligtnin1-3 with AquaUp1-3 and GreenWd1-3 (one unit), P.Driver, AirHoc1-3 with PitHoky1-3, NumbrBl1-3, RollAro1-3, Blizzard/HeatBrth/ElecShok/WoodPwdr |
| 6 | 14 | over 1 KB, 13.6 KB | RedSun, DarkNeo, SignlRed, BigHamr1-3 with GodHammr, MetlGer1-3 |
| 18 | 51 | 15.7 KB with the Navis' own objects | the Navi chips: each spawns its Navi's first-kind type, so a moved Navi's copy serves his chips (as BN5's Navi chips run the Navis' copies) and the chips of the ten Navis not moved wait on Phase 2 |

### Past 511: one answer for BN4's chips and BN5's

BN6's chip id is nine bits: a folder entry is the id in its low 9 bits
and the code in its high 7 ([Chips, folders and the Library](../rom/chips.md)),
a reward entry the id, the code from bit 9 and its kind from bit 14, the
pack's extension, the ownership marks (516 bytes) and their keys (512)
and the Library's flags (`0x1E20` + id, the comparisons' block from
`0x2020`) sized for 512 ([The chip registry and BN5's chips'
code](../rom/chips-code.md), The chip registry), and the wire's
`chip_last` 511 (`shared/online/protocol.json`). The registry has taken
every free id but 10 (365-367, 505-511). Two pieces of work meet that
wall: BN4's 198 (its 191, its 6 of BN5's unregistered names and its
Giga FinalGun) and BN5's own unregistered chips the gap audit counted
(105 named; those a run would give: the Darkloids' and Nebula's 21 Navi
chips, the five Program Advances made with them, Gigas still to trace).
One answer serves both.

The three ways, measured:

- **Reusing ids a run never gives.** BN6's 313 folder ids hold 26 a run
  cannot give by `chip_pool.c`'s own rules (a scratch read of Gregar's
  records): 17 of the other version (Falzar's Navi chips 242-256, Otenko
  and one more, library flag 1) and 9 cut from the US release (no
  picture of their own: 275-280, 311-313). 21 of them lie in BN6's Navi
  chips' range (221-280, which AntiNavi and the Navi chip's freeze test),
  just what a Navi chip wants. But 26 is a seventh of the need, the
  other version's ids are Falzar's should that open track come, and a
  reused id keeps BN6's Library number and flags, which its Library
  screen reads. A stopgap at most; not chosen.
- **Remapping** (a window of ids lent to the other games' chips a save
  holds, the chip's identity, its game and number, kept in the save):
  BN6's format untouched, but the window is 148 ids at most (the
  registry's 138 own and the 10 free), fewer than BN4's chips alone; a
  slot must be taken back when no folder, pack, stock or reward row
  names it; the records written into the core's copy change mid-run, so
  a checkpoint carries the map; a NetBattle's pair cores need one map
  for both folders, built the same in both games. It gives up the
  registry's first rule, the same ids at every boot. Not chosen.
- **A wider id: ten bits, 1024 ids.** Chosen. Static, the registry's
  rule kept, every row of either game from 512 at an id fixed once.
  Its cost is one audit, done once for both games.

What ten bits take. Located, then read (unit 1's census, [The chip
registry and BN5's chips' code](../rom/chips-code.md), Ten-bit chip
ids): in BN6's code (to `0x08150000`) 47 places take the low nine bits
by a shift pair (`lsls #23`, `lsrs #23`), 26 load a literal whose low
half is 0x1FF (18 plain), 119 shift right by 9 and 402 left by 9; of all
of them 128 are code the game reaches, the rest data read as Thumb. The
census's verdicts below stand: the hand past the Custom screen holds id
and code apart, so a chip's attack takes an id, never an entry; the
reward splits its own entry at `BN6_REWARD_SPLIT`; the folder editor
keeps its items as id << 7 | code in a halfword, ten bits id << 6 | code
(the codes to 28 fit six bits).

- **Folders:** an entry becomes the id in its low 10 bits and the code
  from bit 10 (the code needs 5 of the 6 left; bit 15 spare). Every
  reader patched; the ROM's tables of entries re-encoded as the core's
  copy is made (the six preset folders, the 18 shops' stock, the five
  trader pools, Mystery Data's lists, the Program Advances' recipes:
  the tables the registry's audit already listed), the grown recipe list
  written in the new form.
- **Rewards:** a reward entry cannot hold ten bits beside its code and
  kind (17 bits). An id from 512 is written in a wide form: kind 2 or 3
  (HP, BugFrags) with bit 13, which no row's amount reaches, the kind its
  id's block of 256, the id's low byte and the code below; a hook at BN6's
  split of a reward reads it (built; the first plan, a reserved id 511
  and a table by battle, would have made a saved state's reward depend on
  the table's order).
- **The engine's tables:** the ownership marks and their keys, the
  Library's flags (moved past `0x2020`'s block to flags BN6 leaves
  alone), the per-side use counts and the Program Advance bits sized for
  1024; the pack's extension 12 bytes an id from 512 (2.4 KB for BN4's,
  more for BN5's) in RAM BN6 leaves alone, to be found as issue #82's
  survey found the first.
- **Saves:** `run.state` holds BN6's RAM, its folders in the old form: a
  run saved before is converted once as it loads (its folders' entries
  rewritten, `RUN_MAGIC` bumped with the converter, so no run is lost);
  the profile's Library grows to 1023 (its version bumped, an old one
  read into the larger array); a `.cwsave` carries the profile's version
  ([Moving saves](../engine/saves.md)).
- **Trades and the NetPass:** the wire's `chip_last` to 1023 and the
  protocol's version bumped (two builds that differ never pair). A chip
  is named by its id on every game that registers it, as BN5's are, and
  the board leaves an offer out for a game that cannot name its chip, as
  it does BN5's for a game without BN5's ROM. BN5's new rows are BN5's:
  inside the NetPass's full set already, they trade and fight as BN5's
  chips do. BN4's chips follow call 1 (Online, below).
- **NetBattles:** both games' pair cores install the same build's
  registry and compare the installs' record, as now; the ids change
  nothing there.

**The first batch, in the ten free ids, before ten bits** (built 9
November 2026 for ShadeMan's and BlizzardMan's six, at 505-510, each
`chip_*` spec PASS; BigNoise not yet: [The chips' code](chips-code.md)): BN5's
ShadeMan and BlizzardMan Navi chips, V1, SP and DS (six, their Navis
moved already, a chip unit each as NumbrMan's, the gap audit's M), at
505-510, and BigNoise (Pulsar3 and ShadeMan's SP or DS, BN5's recipe) at
365 once ShadeMan's play. 511 is held for the wide rewards above; 366
and 367 stay spare. BN4's ShadeMan V1, SP and DS are the same Navi's: by
Navi and version (unit 0 below) they pay those rows, so BN4 needs 195
ids, not 198. No chip of BN4's own goes into the free ids: its six on
BN6's code wait for ten bits, which they would otherwise use up.

## The design

- **The experience:** the old guard's arsenal. A veteran drafts the
  chips last held in 2004 (CircGun's crosshair, RollAro's arrows,
  GutPnch, the Elem chips, MagBolt, BigHamr, the element columns) in the
  run he already plays, none of them behind a version or a third clear.
- **The dialectic** stays the run's: what you bring against what you
  find. A BN4 chip is a find that asks the folder's question again (its
  codes, its MB, the Program Advance it might complete), on BN4's ground,
  where the viruses and Navis it answers stand.
- **The loop layers:** the session (finds, stock and rewards on BN4's
  areas and from BN4's visitors), the act (a BN4 Navi's round or Guardian
  Data paying his own chip at the version fought), the meta (the
  Library's older Net count grows: variety, never power).
- **The patterns:** hand-made units in a generated run (each chip as BN4
  made it: its codes, rarity, MB and power read from the player's ROM);
  identity by net (what a net gives tells where you are: BN4's ground
  gives BN4's chips, as BN5's territories give BN5's); an honest
  telegraph (a chip that cannot play never shows: it sits out, never
  enters a pool, and a save without BN4's ROM shows "Needs BN4"); meta as
  variety; a bonus with a drawback for the DarkChips, by the flame's
  price.
- **Against [the core loop](../game/core-loop.md):** it serves its fifth
  lack, variety a veteran feels, inside actions the run has (no walk, no
  screen, no battle and no line added: a chip's name and description are
  BN4's, read from the ROM). Its risk is **complexity over depth**: a
  pool of some 530 chips across three games dilutes the draft, a code or
  a Program Advance harder to complete. The answer is the net's half
  (below): on BN6's ground the draft is BN6's as now, and BN4's chips
  come where BN4 is. No power creep: their rarity is BN6's scale, gated
  by the act's tier as BN6's and BN5's are.

### Which chips join

All 191 and BN4's 6 of BN5's unregistered names but BugCharg, each as
its code is proved, one chip per name as the registry's rule has it: a
chip the run has by name stays the run's (BN6's, else the registry's),
and BN4's of a name of its own is BN4's. BugCharg reads the NaviCust's
bugs, BN5's of the name stays out for that, and BN4's with it. The
Program Advances join with their parts. A chip that sits out (its code
not moved, or its ROM not read) is registered (its id fixed, its
placeholder kept) and never enters a run's pools.

### How they enter a run

- **BN4's visitors' rewards:** `xchips_reward_bn4` gives the registry's
  id where BN4's chip plays, the stand-in's 200 zenny where it sits out,
  as BN5's visitors' rewards do. In the six moved families' rows, 66 of
  the 210 entries above 3/8 HP are chips (a scratch tally): about a third
  of a visitor's picks become a chip where they paid 200 zenny.
- **BN4's Navis:** a round of the tournament against a BN4 Navi pays his
  own Navi chip at the version fought, and his Guardian Data where he
  guards, replacing call 7's stand-in once his chip plays; a Navi beaten
  with dark power gives his DS (Phase 2's rule, [Battle Network 4 in a
  run](bn4.md), What players said).
- **The finds and the stock:** BN4's catalogue (its chips that play,
  never a DarkChip nor a Program Advance) is the older Net's half of the
  finds on BN4's areas, as BN5's catalogue is on BN5's ([One net](one-net.md),
  the older net's half): the Net Dealers' stock, Mystery Data, the vaults',
  Servers' and gift's picks, a trader's list half each rarity's. In the
  deep, the older Net's half draws from the catalogues of the games
  present.
- **AsterLand:** unchanged. Its Chip Trader trades from BN6's own list,
  as the town is BN6's ([AsterLand and the Cyber
  Academy](../world/home/asterland-and-academy.md)).
- **The DarkChips:** BN4's five by BN6's flame on BN4's areas, at BN6's
  price (20 max HP a battle its dark power ran in), as BN4's dark rule
  (its karma) is not taken ([DarkChips](../game/meta/darkchips.md)).
- **The Library:** the profile's Library to 1023; BN4's chips counted in
  the older Net's ("those held of those a run can find now") and listed
  on the second screen under it, BN6's own Library screen by its own
  numbers (its Mega, Giga and P.A. tabs where they have room, as BN5's).

### Rarity and codes

As BN4's records have them (above): the rarity a tier as BN6's, the
codes BN4's (three or four, \* in 38 of 100 standard chips), the MB and
the power BN4's. A trader picks a code the pack lacks three times in four
as for BN6's ([What BN6 does](../game/economy/bn6.md)); a dealer prices by
the tier as BN5's chips are priced.

### The economy's bands

No band moves. Act by act a run finds as many chips as now; what changes
is whose: on BN4's areas the older Net's half is BN4's, whose rarity is
BN6's within a few points (above), so the good chips' share stays inside
its fifth. The visitors' rewards shift a little from zenny to chips (a
third of a BN4 visitor's picks, about 60 zenny less and 0.3 chips more a
BN4 visitor) and the tournament pays a Navi chip for a Navi chip. `build.py
economy` does not see visitors (its runs install no families): the unit
that brings BN4's chips into runs adds them to the report or tallies them
apart, before and after, as 5 November's tally did ([Battle Network 4 in a
run](bn4.md), Its viruses' rewards).

### Online

The rule today is the NetPass's ([The NetPass](../online/netpass.md)):
the Arena, trades and the DeepNet dive need the full set of ROMs (BN6's
and BN5's), so no pair ever lacks a chip the other holds. "A future game
is one entry there", its stamps open again until a player has it in. BN4
is not in the set: its viruses, areas and Navis touch no collection, so
nothing of BN4's crossed. Its chips will.

Two ways keep a pair whole. BN4 joins the full set (the NetPass's way: no
rule to learn, but every player without BN4's ROM loses the Arena and
trades until he adds it), or **BN4's chips stay home**: they sit out of
every NetBattle on both games' pair cores (the deck's sit-out by id,
which both games know from the folder), are left off the spares list and
the trade board, and the NetPass stays BN6's and BN5's. The second is one
rule ("the oldest chips stay home", said once at the Arena's desk), no
ROM's bytes cross, and no player loses a stamp. The NetPass's own
precedent took its full set at BN5's measured 87% of sessions; BN4's
share is not measured yet (0.20.0 is its first release). Recommended:
they stay home until the run-start events show BN4's share near that,
then BN4 joins the set with the notice the NetPass asks for (the owner's
call 1, below).

### EXTRAS

No switch of its own. **Older nets** off already leaves out every other
game's areas, Navis, visitors and finds ([The extras](../game/meta/extras.md)),
and BN4's chips come only through those, so the switch keeps them out
too: a run with it off finds, buys and earns no BN4 chip. A BN4 chip
already in the pack still plays where BN4's ROM is read: the switch
chooses what the net offers, not what MegaMan holds. A switch apart from
BN5's would split the one older Net of call 5.

## The build plan

Each unit ships on its own; each moved chip proved against BN4's own
battle frame by frame, as BN5's were. Sizes are estimates in working days
of one agent.

0. **Pairings, now (half a day).** `pair_bn4` pairs across a class where
   BN6's chip of the name is a folder chip (ten chips), by Navi and
   version (ProtoMan SP and DS BN6's; NumberMan's the registry's), and
   to the registry's own ids (its 36, Z Saver among them), each playing
   where its code is installed. 50 BN4 rewards stop paying the stand-in.
   Proof: unit tests of the pairing with made-up names (no ROM in CI);
   a scratch tally of the six families' rows before and after; `build.py
   pacing`.
1. **Ten-bit chip ids (3 to 5 days, the largest; serves BN5's rows too).**
   Every site above read in bn6f, patched or shown out of a run's reach,
   listed in docs/rom/chips-code.md as issue #82's audit was; the ROM's
   tables of entries re-encoded; the wide rewards' hook at id 511; the
   pack's extension's RAM found and claimed in `src/emu/free_space.h`;
   the engine's own side; `RUN_MAGIC` bumped with the folders' converter,
   the profile's version, the wire's `chip_last` and the protocol's.
   Proof: `CI=1 build.py test` (the registry's rules to 1024; a chip at
   id 600 given, held in the folder and pack, dealt, used, rewarded,
   counted in the Library, traded by `online_trade_settle`, saved and
   continued); the autopilot's seeds 1-12 the same run log for run log
   as the build before (no BN4 chip in them); `build.py lint`.
2. **BN4's registry rows (2 days, beside 1).** Its 194 rows (195 less
   BugCharg) at ids from
   512 (`XREG_PENDING` until their code plays), each record made from
   BN4's in BN6's layout field by field (`xchips_record.c` with BN4's
   +6 to +8 and its kind's numbering read as its icon's), names,
   descriptions, icons and art read from the player's ROM, placeholders
   "Needs BN4" without it. Proof: unit tests (records from made-up BN4
   records, placeholders, ABC places); a chip in the folder editor with
   BN4's picture and description; a run saved with BN4's chips continued
   without BN4's ROM (placeholders, sitting out, MegaMan's line).
3. **The chips' differential against BN4 and the generator's BN4 path (2
   to 3 days, beside 1).** A chip spec whose reference is BN4's battle
   (`tools/bn4_lab.py`'s, the chip given in BN4's folder and its key byte
   written for BN4's chip gate, `BN4_CHIP_GATE`, as the Souls' were);
   `tools/xchiprecipe.py` reading BN4 (its families as MegaMan's states,
   0x0C's one table for support and Navi chips, the engine base paired
   or ported once, `tools/bn4_twins.py`'s audits); the census written
   out by the tool into `.build/` (numbers and names, never committed).
   Until 1 lands a spec may give its chip one of the 10 free ids.
4. **The six on BN6's code: fallen away** (Where things stand; each
   joins unit 5). As planned: **(1 day, after 2 and 3).** `XREG_RECORD` over
   their twins: `chip_hole`, `chip_drkline`, `chip_timebomb`,
   `chip_lifeaura`, `chip_blakbarr`, `chip_ball` PASS against BN4's own;
   RollSP and RollDS with BN4's computed powers (`chip_rollsp`,
   `chip_rollds`).
5. **The moved units, cheapest first (about 15 to 20 days in all, in
   parallel).** The 48 units in the table's order, each its `chip_*`
   specs PASS, its mutations caught, its row `XREG_MOVED`. Independent
   by family: several agents at once, each a family, the shared base
   first. The Program Advances with their parts (BN4's recipes after
   BN6's and BN5's in the grown list, BN6's recipe first where two are
   the same; `pa_*` specs; the per-battle bits past id 383 grown in 1).
6. **The Navi chips of the eight moved Navis (4 to 6 days, after 3).**
   Their 23 chips on the Navis' copies, SP and DS by BN4's computed
   powers (`chip_gutsman` and on). Then call 7's stand-in steps aside for
   a round whose Navi's chip plays. The other ten Navis' chips follow
   their Navis (Phase 2).
7. **Into runs (1 to 2 days, after 2; each chip as it plays).** BN4's
   catalogue in the older Net's half on BN4's areas and the deep, the
   flame's DarkChips, the rounds' and Guardian Data's Navi chips, the
   Library, the second screen; `LAYER_MAKE` bumped (the stock's rolls
   change). Proof: the game-design skill first for any number moved;
   `build.py economy` (0 outside the bands, before and after in the
   commit, visitors counted); `build.py pacing`; `build.py test`
   (`LAYER_MAKE_HASH`); autopilot runs from seeds 1-12 with BN6 and BN4
   and with all three, fresh data folders, the run logs read; captures of
   a BN4 chip in a dealer's stock, on a results screen and as a round's
   prize.
8. **Online and EXTRAS (1 day, after 7).** The rule the owner chooses
   (call 1): the sit-out on both pair cores, the spares list's and the
   board's filter, the Arena desk's one line (a words file), or BN4's
   entry in `netpass.c` with the protocol's bump; `test_extras` with
   Older nets off and BN4 read. Proof: `build.py online-pair` with a BN4
   chip in one folder; a trade refused and offered as the rule says.

In parallel: 0 at once; 1, 2 and 3 side by side (other files: the
engine's id sites, the registry and records, the tools and specs); 4, 6
and the units of 5 as soon as 3's path runs, one agent a family; 7 and 8
once 1 and 2 are in. What can ship first: 0 alone, then 1 with 2 and 4
(six chips and the Navi chips' foundation), each later unit in the
release it is proved by.

## The owner's calls

Asked and answered on 9 November 2026:

1. **Online: BN4 joins the NetPass's full set now**, over the
   recommendation to wait. BN4's chips go into NetBattles and trades
   as they are proved, and BN4 Red Sun's ROM becomes part of the
   common ground the Arena and Trade Central ask of a pair. A player
   without BN4's ROM loses them, so the change is announced ahead as the
   NetPass asks (unit 8; [The NetPass](../online/netpass.md)).
2. **Ten-bit chip ids first, for both games**, as recommended: the
   pairings (unit 0) ship before them, and BN5's ShadeMan and BlizzardMan
   chips take the free ids meanwhile.
3. **All of BN4's chips, as each is proved**, as recommended, with a cut
   list kept open for what the statistics show unused.

Decided here by the data and the standing rules, not asked: no EXTRAS
switch of BN4's own (call 5's one older Net); BN4's DarkChips by BN6's
flame (BN4's karma not taken); BugCharg out (BN5's rule for it); BN4's
rarity and codes as its records hold them (BN6's scale, measured).

## How the census was made

Scratch Python over the three ROMs (in the session's scratchpad, not
kept): names read through each game's text archives
(`BN4_CHIP_NAMES`, BN5's `BN5_CHIP_NAMES_LOW`, BN6's `RomLayout`
`chip_names`) and their punctuation mapped; records through
`BN4_CHIP_RECORDS`, `BN5_CHIP_RECORDS` and BN6's; the registry's rows
parsed from `src/core/xchips_registry.c`; the code split by the families
the name pairs give and, for the rest, `tools/xrecipe_bn4.py`'s
`Pairs4` index of BN6's routine bodies with a walk (`xrecipe_thumb.py`)
from each family's state through its subfamily table and the objects it
spawns; the nine-bit sites by a scan of BN6's Thumb for the shift pairs,
the literal and the shift by 9. Unit 3 makes it a tool. What was not
done: the differential (every "on BN6's code" above is a candidate the
specs confirm), the generator's audits (the code sizes are upper bounds),
a reading of the nine-bit sites (counted, not read), and the PA checker's
trace (`BN4_PA_LIST_REF`'s reader assumed).
