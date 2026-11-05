---
type: Research
title: "BN4's Navis in BN6's engine"
description: "What moving BN4 Red Sun's eighteen new Navis into BN6's battle engine takes, Navi by Navi: how each runs (two builds, the Soul Navis on MegaMan's own states), what it calls and spawns against BN6's twins, the order by cost, the engine routines to port once and what the Navi recipe generator needs for BN4."
tags: [multirom, bn4, navis, research]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-05T01:12:00Z }
sources:
  - id: bn4-rom
    resource: "Mega Man Battle Network 4: Red Sun (USA), B4WE, SHA-1 a97e96a7da03abd70f7953328e511b9fa29179f1"
    title: "The owner's BN4 Red Sun ROM, read by tools/bn4_navis.py and played in romlab"
  - id: bn6-rom
    resource: "Mega Man Battle Network 6: Cybeast Gregar (USA), SHA-1 89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6"
    title: "The player's BN6 ROM, the engine they move into"
---

# BN4's Navis in BN6's engine

The map Phase 2 of [Battle Network 4 in a run](bn4.md) starts from: what
moving BN4 Red Sun's eighteen Navis new to a run into BN6's battle engine
takes, read from the two ROMs by `tools/bn4_navis.py` (its report in
`.build/bn4-navis/report.md`), the addresses and how each was found in
[BN4 Red Sun's tables](../rom/bn4.md) (BN4's Navis; What BN4's Navis call
and spawn). GutsMan is derived, not yet built ([GutsMan derived](#gutsman-derived),
below). BN5's Navis moved as
[Transplanted Navis](navis.md) tells, through BN6's Navi tables; BN4's
families by a path of their own ([Transplanted families](families.md), Billy
first). BN4's Navis need both.

## How a BN4 Navi runs

A BN4 Navi is an object type of its own, as BN4's virus families are, not
BN6's one Navi type with a table per AI index. Its routine runs a family's
jump table (init, update, dying); its update calls the Navi pre-step, the
Navi state runner with its state table (each state run with r6 its
collision data, r7 its AI data) and the collision's present, and seven of
them another call before or after the runner, the place BN6's hook A and
post hook have. Its table opens with BN4's six generic Navi states, its own
from 6, as BN5's 0-5 and BN6's 0-7 open theirs; a Navi may put its own in a
generic one's place (Roll's 1 and 2, WindMan's flinch and paralysis). Only
two of the six match BN6's by body, so BN6's generic states stand in for
all six, as BN6's 0-5 do for BN4's families'.

Its records: four ids in a row from 128 (Roll; V1, V2, V3, SP), its battle
sprite in list 2, HP from BN4's own ladder (GutsMan 500 to 1700, Duo 2000
to 3500). BN4's dark Navis (ids 240-252, MgaMDS to WodMDS) are Mettaur's
records in Red Sun but NumMDS: there is no DS Navi to move.

They come in two builds:

- **The Soul Navis**: Red Sun's six (Roll, GutsMan, WindMan, SearchMan,
  FireMan, ThunderMan) and Blue Moon's four of the eighteen (MetalMan,
  JunkMan, AquaMan, WoodMan; ProtoMan and NumberMan are built the same).
  Each takes its side's AI data at its init (one a side, 0xC0 bytes), its
  decision (state 6) runs the AI data's +0x64 as its sub-state, and its
  table names **MegaMan's own states** after its own: its attacks are states
  44 to 111 of MegaMan's table of 116, shared code (GutsMan's 7-10 are
  MegaMan's 67-70). Seen in romlab: GutsMan V1 alone against MegaMan ran his
  decision and his step (MegaMan's 67) in turn, his machine gun (MegaMan's
  70) and a dash (68), his AI data's sub-states and attack variables
  changing with them. Which of MegaMan's own actions reach those states (the
  Souls', assumed) is not read; none of them is two of the eighteen's.
- **The others**: TopMan, BurnerMan, ColdMan, SparkMan, LaserMan, KendoMan,
  VideoMan and Duo. No AI data, every state their own, their variables in
  the object from +0x60 as the families'. Each calls routines in another
  type's block: NormalNavi's (where to move next; six of them), HealNavi's
  (ColdMan), BurnerMan's (LaserMan), VideoMan's (Duo).

BN4's AI data against BN6's: BN4's is 0xC0 bytes, one a side; BN6's 0x100,
from a pool of eight. The Navi's own block, +0x64 (12 bytes), is BN6's +0x80
(0x20; BN6's HeatMan's decision reads its sub-state there), and the attack's
variables, +0x70, BN6's +0xA0 (0x50 in both). The Soul Navis' own code and
MegaMan's states they run read and write only those two blocks (+0x64 to
+0x90); the engine routines they call reach +9 to +0x44 too (an attack's
end), to map onto BN6's fields as those routines are ported.

## Navi by Navi

Bytes are code the walk reached: its own, MegaMan's states it runs, and the
blocks of the objects it spawns that BN6 has no twin of. Calls are its
states' calls into the engine, by the class of the pair (SAME, LAYOUT,
DIFF, none).

| Navi | Ids (HP V1-SP) | Build | States: own, MegaMan's | Bytes: own + MegaMan's + objects | Objects | Calls S/L/D/none | Unusual |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Roll | 128-131 (700-1500) | Soul | 1, 2, 6; 44-48 | 570 + 2148 + 736 | 4 | 14/4/4/6 | her own decision and dying states; MegaMan's 48 spawns an enemy by its id on a free panel (a summon); reads an enemy's record; a call before the runner |
| GutsMan | 132-135 (500-1700) | Soul | 6; 67-70 | 634 + 956 + 1544 | 4 | 10/4/4/5 | seen in romlab |
| WindMan | 136-139 (800-1800) | Soul | 3, 4, 6; 63-66 | 616 + 944 + 2880 | 5 | 5/4/3/5 | two first-kind objects (0x5C) set out at his spawn; the families' dying |
| SearchMan | 140-143 (900-1800) | Soul | 1, 6; 58-61 | 632 + 1646 + 1868 | 4 | 11/4/4/7 | his collision data's +0x1A and flag 4 set and cleared (his hiding, assumed): three routines of his alone |
| FireMan | 144-147 (500-2000) | Soul | 3-6; 72-76 | 824 + 1496 + 2154 | 5 | 11/4/3/8 | a first-kind object (0x57) at his spawn; calls after the runner |
| ThunderMan | 148-151 (800-2000) | Soul | 1, 6; 85-87 | 582 + 736 + 1468 | 3 | 5/4/3/5 | a first-kind object (0x33, with the families' dying) at his spawn |
| MetalMan | 160-163 (800-2000) | Soul | 1, 6; 101-105 | 1016 + 1164 + 2996 | 7 | 15/4/4/5 | cracks panels; a call before the runner |
| JunkMan | 164-167 (800-1800) | Soul | 1, 6; 109-111 | 906 + 736 + 1476 | 3 | 11/4/6/5 | two calls before the runner; two routines whose BN6 twins differ (the pause test, `0x0801466A`) |
| AquaMan | 168-171 (500-1800) | Soul | 6; 81-84 | 538 + 1272 + 2568 | 5 | 9/4/6/3 | WindMan's first-kind 0x5C at his spawn |
| WoodMan | 172-175 (1000-2500) | Soul | 6; 77, 106-108 | 574 + 1204 + 1044 | 3 | 14/4/3/6 | MegaMan's 77 is NumberMan's too |
| TopMan | 176-179 (600-1600) | other | 3, 6-11 | 2414 + 0 + 784 | 2 | 19/4/3/6 | his flinch his own, ending in BN4's |
| BurnerMan | 184-187 (700-1900) | other | 0, 3, 6-10 | 1702 + 0 + 2200 | 4 | 12/4/3/6 | his spawn state his own; two calls before the runner; tests time stop |
| ColdMan | 188-191 (800-1900) | other | 3-12 | 2146 + 0 + 1350 | 4 | 12/5/3/6 | cracks panels |
| SparkMan | 192-195 (600-1800) | other | 6-11 | 1362 + 0 + 1840 | 5 | 11/4/3/1 | the fewest calls with no twin |
| LaserMan | 208-211 (1000-2200) | other | 5-12 | 1942 + 0 + 1780 | 5 | 14/4/3/4 | three calls before the runner, a time stop test among them |
| KendoMan | 212-215 (800-2000) | other | 3, 5-13 | 2654 + 0 + 1536 | 6 | 17/4/4/7 | reads a side's player's AI data (+0x4C, by the battle state's objects) |
| VideoMan | 216-219 (800-1700) | other | 3, 6-12 | 1568 + 0 + 2844 | 6 | 13/4/3/2 | two calls after the runner; tests time stop |
| Duo | 224-227 (2000-3500) | other | 1, 2, 4, 6-15 | 3030 + 0 + 3944 | 10 | 13/4/2/5 | his own pre-step; his state 2 an engine routine of its own; four third-kind objects (0x61) at his spawn; cracks panels |

Four test whether time is stopped (BN6's twin of the test the same body);
whether any stops it was not looked for. The field changes their own code
makes are BN4's panel crack (ColdMan, Duo; MetalMan's kin of it); their
objects' code was not searched for others.

## What they spawn

The eighteen spawn 64 objects of BN4's: 3 of the first kind (the companions
set out at a spawn), 48 of the third (their attacks), 13 of the fourth
(effects and markers), each through a spawner in its own block, as Billy's
thunder is. Against every BN6 type of their kind, by their init's and
update's bodies, one has a twin: the third kind's 3, the collision region.
The fourth kind's 0, the effect object nine of them spawn, is spawned by
BN6's twin spawner and maps by its sprite as BN5's effects do. Every other
moves with its Navi, its collision data's offsets and its element's place
rewritten as BN4's families' are; where BN6 runs the same kind of attack, an
audit may take BN6's object instead, as Billy's thunder took BN6's 0x2A.
Their own calls into the engine are mostly the third kind's collision
set-up, present and frame, the families' pairs.

## The order, by cost

The cost is the code to move (the table's bytes, about 70 KB for all
eighteen before their art, the shared objects counted with each) and the
mechanisms each adds; the first of each build carries its build's
machinery. Each in the order adds one mechanism, as the reverse-engineering
skill orders units.

1. **GutsMan** opens the Soul Navis' path: their AI data mapped onto BN6's,
   MegaMan's states walked and copied as his, the state set and the
   attack's end ported (all ten use them). He is small (3.1 KB), sets out
   no companion, ports one routine of his own, and his states are seen in
   romlab.
2. **ThunderMan**, the smallest of all (2.8 KB), adds a companion at
   spawn: BN6's spawn-start word, as BN5's NumberMan's companion.
3. **WindMan**: two companions of one kind (AquaMan's kind later) and the
   families' dying.
4. **FireMan**: a companion of another kind and calls around the runner
   (BN6's hook A and post hook).
5. **SearchMan**: hiding, his three routines.
6. **Roll**: her decision and dying states her own, and the summon (BN6's
   spawn of an enemy into a fight under way, `BN6_SPAWN_ENEMY_1`, its twin
   to audit).
7. **LaserMan** opens the other build: no AI data, every state his own
   (1.9 KB), calls before the runner, two helpers in other Navis' blocks,
   two routines of his own.

Then Blue Moon's four (WoodMan 2.8 KB with no new mechanism, JunkMan with
three audits, AquaMan on WindMan's companion kind, MetalMan the largest of
them with seven objects) and the others: SparkMan (one routine with no twin),
TopMan, ColdMan (the panel crack), BurnerMan, VideoMan, KendoMan (a
player's AI data read, BN6's layout of it to check) and Duo last (7 KB, ten objects,
his own pre-step, four companions).

## The engine's routines to port once

Of the 80 engine routines the eighteen's states call (their type's routine,
init and update left out: BN6's Navi tables stand in for them), 44 have a
BN6 twin of the same body or layout, 6 are BN4's generic Navi states, 6 the
families' work already paired or copied (`tools/bn4_twins.py`: the can-move
check, the panel reserved, the collision's panels, the counter time,
a panel's data, a subsystem's byte), and 24 have no twin or one of another body:

- **For the ten Soul Navis**: the state set (`BN4_SET_ACTION`) and the
  attack's end (`BN4_ATTACK_END`; FireMan's and WoodMan's kin `0x0800C9FC`,
  WoodMan's `0x0800CA4A`).
- **Shared by several**: the other side's player's row (`0x0800A17A`, nine
  Navis), a panel marked in a grid at `0x02037ABE` (`0x08009DA0`, four), a
  collision region set (`0x08012ED0`, four), a can-move check's kin
  (`0x0800AD54`, three), a side's player found (`0x0800C456`) and the pause
  test (`0x080406A4`; DIFF, three each), the panel crack and `0x08009EE4`
  (two each), an object's facing (`0x0800ACA6`, DIFF, two).
- **One Navi's each**: twelve, SearchMan's three, LaserMan's two, Roll's
  two, and GutsMan's, KendoMan's, MetalMan's, WoodMan's and JunkMan's one
  (the report lists them).

BN4's families call 71 engine routines with no twin; six of the Navis' 24
are among them. The first BN4 Navi ports these 24 once, or each Navi the
ones it calls, as the order above reaches them.

## GutsMan derived

`tools/xnavirecipe_bn4.py` derives a BN4 Navi's recipe from the two ROMs,
GutsMan first, its report in `.build/xnavirecipe/bn4_report.md`. It walks him
as `tools/xrecipe_bn4.py` walks a family (its functions reused: his block, the
third-kind objects he spawns moved with him, each call paired by
`tools/bn4_twins.py`), and what a Soul Navi has more:

- **His place in BN6**: a Navi of BN6's one Navi object, by his AI index in
  the grown Navi tables, as BN5's Navis run. BN6's Navi status dispatcher
  (`0x0801AF44`) loads r6 and r7 from +0x54 and +0x58 before a state, as
  BN4's Navi runner does; BN6's virus dispatcher (`0x0801B1C4`) loads r6
  alone, so a Soul Navi cannot run as a family. BN6's Navi generic states
  0-7 open his table, his decision (6) is BN6's 8 and MegaMan's 67-70 his
  9-12.
- **MegaMan's states** walked in MegaMan's block and copied as routines,
  with what they reach: the callbacks they pass (`0x080F077D`, `0x080F0783`,
  `0x080F07D1`) and the tables past their pools, 956 bytes.
- **His AI data's blocks** as BN6 lays them out: a register tracker from the
  runner's r7 and each +0x58 load finds every pointer into it, made by an
  `adds` or a `movs` and `adds` (`movs r6,#0x64; adds r6,r6,r7`): +0x64 is
  BN6's +0x80, +0x70 BN6's +0xA0 (7 immediates). The attack's end's clear
  (`BN4_ATTACK_END_CLEAR`) writes +9, +0xA, +0xB, +0x30 and +0x32 out of
  them, copied with its offsets (an interim, BN6's meaning of them unread).
- **The engine's routines** with no BN6 body copied with him: the state set
  and the attack's end, the free panels of a side, one of them at random, a
  panel of his side at random (the callback his decision hands his step) and
  its masks. Five new pairs, each body read: GetRNG, the status a hit gives,
  a side's player (audited), the floor test and the panel crack (BN6's
  object_crackPanel, audited: BN4's test of flag 0x20000 never holds on a BN6
  panel). Of his 59 calls out, 35 are SAME, 7 LAYOUT, 16 DIFF (each audited)
  and one the dispatch's spawner.
- **His numbers** (a decision of the generator's): seat (V1 id - 128) / 4,
  AI 48 + seat, ids 0x380 + 4 x seat, past every family's ids (GutsMan AI 49,
  ids 0x384-0x387); his palette 0 for every version (`BN4_NAVI_PALETTE`'s
  step 0, as BN6's actor init gives a step of 0); his objects' list 3 sprite
  copied into list 4, its load's list offset made list 4's.

The pipeline's control for a Navi holds: BN4's battle with GutsMan V1 alone
against MegaMan idle (2000 HP), fought twice, is the same at every one of
1200 fight frames (`tests/xdiff/bn4_gutsman_control.json`, PASS), his
machine gun and his other objects in it ([The differential](../devtools/differential.md),
The BN4 trace for a Navi).

Seven decisions stay open, so no C file is written yet:

1. **His room**: XN_MORE's four slots are the Darkloids'. Proposed: FREE_TAIL's
   free bytes (`EMU_FREE + 0x720000` to FREE_LIB_TEAM), his home, code and
   art; its region in `src/emu/free_space.h` and its installer to add. All
   eighteen's art may not fit there (a battle sprite of tens of KB each).
2. **His battle sprite** in BN6's Navis' list 2 at a number of its own, and
   its arm points' record (`BN6_ARM_POINTS` reads a list 2 number's; BN4's
   records unread): the Navis' numbering (`src/emu/xnavi_sprites.h`) to
   extend for BN4's.
3. **Four sounds** BN6 has no song of (0xA2, 0xA3, 0xA4, 0xA7) and no slot
   left for them: BN4's families take all twelve of 0x34-0x3F. The owner's
   call, as each BN4 sound's slot was.
4. **A sound one of his objects plays** by a number its spawner passes
   (`0x080D0B18`'s `ldrb r0,[r7,#4]`), to follow back to the states that
   spawn it.

Then the C file (`src/emu/xnavi_bn4.c`, an XNavi), a BN4 game in
`xnavi_install.c` (no Navi tables: his records his four enemy records, his
name from BN4's archive), the BN4 trace's map of a Navi's AI data and his
object type in `tools/xdiff_bn4.py`, and `gutsman_v1` with its mutations.

## What tools/xnavirecipe.py needs for BN4

A BN4 path beside BN5's, as `tools/xrecipe_bn4.py` is beside
`tools/xrecipe.py` (the walk, the pairs and the classes are
`tools/bn4_navis.py`'s):

1. A Navi found by its first-kind type, its enemy record's +2 (BN4 has no
   Navi tables): its jump table, its update's calls, its state table to
   where its code begins.
2. Its generic 0-5 as BN6's 0-5, a generic state it replaces walked as its
   own, its own from 6 as BN6's from 8; a call into a generic state from its
   own code (TopMan's flinch ends in BN4's) aimed at BN6's of that place,
   audited.
3. Its update's other calls into BN6's hook A and post hook; Duo's pre-step
   audited against BN6's four.
4. struct1 and struct2 from its init and its four records; its init's
   spawns as its companions (BN6's spawn-start and spawn-end words).
5. For a Soul Navi: MegaMan's states its table names walked in MegaMan's
   block and copied as its own; the AI data's offsets rewritten (+0x64 to
   +0x80, +0x70 to +0xA0); its init's taking of the AI data dropped, as
   BN6's pool gives every Navi one.
6. Its calls into another type's block walked and copied with it, as
   ShadeMan's states in another AI's code were in BN5.
7. Its objects through the second-level dispatch, one decision a type for
   every track (BN4's in `tools/bn4_twins.py`, as BN5's in
   `tools/bn5_twins.py`), their collision offsets and element rewritten.
8. Its sounds, effects and sprites into issue #82's Navi ranges, its name
   from BN4's archive (its entries end 0xE5).
9. Seats past the Darkloids' (AI 43 on, ids from 0x1F0) and a room of its
   own in free space: `XN_MORE`'s four slots are the Darkloids'.

Each proved as BN5's are: `build.py xdiff` against BN4's own battle on the
guest core ([The differential](../devtools/differential.md), The BN4
trace), V1 alone, V1 against MegaMan's inputs, SP.
