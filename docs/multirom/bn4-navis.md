---
type: Research
title: "BN4's Navis in BN6's engine"
description: "What moving BN4 Red Sun's eighteen new Navis into BN6's battle engine takes, Navi by Navi: how each runs (two builds, the Soul Navis on MegaMan's own states), what it calls and spawns against BN6's twins, the order by cost, the engine routines to port once and what the Navi recipe generator needs for BN4."
tags: [multirom, bn4, navis, research]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-05T17:40:00Z }
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
below), FireMan built and under proof ([FireMan built](#fireman-built)). BN5's Navis moved as
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
  (`XNAVI_SPRITE_BN4`, GutsMan's 0x41); his palette 0 for every version
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

Then: the next Navi.

## FireMan built

FireMan (BN4's type 0x0C, ids 144-147) is the second Navi `tools/xnavirecipe_bn4.py`
derives whole (`NAVIS`), at seat 4: BN6's Navi AI 52, ids 0x390-0x393 (`foes 390@52`), his battle
sprite at list 2 0x44. His addresses and how each was read are in [BN4 Red Sun's tables](../rom/bn4.md)
(FireMan derived). What he adds over GutsMan, each a function of the generator's own:

- **A call after the runner** (`navi_update`): his type routine runs its jump table and nothing after
  it; his update calls the pre-step, the runner, his own `0x080B81B0`, the after-step and the present.
  His call is BN6's post hook (table 5), which BN6's Navi frame runs after its status dispatcher
  (`0x080F36A8`) as BN4's runs it after the runner. It tells his companion to set its animation up
  again where his own changed (+0x10 against +0x11). His after-step runs before his present in BN4
  and after it in BN6 (the Navi object's routine, `0x080F3664`); no spec has shown the order to matter.
- **His own generic states 3-5** (`own_generic`): each tells his companion (on its first frame) and
  ends in BN4's generic state of its place. Each is his own in BN6's table at that place, its tail
  aimed at BN6's generic 3, 4 and 5 (flinching, paralysed, dragged). No spec reaches them: MegaMan
  idle never makes him flinch.
- **His init's tail at BN6's spawn's end** (`init_tail`, `XnSpawned`; `xnavi_install.c`
  `spawned_set`): his init updates his sprite, spawns his companion and keeps it at his +0x74, and
  runs his whole update, where GutsMan's runs nothing. BN6's actor init does none of these, so a shim
  of his home is his spawn's end hook (table 7, where GutsMan's collision argument shim goes). It
  writes his collision's own r3 (0x21), his +0x11 his +0x10 (BN4's init writes 0, BN6's actor init
  0xFF), spawns the companion through its moved spawner (r4 0xCF10: list 4, the copy of BN4's sprite
  8) and stores it at +0x74. Then it runs BN6's sprite update and BN6's Navi frame (`BN6_NAVI_FRAME`).
  Each call is `mov r1,pc; adds r1,#5; mov lr,r1; bx r0`, so lr carries a bl's Thumb bit; the
  shim returns by `pop {pc}`.
- **His companion**, the first kind's 0x57 (`init_tail`, `spawns1_of`, `companion_art`): a sprite
  drawn on him (list 4's 8, his animation plus its +7, his place and flags copied each frame). It is
  moved as the families' third-kind objects are, through the dispatch's first kind (its number 0xE0).
  Its five calls out are paired: two sprite setters by their bytes, BN6's `0x08002DB4` and
  `0x08002F02`, and its sprite step `0x08014446` copied (BN6's nearest, `0x0801BBF4`, updates the
  sprite in the frame a new animation is set up, a frame ahead of BN4's). Its mode (+9) comes from
  the battle state's +0x32 bit 0 and its settings' +6 bit 0x20: 8 drawn, 0 or 4 waiting on the
  battle's appear. Mode 4 draws his flame's appear through BN4's tile buffer (`0x02037810`, with
  `0x0800087C` and `0x08003578`, which have no BN6 twin read). **An interim:** mode 4 runs as mode 0
  (the appear waited on, then shown; `UNWALKED_TABLES`, `COMPANION_MODES`). Neither spec reaches it:
  BN4's battle as the differential sets it up starts the companion in mode 8 (`bn4_fireman_control`'s
  trace).
- **The rest, as GutsMan's machinery takes it**: six third-kind objects moved (0x09, 0x0A, 0x19,
  0x1A, 0x1F, 0x21); the attack's end that keeps the attack's +5 at AI data +0x3A copied
  (`BN4_ATTACK_END_KEEP`); five new pairs in `tools/bn4_twins.py`, two of them audited (the hit
  kind's setter, BN6's masking the element's low four bits; the pause byte, BN6's with a `tst`); the
  third kind's 0x1A's kinds' sprite table (`SPRITE_TABLES`); a table in MegaMan's block past the last
  code walked (`MM_TABLE_SIZES`); five sounds in the pool (0x8F, 0xA8, 0xA9, 0xC0) and the families'
  slot 0x36 for 0x9D.
- **The palette, read again**: `BN4_NAVI_PALETTE` takes its number's byte at `0x0800B9B1` times the
  version's at `0x0800B9AC`, the other way round from what the generator read for GutsMan. Both give
  1, so V1-V3 have palette 0 and SP palette 2. BN6's step 0 gives 0 to every version, so
  GutsMan's and FireMan's SP palettes are an interim (`navi_palette`'s note). V1's 0 holds, and
  GutsMan's recipe is unchanged.

The comparator tells a moved first-kind object by its type, not its NameID
(`xdiff_snap.identity`, `T1_MOVED`; `xdiff_bn4.Bn4Canon.t1_moved`: BN4's companion has id 0, which
reads as MegaMan's). Its record keeps its own +9 and id, and its spawn word's sprite number is read as
the recipe's copy.

`bn4_fireman_control` (BN4's battle fought twice) PASSes over its 1200 frames. **`fireman_v1`**
runs both battles to their end (BN4's 1200 fight frames, BN6's 1201) and FAILs at t1, its verdict
recorded while the port is under way. At t1, FireMan's own record, AI data and collision data match
BN4's. His companion's sprite count (+0x91) is BN4 2, BN6 1, not yet explained. GutsMan's opening
count (legit.json's `opening_count`, a Navi's) is a lead to verify, not an allowance to stretch over
the companion. No mutation of his port is added until his spec PASSes. Nor is he in the tournament's
list of proved BN4 Navis (`src/layer/bracket.c` has none on this branch).

### Battle sprites by battle, planned

The same rule as the sounds: a BN4 Navi's battle sprite loaded for his
battle from the player's ROM into one shared room, not all eighteen
resident. Measured from BN4's list 2: the ten Soul Navis' sprites are raw
in BN4 (their rooms 36-60 KB, GutsMan's 60 KB), the other eight LZ77'd
(98 KB compressed, 264 KB decompressed, Duo's the largest at 61040
bytes). One room of 64 KB (`0x10000`, GutsMan's and Duo's rounded up)
holds any one of them decompressed. The plan, as the sounds' pool:

1. Every BN4 Navi's list 2 number (`XNAVI_SPRITE_BN4` + seat) and arm
   points' record listed at install, his list 2 entry the shared room.
2. Before each frame, the battle's BN4 Navi found as the sounds' pool finds
   him; where the room holds another's, his sprite copied in from the
   player's BN4 ROM (`xnavi_sprite_bytes`, decompressing an LZ77'd one).
3. The copy must land before BN6 draws him: the object's sprite is set at
   his init (`sprite_load` from his struct1's list and number), in the
   frame he spawns, which the step before that frame cannot see. The
   battle's record names its enemies before they spawn (BN6's battle
   settings, `emu_encounter_record`), so the step reads the record at the
   battle's setup instead of the objects, as the director's battle start
   event (`EV_BATTLE_START`) does; the differential's BN6 run sets up its
   battle the same way.

Not built: GutsMan's sprite stays resident (60 KB of the 256 KB art room)
until it is.

### Room for all eighteen, measured

Measured by the generator's walk of all eighteen (their recipes mostly
open but GutsMan's, so the counts approximate the others'): their code and
their objects' 0x18D52 bytes (about 100 KB); their battle sprites 0xAF000
(700 KB, each rounded to 4 KB: GutsMan's and Duo's 60 KB, TopMan's 20 KB);
their attacks' art 0x1D800 (118 KB). The room (448 KB since BN4's bracket
screen took its last 64 KB, `FREE_BRACKET`) holds the code in its 128 KB
and the attacks' art, but not the battle sprites: 256 KB of art holds
about four of them. The rest would need their battle sprites loaded
by battle as their sounds are, into a shared room of a few sprites (the
Navis a battle or a tournament's day holds), from the player's ROM: BN6's
battle preload already copies a battle's compressed sprites into its
cache, so a Navi's sprite copied at his battle's start serves as well as
one kept; the 16 MB copy has no other room past FREE_LIB_TEAM (24 KB).

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
