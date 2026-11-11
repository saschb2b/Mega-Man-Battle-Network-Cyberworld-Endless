---
type: Research
title: "BN4's Souls on BN6's proxy form"
description: "What moving BN4 Red Sun's twelve Souls (Red Sun's six and Blue Moon's six, which run in Red Sun's ROM) onto BN6's proxy form takes: how BN4 takes a form, each Soul's abilities and the code it brings against BN6's twins, what BN5's path supplies and what BN4 must, a cost ranking and which Souls first."
tags: [multirom, bn4, souls]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-12T18:10:00Z }
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
  itself into `0x080EAEFC`, MegaMan's state table, which the Soul Navis' attacks share ([BN4's Navis in BN6's engine](bn4-navis.md); BN6's table starts at 0x10). Three of them use
  one of BN4's chips through `0x0800D406`: WindSoul's AirShot and WindRack,
  ProtSoul's WideSwrd. JunkSoul uses a chip from the battle. BN6 has
  AirShot, WindRack and WideSwrd by name.

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
| JunkSoul | Blue Moon | Obstacle | a chip used this battle (80) | an object of its own as he unites | 3, 96 B | 2 SAME | 0 |
| AquaSoul | Blue Moon | Aqua | family 0x1E (30) | element Aqua; chip charge (20) | 14, 452 B | 5 SAME, 2 LAYOUT, 1 audited | 0 |
| WoodSoul | Blue Moon | Wood | family 0x28 (100) | element Wood; grass, two places | 23, 948 B | 5 SAME, 2 LAYOUT, 1 audited | 2 |

The numbers say less than they seem in two places. The objects a Soul
spawns (WindSoul's wind, FireSoul's and NumSoul's companions, SrchSoul's
and JunkSoul's objects) are counted by their spawners only. Their types'
code is the other BN4 tracks' to move, once, as BN5's objects were. And
JunkSoul is small only because its shot is any chip the battle used: it
needs every chip it can name moved first, and a record of the chips used,
which BN6 does not keep.

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
| The unison's image | BN5's phase 1, the Soul Navi's sprite | family 0x0D, the Soul Navi's picture (list 8, 1-12) |
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
  (`SOULS_SPRITES`, 168 KB). BN4's twelve need 438 KB more, which only
  `FREE_TAIL` has (about 576 KB, `src/emu/free_space.h`). That is a claim
  BN4's Navis and music will compete for, and one `tools/heap3ds.py`
  measures on the 3DS. A Soul's sprite can come in when that Soul does.
- **Numbers in BN6's grown tables.** The power attacks have room (165
  of 256 taken, `soulpower.c`), and so do the attack families (0xD0-0xFF
  spare in `xattack_grow.c`). Three places do not: the charge rows
  (`SOULS_CHARGES` holds 179, 14 to spare, where BN4's twelve name up to
  18 power attacks), the loadouts (`SOULS_LOADOUTS`, BN5's six), and list
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
- **The unison's picture** is the Soul Navi's own in BN4's list 8 (1-12,
  about 1.7 KB each, 20 KB for twelve), apart from the battle sprites the
  BN4 Navis track copies from list 2 ([BN4's Navis in BN6's engine](bn4-navis.md)).

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
12. **JunkSoul**: any chip of the battle, after the chips' track and a
    record of chips used.

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

## ThunSoul in BN6's engine: the plan (12 November 2026)

Its reference stands: `bn4_soul_thun_control` unites it in BN4's own
battle on the guest core, the same twice ([the differential](../devtools/differential.md),
BN4). Its code is small and every call out paired (5 routines, 262 bytes:
4 SAME, 2 LAYOUT, 2 DIFF audited, `tools/bn4_souls.py --calls`), but
BN6's Soul engine is built for BN5's six alone, so the move is mostly
that engine taught a second game's Souls. Measured on dev at 2ec1a4b2,
each place a BN4 Soul needs and the number it would take:

| Part | Where, today | ThunSoul's |
| --- | --- | --- |
| The run's Souls held | `src/director/souls.h` `SOULS` 6, a `uint8_t` mask bit k BN5's Soul 7 + k | a bit past BN5's six (two spare in the byte; all twelve of BN4's widen it, `RUN_MAGIC`) |
| UNITE's held mask and kind | `soulunite_held`, `soulkinds.c`: BN5's kinds | BN4's kind 2 (Elec), the same numbering as BN5's |
| Its charged shot's family | `XATTACK_SOULS` 0xC0-0xCF: BN5's 0xC0-0xC6, Chaos Unison's failure 0xC7 | 0xC8: BN4's state 0x47 moved whole, as `tools/xchiprecipe_bn4.py`'s attack units are |
| Its power attack | `SOUL_POWER_FIRST` 0x94: BN5's 8 moved, 3 empty, 6 Chaos Unison's (`soulpower.c`) | 0xA5: BN4's 0x58 (`0x0800D298`), its return the family's new number |
| Its charge time | `SOULS_CHARGES`, 14 rows spare | one row, 90 frames at every Charge (BN4's `0x080180B8`) |
| Its loadout | `SOULS_LOADOUTS`, 6 bytes by BN5's number, 4 rows spare | the seventh row: no plain shot, no chip charge, the charged shot 0xA5 |
| Its element | the form's element on unison (`soulpower.c`) | Elec |
| MegaMan's look | list 0's 0x45-0x4A BN5's six, the art from 0x4B; `SOULS_SPRITES` full (168 KB) | BN4's list 0 number 6 (about 36 KB) at a number past the art, in room of its own (`FREE_TAIL`) |
| The unison's picture | BN5's phase 1, the Soul Navi's image | BN4's list 8 number 6, its family 0x0D's picture |
| The emotion window's face | `SOUL_FACE_FIRST` 0x17 + BN5's six, 3 spare | the seventh |

The order, each its own proof: the family and the power attack moved
(a generator's BN4 path, `tools/xsoul.py` beside `tools/xchiprecipe_bn4.py`'s
attack units), then the held bit, kind and loadout so it unites in BN6's
battle, then a spec of BN4's battle against BN6's (`souls4` and `souls6`
together) with mutations on the charge time, the family's damage and the
element; the look, picture and face last, each held to BN4's frames.
What a run gives stays the owner's call (above).
