---
type: Research
title: "BN4's Souls on BN6's proxy form"
description: "What moving BN4 Red Sun's twelve Souls (Red Sun's six and Blue Moon's six, which run in Red Sun's ROM) onto BN6's proxy form takes: how BN4 takes a form, each Soul's abilities and the code it brings against BN6's twins, what BN5's path supplies and what BN4 must, a cost ranking and which Souls first; ThunSoul moved, the first, and what is left."
tags: [multirom, bn4, souls]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-25T14:30:00Z }
sources:
  - id: bn4-rom
    resource: "Mega Man Battle Network 4: Red Sun (USA), B4WE, SHA-1 a97e96a7da03abd70f7953328e511b9fa29179f1, read in tools/romlab and by tools/bn4_souls.py"
    title: "The owner's BN4 Red Sun ROM"
  - id: bn6-rom
    resource: "Mega Man Battle Network 6: Cybeast Gregar (USA), SHA-1 89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6"
    title: "The player's BN6 ROM, whose routines the pairer compares"
---

# BN4's Souls on BN6's proxy form

Phase 2 of [Battle Network 4 in a run](bn4.md) brings BN4 Red Sun's Souls
into BN6's battles as BN5's six came ([Transplanted Souls](souls.md)): on
form 8, the proxy BN6 keeps and never offers, united on BN6's Custom
screen by BN5's rule. This is what that takes, measured on the ROM; the
addresses are in [BN4 Red Sun's tables](../rom/bn4.md) (Souls in battle)
and `src/emu/bn4.h`, the per-Soul numbers from `python3 tools/bn4_souls.py`.

In short: every one of BN4's twelve Souls runs in Red Sun's ROM, Blue
Moon's six included, once BN4's chip gate is passed. Each brings little
code of its own (13 to 43 routines, under 2 KB) and pairs most of its
calls with BN6's twins, but BN4 spreads a Soul's passives through its
engine where BN5 kept them in tables by form, and its twelve sprites take
438 KB, more than the Souls' room holds. ThunSoul, AquaSoul and GutsSoul
first.

## How BN4 takes a form

An earlier look at Red Sun saw UNITE light for WindSoul but no form taken:
the Soul's pseudo-chip vanished at OK. The reason is a check of BN4's own.
OK's picks loop (`0x0801F0A0`) puts a Soul's pseudo-chip (0x160 + its
number) into the picks through the pick's gate, and the gate asks
`0x080063C8`, which compares a key byte of the chip id, XOR 0x64, with the
save's byte of the same id. A Soul the story never gave has no byte
written, so its chip is refused and dropped. With that byte written, as the
game writes it when it gives a Soul, the form is taken. The differential's
reference battles of BN4's Souls on the guest core need the same byte
written beside the Soul's flag.

From there BN4's life of a Soul is BN5's in other places:

- The hand keeps the Soul's pseudo-chip first. As the turn starts,
  `0x0800B658` makes it the pending form (AI data +0x10), and the attack
  family 0x0D plays the unison: its phases, the Soul Navi's picture, the
  form written into NaviStats +0x24 (BN5's and BN6's +0x2C). Then
  `0x0800E05C` sets the form's state: its enter routine, turns, loadout,
  charge times and element.
- The turns are 3 for every Soul (by form, `0x080180A8`), counted up in
  AI data +0x18 against +0x19 as each Custom screen opens. The turn after
  the screen that shows 0 ends it, and family 0x11 writes form 0. That is
  BN5's order, so BN6's `soullife.c` already runs a BN4 Soul's life as its
  game does. Seen in romlab: WindSoul's 3, 2, 1 and 0 under his face, and
  MegaMan plain again the turn after.
- A charged shot is a power attack by the loadout's id (`0x0800CA7C`). It
  sets the attack's variables and returns an attack family, by CurAction
  itself into `0x080EAEFC`, MegaMan's state table, which the Soul Navis' attacks share ([BN4's Navis in BN6's engine](bn4-navis.md); BN6's table starts at 0x10). Four of them use
  one of BN4's chips through `0x0800D406`: WindSoul's AirShot and WindRack,
  ProtSoul's WideSwrd, and JunkSoul's Poltrgst (BN4's 381, named past 0xFF:
  `movs r0,#0x7e; adds r0,#0xff`). BN6 has AirShot, WindRack and WideSwrd
  by name, and the run has Poltrgst (BN5's, moved).

All twelve were seen in romlab. Each Soul was given Wind's kind and flag in
the ROM's two tables. Each one united after AirShot, played its unison,
drew its own sprite with its name over the field, and fired its charged
shot: a probe on its power attack and on its family's first instruction,
with B held 300 frames against a Mettaur whose update was stubbed so its
hits could not reset the charge.

## The twelve

Each Soul's row comes from the tables by form (`tools/bn4_souls.py`).
"Brings" is the code walked from its roots (its power attacks, their
families and its enter routine) through every call BN6 has no twin of,
counted for that Soul alone. "Calls out" are the calls BN6 has a twin of,
by tools/xrecipe_bn4.py's pairer: SAME, LAYOUT, or DIFF (audited ones are
those tools/bn4_twins.py already judged for Billy). "Engine" is where
BN4's shared code compares the form with the Soul's number, each a hook
BN6 would need.

| Soul | Game | Unites by | Charged shot (frames) | Its other abilities | Brings | Calls out | Engine |
| --- | --- | --- | --- | --- | --- | --- | --- |
| RollSoul | Red Sun | Recovery | family 0x1B (60) | a tenth of his HP healed at each chip used that is not Recovery | 33 routines, 1,382 B | 10 SAME, 5 LAYOUT, 4 DIFF audited | 1 |
| GutsSoul | Red Sun | panel-cracking | family 0x12 (100) | +30 to attack chips of no kind or panel-cracking; B pressed six times, a rapid buster | 14, 484 B | 6 SAME, 2 LAYOUT, 1 audited, 1 open | 1 |
| WindSoul | Red Sun | Wind | WindRack (60) | AirShot for his plain shot; +10 to Wind chips; an object of wind on the enemy's side; a collision flag | 27, 1,008 B | 10 SAME, 2 LAYOUT, 1 audited, 2 open | 0 |
| SrchSoul | Red Sun | Invisible | family 0x3E (100) | an object of its own as he unites | 13, 664 B | 7 SAME, 2 LAYOUT, 1 audited, 1 open | 0 |
| FireSoul | Red Sun | Fire | family 0x19 (40) | element Fire; chip charge (50); an object beside him; lava and the buster otherwise | 17, 540 B | 5 SAME, 2 LAYOUT, 1 audited | 2 |
| ThunSoul | Red Sun | Elec | family 0x47 (90) | element Elec | 13, 446 B | 4 SAME, 2 LAYOUT, 1 audited | 0 |
| ProtSoul | Blue Moon | Sword | WideSwrd (60) | chip charge (30); the sword families otherwise | 22, 1,060 B | 11 SAME, 3 LAYOUT, 4 audited, 2 open | 8 |
| NumSoul | Blue Moon | Plus | family 0x15 (120) | +10 to attack chips of no kind (BN5's NumberSoul's rule); an object beside him | 14, 478 B | 6 SAME, 2 LAYOUT, 1 audited | 0 |
| MetlSoul | Blue Moon | Break | family 0x14 (80) | a plain shot of its own (family 0x08); chip charge (70) | 43, 1,806 B | 13 SAME, 5 LAYOUT, 4 audited, 1 open | 0 |
| JunkSoul | Blue Moon | Obstacle | Poltrgst (80) | a collision region at his panel as he unites | 3, 96 B | 2 SAME | 0 |
| AquaSoul | Blue Moon | Aqua | family 0x1E (30) | element Aqua; chip charge (20) | 14, 452 B | 5 SAME, 2 LAYOUT, 1 audited | 0 |
| WoodSoul | Blue Moon | Wood | family 0x28 (100) | element Wood; grass, two places | 23, 948 B | 5 SAME, 2 LAYOUT, 1 audited | 2 |

The numbers say less than they seem in two places. The objects a Soul
spawns (WindSoul's wind, FireSoul's and NumSoul's companions, SrchSoul's
and JunkSoul's objects) are counted by their spawners only. Their types'
code is the other BN4 tracks' to move, once, as BN5's objects were. And
JunkSoul is small because its shot is a chip by name, Poltrgst, which the
run has as BN5's moved chip.

The two open DIFF pairs are `0x080CD7E2` against BN6's `0x080C6BDA`, where
an object's element sits at +0x17 in BN4 and +0xE in BN6 (the rewrite
`tools/xrecipe_bn4.py` already makes for the families), and `0x0800ACA6`,
an empty routine in BN4 paired with a working one of BN6's: a wrong pair
in the research list, to be dropped.

## What BN5's path supplies, and what BN4 must

`tools/xsoul.py` derives a BN5 Soul from BN5's tables. BN4 has the same
parts in other shapes:

| Part | BN5 (xsoul.py, the Souls' room) | BN4 |
| --- | --- | --- |
| The Soul's number | BN5's Navi AI index 7-12, `BN6_SOULS` +0 | 1-12 (Roll to Wood): a number space of its own in `BN6_SOULS`, apart from BN5's |
| UNITE's check | BN5's kinds and flags, `soulkinds.c` | the twin check, BN4's kinds (`0x08020008`) in BN4's numbering; the run's rule is BN5's for every Soul |
| Commit, turns, end | BN5's pending record; 3 turns + NaviStats +0x32 | the pseudo-chip in the hand and AI data +0x10; 3 turns by form: BN6's `soullife.c` serves as it is |
| Loadout | 6 bytes by form (`0x0801CA18`) | 4 bytes by form (`0x080184F0`): +1 plain shot, +2 chip charge, +3 charged shot |
| Power attacks and families | `0x0800F368`; families by CurAction - 0x10 (`0x080EB514`) | `0x0800CA7C` (108); families by CurAction (`0x080EAEFC`, 116); three shots BN4's chips' |
| Charge times | BN5's table by power attack | `0x080180B8`, the same shape; each Soul's the same at every Charge |
| Enter and leave | two tables by form | one enter routine by form (`0x0800E0A0`, eight Souls); the leave is family 0x11 |
| Passives | the ready and collision tables by form | none: hand passes in the enter routines, and 14 compares of the form in the engine's shared code |
| The unison's image | BN5's phase 1, the Soul Navi's sprite | family 0x0D, the Soul Navi's battle sprite (list 2, 1-12) |
| His look | list 0 7-12, 25-34 KB each | list 0 1-12, about 36 KB each, 438 KB for twelve; the emotion window's face by form |
| Element, chip charge | tables by form; `soulchips.c` | `0x0800C964`; the loadout's +2 (Fire, Proto, Metal, Aqua) |
| Chaos Unison | DarkChips of the Soul's kind | none: BN4's DarkSoul is form 0xF, 960 frames, not taken ([Battle Network 4 in a run](bn4.md), Not taken) |

So `tools/xsoul.py` needs a BN4 path beside its BN5 one, as
`tools/xrecipe_bn4.py` sits beside `tools/xrecipe.py`. It would take its
roots from `tools/bn4_souls.py`, its pairs from `bn4_twins.py`, rewrite AI
data +0x70 to BN6's +0xA0 and the collision offsets as the families do,
and answer the 14 engine places with hooks of its own.

## Blue Moon's six in Red Sun

They run. Red Sun's ROM refuses them only through their flags (`0xFF` in
`0x08020018`); their kinds, loadouts, power attacks, families, enter
routines, sprites and the Soul Navis' pictures are all present. Given a
flag, each united, drew its own sprite and name, counted its turns and
fired its charged shot as Red Sun's six did. A run can offer all twelve
from one ROM, as the design asked ([Battle Network 4 in a run](bn4.md),
What players said, 3).

## Once for all of them

These come before the first BN4 Soul, whichever it is:

- **Art room.** BN5's six sprites fill the Souls' sprite room
  (`SOULS_SPRITES`, 172,032 bytes: 164,355 taken, 7.5 KB left). BN4's
  twelve take 438,514 bytes, 33,110 (WindSoul's) to 42,324 (RollSoul's) each, ThunSoul's
  35,843 (`tools/xrecipe.py`'s `sprite` over BN4's list 0, 13 November
  2026). An earlier count gave them `FREE_TAIL`, at about 576 KB; the tail
  is 4 KB since the free space's regions moved, and the whole ROM copy has
  about 113 KB free in four gaps, the largest 60 KB in the `world` zone
  (`python3 tools/free_space.py --fit`;
  [Free space](../engine/emulation/free-space.md)). So the twelve cannot
  sit side by side, nor two of them: one room holds the united Soul's
  (The shared room, below).
- **Numbers in BN6's grown tables.** The power attacks have room (165
  of 256 taken, `soulpower.c`), and so do the attack families (0xD0-0xFF
  spare in `xattack_grow.c`). Three places do not: the charge rows
  (`SOULS_CHARGES` holds 179, 14 to spare, where BN4's twelve name up to
  18 power attacks), the loadouts (`SOULS_LOADOUTS`, BN5's six; its 64
  bytes held ten rows, and a fifth BN4 Soul's eleventh placed none of
  the Souls' abilities, so no BN4 Soul united, until it grew to
  eighteen, BN4's twelve, held by a static assert in `soulpower.c`), and list
  0's numbers (`SOULS_LIST0_ENTRIES` 0x60, 0x45-0x57 taken, where twelve
  sprites want twelve; its words have room for 256). Each grows in
  `soul_room.h` before the first BN4 Soul. The emotion window's faces
  grow as BN5's did.
- **The generator's BN4 path** described above, and the differential's
  BN4 Souls on the guest core: the flag, the gate's byte, and a BN4
  battle with a held B. The differential's part is built (12 November
  2026): `souls4=` gives the Souls and `bn4_soul_thun_control` unites
  ThunSoul in BN4's own battle, the same twice ([the
  differential](../devtools/differential.md), BN4).
- **The unison's picture** is the Soul Navi's battle sprite, BN4's list 2
  by the form (33,738 to 60,988 bytes, ThunderMan's 36,388), the sprites
  the BN4 Navis track copies for a battle's Navi ([BN4's Navis in BN6's
  engine](bn4-navis.md)) but drawn by the unison from a room of the Souls'
  own (The shared room, below). Read on 13 November as list 8's mugshots
  (1.7 KB each): the picture's init loads list offset 8, which
  `sprite_load` reads as a byte offset into the lists, so list 2 (22
  November 2026, [BN4 Red Sun's tables](../rom/bn4.md), Souls in battle).

## Cost and order

Ranked cheapest first by the code a Soul brings, the open pairs, its places
in the engine, and the mechanisms it needs that the run does not have yet:

1. **ThunSoul**: 13 routines, no engine place, no enter routine, an element
   (Elec) as the run gives TomahawkSoul and ToadSoul theirs; its charged
   shot is family 0x47 and what it calls.
2. **AquaSoul**: 14 routines, no engine place; element and chip charge are
   S4's and S5's mechanisms with BN4's kind (ToadSoul's Aqua charge has
   done this before).
3. **GutsSoul**: 14 routines; its +30 is a hand pass like NumberSoul's.
   One engine place: the rapid buster on B pressed six times, a hook on
   BN6's buster input.
4. **NumSoul**: 14 routines; BN5's NumberSoul's +10 rule is already
   ported. Its companion is an object of BN4's to move.
5. **FireSoul**: 17 routines; element, chip charge, a companion object,
   and two engine places, one on BN5's lava as BN6 now has it
   (`xpanels.c`).
6. **SrchSoul**: 13 routines and one open pair; its object is BN4's to
   move.
7. **WindSoul**: 27 routines and two open pairs. Its shots are BN6's
   AirShot and WindRack by name, a route like ColonelSoul's chip shot. Its
   wind object is BN4's to move.
8. **WoodSoul**: 23 routines, two engine places on grass (BN6 heals a Wood
   actor on grass already).
9. **RollSoul**: 33 routines; one engine place, in BN6's chip use.
10. **MetlSoul**: 43 routines, the most code, and a plain shot of its own.
11. **ProtSoul**: eight engine places in the sword families, and two open
    pairs.
12. **JunkSoul**: three routines; his shot Poltrgst by name through the chip
    route, his taking of the form a collision region.

The recommendation is to move ThunSoul first, as BN5's KnightSoul was: the
proof of the whole path (the generator's BN4 roots, a BN4 Soul number in
`BN6_SOULS`, UNITE by BN4's kind, the unison's picture, the sprite in its
room) with the least of its own, held to BN4's battle on the guest core.
AquaSoul then adds a chip charge, and GutsSoul a hand pass and the first
engine hook, so the three cover every mechanism of the cheaper nine. Red
Sun's three hard ones (Fire, Search, Wind) and Blue Moon's ProtSoul,
MetlSoul and JunkSoul follow the BN4 tracks they lean on: the objects, the
chips, the sword families. Which Souls a run gives, and how Blue Moon's
six reach a Red Sun player, is the owner's call; the ROM no longer decides
it.

## ThunSoul in BN6's engine

ThunSoul unites and fires its charged shot in BN6's battle where BN4's
and BN5's ROMs are read (BN5's for the Souls' engine itself), held to
BN4's own battle on the guest core. No run gives it: which Souls a run
gives is the owner's call, so only the differential's `souls6` holds it.
Each place a BN4 Soul needs, and what ThunSoul took:

| Part | Where | ThunSoul's |
| --- | --- | --- |
| Its number | `src/emu/soulbn4.h`: BN6's Soul numbers past BN5's 1-12 | 13 (`SOUL_BN4_FIRST`); used bit 13 |
| The Souls held | `soulunite_held`: BN5's six, then BN4's from bit `SOULS_HELD_BN4` | bit 6; the run's mask (`src/director/souls.h`) unchanged, no `RUN_MAGIC` bump |
| UNITE's kind | `soulunite.c`'s check over `soulkinds.c`'s `soulkinds_lights`, BN4's kinds (`BN4_SOUL_KINDS`, BN5's numbering) | Elec (2); a kind whose BN5 Soul is the other version's or not held falls through to BN4's (Two Souls of one kind, below); no Chaos Unison, so a DarkChip of its kind lights none |
| Its charged shot's family | an attack unit of `tools/xchiprecipe_bn4.py`, BN6's grown families from 0xD0 | 0xD7 (BN4's 0x47) |
| Its power attack | copied with the family's unit (`SOUL_POWERS`), its return the unit's number | BN6's grown 0xA5, after the Chaos Unison ones (`soulpower.c`) |
| Its charge times | BN4's row (`BN4_CHARGE_TIMES`) | 90 at every Charge |
| Its loadout | the seventh row of `SOULS_LOADOUTS`, BN4's four bytes as BN6's six | `ff ff 00 a5 ff ff` |
| Its element | `BN4_FORM_ELEMENTS` by form | Elec (3) |
| His look | `soullook.c`'s room for BN4's united Soul (The shared room, below), filled as he is committed | his sprite at BN6's list 0 number 0x5F in palette 0, his face BN6's 0x1D with BN4's digits, no overlay of the proxy's; his unison's picture, ThunderMan's battle sprite, at list 0's 0x80; his buster's flash BN4's arm (kind 0x1D) with its own art |

The plan had put the family at 0xC8, in the Souls' range. Family 0x47 is
BN4's SandRing's too (chip 47), so it is one attack unit among the
chips', which also lets the differential read it as it reads every BN4
attack unit. The state gives its ring the attack's +0xC as its kind, and
ThunSoul's power attack zeroes it, so ThunSoul's ring is kind 0, SandRing1's
row: sprite 3:0x37 and sound 0x91. The unit moves that row alone.
SandRing's records, its other rows and its spec stay with the chips'
generator line. The ring's sprite is chosen on two paths (3:0x37, or kind
3's 3:0x3A), and both are copied. Its sound is a byte of the row, and BN6
plays another song at 0x91, so the unit plays it from the grown song
table's slot 0x300 through its shim
([BN4 Red Sun's tables](../rom/bn4.md), SandRing's family and ThunSoul's
charged shot). A first take moved kind 3's row instead: BN6 drew none of
the ring and played the spread shot's sound, though the damage and frames
held. Filming both battles showed it.

The proof ([the differential](../devtools/differential.md), BN4):
`soul_thun` is BN6's battle alone with ThunSoul united, B held two frames
past its charge, held to BN4's own course in `bn4_soul_thun`'s BN4
trace. The charged shot's state runs from t154 to t177, and a 40 HP
Mettaur is left at 20 at t167, as in BN4, and the ring's sound plays
from slot 0x300. Its three mutations fail it: the
charge 10 frames longer, the shot's damage changed, the element none.
(The damage one went uncaught from 12 November 2026, when the differential
began keeping a poke of a spec that is not BN4's out of BN4's chips'
units: its poke fell on BN6's own code at that address. A spec whose
MegaMan holds a BN4 Soul names BN4's addresses again, `xd_spec_bn4_code`,
since 13 November.)
B held 88 frames gives the plain buster in both games, so the charge is
90 in both. `bn4_soul_thun` fights the two battles side by side. The shot
is the same to the frame there (lag 1), but the spec parts at t1 on the
Soul's form, and its FAIL is recorded with why. BN4's MegaMan is still in
his unison's state at t1 where BN6's crossing ended before the fight.
BN6's proxy keeps its companion overlay, and each game numbers his
palette and power attack its own way. The comparator's BN4 path has no
rules for a Soul yet, and his look is not moved.

## GutsSoul in BN6's engine

GutsSoul (BN4's Soul 2, form 2) is BN6's Soul 14 (k 1 in `soulbn4.c`'s
`forms`, held bit 7). What it took past ThunSoul's parts:

| Part | GutsSoul's |
| --- | --- |
| UNITE's kind | panel-cracking, BN4's own kind 8 of the chip (`soulkinds_bn4_of`: the registry's BN4 record, else BN4's chip of the same name), which BN5's numbering gives Cursor: `bn4_soul_guts` unites by two PanlSht1 (BN4's 178, BN6's moved 559) |
| Its charged shot | family 0x12, an attack unit at BN6's grown 0xD8; power attack 0x24 copied with it (60 at the attack's +8, a punch at the panel ahead, struck through the generic hit); its charge 100 at every Charge. Its punch object (the third kind's 0x1E) is spawned only where the attack's +3 is not 0, which 0x24 zeroes: its rows are the generator's `ROW_TABLES` entry left BN4's |
| His punch's hit | struck and pushed back a panel: the collision region's bits 0x21 at 0x080EBFDE, BN4's struck-and-pushed bit 0x01 with a push bit, given BN6's 0x61 (bit 0x40 with it), as GutPnch's punch is; its sound 0xA4 from slot 0x207, which GutPnch's unit gave it |
| His arm in the punch | the arm's kind 5 (`movs r4,#5` at 0x080EBF9A), BN4's list 3 number 4, which the chips' generator had not copied, so BN6 drew its own 3:4; now copied as BN6's list 3 number 0x7B, as ThunSoul's flash took 0x7A |
| His hand's pass | BN4's enter routine for form 2 (`BN4_GUTS_ENTER`) adds 30 to the bonus of each chip up next of no kind or panel-cracking whose power shows (`BN4_GUTS_PASS`); BN6's twin of BN4's hand keeps that bonus at `BN6_HAND_BONUS`, the attack's +6 beside its power at +8 as the chip is used (both traces: 30 and 100 for PanlSht1). `soulbn4_enter` does it as MegaMan enters the proxy form, its numbers read from BN4's code |

The proof is `bn4_soul_guts` against BN4's own battle (with its control,
`bn4_soul_guts_control`): the unison by two PanlSht1, B held 102 frames
from fight frame 30 for the punch, then PanlSht1 at 200 with its 30 more,
MegaMan at (3,2) and a MettaurSP at (4,2) in its reach. The chip comes after
the charged shot: BN6's support run holds MegaMan's own clock a frame on
from it (the differential's megaman_clock), while a press of B keeps the
battle's, so a charge begun after a chip reads a frame early. Its four
mutations: the charge 10 frames longer, the punch's damage another, the pass
left out (the passives' `SOULPASSIVE_BREAK_PLUS`), another Soul's sprite.
`bn4_soul_guts_atk` (with its control) takes him with an Attck+10 merged into
PanlSht1 on the same Custom screen: 40 in both games, the pass on top of the
merge. BN4's form state zeroes the hand's six bonuses before a form's routine
(`0x0800E068`, its +0x26), but the merge lands after it; BN6 merges at OK and
the pass adds as he enters the form. Its mutation leaves the pass out (10).

**His rapid buster.** BN4's key handler in form 2 counts B's presses (AI
data +0x12, a window of 10 frames at +0x11, 0x0800BE5A to 0x0800BE7A) and at
six asks request 0x20, which his ready state answers first with his state
0x10: eight shots, sound 0xA7, each BN4's buster shot (0x0800B590, BN6's
twin 0x0800FAAC, each reading its own game's buster attack from the
NaviStats), then the count cleared (0x080EBE84 to 0x080EBE88). In BN6's
engine (`src/emu/soulrapid.c`):

| Part | Its place |
| --- | --- |
| The state | the chips' unit `GutsRapid` (BN4's state 0x10 whole, moved as BN6's grown family 0xD9), its sound 0xA7 at slot 0x301, its shots BN6's twin of the buster shot (audited), the arm's flash BN4's effect 5 moved as BN6's 0xC1 |
| The count | in his side's Souls block (`BN6_SOUL_PRESSES` +0x18, `BN6_SOUL_WINDOW` +0x19), since BN6's AI data holds his chaos power at BN4's +0x11; the unit's end clears it there through a stub (`XC4_SOULS_SIDE`, the generator's `UNIT_CALLS_IN`, its two stores' offsets moved) |
| The counting | a hook where BN6's key handler's paths join before a B press's charge (`BN6_RAPID_COUNT`, BN4's twin place), BN4's rule line for line, its ten and six read from BN4's code |
| The request | BN6's charged B's (2), whose answer in BN6's requests makes the calls BN4's answer to 0x20 makes; a hook on its power attack's call (`BN6_RAPID_POWER`) gives the unit's family while the count stands at six, as BN4 answers 0x20 before a charged B |

The proof is `bn4_soul_guts_rapid` (with its control): united as
`bn4_soul_guts` unites, B pressed six times six frames apart, the first five
each a plain shot as it is let go, the sixth the rapid buster, B once more
after it, counted from 0. Its three mutations: asked at seven presses
(`SOULRAPID_BREAK_SEVEN`, `soulbreak` bit 30), seven shots, the count left
standing at its end.

**ThunSoul's Elec chips paralyse** (found 22 November 2026, after ThunSoul's
port): BN4's chip use asks a bonus by form for a chip used uncharged too
(`BN4_CHIP_BONUS`, `0x0800C4C4`), which gives form 6 0x4000 on an Elec chip
whose power shows (kinds 0xC and 2, not left alone): the power word's
paralysis bit, the one BN6's own bonus gives ElecCross's charged chips.
BN6 plays its charged chip's sound for any bonus its own routine returns,
which BN4 plays for a charged chip alone, so soulchips adds it past that
sound (`BN6_CHARGED_BONUS_DONE`), its numbers read from BN4's code
(`soulbn4.c`'s `soulbn4_chip_bonus`). `bn4_soul_thun_elec` holds it: united
by two Ligtnin1, the first used at t30, its power 0x4064 in both games; its
mutation leaves the bonus out (0x64). Ligtnin1 alone strikes nothing there,
so the paralysis's own frames are not yet compared.

## AquaSoul in BN6's engine

AquaSoul (Blue Moon's Soul 11, form 11) is BN6's Soul 15 (k 2 in
`soulbn4.c`'s `forms`, held bit 8), the first of Blue Moon's six, and the
first BN4 Soul with a chip charge. What it took past GutsSoul's parts:

| Part | AquaSoul's |
| --- | --- |
| His flag | Red Sun refuses Blue Moon's six (`BN4_SOUL_FLAGS` 0xFF): the differential gives him the flag of Red Sun's Soul at the same place, FireSoul's, written in the guest's ROM copy (only UNITE's check and the pseudo-chip's gate read them), as its BN5 side gives Team ProtoMan's Souls Team Colonel's |
| His charged shot | power attack 0x23 into family 0x1E, the spread shot's kind 0xD: no unit of his own, the power attack copied with Spreads' unit (BN6's 0xD0) and its kinds' table's row 0xD given BN6's effect and sound as the chips' rows are |
| His chip charge | the loadout's +2 is 0x15, BN4's empty routine (`BN4_POWER_EMPTY`), its charge times (20 at every Charge) serving alone: BN6 takes BN5's empty power of the same times, ToadSoul's 0x1B (`soulpower.c`'s `bn4_chip_power`); ProtSoul's 30 is BN5's 0x18's, MetlSoul's 70 and FireSoul's 50 are none of BN5's |
| What he charges | BN4's test `BN4_CHIP_CHARGE_TEST` by form: for 11 an Aqua chip (BN4's kind 1, `soulkinds_bn4_of`) not left alone; the bonus `BN4_CHIP_CHARGE_BONUS` 0x8000 to its power, which BN6's twin's caller adds as BN4's does (`BN6_CHARGE_BONUS`) and doubles by the same modifier bit; each engine's own sound of it (BN4 0x1BB, BN6 0x87). Their numbers read from BN4's code (`soulbn4.c`'s `aqua_rule`) |
| His face | MegaMan's plain face by the owner's call (above): the top 0x100 bytes of BN4's first mood face and its palette, the count's digit under them (`soullook.c`'s `face4_of`). BN4's own window draws the digits' and palettes' bytes for him: Red Sun has no face of his |
| His animations | the chips' units give MegaMan's animations BN6's art's numbers (the generator's "MegaMan's animation" rows, BN4's 0x0E as BN6's 0x0B for the spread shot); a BN4 Soul wears BN4's art, so a hook on BN6's `sprite_setAnimation` (`BN6_SPRITE_SET_ANIM`) sets BN4's number back while his CurAction is such a unit's family (`xchips_bn4_anims`), his +0x10 left BN6's. Every chip unit of the table in every BN4 Soul, not his alone; BN6's own families' numbers are not mapped yet |

The proof is `bn4_soul_aqua` (with its control): united by two Bubbler P,
B held 32 frames for the charged shot, A held 22 frames for Bubbler
charged and doubled, a MettaurSP at (4,2) before MegaMan at (3,2). Its five
mutations: the charged chip not doubled, no chip charged, the charge 10
frames longer, his element none, another Soul's sprite. Not moved: the
unison picture's arm (`BN4_SOUL_PICTURE_DECOS`' 0x13 for him), which BN6's
picture does not draw.

## SrchSoul in BN6's engine

SrchSoul (Red Sun's Soul 4, form 4, kind Invisible) is BN6's Soul 16 (k 3 in
`soulbn4.c`'s `forms`, held bit 9), the first BN4 Soul with an object of its
own. What it took past AquaSoul's parts:

| Part | SrchSoul's |
| --- | --- |
| His charged shot | power attack 0x0A into family 0x3E, an attack unit at BN6's grown 0xDA: five shots of 10 at the nearest foe ahead of him, a scope (effect 0x52, BN6's 0xBE) on it. Its target finder (`0x0800FB3C`) is copied with its panel flags' table: BN6's nearest (`0x08016054`) reads a side flip and keeps a column past his where BN4's keeps his own |
| His object | his taking of the form (`BN4_FORM_ENTERS`' 4, `0x0800E13E`) spawns the fourth kind's 0x44, which scans the other side for hidden foes (collision flags 0x206), puts a scope on each, and ten frames on lays a hit area and ends. The generator copies the routine with his charged shot's unit (`SOUL_ENTERS`) and moves the object at a fixed dispatch number of the Souls' own, 0xA4 (`TYPES4_SOULS`, the claim's 0xA4-0xA7 past bn4-air-units' 0xA0-0xA3); `soullife.c` runs the copy as he enters the proxy form (`soulbn4_enter_copy`) |
| Its moment | BN4 runs a form's routine as the form is written, inside its unison (`BN4_FORM_STATE`), so its object is gone before the fight's first frame; BN6's crossing ends nearer the fight, so its object is still in play at t1-t3 and its hit area meets MegaMan there (`BN6_COLL_MET`). The comparator reads that as one state (`soul object`) |
| His shots' sound | BN4's 0xBB, copied at slot 0x283 (DrkVulcn's), played on BN6's player 22 (`src/audio/xsong.c` XSOUNDS' `player`): on BN4's player 14 BN6's own release of a charged shot (0x72, player 14 in BN6, 26 in BN4) cut his first two shots' sound |

The proof is `bn4_soul_srch` (with its control): united by two Invis, B held
102 frames, his five shots at a MettaurSP at (5,2). Its three mutations: the
charge 10 frames longer, the shots' damage another, another Soul's sprite.
The object's scan finds no hidden foe there, so no frame of the fight tests
it yet.

## NumSoul in BN6's engine

NumSoul (Blue Moon's Soul 8, form 8, kind Plus) is BN6's Soul 17 (k 4 in
`soulbn4.c`'s `forms`, held bit 10), the fifth BN4 Soul and the first with
a companion. What it took:

| Part | NumSoul's |
| --- | --- |
| His charged shot | power attack 0x11 into family 0x15, an attack unit at BN6's grown 0xDB (the claim's 0xD8-0xDF; SrchSoul took 0xDA): he throws a die (the third kind's 0x05, 40 HP) that bursts on the foe. Its object's two calls out (`0x0800AC20`, `0x0800B0F6`) have audited twins; its throw's sound 0x90 is SeekBom's copy at slot 0x206, its burst's 0xC8 PropBom's at 0x205 |
| His pass | his taking of the form (`BN4_FORM_ENTERS`' 8, `0x0800E134`) adds 10 to the hand's chips of no kind whose power shows (`BN4_NUM_PASS`), BN5's NumberSoul's rule: `soulbn4_enter`'s passes, generalised from GutsSoul's |
| His companion | BN4 spawns it at the unison and as the Soul ends (`BN4_SOUL_COMPANION`, `0x0800B7E8`): first-kind 0x57, its word `BN4_FORM_COMPANIONS`' by form (list 4's 1, 2,068 bytes), kept at his AI +0x48, following his animation and place each frame. BN6's companion overlay (first-kind 0x56, `BN6_OVERLAY_TYPE`) is that object with a mask by animation added (its update `0x080C5C34` against BN4's `0x080CC46E`, its spawn `0x080C5D18` the twin of `0x080CC5F4`), so `soulcompanion.c` gives BN4's Souls' companions an overlay kind of their own, 61 (`SOUL_OVERLAY_BN4`, in the Navis' grown tables under Team ProtoMan's 62-63): its masks all set, so it keeps his depth as BN4's does, its sprite list 0's 0x81, which the shared room fills after his picture. `soullife.c` spawns it through BN6's own form routine (`BN6_FORM_OVERLAY_SPAWN`) as he enters the proxy form, at his `BN6_T1_OVERLAY` where BN6's own tells of an overlay find it, and takes it away through BN6's own (`BN6_FORM_OVERLAY_TAKE`) as he leaves; its animation is his BN4 number (`soullook.c`'s `anim_set` through its parent). FireMan's moved 0x57 (BN6's 0xE2) is no road: spawned beside a Soul it held BN6's battle on the Custom screen |
| The room | five BN4 Souls' loadouts are eleven rows, past `SOULS_LOADOUTS`' ten: the room grew to eighteen (#821) |

The proof is `bn4_soul_num` (with its control): united by Attck+10 (BN6's
Atk+10) on the first Custom screen, B held 122 frames for his die, A at 220
for MokoRus1 with his pass's 10. Its three mutations: the charge 10 frames
longer, the pass left out, the companion's masks a step behind him. FireSoul
shares the companion (list 4's 0, 9,288 bytes, which fits after FireMan's
picture) beside an object of his own as he enters (FireSoul in BN6's engine,
below).

## FireSoul in BN6's engine

FireSoul (Red Sun's Soul 5, form 5, kind Fire, element Fire) is BN6's Soul
18 (k 5, held bit 11), the first BN4 Soul with an attack of his own for a
chip he charged and the first with both a companion and an object as he
enters. What it took:

| Part | FireSoul's |
| --- | --- |
| His charged shot | power attack 0x0C into family 0x19, an attack unit at BN6's grown 0xDC: a flame of 50 two panels ahead, Fire. Its flame is the third kind's 0x09, which Elem's unit moved first, so his unit copies its spawner alone (`SHARED3`) and that spawner's palette store at `0x080CE4BC` is named in `IMMS`. Its sound 0xA8 is Elem's copy at slot 0x285 |
| His chip charge | power 0x66, BN4's second empty routine (`BN4_POWER_EMPTY2`, `0x0800CCD0`): a Fire chip held 50 frames (`BN4_FIRE_TEST_FORM`, no flag tested). BN5 has no empty power of those times, so BN4's Souls have power attacks of their own past Team ProtoMan's (`SOUL_POWER_BN4_MORE`, 0xE0 on, two by k, `soulpower.c`), which BN6 uses the chip charged by as it does BN5's empty ones |
| His flame for a chip charged | BN4's chip use runs `BN4_FIRE_CHARGED` (`0x0800D638`) in place of the chip's setup where the chip is charged and his the form (`0x0800B776`, `0x0800B78A`): family 0x19 at 150, Fire, the chip's id 0. The generator copies it with his unit (`SOUL_CHARGED`, its `+0x70` BN6's `+0xA0` and its family his number), and `soulcharged.c` runs the copy after BN6's setup of the chip charged and sets the family it returns (`BN6_CHARGED_CHIP_SET`) |
| His grass | his taking of the form (`0x0800E11C`) spawns the fourth kind's 0x12, a setter of panels (r4: type 6, mode 3, 10 frames), which blinks and then sets the middle row and (2,1), (2,3), (5,1), (5,3) to grass, BN6's number too. The generator moves it at the claim's dispatch 0xA5 with his entering (`SOUL_ENTERS`, `TYPES4_SOULS`). BN4's runs inside the unison, BN6's as he enters the proxy form, so the comparator's `soul object` reads BN6's blinks and its panels still plain in the fight's first frames as the same state, and a panel's blink each game keeps |
| His companion | list 4's 0 (9,288 bytes), after FireMan's picture in the shared room, as NumSoul's. He has an object as he enters too: `soulcompanion.c` writes a stub that spawns the one and runs the other (`SOULS_BN4_BOTH`) |
| His lava | where BN4's lava (type 8) hurts an actor or spares a Fire one, it heals FireSoul 50, adds as much to his collision data's Fire tally, turns plain and shows effect 7 and sound 0x9A (`BN4_LAVA_STEP`'s form 5, `BN4_LAVA_FIRE_HEAL`). BN6's engine's lava is BN5's (`XPANEL_LAVA`, `xpanels.c`), which spares a Fire actor: `soullava.c` gives FireSoul BN4's step through `xpanels_lava_own`, a Thumb of BN6's twins (`BN6_OBJECT_HEAL`, `BN6_SET_PANEL_TYPE`, effect 7 as Repair's unit moved it, 0xCE, and sound 0x9A as Walla's family copied it, 0x39) |

The proof is `bn4_soul_fire` (with its control): united by two HeatShot, B
held 42 frames for his flame of 50, A held 52 for the HeatShot turned into
his flame of 150, against a MettaurEX (250 HP) off his grass, which keeps
50 in both. Its five mutations: the charges 10 frames longer, his element
none, the flame for a chip another power, plain panels for grass, the
companion a step behind. His lava is `bn4_soul_fire_lava`: lava given under
him at fight frame 30 (BN4's type 8, BN6's 0xD), healed and cooled in both,
BN4's sound 0x9A and its copy at the same frame; its three mutations another
effect, no sound, and the Thumb ended at once.

## MetlSoul in BN6's engine

MetlSoul (Blue Moon's Soul 9, form 9, kind Break) is BN6's Soul 19 (k 6,
held bit 12), the first BN4 Soul with a plain shot of his own. What it took:

| Part | MetlSoul's |
| --- | --- |
| His charged shot | power attack 0x13 into family 0x14, an attack unit at BN6's grown 0xDD: a punch of 150 a panel ahead that knocks its foe a panel back. Its hit's bits (the collision region's r7, `0x080EC106`) are BN4's struck-and-pushed 0x23 and BN6's 0x63, as GutsSoul's punch's (`IMMS`); its hit word 0x06050601 a number (`NUMBERS`); its swing's sound BN4's 0xA2, BigHamr's copy at slot 0x243, its punch's 0xB7 the families' at 0x37 |
| His plain shot | power attack 0x0B, his buster's damage with the hit word 5, into BN4's buster family 8: copied with his unit (`SOUL_PLAINS`), its family BN6's buster's 0x11, on the second of BN4's own power attacks (`MORE_OTHER`, the loadout's +6, `BN6_AI_POWER_A`). BN4 makes the damage its NaviStats +5 plus 1, a field BN6 keeps otherwise, so the copy's call is aimed at BN6's own buster damage (`XC4_BUSTER_DAMAGE`, `0x0801265A`) and its +1 made 0 (`UNIT_AIMS`, `IMMS`) |
| His chip charge | power 0x0D, BN4's empty routine at 70 frames, BN4's own power attack (FireSoul's way); BN4's test charges a Break or no-kind chip not left alone whose power shows (`BN4_METL_TEST_FORM`) and its bonus doubles it (`BN4_METL_BONUS_FORM`), each read from BN4's code |

The proof is three specs, each with its control, against a MettaurEX
(250 HP): `bn4_soul_metl`, his punch (B held 82 frames), which knocks the
MettaurEX back; `bn4_soul_metl_chip`, an AirHoc1 held 72 frames and used
charged, doubled; `bn4_soul_metl_plain`, a plain shot held to t34, the
frame its power attack sets the attack's variables, since what each buster
does after is the differential's gap for every plain shot (BN6's family
0x11 spawns a first-kind object BN4's state 8 does not, and the buster's
animation is each art's number). Their seven mutations: the charges longer
(the punch's, the chip's), the punch not pushing, the chip not doubled, no
chip charged, the plain shot one stronger, its hit word another.

## WoodSoul in BN6's engine

WoodSoul (Blue Moon's Soul 12, form 12, kind and element Wood) is BN6's
Soul 20 (k 7, held bit 13). BN4's code names him in two places of the
player's frame and in his taking of the form. What it took:

| Part | WoodSoul's |
| --- | --- |
| His charged shot | power attack 0x57 into family 0x28, an attack unit at BN6's grown 0xDE: a tornado two panels ahead, the third kind's 0x31, copied with it; its sound 0x126 copied at slot 0x303, and the arm's kind 0x0F of the arm's table copied with the others. The state's path for its subfamily 2 (Static's, BN4's chip 27, no row of the registry) asks a count no BN6 routine keeps (`0x08043014`): the stub that returns 0, as WoodSoul never takes that path (`UNIT_AIMS`) |
| His merge | his taking of the form (`BN4_WOOD_ENTER`, `BN4_WOOD_MERGE`) joins each Wood chip of his hand and the chip of no kind after it whose power shows: its power, bonus and extra are added to the Wood chip's and it leaves the hand. `soulbn4_enter` does the same on BN6's hand, whose layout is BN4's; BN4 moves its two byte arrays by two bytes a place, BN6's code by one, as BN6 keeps them |
| His guard | BN4's player's frame passes over the status a hit gave him where his form is 12 (`BN4_STATUS_GUARD_FORM`): no paralysis, confusion, blindness or hold takes him. BN6's player's frame has the same gate for its own forms (`BN6_PLAYER_STATUS_CALL`, forms 7 and 0x13); a hook there (`soulguard.c`) passes over the call for a BN4 Soul so guarded, the form read from BN4's code |
| His grass | on grass BN4 heals him 1 a frame (`0x08012FB6`, form 12). BN6 heals any Wood object on grass 1 a frame already (`0x0801A1D2`), so his element gives it |

The proof is three specs, each with its control: `bn4_soul_wood`, his
tornado on a MettaurEX (B held 102 frames); `bn4_soul_wood_merge`,
ElemLeaf and MokoRus1 joined and used as one; `bn4_soul_wood_guard`, Billy
V1's ball striking him, unparalysed in both games. Their four mutations: his
charge 10 frames longer, the tornado one stronger, the merge left out, the
guard left out. The guard's spec is the first where a foe's object meets a
BN4 Soul: BN6's hit pass then counts the doubling of his Wood body against
the Elec ball in the ball's collision data (+0x74, +0x75), where BN4's
shifts the damage at once and keeps no count (the differential's
`BN4_LACKS`), and the ball's spark's place is drawn in Billy's copy of
BN6's ball in one game and in Thunder's moved ball in the other (the
replay's draw by its site, [the differential](../devtools/differential.md)).

## WindSoul in BN6's engine

WindSoul (Red Sun's Soul 3, form 3, kind Wind) is BN6's Soul 21 (k 8,
held bit 14). His two shots are chips by BN4's chip route
(`BN4_CHIP_ROUTE`), and his taking of the form floats him and raises a
wind on his side. What it took:

| Part | WindSoul's |
| --- | --- |
| His shots | his plain shot, power attack 0x6B, is AirShot at 5 (the hit word 0x85, subfamily 1), and his charged shot, power attack 0x09, WindRack at 50 (the hit word 0x9E), each a chip set up by BN4's route with the power attack's own stores after. Both chips are BN6's own by name, so, by the rule that a chip both games name plays as BN6's, `soulroute.c` copies each power attack with BN6's id of its chip and its call aimed at BN6's twin of the route (`BN6_CHIP_ROUTE`, `0x080129A6`, through a `bx r3` of the copy's, past a `bl`'s reach), into `SOULS4_ROUTES`. ProtSoul's WideSwrd is the same shape |
| His pass | 10 more to each Wind chip of his hand whose power shows, GutsSoul's shape (`soulbn4_enter`'s passes, `BN4_WIND_PASS_KIND`, `BN4_WIND_PASS_ADD`) |
| His shoes and wind | his collision's flags ORed with 0x30 (BN6's bits: floating and 0x10), his collision set up again (regions 0x10 and 2), and a wind of his side (`BN4_WIND_SPAWN`, its level 1) kept at his AI +0x5C; every form's state set clears the flags and ends the wind (`BN4_FORM_RESET`). `soulwind.c` does this at the end of his next frame (`BN6_PLAYER_LAST_CALL`), where BN4's wind starts on the fight's clock: the set-up through `BN6_COLL_SET_UP`, BN6's own wind through `BN6_WIND_SPAWN`, the twin WindMan's move pairs with it, its level as BN4's and its spawner word cleared as BN4's spawn leaves it, so that it holds for nothing (a hook on `BN6_WIND_HOLD` steps on for a wind with no spawner, BN4's state 4 having no hold). The flags are set there too: BN6's form's state set after the proxy form is entered sets his flags anew. BN6 keeps Full Synchro's aura at AI +0x5C, so the wind is found in his side's slot as he leaves |

The proof is three specs, each with its control: `bn4_soul_wind`, his
wind and its gusts on a MettaurEX from t2 and WindRack charged (B held 62
frames), held through t94, the frame its variables are set;
`bn4_soul_wind_pass`, an AirShot used with his pass's 10, held through
t32; `bn4_soul_wind_plain`, his plain shot, held through t34. After those
frames each game runs its own AirShot and WindRack (BN4's arm, BN6's
first-kind object), as `bn4_chip_cannon`'s held verdict records for a
chip both games name; the plain shot's variables and BN6's AirShot's
object come in one frame, so its copy is held by WindRack's, made the
same way. Four mutations: the charge longer, WindRack one stronger, the
wind and shoes left out, the pass left out. The comparator's part:
BN6's Wind bit in his attack's +2 (BN6 keeps a chip's second element there,
BN4 not), WindRack's id by the spec's `chips`, the wind's word at AI
+0x5C, the wind spawn's leftover r3, the unison's sounds counted beside
BN6's of that frame, and BN4's charge glow kept out of the natives' ranks
as BN6's was ([the differential](../devtools/differential.md)).

## ProtSoul in BN6's engine

ProtSoul (Blue Moon's Soul 7, form 7, kind Sword) is BN6's Soul 22 (k 9,
held bit 15). What it took:

| Part | ProtSoul's |
| --- | --- |
| His charged shot | power attack 0x3E, WideSwrd by BN4's chip route, the same shape as WindSoul's shots: BN6's own WideSwrd through `soulroute.c` |
| His chip charge | a Sword chip not left alone held 30 frames (`BN4_PROT_TEST_FORM`, BN5's empty power of those times), doubled (`BN4_PROT_BONUS_FORM`), AquaSoul's shape (`soulbn4.c`'s `alone_kind`) |
| His shield | his taking of the form sets his B and left to power attack 0x6A (`BN4_PROT_ENTER`, `BN4_PROT_LEFT`), BN4's Guard state's subfamily 4, moved as an attack unit at BN6's grown 0xDF (its shield the third kind's 0x2B, sound 0xC1 at slot 0x304; its third kind's 0x00 of kind 6, its kinds' table copied with it; the Guard chips' shockwave with it, which his subfamily never spawns). It is the Soul's other power on BN4's own power attacks, his loadout's B and left (`SoulBn4`'s `left`). Its power sets his B and left's lock, 0x28 frames: BN4 keeps it at AI +0x1A (`BN4_AI_LEFT_HOLD`), BN6 at +0x15 (`BN6_AI_LEFT_HOLD`, the store aimed there by `IMMS`). BN4's lock runs on past the attack, where BN6's attack's end sets it anew (`BN6_ATTACK_END_LEFT`); `soullock.c` passes over that store for a BN4 Soul |
| His look | BN4's other seven places of form 7 pick ProtoMan's arm (0xD) for its sword states' look; a chip both games name plays as BN6's, BN6's swords their own code, so they stand as BN4's alone (a look's hook, no differential's, if wanted) |

The proof is three specs, each with its control: `bn4_soul_prot`,
WideSwrd charged (B held 62 frames), held through t96, the frame its
variables are set; `bn4_soul_prot_chip`, the WideSwrd left used charged and
doubled, held through t64; `bn4_soul_prot_shield`, his shield on B and left
and its lock run out, 160 frames. Six mutations: the charge longer, the
charged shot another chip, the chip charge left out, the charged chip not
doubled, the shield one stronger, its lock a frame shorter. The B and
left's window (BN4's AI +0x16, BN6's +0x13) and lock are paired in the
trace, and the comparator reads MegaMan's AI words that hold such counts as
numbers ([the differential](../devtools/differential.md)).

## RollSoul in BN6's engine

RollSoul (Red Sun's Soul 1, form 1, kind Recovery) is BN6's Soul 23 (k 10,
held bit 16). What it took:

| Part | RollSoul's |
| --- | --- |
| His charged shot | power attack 0x22 into family 0x1B, the arrow (MegaMan's state 0x1B, its object the third kind's 0x0B, its kinds' table at `0x080CE5B0` copied with it, its sprite 3:0x10 BN6's 0x88; its effect 0x56 BN6's 0xBE; its sound 0x132 RollAro's copy at slot 0x208), moved as an attack unit at BN6's grown 0xE0 |
| Its lag | the arrow's lag (state 0x1B's 4) counts its frames by the free panels before him (`BN4_SHOT_LAG`, BN6's twin `BN6_SHOT_LAG` by body, their tables' row 0 the same bytes) and ends early in a step his pad asks. BN4 lays the step out by its panel (`BN4_STEP_ASK_PANEL`, `BN4_STEP_ASK_START`), BN6 by its direction (`BN6_STEP_ASK`, `BN6_STEP_ASK_PANEL`, `BN6_STEP_ASK_START`), so two stubs of the chips' own (`XC4_STEP_OUT`, `XC4_STEP_START` in `FREE_BN4_CHIP_STUBS`) make BN6's step in BN4's order, the attack's end between; the step is BN6's own from there, as every step of his is |
| His heal | after a chip of another kind than Recovery, his max HP's tenth healed, the heal's effect at him (BN4's 7, Repair's moved, BN6's 0xCE) and its sound (BN4's 0x9A at slot 0x39): BN4's chip use at `0x080EED04` (`BN4_ROLL_HEAL_FORM` and the rule's four numbers, read from BN4's code by `soulbn4_heal`), a hook at the same place of BN6's (`BN6_CHIP_USE_HEAL`) and a stub in the Souls' room for the effect and the sound (`SOULS4_HEAL`, `soulroll.c`) |

His B and left is MegaMan's own shield, power attack 0x26 in BN4 (his
form sets none: `BN4_FORM_ENTERS`' 1 is a `mov pc,lr`), BN6's own in BN6.
The proof is three specs, each with its control: `bn4_soul_roll`, the
arrow (B held 62 frames) and its lag run out, 180 frames; `bn4_soul_roll_step`,
UP in the lag, the step asked on the same frame in both, held at t110,
where the step's own count begins (BN4's style 10 frames, BN6's 4: his walk
BN6's own); `bn4_soul_roll_heal`, AirHoc1 used (a moved chip, BN4's code in
both), the heal at full HP, its effect and sound. Five mutations: the
charge longer, the arrow one stronger, the step left out, the heal left
out, the heal's effect another. The comparator reads the step's state and
variables as each engine's own (souls.json's `step`).

## Two Souls of one kind

BN5's twelve Souls take twelve different kinds, so its check, "the first
Soul of the chip's kind, this version's and held", never chooses
([Team ProtoMan: foes and Souls](../game/team-protoman.md), His Soul). BN4's
twelve take BN5's kinds again, but for GutsSoul's panel-cracking; BN5's
SearchSoul's Cursor has no Soul of BN4's, so the two Search Souls never
meet:

| Kind | BN5's Soul | BN4's Soul |
| --- | --- | --- |
| Invisible | ShadowSoul | SrchSoul |
| Recovery | MeddySoul | RollSoul |
| Wind | GyroSoul | WindSoul |
| Fire | NapalmSoul | FireSoul |
| Elec | MagnetSoul | ThunSoul |
| Sword | ProtoSoul | ProtSoul |
| Plus | NumberSoul | NumSoul |
| Break | KnightSoul | MetlSoul |
| Obstacle | ColonelSoul | JunkSoul |
| Aqua | ToadSoul | AquaSoul |
| Wood | TomahawkSoul | WoodSoul |

The check over both games' Souls is `soulkinds_lights` (`src/emu/soulkinds.c`),
a rule of the run's, not of either game's:

- BN5's Soul of the kind comes first where it is this version's and the
  run holds it: lit unless it united this battle already.
- Where BN5's is the other version's, or the run has not won it, BN4's
  Soul of the kind lights: held, its charged shot moved, not united this
  battle. Until 13 November 2026 a Team Colonel Soul's kind lit none of
  BN4's even where the run held BN4's alone; no moved Soul met that
  (ThunSoul's Elec is MagnetSoul's, the other version's), AquaSoul would
  have, against ToadSoul.
- **A run holding both Souls of a kind** unites BN5's alone: once it has
  united, the kind lights nothing more that battle. Whether the second
  lights then, or the player picks, is the owner's call (issue #499), to
  be made before a run gives two of a kind.

Held ROM-free by `tests/test_core.c`'s `test_soul_lights`, each case
above over BN5's kinds as its ROM has them and two of BN4's.

What is left, in order:

1. ~~**The rest of the look**~~ (22 November 2026, The shared room,
   below): the unison's picture and his buster's flash moved, the room
   filled as a Soul is committed.
2. ~~**The comparator's BN4 Souls.**~~ (22 November 2026: `tools/xdiff_bn4_souls.py`,
   legit.json's bn4 `souls`, [the differential](../devtools/differential.md), BN4):
   BN4's unison's last frame on the fight's clock and the battle's actors
   held through it (BN4's actors' runner gives each its held routine while
   time is stopped, 0x080104DC, which copies its +0x36 to +0x30: a write
   romlab's watch caught at 0x080104E6), the attack's variables' +5 its
   unison leaves (written at 0x08013B22, caught the same way), his ready
   state's clock after it, the power attack and his NameID by each game's
   number, the Soul's form and turns held to BN6's Souls block, the
   pending form, BN4's own counters and the buster's charge.
   `bn4_soul_thun` PASSes whole, its four mutations caught.
3. **AquaSoul and GutsSoul**, adding a chip charge, a hand pass and the
   first engine hook (Cost and order, above). Past ThunSoul's 13, the used
   word has 14 and 15 left below BN5's chaos bits (from 16), so the twelve
   need a word of their own; and the run's held mask grows past a byte
   (`RUN_MAGIC`) once a run gives them.

## The look, located

What ThunSoul's look takes from BN4's ROM, read on 13 November 2026 (the
addresses in [BN4 Red Sun's tables](../rom/bn4.md), The emotion window's
faces, and Souls in battle):

| Part | In BN4 | Size | BN6's place for it |
| --- | --- | --- | --- |
| His sprite | list 0, number the form (`BN4_STATS_SPRITES`) | 35,843 bytes | the shared room (below) |
| The unison's picture | list 2, number the form (the picture's init, `BN4_SOUL_PICTURE_LIST` and `BN4_SOUL_PICTURE_NUMBER`) | 36,388 bytes; GutsSoul's 60,988 the largest | the shared room's `SOULS4_PICTURE`, BN6's list 0 number 0x80 |
| His face | `BN4_SOUL_FACES`, the face the form + 4 | 0x100 bytes, and a palette of 0x20 | the shared room; `SOULS_FACES` and `SOULS_FACE_PALS` have three entries to spare past BN5's six, one taken |
| The count's digits | `BN4_FACE_DIGITS`, six of 0x80 bytes by 5 - the count | 0x300 bytes | beside the face: BN6's step reads 10 - the count, so the address given is five digits before the copy; counts 0 to 5 are there, SoulT+1's 4 among them, where BN6 holds a Soul's turns to 9 |
| His palette | 0, written by the unison (`0x080EBB9E`) | | `soullook.c`'s palette hooks, as BN5's |
| His buster's flash | the arm, BN4's third kind's 0x02 (`BN4_T3_ARM`), kind 0x1D: list 3 number 0x36, its animation the battle NaviStats' Navi (0 for MegaMan), spawned by his charged shot's family 0x47 (`movs r4,#0x1d` at `0x080EDCAC`) where the attack's +0xC is not 3 | 0x300 bytes | the arm's kinds' copy (`tools/xchiprecipe_bn4.py`'s table `0x080CD524`), its sprite BN6's list 3 number 0x7A |
| His arm's points | none: BN4's arm draws at his place less its kind's height (+3 of its row, its update `0x080CD666`) for every form, and no reader of points by sprite is in BN4's ROM | | BN6's chips in a BN4 Soul take MegaMan's own row |

**Blue Moon's six have no face in Red Sun's ROM.** The window's tables
lie end to end: five mood faces, six Soul faces, six digits, eleven
palettes. A face is the form + 4, so forms 7-12 read the digits' and the
palettes' bytes as tiles and colours. Derived from the layout
(`BN4_SOUL_FACE_COUNT`); the look of 4 November that saw all twelve unite
watched the sprite and the name, not the window. Their sprites and
pictures are whole (the table above counts them). A run that gives Blue
Moon's six needs their faces from somewhere. **The owner's call (22
November 2026): MegaMan's plain face for all six of Blue Moon's Souls**,
his normal mood face, the first of Red Sun's own faces, with the turns'
digits under it as for Red Sun's six. No Blue Moon ROM is read, now or
later.

## The shared room

No gap of the ROM copy holds two of the sprites, so they cannot be
resident as BN5's six are. **The owner's call (13 November 2026): one
room, the united Soul's**, over a larger ROM copy (the cartridge's
address space is 32 MB and the copy 16; every platform would pay the RAM,
the New 3DS first) and over regions shrunk to what they hold (room for a
few sprites at best, not for 438 KB).

`FREE_BN4_SOUL_LOOK` is that room: 104 KB in the `world` zone's free
space (45 KB when placed on 13 November 2026, grown on 22 November by the
unison's picture, which is a battle sprite and not a mugshot; the zone
holds 439 KB of free space since the measured pass,
[Free space](../engine/emulation/free-space.md), The measured pass), laid
out in `soul_room.h`:

| Part | Room | From BN4 |
| --- | --- | --- |
| `SOULS4_SPRITE` | 0xA800, the largest of the twelve (RollSoul's 42,324) | list 0 by his form |
| `SOULS4_PICTURE` | 0xEF00, the largest Soul Navi's (GutsMan's 60,988) | list 2 by his form, his unison's picture |
| `SOULS4_FACE` | 0x100 | `BN4_SOUL_FACES` by his form - 1; its palette, `BN4_FACE_PALS` by his form + 4, goes into `SOULS_FACE_PALS` at the face's number |
| `SOULS4_DIGITS` | 0x300, the same for every Soul | `BN4_FACE_DIGITS`, six |
| `SOULS4_HELD` | a byte | the form whose look the room holds, 0xFF none |

BN6 reaches it by one number each, whichever Soul is in it: list 0's
0x5F (`SOUL_LIST0_BN4`, the first list's last; `tools/xsoul.py`'s art
stops one under it) for his sprite, list 0's 0x80
(`SOUL_LIST0_BN4_PICTURE`, BN4's own numbers 0x80-0x8F past Team
ProtoMan's, [Claims](../project/claims.md)) for his picture, and the
emotion window's face 0x1D (`SOUL_FACE_BN4`, past BN5's six).
`soullook.c`'s hooks give a BN4 Soul those where they give a BN5 Soul its
own: the sprite, palette 0 (BN4's unison writes 0 into his object's
palette, `0x080EBB9E`, as BN5's draws a Soul in 0), the face with his
turns under it, and BN4's digits, which BN6's step reads at 10 - the
count: the address given is five digits before the copy, so counts 0 to
5 are there, SoulT+1's 4 among them. `soullife.c` gives a BN4 Soul no
overlay on the way in or out, where the proxy's own is HeatCross's: BN4
gives ThunSoul no enter routine (`BN4_FORM_ENTERS`' form 6 is its
nullsub). A Soul that has one brings it with its move.

**Filled as a Soul is committed.** The install leaves the room empty,
each moved Soul's look measured to fit it. Before each frame of BN6's
core (`soullook_frame`, beside the Navi chips' rooms in `xinstall.c`)
the Souls' block names the BN4 Soul a side is committed to or stands in,
MegaMan's side's first; where `SOULS4_HELD` names another, the room is
filled from Red Sun's ROM: his sprite, his picture, his face and its
palette, his form last. The Custom screen commits him frames before the
crossing loads his sprite, so the room is his before anything draws it.
The record lives in the core's copy, which no state carries, and the
Souls' block lives in RAM, which every state carries: a state loaded (a
rollback's, a test battle's, a CONTINUE's) is followed by the next
frame's check, as a BN4 Navi chip's rooms follow theirs
(`xnavi_bn4_chip.c`). `soul_thun` holds it: his sprite's frame read from
the room at BN4's art's offsets, and its mutation writing the record
ThunSoul's after the install leaves the room unfilled and fails it.
Seen by film against BN4's own battle (`bn4_soul_thun` with `film`, 13
and 22 November): his body, his face and the 3 under it, and his buster's
flash, are BN4's through the charge and the shot.

**The unison's picture.** BN6's unison runs BN5's phase 1 for a Soul
(`soullook.c`'s phases), which spawns BN6's copy of BN5's image object;
its sprite hook gives a BN4 Soul list 0's 0x80, the room's picture, where
a BN5 Soul's is his Navi's battle sprite in list 2, and no decoration of
a BN6 Navi's. BN4's own picture (the fourth kind's 0x13) plays the same
part: his Soul Navi's battle sprite, animation 0, palette 0. Its
timings and sounds are BN5's phase's, the run's rule for every Soul;
BN4's arm on the picture for WindSoul, FireSoul, NumSoul and AquaSoul
(`BN4_SOUL_PICTURE_DECOS`) comes with their moves. The unison ends
before the fight in BN6, so no spec's trace reaches it; it was looked at
in BN6's frames before the fight.

**His buster's flash** is BN4's arm, which his charged shot spawns (kind
0x1D, list 3's 0x36): the chips' generator copies the arm's kinds a unit
spawns, and kind 0x1D was missing, so BN6 drew its own list 3 number
0x36 in its place. It is copied with the rest (BN6's list 3 number 0x7A)
since 22 November, and the film shows it as BN4 draws it.

**His arm's points:** BN4 has none to move. BN6 reads a row of points by
list-0 number; BN4's ROM has none of that reader's instructions (BN5's
`0x0801516E` searched for, found in BN5 and BN6 alone), and BN4's arm
draws at his place less its kind's height in every form (`0x080CD666`).
BN6's own chips in a BN4 Soul keep MegaMan's row.

What the room does not do yet:

- **More than one MegaMan.** The room serves one. Two a side, or a
  NetBattle with BN4 Souls on both sides, needs a room a seat; no
  NetBattle takes a BN4 Soul yet (the differential's `souls6` alone).
- **Blue Moon's six.** `room4_fits` refuses a form past
  `BN4_SOUL_FACE_COUNT`: they have no face in Red Sun's ROM (above). By
  the owner's call the room takes MegaMan's plain face for them, as each
  of them is moved (AquaSoul first).
