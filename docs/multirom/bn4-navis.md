---
type: Research
title: "BN4's Navis in BN6's engine"
description: "What moving BN4 Red Sun's eighteen new Navis into BN6's battle engine takes, Navi by Navi: how each runs (two builds, the Soul Navis on MegaMan's own states), what it calls and spawns against BN6's twins, the order by cost, the engine routines to port once and what the Navi recipe generator needs for BN4."
tags: [multirom, bn4, navis, research]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-08T19:40:00Z }
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
and spawn). GutsMan is built and proved ([GutsMan built](#gutsman-built),
below), ThunderMan, WindMan, FireMan, SearchMan and AquaMan too
([ThunderMan built](#thunderman-built), [WindMan built](#windman-built),
[FireMan built](#fireman-built), [SearchMan built](#searchman-built),
[AquaMan built](#aquaman-built)), the six in
the tournament's pool on their plates; their companions made by one mechanism ([Companions](#companions)). Roll is built and proved to her summon
([Roll built](#roll-built)): the enemy she summons is BN6's own Mettaur,
not yet compared with BN4's. BN5's Navis moved as
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
| Roll | 128-131 (700-1500) | Soul | 1, 2, 6; 44-48 | 570 + 2148 + 736 | 4 | 14/4/4/6 | her own decision and dying states; MegaMan's 47 spawns an enemy by its id on a free panel (a summon); reads an enemy's record; a call before the runner |
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

## GutsMan built

`tools/xnavirecipe_bn4.py` derives a BN4 Navi's recipe from the two ROMs,
GutsMan first, and writes the whole ones into `src/emu/xnavi_bn4.c` (its
report in `.build/xnavirecipe/bn4_report.md`). `xnavi_install.c` takes a
BN4 game and installs them where BN4's ROM is read, after BN4's families
(`xinstall.c`). GutsMan V1 to SP are BN6's Navi AI 49, ids 0x384-0x387,
fought with `foes 384@52`. The generator walks him as `tools/xrecipe_bn4.py`
walks a family (its functions reused: his block, the third-kind objects he
spawns moved with him, each call paired by `tools/bn4_twins.py`), and
what a Soul Navi has more:

- **His place in BN6**: a Navi of BN6's one Navi object, by his AI index in
  the grown Navi tables, as BN5's Navis run. BN6's Navi status dispatcher
  (`0x0801AF44`) loads r6 and r7 from +0x54 and +0x58 before a state, as
  BN4's Navi runner does; BN6's virus dispatcher (`0x0801B1C4`) loads r6
  alone, so a Soul Navi cannot run as a family. BN6's Navi generic states
  0-7 open his table, his decision (6) is BN6's 8 and MegaMan's 67-70 his
  9-12. BN6's pre-step A, empty hooks and no companion; his ice block 2.
- **MegaMan's states** walked in MegaMan's block and copied as routines,
  with what they reach: the callbacks they pass (`0x080F077D`, `0x080F0783`,
  `0x080F07D1`) and the tables past their pools, 956 bytes.
- **His AI data's blocks** as BN6 lays them out: a register tracker from the
  runner's r7 and each +0x58 load finds every pointer into it, made by an
  `adds` or a `movs` and `adds` (`movs r6,#0x64; adds r6,r6,r7`): +0x64 is
  BN6's +0x80 (`BN6_AI_NAVI_STATE`), +0x70 BN6's +0xA0. The attack's end's
  clear (`BN4_ATTACK_END_CLEAR`) writes +9, +0xA, +0xB, +0x30 and +0x32 out
  of them, copied with its offsets (read and settled as inert on a Navi's
  AI data, below).
- **The engine's routines** with no BN6 body copied with him: the state set
  and the attack's end, the free panels of a side, one of them at random, a
  panel of his side at random (the callback his decision hands his step) and
  its masks. Five pairs, each body read: GetRNG, the status a hit gives,
  a side's player (audited), the floor test and the panel crack (BN6's
  object_crackPanel, audited: BN4's test of flag 0x20000 never holds on a BN6
  panel). Of his 59 calls out, 35 are SAME, 7 LAYOUT, 16 DIFF (each audited)
  and one the dispatch's spawner.
- **His numbers**: seat (V1 id - 128) / 4, AI 48 + seat, ids 0x380 + 4 x
  seat, past every family's ids; his home, code and art in BN4's Navis' own
  room (`FREE_BN4_NAVIS`, `EMU_FREE + 0x720000` to `FREE_BRACKET`, BN4's
  bracket screen's room past it: the owner's call of 5 November 2026); his battle sprite at list 2 0x40 + seat
  (`XNAVI_SPRITE_BN4`, GutsMan's 0x41; its bytes the room the battles share, [Battle sprites by battle](#battle-sprites-by-battle)); his palette 0 for every version
  (`BN4_NAVI_PALETTE`'s step 0, as BN6's actor init gives a step of 0); his
  objects' list 3 sprite copied into list 4, its load's list offset made
  list 4's. The art's and the dispatch's numbers continue past BN4's
  families' (the generator derives theirs first: list 4 from 0xC9, effects
  from 0x99, the dispatch's types from 0xE5); a sprite or an effect a
  family already copied is his too at its number (CirKill's 4:0x15 at
  0xC8, Billy's effect 0x15 at 0x90), found among the families installed
  before him.
- **A moved object's kinds' table** (`bn4_twins.TABLES`) read by the kinds
  he spawns, not the family's that names it: the third kind's 0x0B, CirKill's
  shot, is his machine gun's too, its kind 5 (his spawn's r4).
- **His sounds** in the pool the owner's call 11 gave them ([Battle Network
  4 in a run](bn4.md), The owner's calls): BN6's empty song slots no other
  track takes, 0x32, 0x33, 0x63, 0xD0 and 0xE0 (the five to 0xFF a `movs`
  or a byte names: a search of BN6's code for a `movs r0` of each before
  its PlaySound found none that plays them) and 0x104, 0x117, 0x148, 0x150,
  0x155, 0x159, 0x15D, 0x167, 0x16B, 0x16E, 0x172 (docs/rom/sound.md). A
  Navi's k-th sound its k-th slot, so his code names the pool's slots
  whichever Navi fights: GutsMan's 0xA2, 0xA3, 0xA4, 0xA5 (his third kind
  0x1E's kind 0, by its table's byte) and 0xA7 at 0x32, 0x33, 0x63, 0xD0,
  0xE0, his 0x1E's other kinds' 0xA6 the families' slot 0x38. Each Navi's
  songs are packed from the player's ROM into his art's room at his
  install (`xnavi_bn4_sounds.c`), and before each frame
  (`emu_before_frame`) the pool's sixteen song table entries are pointed at
  the songs of the BN4 Navi among the battle's objects, where they point
  at another's. One battle holds one BN4 Navi, so Duo's 10 bound it (the
  measured most of one); the binding count is of sounds named by one
  `movs` or a byte, which only five slots take: TopMan and VideoMan name
  six so, one short, and why 0x32 and 0x33 stay the pool's. The owner's
  call of 5 November 2026 settles it: such a sound whose slot is past
  0xFF plays through a shim of its 16-bit number. Built for a `movs r0`
  and its call to PlaySound (`XvSoundCall`, written by the generator's
  wide_sounds where a sound's slot is past a byte): the `movs` left as
  BN4's, the call aimed at a shim of the Navi's home (`xvirus_shim_sound`:
  `movs r0,#hi; lsls r0,r0,#8; adds r0,#lo; b` the PlaySound veneer the
  call had), as a NameID range's shim loads BN6's bounds; the song rows'
  slot is a u16 (`XvSong`). Proved by an experiment (not kept): GutsMan's
  recipe written with the pool's 0x104 first, his 0xA2 played through the
  shim (the hooks' sound event 0x104 from his attack's object), and
  `gutsman_v1` PASSes with his 0xA2 paired at 0x104. It showed a clash the
  install had not: his collision argument's shim and his code's first
  shim both at `XV_HOME_SHIMS` (a staged write is not seen by the next's
  free test), now apart (the collision's past the most his code's shims
  take, `xvirus_shims_room`). A table's byte past 0xFF (a shared play,
  GutsMan's third kind 0x1E's kinds) stays a decision of the generator's:
  its shim would map the byte, made the sound's index, to its slot.
- **His arm points** (`BN6_ARM_POINTS` reads a list 2 number's record): BN4
  has none to bring, as far as a search reaches: no routine of BN5's arm
  points' body (`0x08015140`, BN6's `0x08018810`) in BN4's first MB,
  searched by its opening's bytes, its record finder's tail (`0x080151A0`)
  and BN6's record finder's shape (`ldr r0,[r2,r0]`, a record size,
  `muls`, `mov pc,lr`), which finds BN5's (`0x080151AA`, 0x3C a record) and
  nothing in BN4. The owner's call is to read them from his sprite; until
  that is built BN6's MegaMan's record stands in at 0x41
  (`xnavi_sprite_arms_bn4`), an interim.

The pipeline's control for a Navi holds: BN4's battle with GutsMan V1 alone
against MegaMan idle (2000 HP), fought twice, is the same at every one of
1200 fight frames (`tests/xdiff/bn4_gutsman_control.json`, PASS).

**`gutsman_v1`** ([The differential](../devtools/differential.md), The BN4
trace for a Navi): GutsMan V1 alone at (5,2) against MegaMan idle at (2,2)
with 2000 HP, 1200 fight frames, his BN4 battle against his move. Both
battles run their 1200 frames and are traced and compared, his object and
AI data matched by role, its verdict recorded in the spec while the port
is under way (the suite holds the frame). What took it to t266, one root
cause at a time:

1. **His collision's setup argument** (`BN6_COLL_SETUP_ARG`, the bits a hit
   of his body ORs into the struck's): BN4's init passes 0x21
   (`0x080B7436`), BN6's actor init 3 to every actor (`0x08016FE8`). Both
   games' pre-steps read the bits alike (BN4's Navi pre-step `0x080135E6`,
   BN6's `0x0801AEB2`: bit 1 a flag 4, bit 2 a flag 2, a heavier bit, 0x20
   among them, a flag 0x10, or with bit 1 a flag 0x100), so the port keeps
   his own: `XNavi.coll_arg`, written over at his spawn's end by a shim of
   his home (his table 7). The shim returns by `mov pc,lr`: BN6 calls a
   hook by `mov lr,pc` and `bx r0` (`0x0800F430`), so lr carries no Thumb
   bit and a `bx lr` leaves Thumb (the battle reset to the title until it
   did).
2. **His AI data's fields each game keeps for itself**, each read in both
   games' code and allowed on a moved Navi's AI data alone
   (legit.json's bn4 allowed, `navi_ai`): BN6's actor type and AI index
   (+0, +1: its actor setup `0x08007720`-`0x08007726`, read by its spawn
   hooks `0x0800F420` and its Navi frame), its level by the foe's HP (+0xC:
   its actor init `0x08017068`-`0x08017088`, MegaMan's 2000 HP giving 10),
   +0xE (0xFF, `0x08007766`), its target (+0x78, `0x0800F31E`) and its pool
   bit (+0x7C-+0x7F, `BN6_AI_POOL_BIT`: entry 1's 0x40000000); BN4's side's
   control block (+0x50, `BN4_AI_CONTROL`). GutsMan's code reads none of
   them (the generator's AI tracker: his blocks, +0x64 on). The virus
   masks (`BN4_LACKS`), a virus's reasons, hold for none of his AI data.
3. **His sprite's count** (`obj +0x91`): his opening animation counted by
   the frame's clock, as MegaMan's (legit.json's bn4 opening_count): his
   after-step (BN4 `0x080143FC`, BN6's twin `0x0801BBAC`, both with their
   pause test) runs the sprite update at every fight frame and none of the
   paused opening's; from BN4's t2 his states' animations follow a frame
   behind, the battle's offset.
4. **His draws**: the differential keyed his RNG draws by BN4's places
   only once his code's room (`XN_BN4_CODE`) was among the spec's
   families' (`src/dev/xdiff_events.c`); 73 draws replayed since.

5. **His attack's object at t265**, BN4's third-kind 0x1C (the dispatch's
   0xE6): its +0x3C is the spawn's r3, which his spawner (`0x080D0894`:
   `push {r0,r1,r2,r5}`, `movs r0,#0x1c`, the spawn) never sets; BN4's
   spawn (`0x08003210`) stores r1-r3 at +0x34, +0x38, +0x3C and r4 at +4,
   so each game keeps its caller's leftover register there (BN4's the
   battle state's pointer, BN6's 3), and no code of the object reads it:
   allowed (legit.json's bn4 allowed, `spawn_leftover`). Its sprite state
   is the slot's last occupant's (it sets up no sprite), now compared from
   its sprite's set-up on in BN4's comparator as in BN5's (`leftovers`).

6. **The screen's shake** (t266): his attack shakes the camera
   (`BN4_CAMERA_SHAKE_SET`, `0x0802608C`, BN6's `0x080302A8` the same
   body), and every sprite's screen place takes its offset. The shake each
   frame is the same code in both (`BN4_CAMERA_SHAKE` `0x08025FE0`, BN6's
   `0x080301E8`: two draws masked by the same table, `0x08026068` and
   `0x08030284`), but BN4 draws from its first RNG word (GetRNG
   `0x08001B00`) and BN6 from its second (`0x0800154C`), as BN5 does
   against BN6. Not the port's: the comparator pairs the two draws as it
   pairs BN5's (legit.json's bn4 rng_twins, with their word), and the
   shake is the same in both.
7. **A cracked panel's flags of the last frame** (t266): his attack cracks
   panel 1,1, and each game's field step (BN4 `0x08009240`, BN6
   `0x0800BFC4`) stores a cracked panel's flags after its update, BN4's at
   +8 (`BN4_PANEL_FLAGS_LAST`), BN6's at +0x18 (`BN6_PANEL_FLAGS_LAST`,
   which its update `0x0800C404` reads: a cracked panel left by what stood
   on it breaks). A field the trace's map lacked, now mapped
   (`src/dev/xdiff_bn4_trace.c`).
8. **Sounds**: the panel crack's (BN4 0x95, BN6 0x97, each played by the
   twin crack routines) and his copies at the pool's slots (0xA2 at 0x32,
   0xA3 at 0x33, 0xA7 at 0xE0) paired by the routines that play them
   (legit.json's bn4 sounds).
9. **His rocks** (t295): his attack's object drops a rock on the panel of
   the other side's player and two on free panels shuffled. It reads that
   player inline from the battle state (`0x080D06C4`: [eToolkit +0x18]
   +0x44 + 4 x (its side ^ MegaMan's ^ 1)); BN4 keeps the players a word
   each from +0x44 (`BN4_BATTLE_PLAYERS`, MegaMan's side's first, as
   `BN4_FIND_PLAYER` reads them), BN6 one a side 0x10 apart from +0xD0
   (`BN6_BATTLE_PLAYERS`). The port's: the generator gives the index's
   shift and base BN6's (`tools/xnavirecipe_bn4.py` battle_players: `lsls
   #4`, `adds #0xD0`), which is the side wanted where MegaMan's side is 0,
   as in every battle a BN4 Navi is fought in (none a NetBattle's). The
   comparator then read a pointer into the rocks' sprite as the sprite
   before it in BN4's ROM, whose copy's room (rounded up) reaches past its
   end: a pointer is now the sprite's whose start is nearest below it.
10. **The rocks' debris** (t361): each landing rock spawns four of the
    fourth kind's 0x09 (twin spawners, `BN4_SPAWN_DEBRIS` and
    `BN6_SPAWN_DEBRIS`), whose inits differ only in the sprite they set
    up: BN4's list 4 0x0E (its rocks', which GutsMan's recipe copies as
    0xCA for his rocks), BN6's list 4 5 (its own rock art). Each engine's
    own art, allowed on its sprite state alone (legit.json's bn4 allowed,
    "the rock's debris, each engine's own art"), as the hit's spark is.

**`gutsman_v1` PASSes** over its 1200 frames (his step, decision, machine
gun, rocks and their debris, the cracked panel, his sounds), its recorded
verdict removed. Five mutations break the port and each is caught: a
CurAction off by one (t1), his AI block at BN4's +0x64 (t1), his attack's
sound at another pool slot (t267), his collision argument BN6's 3 (t1) and
his rocks' player read at BN4's +0x44 (t295); with the rest, 186 of 186.

**The attack's end's clear, read**: BN4's AI data +9, +0xA, +0xB, +0x30 and
+0x32 are MegaMan's buster charge (`0x0800BB52`-`0x0800BB7A`: +0x30 and
+0x32 the chip charge's and the shot's counts against the cached times at
+0x34 and +0x36, their levels into +0xA and +0xB; +9 read with his keys at
`0x0800BBDA`), which a Soul Navi's attack's end clears because he runs
MegaMan's states. BN6 keeps a player's charge elsewhere (+0x1B, +0x1D,
+0x1E, cleared by its own `0x08012EA8`), and its code reads AI data +9,
+0xA, +0xB and +0x32 only for a player (a search of every `ldr rX,[r5,#0x58]`
and the ten instructions after it in BN6's first 1.1 MB: +9 the HP bug's
count, `0x08010254`-`0x0801028E`, and +0xA, `0x080102DC`, both through
`BN6_SIDE_PLAYER_GET`, which never returns a Navi; +0xB `0x08013DEA` and
+0x32 `0x08014442`-`0x0801445E` in the player's step), none +0x30. On a
Navi's own AI data the clear writes zeros nothing reads, and GutsMan's code
reads none of them (the generator's AI tracker): it stays copied with BN4's
offsets, settled as inert. A Navi whose code reads them would need them
mapped.

The New 3DS's heap with GutsMan installed (`python3 tools/heap3ds.py
--seeds 1-3 --cases bn4,all`): BN6 and BN4 peaked at 94.1 MB of 96 with
no allocation refused and no crash; all three ROMs read at once at 96.0
MB, 15 refused in 2 of 3 runs (maps' mirror images, `area_src.c`), as
[the 3DS's memory](../engine/emulation/3ds-memory.md) measured before
him: his install writes the core's copy alone, not the heap.

**In a run** he is a tournament's round ([The tournament](../game/tournament.md),
As built): `src/layer/xguardian_bn4.c` lists the proved BN4 Navis, a line
each (his V1 id, his own plate on BN4's bracket, his look on a card), and
the bracket's pool takes each where his four versions are installed. A
Navi joins it with his line once his spec PASSes and every mutation of it
is caught.

Then: the next Navi.

### Battle sprites by battle

Built on 5 November 2026, the same rule as the sounds: a BN4 Navi's battle
sprite loaded for his battle from the player's ROM into one room the
battles share, none resident. Measured from BN4's list 2: the ten Soul
Navis' sprites are raw in BN4 (their rooms 36-60 KB, GutsMan's 60 KB), the
other eight LZ77'd (98 KB compressed, 264 KB decompressed, Duo's the
largest at 61040 bytes). The room is 64 KB (`FREE_BN4_NAVI_SPRITE`, the
art's last `0x10000` before the bracket's faces; `XN_BN4_SPRITE` in
`src/emu/xnavi_recipe.h`), and holds any one of them unpacked
(`tests/test_core.c` holds it to Duo's).

1. **The generator** places every Navi's battle sprite there
   (`tools/xnavirecipe_bn4.py battle_sprite_place`, which the families'
   `art_of` asks through `alloc.battle_place`): his first sprite row reads
   `XN_BN4_SPRITE, XN_BN4_SPRITE_SIZE`, and his art (his attacks' sprites,
   his songs) ends where the room begins. A Navi the other generators add
   takes the room with no change of theirs. GutsMan's art fell from 75 KB to
   15 KB.
2. **His install** (`xvirus.c sprites_set`, `src/emu/xnavi_bn4_room.c`)
   copies none of it: his list 2 entry (`XNAVI_SPRITE_BN4` + seat, 0x41) is
   pointed at the room, the sprite checked to fit it, and BN4's list 2
   number of it written at his seat in a table beside the homes
   (`XN_BN4_SPRITES`, a byte a seat, 0xFF none; GutsMan's 0x02).
3. **The fill**: as a battle's record is written (`emu_encounter_set`,
   `src/director/encounter.c`), before BN6 sets the Navi up at his spawn
   (his init's `sprite_load`), the room is filled with the sprite the table
   names for the BN4 Navi among its entities, read from the player's BN4 ROM
   (a raw one straight from its bytes, an LZ77'd one unpacked by
   `xnavi_sprite_bytes`, 61 KB at most taken from the heap and let go at
   once), in the core's copy alone.
   BN6 draws a list 2 sprite whose pointer has no bit 31 straight from
   where it points. Every battle a run fights and the differential's BN6
   run write their record so (`emu_battle_force`).

Proved: `gutsman_v1` PASSes with his sprite in the room, and the comparator
reads a pointer into the room as the battle's Navi's sprite
(`tools/xdiff_bn4.py sprite_rows`: a room row is his only where his
recipe's ids are the spec's foes, so a second Navi's row there is left out).
The mutation "GutsMan's battle sprite room filled with another's"
(`tests/xdiff/mutations.json`: his seat's byte made BN4's list 2 number 3)
is caught at t1, his sprite's frames. The New 3DS's heap (`python3
tools/heap3ds.py --seeds 1-3 --cases bn4,all`, 5 November 2026) as before
the room: BN6 and BN4 peak at 94.1 MB of 96 with none refused; all three
ROMs at 95.98 MB, 15 refused in 2 of 3 runs (the maps' mirror images, as
before). Those runs meet no BN4 Navi, so the fill's own unpacking is not
in them; the ten Soul Navis' raw sprites take none.

### Art and songs by battle

Built on 8 November 2026, the battle sprites' rule for the rest of what a
BN4 Navi draws and plays: his attacks' and effects' sprites and his pool's
songs loaded for his battle from the player's ROM into one room the
battles share, none resident. The art's room (`FREE_BN4_NAVI_ART`,
`XN_BN4_ART` to `XN_BN4_END`) ran out at SearchMan (below, SearchMan
built) and again at Roll, whose songs ended 0x5740 past it with seven
Navis in. Measured by the generator (`.build/xnavirecipe/bn4_report.md`,
The art's room, by battle), each Navi's sprites and songs, rounded to
their rooms: GutsMan 15.0 KB, ThunderMan 27.8, WindMan 40.8, FireMan 28.4,
SearchMan 71.8 (WindMan's effect sprite 5:0x00 among it, his own copy
now), AquaMan 34.2, Roll 29.8; the largest,
SearchMan's 0x11F00 bytes, in a room of 0x2C000. So the code and art
split SearchMan moved is back where it was (`+0x750000`): the code's room
0x20000, the art's 0x2C000 (the eleven Navis to come take the code's
room's rest, about 6 KB each so far).

1. **The generator** (`tools/xnavirecipe_bn4_art.py`) lays each Navi's art
   and songs out from the room's start, the Navis over one another. A
   sprite another Navi copied first (his list number: `navi_sprite`,
   `navi_effect_sprites`, `navi_shared`, as the fold kept them) is listed by
   each at a place of his own layout, past his own (`own_rows`): the list's
   number stays one, its place each Navi's. What a family copied stays in
   the families' art, resident, and is listed by none of them.
2. **His install** writes no byte of the room (`src/emu/xnavi_bn4_art.c`
   `xnavi_bn4_art_row`, from `xvirus.c sprites_set` through
   `xnavi_bn4_room_row`): each sprite checked to fit its room, its list
   entry pointed at his place where it is free (one a Navi before him
   pointed into the room kept); his songs packed only to name their
   headers for the pool's slots (`xnavi_bn4_sounds.c`); his seat's byte of
   a second table beside the homes made his own (`XN_BN4_ARTS`, past
   `XN_BN4_SPRITES`, 0xFF none; SearchMan's at `0x08F2C81C`, 0x03).
3. **The fill**: as a battle's record is written, with his battle sprite
   (`xnavi_bn4_room_for`), the room is filled with the art and songs of the
   Navi his seat's byte names (`xnavi_bn4_art_for`): each sprite read from
   the player's BN4 ROM (a raw one straight from its bytes, an LZ77'd one
   unpacked and let go), each song packed (`xsong_pack`, its header where
   the install named it), and each of his sprites' list entries pointed at
   his place, in the core's copy alone. The pool's slots are pointed at his
   songs before each frame as before (`xnavi_bn4_sounds_frame`).

The comparator reads a pointer into the art's room as the battle's Navi's
sprite alone (`tools/xdiff_bn4.py sprite_rows`: an `XN_BN4_ART` row is his
only where his recipe's ids are the spec's foes, as a battle sprite's
row). The mutation "SearchMan's art's room filled with another's" (his
seat's byte of `XN_BN4_ARTS` made AquaMan's seat, 0x0A: AquaMan's sprites
and songs loaded, SearchMan's entries left on another's bytes) is caught
at t216, his marker's sprite's frames. HEAP_RESULT

### Room for all eighteen, measured

Measured by the generator's walk of all eighteen (their recipes mostly
open but GutsMan's, so the counts approximate the others'): their code and
their objects' 0x18D52 bytes (about 100 KB); their battle sprites 0xAF000
(700 KB, each rounded to 4 KB: GutsMan's and Duo's 60 KB, TopMan's 20 KB);
their attacks' art 0x1D800 (118 KB). The room (448 KB since BN4's bracket
screen took its last 64 KB, `FREE_BRACKET`) holds the code in its 128 KB
and the attacks' art, but not the battle sprites: 256 KB of art holds
about four of them. So their battle sprites are loaded by battle, as
their sounds are, into one room ([Battle sprites by battle](#battle-sprites-by-battle),
above): one battle holds one BN4 Navi, and a tournament's rounds are
battles apart. The 16 MB copy has no other room past FREE_LIB_TEAM (24 KB).

## Companions

ThunderMan, WindMan and FireMan each spawn companions in their inits,
which BN6's actor init has none of. Three ports began with three ways to
make them again. One mechanism now covers all three, and the next Navis
take it as it stands (AquaMan's did, with his panel kept at his +6 and his
companion's kind read from its spawn, [AquaMan built](#aquaman-built)):

- **The generator reads every init the same way**
  (`tools/xnavirecipe_bn4_init.py` `init_calls`). A spawn is a call of a
  routine in a first-kind type's block that spawns that type
  (`BN4_SPAWN_1`, r0 a `movs`). Its registers are read: r4 a constant made
  by a `movs`, `lsls` and `adds` (FireMan's sprite word 0x810, WindMan's
  kinds 2 and 3), or r0 `hi` << 16 | a table's byte by the version with r1
  the version (ThunderMan's). So are the words the init keeps after it:
  the companion's +o given the Navi's address + v (WindMan's +0x60), the
  Navi's +o given the companion (FireMan's +0x74, WindMan's +0x64). The
  type moves with the Navi through the dispatch's first kind. The rest of
  the init is read too: its +0x11 written (FireMan's), its sprite updated
  (FireMan's), its update run (all three), its collision's flags
  (WindMan's).
- **Where the spawns are made.** Where the init spawns after its
  collision's set-up (ThunderMan's), they go at BN6's companion start,
  which follows the actor init's collision set-up
  (`BN6_ACTOR_COMPANION_CALL`). Where it spawns before (WindMan's, FireMan's)
  or updates its sprite, they go at the spawn's end (table 7), past the
  whole of BN6's actor init. The spawn's end also takes the init's other
  work, in this order: the collision's own r3 (`XNavi.coll_arg`), +0x11
  given +0x10, the spawns, the sprite update, BN6's Navi frame. A Navi
  whose init runs its update gets BN6's Navi frame there; running it or
  not leaves ThunderMan's first difference where it is.
- **The installer writes one or two shims of the Navi's home**
  (`src/emu/xnavi_init.c`, `XnSpawn` and `XnInit` in
  `src/emu/xnavi_recipe.h`): the project's own Thumb with a literal pool.
  A spawn is a `bl` to its moved spawner, so lr carries the Thumb bit.
  BN6's own routines, past a `bl`'s reach, are called by `mov r1,pc;
  adds r1,#5; mov lr,r1; bx r0`. The shims return by `pop {r4, pc}`, as
  BN6 calls both places by `mov lr,pc; bx r0`. GutsMan, with no init work
  past his collision's r3, keeps that shim alone, byte for byte as before.
  The three mechanisms before it are gone: `XnSpawned` and `spawned_set`
  (FireMan's), `XnCompanion4` and `companion4_set` (ThunderMan's),
  `XnCompanion` and `companions_set` (WindMan's).
- **Dispatch numbers.** Each moved first-kind type takes the dispatch's
  next number, after the families', in the order the generator derives the
  Navis. That order is the order by cost (`NAVIS`: GutsMan, ThunderMan,
  WindMan, FireMan): ThunderMan's 0x33 is 0xE0, WindMan's 0x5C 0xE1,
  FireMan's 0x57 0xE2. The three branches had each taken 0xE0.
- **Their battle sprites** are in the room the battles share
  (`battle_sprite_place`, `XN_BN4_SPRITE`), none resident: list 2 0x41
  GutsMan, 0x42 WindMan, 0x44 FireMan, 0x45 ThunderMan. The comparator's
  `sprite_rows` keeps the room's row only for the Navi whose ids
  (`.names`) the spec's foes name, so each spec reads its own Navi's
  sprite there.
- **Companions' interims**, kept by the module: the routines no battle of
  theirs reaches are left as BN4's bytes (`CUT`, `UNWALKED_TABLES`), and
  a mode is run as another (`COMPANION_MODES`). WindMan's companions'
  init (the first kind's 0x5C, `0x080CCD0A`-`0x080CCD30`) is the same
  choice FireMan's companion makes: 8 where `0x08007A8C`'s bit 0 is set,
  else 4 where its settings' +6 has 0x20, else 0. Its state 4 is BN4's raw
  bytes (`CUT`), and with them BN6's battle never reached its fight. Run
  as its state 0 (`movs r2,#4` at `0x080CCD2E` made 0, as FireMan's at
  `0x080CC464`), it does.

The comparator tells a moved first-kind object by its type, not its NameID
(`xdiff_snap.identity`, `T1_MOVED`; `xdiff_bn4.Bn4Canon.t1_moved`): BN4's
companions have id 0, which read as MegaMan's. Its record keeps its own +9
and id, and a spawn word's sprite number is read as the recipe's copy.
Where a mutation pokes BN5's code (`poke5`), the comparator no longer
looks among BN4's Navis' moved ranges (`src/dev/xdiff.c` pokes_put). A
BN4 block covering the same address, moved after BN5's, had taken
HotBody's sound mutation off its copy.

## ThunderMan built

The second in the order, proved: his type 0x32, Navi AI 53, ids
0x394-0x397 (`foes 394@52`), his sounds 0xAD and 0x143 at the pool's
0x32 and 0x33. The addresses and how each was read are in
[BN4 Red Sun's tables](../rom/bn4.md) (ThunderMan derived). What he adds
to GutsMan's path:

- **His state 1** only calls BN4's generic 1 (`push {lr}`, the call,
  `pop {pc}`), so BN6's generic 1 stands in (`own_generic`'s `wraps`).
- **His companion**, the first kind's 0x33 (`BN4_THUNDERCLOUDS`, which
  spawns his clouds, the third kind's 0x6A), set out after his collision's
  set-up with r0 0x14 << 16 | his version's byte of 20, 40, 60, 80 and r1
  his version: one `XnSpawn` at his companion start. BN6's spawn keeps the
  version at +4 as BN4's does. Its dying (`BN4_DYING`) is copied with him.
- **His objects** (`tools/xnavirecipe_bn4_objects.py`): the third kind's
  0x6A, 0x6F, 0x70 and 0x71 moved; the spawn word 0x04050301 a number;
  0x71's list 5 sprite (`list5_sprites`); his sound 0x143, made by a
  `movs` and an `adds` of 0xFF, at its slot (`wide_movs`). The can-move
  check's kin and an x nudge copied; the panel break against BN6's
  `0x0800CA8C`, audited; BN4's flag 0x1187 test and paused blink against
  BN6's battle_isPaused and a nullsub, audited as interims.
- **His palette**: read as FireMan's (below), V1-V3 0, as BN6's step 0
  gives. His branch had read BN4's two tables the other way round and
  called it an interim; that interim is gone.

- **His companion and cloud start with his states** (`XV_FIGHT_FLAGS`,
  `src/emu/xvirus_dispatch.c`; `tools/xnavirecipe_bn4_objects.py`
  `FIGHT_GATED`): neither runs a state. Each update runs where the battle's
  flags have bit 0, the fight's (BN4 `0x08007A8C`, BN6's `0x0800A2F0` the
  same body), else calls the paused blink alone (his companion's
  `0x080C3B42`, his cloud's `0x080DAB26`). BN4's state runner runs his
  states through the paused opening, so his first own state and his cloud's
  first gated frame are the fight's first in BN4. BN6's runs none there
  (the pause test in `0x0801B1C4`), so his first own state comes a frame
  into the fight, the battle's offset every BN4 spec allows. Before this,
  his cloud ran from the fight's first frame and was a step ahead of
  everything else in BN6 (`thunderman_v1` at t1: its +7 2 against 1, +0x3A
  6 against 5). His two gate calls now go to the dispatch's stub: the
  battle's flags, with bit 0 only once the battle timer is past 1. BN6's
  fight routine (`0x080080D2`) sets bit 0 and ticks the timer
  (`0x0800A6A6`) before the objects run, so the stub opens a frame later,
  together with his first own state, as BN4 has them. With that, his
  cloud's bolts, their hits on MegaMan, their sparks, sounds and draws all
  land at the battle's offset as everything else does. The gate is aimed by
  his BN4 type, so WindMan's and FireMan's companions keep theirs.
- **His cloud's animation from the opening** (legit.json's bn4
  `opening_animation`, `tools/xdiff_bn4_opening.py`): the cloud sets its
  animation in its init. The third kind's after-state (BN4 `0x080144FE`,
  BN6 `0x0801BCA6`) then runs the sprite update every frame with no pause
  test, its +0 0x04 keeping it running through the paused opening. A probe
  of hooks (temporary) saw BN4's opening 227 frames from the cloud's spawn
  to the fight and BN6's 206, from each engine's banner and each game's
  Custom screen. Each cloud enters the fight at its own place in its
  18-frame loop, BN4's 21 frames on. The comparator holds that place to the
  openings' lengths modulo the loop, exactly from the first animation set
  in the fight (t12).
- **What the comparator took besides**: his sounds paired by the routines
  that play them (0x148, 0xB8, 0x143; his 0x70's 0xAD at the pool's 0x32 in
  his battles alone, a row's `foes`, since Billy's thunder plays BN4's 0xAD
  as BN6's own 0xBA); his 0x70's +4 and his 0x71's +5 and +6, each its
  spawn's r4 that his states never set (`spawn_leftover`, both walked for a
  reader); a hit's spark taking no rank in the family's renderer places (its
  life is each engine's art); a frame of hits by element laid out in the
  BN4 trace (BN4's +0x2A and +0x3C, BN6's +0x82 and +0x94, by both hit
  passes and their sums); and BN6's own Elec tally for its freeze (+0x78),
  which BN4 has none of.

`bn4_thunderman_control` PASSes. **`thunderman_v1` PASSes** over its 1200
fight frames: his records, his companion's, his cloud's, every object he
spawns and MegaMan's HP alike. Four mutations of his port are caught: the
gate a frame early, his clouds' strength by another version, his bolt's
sound at another pool slot, and his cloud on another animation. He is
not yet in the tournament's list of proved BN4 Navis. A line there takes
his plate on BN4's bracket (`BN4_ENTRANT_ID`), and of ids 0x00-0x16 only
GutsMan's, AquaMan's, Roll's, SearchMan's and KendoMan's are named
([BN4's tournament screens](bn4-tournament.md)).

## WindMan built

The third in the order, proved by `windman_v1`: his type 0x30, Navi AI 50, ids
0x388-0x38B (`foes 388@52`). He was held from the install while his
battle could not reach its fight; with the companions' mechanism above it
does, and he is installed (`xnavi_bn4[]`; the generator's `HELD` is
empty). What he adds:

- **His own flinch and paralysis** (his table's 3 and 4): each ends his
  wind (his +0x60, `BN4_WINDMAN_WIND`) and runs BN4's generic state of its
  place, walked as his own with the tail aimed at BN6's.
- **His dying**, the families' (`BN4_DYING`): BN6's Navi object's dying
  stands in, as for every Navi (`DYINGS4`).
- **His collision's flags** (`BN4_SET_FLAG`'s 0x30): BN6's struct2 flags
  6, which BN6's actor init turns into the same flags and region 0x10.
- **His wind**: his decision spawns the fourth kind's 0x41 of his side
  (`BN4_WIND_SPAWN`), BN6's of the same steps (`BN6_WIND_SPAWN`); its slot
  is BattleState +0x88 in BN4 and +0xC0 in BN6.
- **His companions**, the first kind's 0x5C (`BN4_COMPANION`), kinds 2
  and 3, each one's +0x60 the address of his +0x60, his +0x64 the second:
  two `XnSpawn`s at his spawn's end (his init spawns them before its
  collision's set-up). Their kinds' sprites are copied by
  `bn4_twins.TABLES`. Their state 4 runs as 0 (Companions, above).
- **His sounds past 0xFF** (0x126, 0x133), made by a `movs` and an `adds`
  of 0xFF: the `movs` given the pool's slot, the `adds` 0 (`wide_movs`).

`bn4_windman_control` PASSes. **`windman_v1`** (WindMan V1 alone at (5,2)
against MegaMan idle at (2,2) with 2000 HP, as `gutsman_v1`) runs his
whole battle: his own records, his companions', his winds', his shots'
and their sounds are the same in BN4's and BN6's in all 1200 fight frames.
What it took, each from both games' code
([BN4 Red Sun's tables](../rom/bn4.md), WindMan's wind and its gust in
both engines; `tests/xdiff/legit.json`'s `bn4`):

- **The battle state's wind slots, mapped.** His wind's +0x38 is the r2
  the slot's reader leaves (BN4 `0x080E69E8`, BN6 `0x080E677C`), a pointer
  to his side's slot: BattleState +0x8C in BN4, +0xC4 in BN6. The
  comparator reads a pointer into BN4's battle state from +0x88 to +0x97
  as BN6's from +0xC0 (`bn4.battle_state`, `tools/xdiff_snap.py`
  `state_off`).
- **BN6's wind keeps its spawner.** BN6's spawn stores it at the wind's
  +0x4C (`0x080E6692`), BN4's has no such store, and BN6's wind reads it:
  its state 4 (`BN6_WIND_HOLD`) skips its step while the spawner's
  CurAction is 4 or his collision holds 0x80010000. Allowed on that word
  alone, BN4's 0 against a moved Navi in BN6 (`spawner_kept`). The hold
  differs from BN4's wind only in a frame where his CurAction is 4 before
  his state 4 has ended the wind, which no spec reaches yet.
- **His second wind's +0x3C**, the r3 his attack's state was run with
  (BN4 the battle state's pointer, BN6 3), is a spawn leftover as
  GutsMan's attack's is. No code of either wind reads it.
- **His sounds' copies paired**: 0xB2 (his air shot, the third kind's
  0x63) BN6's pool slot 0x32, 0x133 (the third kind's 0x65) slot 0xD0, as
  GutsMan's are; the wind's own start sound pairs BN4's 0x121 with BN6's
  0xAD by the two winds' inits.
- **A hit's spark takes no renderer rank.** Its life is each engine's own
  art's (allowed), so one still drawn in BN4 alone had moved his
  companions' tile places' ranks (t183).
- **The gust, each engine's own.** Each wind's step spawns the third
  kind's 0x49 (`BN4_GUST_SPAWN`, `BN6_GUST_SPAWN`), whose inits differ in
  one immediate: BN6's passes 0x80 to sprite_load (`0x080CEB90`), stored
  at its sprite state's +3 (+0x93 0x8E against 0x0E). With that immediate
  made 0 (`--poke 080CEB90:2000:2`) +0x93 is alike and nothing else
  moves: that bit allowed. Its frames' pointers are each game's own
  address of the same art (list 0xC's 0x2E, one CRC32 in both), read by
  their offset into it where both games' sprite events name the same
  bytes (`own_art`).

- **The gust pushing MegaMan, each engine's own slide.** BN6 keeps a
  step's kind at the object's +0x0F (`BN6_STEP_BEGIN` sets it, its slide's
  end clears it, its slide's step reads it), which BN4's twin
  (`BN4_STEP_BEGIN`) keeps nowhere: allowed on MegaMan, BN6's side alone.
  A push against the field's edge leaves BN4's slide step (+0x1B to +0x1D)
  where BN6's step routine returns zeros (it tests the panel inside,
  `0x0800E480`; BN4's after, `0x080102CC`): allowed, BN4's side alone. His
  collision's last move's direction (+0x0C) is the same move's in both but
  held by another clock (BN4's collision panels' update writes it on the
  frame his panel changes, BN6's slide once at its end and keeps it):
  allowed only where one side holds 0 and the other his last move's
  direction (`slide_direction`).
- **The world's natives ranked as the family is.** The gusts' renderer
  places are read as their ranks among the natives drawn (`is_native`,
  `ranks_into`), MegaMan's own art having moved their raw places.
- **A slot let go, named by its last occupant** (`let_go`): a gust keeps a
  pointer to its wind after the wind is let go, and BN6's slots are one on
  (its MegaMan's companion holds a fourth-kind slot BN4's has free).

So **`windman_v1` PASSES**: 1200 fight frames, the family's records and the
world's the same, its recorded verdict removed. Four mutations of his port
are caught (`tests/xdiff/mutations.json`): his air shot's sound at another
pool slot (t105), his wind spawned with another r4 (t1), his air shot's
collision word left at BN4's place (t179), his air shot's speed shifted
(t104).

His companions' state 4 is not reached here: both games' companions run
CurAction 8 from the fight's first frame, their init's choice where
`0x08007A8C`'s bit 0 is set (`0x080CCD0A`-`0x080CCD14`); their first state
made 0 in BN6's copy (`--poke 080CCD0E:2200`) still PASSes, the state 0
they begin in leading to the same 8 before the fight. The interim that runs
4 as 0 stays as the Companions section says, unproved either way.

**Not yet in the tournament's `proved[]`**: a line there needs his plate on
BN4's bracket (`BN4_ENTRANT_ID`), and [BN4's tournament
screens](bn4-tournament.md) names five plates by sight, his not among them.
A match of each plate's colours against the Navis' battle sprites' failed
its controls (GutsMan's, Roll's and KendoMan's plates were not their best
match), so it names nothing. His plate can be read from his battles
(`BN4_OPPONENT_BATTLES`: entrant 0x00's first battle 4, 0x01's 8, and so
on) once the battle a number starts is located, or named by sight. Two
searches for that came up empty (8 November 2026): no run of 12-byte battle
records in BN4's ROM indexed by those numbers puts GutsMan, AquaMan, Roll,
SearchMan and KendoMan at their plates' battles, and no table of pointers
to battle records puts GutsMan at 4, AquaMan at 0x10 and Roll at 0x1C.

## FireMan built

The fourth in the order, proved by `fireman_v1`: his type 0x0C, Navi AI 52, ids
0x390-0x393 (`foes 390@52`). His addresses are in
[BN4 Red Sun's tables](../rom/bn4.md) (FireMan derived). What he adds
over GutsMan:

- **A call after the runner** (`navi_update`): his update calls the
  pre-step, the runner, his own `0x080B81B0`, the after-step and the
  present. His call is BN6's post hook (table 5), which BN6's Navi frame
  runs after its status dispatcher (`0x080F36A8`). It tells his companion
  to set its animation up again where his own changed. His after-step runs
  before his present in BN4 and after it in BN6; no spec has shown the
  order to matter.
- **His own generic states 3-5** (`own_generic`): each tells his companion
  and ends in BN4's generic state of its place, its tail aimed at BN6's.
- **His init's work at his spawn's end** (one `XnInit`): his collision's
  own r3 (0x21), his +0x11 his +0x10 (BN4's init writes 0, BN6's actor
  init 0xFF), his companion spawned (r4 0xD710: list 4, the copy of BN4's
  sprite 8) and kept at his +0x74, his sprite updated, BN6's Navi frame.
- **His companion**, the first kind's 0x57: a sprite drawn on him (list
  4's 8), its sprite setters paired, its sprite step copied (BN6's
  nearest, `0x0801BBF4`, updates the sprite in the frame a new animation
  is set up, a frame ahead of BN4's). Its mode 4, his flame's appear
  through BN4's tile buffer, runs as mode 0 (`COMPANION_MODES`): an
  interim neither spec reaches.
- **The rest, as GutsMan's machinery takes it**: six third-kind objects
  moved (0x09, 0x0A, 0x19, 0x1A, 0x1F, 0x21); the attack's end that keeps
  the attack's +5 at AI data +0x3A copied (`BN4_ATTACK_END_KEEP`); five
  new pairs in `tools/bn4_twins.py`, two audited; the third kind's 0x1A's
  kinds' sprite table (`SPRITE_TABLES`); a table in MegaMan's block past
  the last code walked (`MM_TABLE_SIZES`).
- **The palette, read right**: `BN4_NAVI_PALETTE` takes its number's byte
  at `0x0800B9B1` times the version's at `0x0800B9AC` (0, 0, 0, 2). Every
  Navi so far gives 1 or 2 by its number, so V1-V3 have palette 0, as
  BN6's step 0 gives; their SP palettes are an interim.

`bn4_fireman_control` PASSes. **`fireman_v1` PASSes** over its 1200
frames (his decision, his flames and their chain, his companion, his
sounds), its recorded verdict removed. What took it there from t1, one
root cause at a time:

1. **His companion's sprite count at t1** (`obj +0x91`, BN4 2, BN6 1): its
   init (`0x080CC3F8`, run by his at his spawn, in the paused opening) sets
   its animation up and runs the sprite update once; its mode 0
   (`0x080CC4D4`) ends in `BN4_FIRE_COMPANION_SPRITE` (`0x080CC5D0`), which
   runs the after-step (`0x080143FC`, BN6's twin `BN6_AFTER_STEP`
   `0x0801BBAC`, the same but for the pause test's routine) unless time is
   stopped. The after-step runs the sprite update at every fight frame and
   none of the paused opening's, so the count is the frame's clock, as
   GutsMan's own (`opening_count`): 2 at t1 in BN4 and at BN6's t1, 1 at
   BN6's t2. Not the port's: the comparator reads a Navi's companion's
   opening count at the same fight frame, BN4's t1 and t2 alone
   (legit.json's bn4 `companion_opening_count`, `tools/xdiff_bn4.py`
   `companion_count`). The animation it copies from him from his first
   state's on is a state's and follows with the battle's offset.
2. **His flame's element** (t237, the third kind's 0x09: its +0xE BN4 1,
   BN6 0, and its collision's hit kind): its spawner
   (`BN4_FIRE_FLAME_SPAWN`, `0x080CE49C`) stores its element at BN4's +0x17
   (the recipe's BN6's +0xE) and then its palette at BN4's +0xE
   (`0x080CE4BC`), which in BN6 fell on the element. The port's: a moved
   third-kind type's palette (`BN4_T3_PALETTE`, its sprite's +4 by
   `BN4_SPRITE_PALETTE_SET`) is BN6's +0xF (`BN6_T3_PALETTE`), which BN6's
   engine reads only on its actors and the trace lays BN4's +0xE at:
   `tools/xrecipe_bn4.py` `palette_imms`, its init's read (`0x080CE358`), its
   chain's (`0x080CE486`) and the spawner's store. No family's code is
   changed by it.
3. **His sounds** (t238): his copies at the pool's slots (0xA8 at 0x33,
   0x8F at 0x32, 0xA9 at 0x63, 0xC0 at 0xD0) paired by the routines that
   play them (legit.json's bn4 sounds), as GutsMan's.
4. **The renderer's ranks** (t271, his companion's tile place, BN4 rank 3,
   BN6 2): the spark of his flame's hit on MegaMan is each engine's own art
   and life (allowed); BN4's was still drawn, between his flames and his
   companion, where BN6's had ended, and took a rank in BN4 alone. The
   family's renderer ranks are read without a hit's spark (legit.json's bn4
   `spark_ranks`, `tools/xdiff_bn4.py` `ranks_into`, the rule ThunderMan's and
   WindMan's battles found too).

The order of his after-step and present (BN4 before, BN6 after) made no
difference in the 1200 frames. Six mutations break his port and each is
caught: his flame's palette stored at BN4's +0xE (t237), its element at
BN4's +0x17 (t237), its palette read at BN4's +0xE (t237), his sound 0xA8
at the pool's first slot (t238), his decision's CurAction off by one (t1)
and his companion's tell at its +0x10 (t335); with the rest, 193 of 193.

Two interims stay, neither reached by a spec: his companion's mode 4 (its
appear through BN4's tile buffer, where the battle's settings' +6 has
0x20) runs as mode 0, as WindMan's companions' state 4; his SP's palette
(`BN4_NAVI_PALETTE` gives SP 2, BN6's step 0 none).

**The three merged** (8 November 2026): ThunderMan's, WindMan's and
FireMan's proofs, each made apart from the same start, in one tree, every
spec passing together. Each had left a hit's spark out of the renderer's
ranks; one rule does it now (`ranks_into`, legit.json's bn4 `spark_ranks`).
ThunderMan's gate (`XV_FIGHT_FLAGS`, `FIGHT_GATED`) stays his type's
alone: WindMan's and FireMan's code reads the same battle flags
(`0x08007A8C`, called at BN6's `0x0800A2F0`; WindMan's companions' init
chooses their first state by its bit 0) and their specs pass without the
gate, no object of theirs a step ahead. Every row the three added to
legit.json and every mutation is kept: merged, every spec holds its
verdict (the eighteen BN4 family specs, the four Navis' and every BN4
control PASS) and `--mutations` catches 201 of 201.

**Not yet in the tournament's pool**: `src/layer/xguardian_bn4.c`'s line
needs his plate on BN4's bracket (`BN4_ENTRANT_ID`), and his is not named.
By sight only GutsMan's 0x00, AquaMan's 0x03, Roll's 0x04, SearchMan's
0x09 and KendoMan's 0x11 are ([BN4's tournament screens](bn4-tournament.md));
his V1's battle record is `0x080FAA08` (number 22 of the records
`0x08007A94` reads at `0x080FA900` + 12 x n), but how
`BN4_OPPONENT_BATTLES`' numbers (GutsMan's plate 0x00 gives 4, his record's
number 13) reach a record is not read: neither a search for
the readers of `[eToolkit +0x44] +0xA` (one, a script's comparison at
`0x0802C688`) nor for a table from the numbers to the records found it.
A palette-distance match of the Navis' battle sprites against the plates
named none of the known five right, so it names none. Of the four, GutsMan
alone is in the pool.

## SearchMan built

The fifth in the order, proved by `searchman_v1`: his type 0x2B, Navi AI 51,
ids 0x38C-0x38F (`foes 38C@52`), in the tournament's pool on his plate
0x09 (`BN4_PLATE_SEARCHMAN`, named by sight). His addresses are in
[BN4 Red Sun's tables](../rom/bn4.md) (SearchMan derived and his hiding in
both engines). What he adds over the four before him:

- **His hiding, the engine's invisibility at BN6's places.** BN4's
  collision data keeps the invisibility's frames at +0x1A
  (`BN4_COLL_INVISIBLE`) where BN6 keeps them at +0x26
  (`BN6_COLL_INVISIBLE`): each game's timer is the same routine at its
  offset (`BN4_INVISIBLE_STEP`, `BN6_INVISIBLE_STEP`), so are the clear on
  a hit and the two setters his states call (`BN4_HIDE`, `BN4_SHOW`; BN6's
  twins `BN6_HIDE`, `BN6_SHOW`, LAYOUT pairs in
  `tools/xnavirecipe_bn4_objects.py`). His own code's read of +0x1A is
  given BN6's +0x26 (the generator's collision map, `COLL4TO6`).
- **The hide step, run before each of his states.** BN4's Navi and
  player runners call the hide step before every state
  (`BN4_HIDE_STEP`, at `0x08013F12`), after the blink step has set the
  object's drawn bit: the invisible is not drawn, and a marker (the fourth
  kind's 0x3E) is kept at the collision data's +0x7C. BN6's status step
  has the same place but calls nothing there (`BN6_STATUS_HIDE`, a flag
  read); its own twin, `BN6_HIDE_STEP`, has no caller. Before the port
  BN6's SearchMan stayed drawn while hidden (`searchman_v1` at t216, his
  +0 0x37 against 0x31). The port: BN4's hide step is copied with him
  (its +0x7C BN6's +0x50, `BN6_COLL_HIDE_MARKER`) and run by stubs of his
  home before each of his states (`XNavi.hide_step`, `src/emu/xnavi_init.c`
  `hide_set`: each state's stub loads the state and branches to a shared
  part that calls the hide step and goes on to the state with the
  dispatcher's return). BN6's own hide step would not do: its marker, BN6's
  0x3E, draws other art and spawns effect 0x5D on its first update
  (`0x080E60B2`), so BN4's 0x3E is moved with him (`MOVE4`).
- **Fourth-kind objects moved.** His scope (the fourth kind's 0x2E,
  `BN4_T4_SCOPE`: it walks over the panels toward MegaMan and tells his
  AI data's attack variables its lock, `0x080E571A`, at BN6's +0xA0), his
  shots' trails (0x31, its kinds' sprites in a table before its routine,
  `T4_SPRITE_TABLES`; BN6's 0x31 runs its kinds by its own tables) and his
  marker are the first fourth-kind types a BN4 Navi moves: the generator's
  `moved4` takes a call into a fourth-kind type's spawner with no BN6
  twin, or one of `MOVE4`, and its type the dispatch's fourth kind's next
  number (0xE0 on). The comparator reads them back as BN4's
  (`tools/xdiff_bn4.py` `t4`, `tools/xdiff_snap.py` `identity`).
- **Effects by other routes.** His shot's hit spawns effect 8 with a flip
  ORed into r4 (`FLIPPED_EFFECTS`), and his shot's burst passes effect 0
  in r2 to the engine's pattern spawner (`0x0801457E`, BN6's
  `0x0801BD3C` the same bytes; `EFFECT_CALLS`, read by `pattern_effects`
  since the fold, below): both moved, BN4's look in
  BN6 (0x9C, and WindMan's 0x9B, whose sprite he lists too so his install
  finds it, `navi_shared`).
- **Sounds past a byte.** Nine pool sounds, four of them made by a `movs`
  and an `adds` at slots past a byte (0x105, 0x109, 0x128, 0x12F at
  0x104, 0x117, 0x148, 0x150): their calls go through the shim
  (`XvSoundCall`), the `movs` and `adds` left as BN4's.
- **Copied leaves**: a side's panels by flags (`0x0800A2E4`) and his
  shot's hits test (`0x080DF332`, its +0x54 BN6's +0x70); the visibility
  test his marker calls (`0x0800B0CA`) paired with BN6's `0x0800EB6C`,
  audited (the same answer wherever the side is not MegaMan's).

`bn4_searchman_control` PASSes. **`searchman_v1` PASSes** over its 1200
frames (his decision, hiding, shots, bursts, trails and sounds; not his scope, below).
What took it there from t1, one cause at a time: the install refused (two
copied routines overlapping his moved regions: a copied routine inside a
moved region is that region's code now); his invisibility's frames and
marker unmapped in the BN4 trace (`src/dev/xdiff_bn4_trace.c`, t215 and
t216); the hide step (t216, above); BN6's own marker (t216); his pool
sounds (legit.json's bn4 sounds, his ids alone by `foes`); BN6's own
effect 0 for his burst (t384); BN6's own 0x31 for his trails (t741); and
one engine difference, allowed: his trail's kind 0 keeps what
PlaySoundEffect leaves in r0 as its begun byte (+9, `0x080E597A`) and only
tests it against 0, and each game's MP2K leaves its own there (BN4 0x28,
BN6 0x48; legit.json's bn4 `spawn_leftover` bytes, that pair alone).

Seven mutations break his port and each is caught: his hide step's marker
read at BN4's +0x7C (t217), its clear of the drawn bit made a set (t216),
his invisibility read at BN4's +0x1A (t218), his trail spawned as another
kind (t741), his sound 0x90 at the pool's second slot (t250), his burst's
effect left at BN4's id (t384), his decision's CurAction 8 off by one
(t211); with the rest, 208 of 208.

**What `searchman_v1` does not reach.** In its 1200 frames, MegaMan idle,
SearchMan runs his decision and MegaMan's states 58-60 (BN4's CurActions
6-9), never 61 (his 10): his scope (the fourth kind's 0x2E), the third
kind's 0x60 and the trails' kinds 1 and 2 it spawns are moved and
installed but unproved. Two first mutations, on his scope's lock and his
CurAction 10, were not caught for that, and were replaced by the two
above. A spec where his decision takes state 61 (MegaMan moving, or his
SP) is the next to write.

**The art room ran out with him.** The generator's art room (attacks'
sprites and songs, `FREE_BN4_NAVI_ART`, 0x2C000) held the four Navis
before him and not his 49 KB (his songs ended 0x300 past it), while the
code room (0x20000) holds the five's code in about 30 KB. The split moved, an interim:
the art from `+0x748000` (0x34000), the code room 0x18000. The [room for
all eighteen](#room-for-all-eighteen-measured) underestimated the art
(songs uncounted, small sprites rounded to 0x800): at about 40 KB a Navi
the eighteen need more than the room holds, so a Navi's attacks' art and
songs will have to be loaded by battle, as the battle sprites are; the
owner's call. Done since ([Art and songs by battle](#art-and-songs-by-battle)):
the split is back at `+0x750000`.

## AquaMan built

The sixth proved, the first of Blue Moon's four, by `aquaman_v1`: his type
0x0D, Navi AI 58, ids 0x3A8-0x3AB (`foes 3A8@52`), his seat 10 by his BN4
id (168). His addresses are in [BN4 Red Sun's tables](../rom/bn4.md)
(AquaMan derived). He adds no new machinery to the Soul Navis' path; what
his recipe asked of the generator:

- **His panel at his +6** (`BN4_AQUAMAN_PANEL`): his init stores his
  column and row there (`0x080B8264`: `ldrh r0,[r5,#0x12]; strh
  r0,[r5,#6]`), his decision reads it as his last panel and writes it anew
  (`0x080B833E`, `0x080B8344`); BN6's spawn leaves a Navi's +6 0. The
  generator reads the pair in his init (`tools/xnavirecipe_bn4_init.py`
  `HOME_FORM`), and his spawn's end makes it (`XnInit.home`,
  `src/emu/xnavi_init.c`: `ldrh r0,[r5,#0x12]; strh r0,[r5,#6]` after his
  collision's r3, before his companion's spawn, where BN4's init makes it).
- **His companion**, the first kind's 0x5C as WindMan's, its kind 1 (his
  spawn's r4): the kinds' table (`BN4_COMPANION_KINDS`) is read by the kinds
  his spawns name (`table_kinds1`), so kind 1's sprite, list 4's 0x0C, is
  copied and its byte given the copy's. He moves his own copy of the type,
  at the dispatch's 0xE3. Kind 1's routine (`BN4_COMPANION_KIND1`) hides it
  while his animation is not 0 and sets its animation up again once it is.
- **His effects**: 0x23 spawned with his side in r4's second byte (his
  copied MegaMan state, `lsls r4,r0,#8; adds r4,#0x23` at `0x080F459E`:
  the `adds` given the moved id, `facing_effects`); 0x25 handed in r2 to an
  engine routine that spawns it on the panels around a place by a pattern
  (`BN4_EFFECT_PATTERN`, BN6's twin `BN6_EFFECT_PATTERN` the same body:
  `EFFECT_CALLS`, `pattern_effects`). An effect's sprite another Navi
  copied (WindMan's 5:0x00) is listed by him too at its place, as are his
  objects' sprites another copied (FireMan's list 3's 0x02 for his third
  kind 0x24, `navi_sprite`): the installer finds an effect's sprite only
  among the recipe's and the families' rows (`xvirus.c` `sprite_of`), and
  without it his install was refused. An effect of a sprite his objects
  draw takes their copy (his 0x23, list 4's 0x0F), not a second.
- **The rest, as GutsMan's machinery takes it**: three third-kind objects
  moved (0x23, 0x24, 0x27); the spawn word `0x04050201` a number (the
  collision region's, BN6's own attacks pass it too); his five sounds in
  the pool (0x90 at 0x32, 0x9E at 0x33, 0xA0 at 0x63, 0xA1 at 0xD0, 0xC8 at
  0xE0); his collision's own r3 0x21; his palette 0 for V1-V3 (SP's 2 the
  interim GutsMan's and FireMan's share).

`bn4_aquaman_control` PASSes. **`aquaman_v1` PASSes** over its 1200
frames (his decision, his attacks' three objects and their effects, his
companion, his sounds). What took it there, one root cause at a time:

1. **His install refused** ("its effect 0 draws none of its sprites"): his
   effect 0's sprite was WindMan's copy, which the installer does not look
   among; his recipe lists it now (above).
2. **His +6 and +7 at t1** (BN4 5 and 2, BN6 0 and 0): his panel, kept by
   his init (above).
3. **His companion's sprite at t2** (its count, BN4 0 and BN6 2, and its
   frame): its init sets its animation up in the paused opening, and its
   mode 8 runs the after-step (BN4 `0x080143FC` at `0x080CCEB0`, BN6's twin
   `0x0801BBAC`), whose pause test opens at the fight's first frame in both
   games: its animation runs by the fight's frames, not by his state runner,
   which BN6 opens a frame later (the battle's offset). FireMan's companion
   copies his animation from his first state on (`companion_opening_count`,
   two frames); AquaMan's keeps its own until kind 1's routine sets it up
   again from his state (BN4's t11, BN6's t12). The comparator compares a
   held companion's sprite state at the same fight frame until it first
   steps with the battle's offset (legit.json's bn4 `companion_clock`,
   `tools/xdiff_bn4_opening.py` `CompanionClock`); in `aquaman_v1` its count
   reads 1, 0, 2, 1, 0, 2, 1, 0, 2, 1 at BN4's t1-t10 and BN6's of the same
   fight frames.
4. **His sounds** (t32): his copies at the pool's slots paired by the
   routines that play them, in his battles alone (`foes`: the pool's slot
   is each Navi's own; WindMan's 0xC8 is at 0x33).
5. **His effect 0x25 on the panels** (t51): BN6's own effect 0x25 around
   his third kind 0x23, where the effect his code hands
   `BN4_EFFECT_PATTERN` is moved (above).
6. **His third kind 0x24's sprite** (t525): his copy of list 3's 0x02
   and FireMan's, two rows of one BN4 sprite, read by the comparator as the
   first; FireMan's copy is his too now (above).

Six mutations break his port and each is caught: his panel's store at his
spawn's end a nop (t1), his companion's kind 1 sprite another's (t1, through
the companion's clock), his panels' effect left BN4's 0x25 (t51), his
side's effect left BN4's 0x23 (t31), his sound 0x9E at the pool's first slot
(t32) and his decision's CurAction off by one (t125).

**In the tournament's pool**: `src/layer/xguardian_bn4.c`'s line, his V1
id 0xA8 on his own plate (`BN4_PLATE_AQUAMAN`, 0x03), "The Rushing Tide"
in a water blue (the owner's to change). `LAYER_MAKE` stays 112.

Interims left, none reached by `aquaman_v1`: the 0x5C's state 4 run as 0
(WindMan's, Companions); his SP's palette; sounds past a byte his code names
none of.

## Roll built

The seventh in the order, built on 8 November 2026 and proved to her summon:
her type 0x1D, Navi AI 48, ids 0x380-0x383 (`foes 380@52`), her seat 0.
Her addresses and how each was read are in [BN4 Red Sun's
tables](../rom/bn4.md) (Roll derived). What she adds:

- **A call before her pre-step** (`navi_update`): her update calls
  `0x080BC340` (her summons' words, +0x60 and +0x64, let go where their HP
  is 0), then the pre-step, the runner and the present; her type routine
  runs the after-step. Her call is BN6's hook A (table 2), which BN6's Navi
  frame runs after its pre-step (`0x080F368C`); the pre-step reads and
  writes her own collision and statuses, her call her words and her
  summons' HP, so the order is not seen.
- **Her own states 1 and 2**: each does a little (+6 cleared; her
  summons' HP made 0 as she is deleted) and ends in BN4's generic state of
  its place, its tail aimed at BN6's (`own_generic`).
- **MegaMan's 44-48** as her 7-11: 44 her attack with the third kind's
  0x02 (kind 7, its sprite in a kinds' table of list 3, copied into list 4:
  `tools/xnavirecipe_bn4_objects.py` `list_tables`), 45 a shot (the third
  kind's 0x0B, CirKill's shot, its kind her version's byte of `0x080BC300`:
  `KINDS_BY_VERSION`), 46 her warp, 47 her summon, 48 a move with her
  callback. 46 and 47 spawn her flash, the fourth kind's 0x17, which BN6
  has no type of: moved with her through the dispatch's fourth kind (the
  first moved fourth-kind type: `tools/xnavirecipe_bn4_summon.py`
  `MOVED4`, `xrecipe_bn4.spawns_of` taking a `moved4`), its effect 0x2E
  named by `lsls #8` and `adds` (its parameter a draw), moved as BN6's 0x9C
  (`xrecipe_bn4.effects_of`).
- **Her summon** (`tools/xnavirecipe_bn4_summon.py`): MegaMan's 47 calls
  `BN4_SPAWN_ENEMY` with an enemy id from her version's list
  (`BN4_ROLL_SUMMONS`: V1 1; V2 2 and 0xE; V3 3, 0xF and 0x39; SP 4 and
  0x10). BN4's picks a free panel of her side (its masks by side and one
  draw), spawns the record's first-kind type with +5 2 (BN4's summoned
  appear, `0x0801002A`) and writes its fields from the record. BN6's spawn
  of an enemy into a fight under way, `BN6_SPAWN_ENEMY_1`, does the same
  from BN6's records (its AI data's +2 1, BN6's summoned appear
  `0x080164A0`), so the routine is not copied: the call is aimed at a shim
  in her code room (`XnSummon`, written by `src/emu/xnavi_init.c` with
  `xvirus_shim_summon`) that picks the panel as BN4's does (her copy of
  `BN4_FREE_PANELS`, the same masks, one draw of BN6's GetRNG twin), gives
  the id BN6's and calls BN6's routine, then plays her summon's sound. Its
  twin audited: the panel, id, HP and side the same at t1008. The ids:
  Billy's are moved (0xE-0x10 as 0x2C1-0x2C3, its recipe's); Mettaur's
  (1-4) are BN6's own Mettaur by name, as a run's visitors take them
  ([Battle Network 4 in a run](bn4.md), Only what is new visits); 0x39
  (V3's, its type 0x11) is moved by no recipe and summons nothing yet, an
  interim. Her +0x60 and +0x64 hold BN6's objects as BN4's.
- **Her sounds' player**: BN4 plays 0x96 and 0x131 on its player 22 and
  its hit sound 0x6B on 10; BN6 keeps its 22 for its hit sound 0x6B alone.
  On BN4's player, BN6's hit sound cut her attack's 0x131 (`roll_v1` at
  t204), so a pooled song whose BN4 player BN6 keeps for one song of its
  own goes on the BN6 player most of that BN4 player's numbers take in BN6,
  9 (`tools/xnavirecipe_bn4.py` `song_player`, `XvSong.player`). No other
  BN4 Navi's pooled song is on such a player.

`bn4_roll_control` PASSes (her summon at t1008 among it). **`roll_v1`**
(Roll V1 alone at (5,2) against MegaMan idle at (2,2) with 2000 HP, as
`gutsman_v1`) is the same in t1-t1007: her decision, MegaMan's 44-48, her
flash and its effects, her shots and their hits, her pool sounds, every
draw replayed. What the comparator took: her flash's +4, the spawn's r4
that her states never set (`spawn_leftover`, BN4 0x13, BN6 her AI data's
address); a moved fourth-kind type read back as BN4's (`Bn4Canon.t4`,
`xdiff_snap.identity`); her pool sounds paired (0x96 at 0x32, 0xCE at
0x33, 0x131 at 0x63, 0x132 at 0xD0, in her battles alone).

**At t1008 her summon's Mettaur is each engine's own**: the same panel,
id, HP and side, but BN4's Mettaur runs BN4's code (its type 2, +5 2, its
+0xB, +0xF and +0x20, its sprite, its collision's +0x28, its panel
reserved) and BN6's runs BN6's. Neither the port nor the comparator can
make them one: BN4's Mettaur has not been mapped onto BN6's
(`bn4_mettaur_control` against BN6's own parts at t1), and moving it as a
family takes twelve decisions of the families' generator (four own
states, five calls with no twin, a literal, a sprite and a sound). Read
from the two traces after t1008: both Mettaurs step from (4,1) to (4,2),
but on different clocks, and BN6's shockwave strikes MegaMan at t1195
(1880 to 1870 HP), BN4's not by t1200. So a native's reading would end
INCONCLUSIVE there too. `roll_v1` records FAIL at t1008, and Roll is not
in the tournament's proved list: the owner's call is whether her summon
brings BN6's Mettaur (then the differential compares it as a native, the
world from then on) or BN4's moved for her.

Seven mutations break her port and each is caught: her decision's
CurAction off by one (t1), her AI block at BN4's +0x64 (t1), her flash's
effect another (t66), her warp's sound at another pool slot (t67), her
attack's sprite at BN4's list (t195), her player-22 sound on BN6's
hit player (t204) and her shot of another kind (t385).

## SearchMan, AquaMan and Roll folded

On 8 November 2026 the three branches built side by side from the same
`dev` were merged into one: `NAVIS` reads GutsMan, ThunderMan, WindMan,
FireMan, SearchMan, AquaMan, Roll, which numbers their moved types, list
copies and effects in that order (AquaMan's third kinds from the dispatch's
0xF9, his companion's sprite copy 4:0xDF, his side's effect 0x9D; Roll's
types 0xFC, 0xFD and 4:0xE3, her flash's effect 0x9E); their seats, homes,
AI indices and ids are by their BN4 ids and did not move. What the merge
settled:

- **Effects passed in a register.** SearchMan's `EFFECT_ARGS` and
  AquaMan's `EFFECT_CALLS` read the same engine routine's calls
  (`0x0801457E`, the pattern spawner): one is kept, AquaMan's
  `pattern_effects`, run inside `effects_of` after the side's and the
  flipped effects (`facing_effects`, SearchMan's `flipped_effects`), so
  SearchMan's recipe is the same bytes as on his branch.
- **The comparator's fourth kind**: SearchMan's `t4` (a moved object's
  type read back as BN4's) kept, which takes Roll's flash too.
- **The generated files** written again, never merged by hand:
  `src/emu/xnavi_bn4.c`, the symbol files, the status; `docs/re/ledger.json`
  merged by hand, every entry kept.
- **The generator's file** passed 1000 lines with Roll: its sounds (the
  pool, the tables' sounds, the calls past a byte, the songs and their
  players) are `tools/xnavirecipe_bn4_sounds.py` now, and the art's room
  `tools/xnavirecipe_bn4_art.py` ([Art and songs by battle](#art-and-songs-by-battle)).
- **The mutations' numbers**: AquaMan's companion's poke follows his copy
  (0xE0, his third kind 0x23's sprite), his and Roll's texts their moved
  effects' new ids; SearchMan's numbers did not move.

Roll's songs ended past the art's room with the seven in, and the room
was made one by battle before any spec ran on the fold.

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
