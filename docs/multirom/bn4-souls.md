---
type: Research
title: "BN4's Souls on BN6's proxy form"
description: "What moving BN4 Red Sun's twelve Souls (Red Sun's six and Blue Moon's six, which run in Red Sun's ROM) onto BN6's proxy form takes: how BN4 takes a form, each Soul's abilities and the code it brings against BN6's twins, what BN5's path supplies and what BN4 must, a cost ranking and which Souls first; ThunSoul moved, the first, and what is left."
tags: [multirom, bn4, souls]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-13T20:46:00Z }
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
| His look | `soullook.c`'s room for BN4's united Soul (The shared room, below) | his sprite at BN6's list 0 number 0x5F in palette 0, his face BN6's 0x1D with BN4's digits, no overlay of the proxy's; the unison's picture and his buster's flash not moved |

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

1. **The rest of the look** (The shared room, below): the unison's
   picture (BN4's list 8 number 6, 1,628 bytes, family 0x0D's) and his
   buster's flash, each held to BN4's frames.
2. **The comparator's BN4 Souls.** `tools/xdiff_bn4.py` gets what
   `tools/xdiff.py` has for BN5's (`united`, the form by legit.json's map,
   the proxy's overlay, the power attack's number), each rule from both
   games' code. Then `bn4_soul_thun` PASSes whole.
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
| The unison's picture | list 8, number the form | 1,628 bytes; 20,570 for the twelve | the shared room keeps 0x800 for it |
| His face | `BN4_SOUL_FACES`, the face the form + 4 | 0x100 bytes, and a palette of 0x20 | the shared room; `SOULS_FACES` and `SOULS_FACE_PALS` have three entries to spare past BN5's six, one taken |
| The count's digits | `BN4_FACE_DIGITS`, six of 0x80 bytes by 5 - the count | 0x300 bytes | beside the face: BN6's step reads 10 - the count, so the address given is five digits before the copy; counts 0 to 5 are there, SoulT+1's 4 among them, where BN6 holds a Soul's turns to 9 |
| His palette | 0, written by the unison (`0x080EBB9E`) | | `soullook.c`'s palette hooks, as BN5's |
| His arm's points, his buster's flash | not located | | BN5's are `SOULS_ARM_ROWS` and `soullook_flash` |

**Blue Moon's six have no face in Red Sun's ROM.** The window's tables
lie end to end: five mood faces, six Soul faces, six digits, eleven
palettes. A face is the form + 4, so forms 7-12 read the digits' and the
palettes' bytes as tiles and colours. Derived from the layout
(`BN4_SOUL_FACE_COUNT`); the look of 4 November that saw all twelve unite
watched the sprite and the name, not the window. Their sprites and
pictures are whole (the table above counts them). A run that gives Blue
Moon's six needs their faces from somewhere: Blue Moon's ROM, BN5's of
the same Navis where BN5 has one (ProtoSoul's and NumberSoul's), or the
Soul Navi's own mugshot.

## The shared room

No gap of the ROM copy holds two of the sprites, so they cannot be
resident as BN5's six are. **The owner's call (13 November 2026): one
room, the united Soul's**, over a larger ROM copy (the cartridge's
address space is 32 MB and the copy 16; every platform would pay the RAM,
the New 3DS first) and over regions shrunk to what they hold (room for a
few sprites at best, not for 438 KB).

`FREE_BN4_SOUL_LOOK` is that room: 45 KB in the `world` zone's gap, the
last of its size (15 KB are left there), laid out in `soul_room.h`:

| Part | Room | From BN4 |
| --- | --- | --- |
| `SOULS4_SPRITE` | 0xA800, the largest of the twelve (RollSoul's 42,324) | list 0 by his form |
| `SOULS4_PICTURE` | 0x800, kept for the unison's picture | list 8 by his form, 1,894 bytes at most |
| `SOULS4_FACE` | 0x100 | `BN4_SOUL_FACES` by his form - 1; its palette, `BN4_FACE_PALS` by his form + 4, goes into `SOULS_FACE_PALS` at the face's number |
| `SOULS4_DIGITS` | 0x300, the same for every Soul | `BN4_FACE_DIGITS`, six |

BN6 reaches it by one number each, whichever Soul is in it: list 0's
0x5F (`SOUL_LIST0_BN4`, the list's last; `tools/xsoul.py`'s art stops one
under it) and the emotion window's face 0x1D (`SOUL_FACE_BN4`, past
BN5's six). `soullook.c`'s hooks give a BN4 Soul those where they give a
BN5 Soul its own: the sprite, palette 0 (BN4's unison writes 0 into his
object's palette, `0x080EBB9E`, as BN5's draws a Soul in 0), the face
with his turns under it, and BN4's digits, which BN6's step reads at
10 - the count: the address given is five digits before the copy, so
counts 0 to 5 are there, SoulT+1's 4 among them. `soullife.c` gives a
BN4 Soul no overlay on the way in or out, where the proxy's own is
HeatCross's: BN4 gives ThunSoul no enter routine (`BN4_FORM_ENTERS`' form
6 is its nullsub). A Soul that has one brings it with its move.

Built for ThunSoul (13 November 2026) and seen by film against BN4's own
battle (`bn4_soul_thun` with `film`): his body, his face and the 3 under
it are BN4's, frame for frame through the charge and the shot.
`soul_thun` holds his sprite to the room's address and his palette to 0,
and its mutations 560 and 561 break each.

What the room does not do yet:

- **Fill as a Soul unites.** One Soul is moved, so the room is his from
  the install (`room4_fill`). The second (AquaSoul) fills it as he unites,
  as a BN4 Navi chip's Navi fills his room as the chip is used
  (`xnavi_bn4_chip.c`); a rollback's snapshot then keeps which Soul is in
  it.
- **More than one MegaMan.** The room serves one. Two a side, or a
  NetBattle with BN4 Souls on both sides, needs a room a seat; no
  NetBattle takes a BN4 Soul yet (the differential's `souls6` alone).
- **The unison's picture.** BN6's unison draws BN5's image object from
  the Soul Navi's battle sprite in list 2; BN4's family 0x0D draws a
  picture of its own from list 8. Not traced; the unison ends before the
  fight in BN6, so the film does not show it.
- **His buster's flash.** BN4 draws one at his arm as he fires (seen in
  the film at t164 and t170); BN6 draws none for him, where a BN5 Soul's
  is BN5's flash object (`soullook_flash`). BN4's is not located.
- **His arm's points.** BN6 reads a row of points by list-0 number;
  BN4's ROM has none of that reader's instructions (BN5's `0x0801516E`
  searched for, found in BN5 and BN6 alone), so the room's sprite takes
  MegaMan's own row. Assumed close enough until the flash is moved.
- **Blue Moon's six.** `room4_fill` refuses a form past
  `BN4_SOUL_FACE_COUNT`: they have no face in Red Sun's ROM (above).
