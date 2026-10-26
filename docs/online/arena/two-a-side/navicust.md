---
type: Feature
title: "His NaviCust, compiled by BN6"
description: "A partner's NaviCust compiled by BN6 (`arena_team_navicust.c`): BN6's own NaviCust reload run over the way in's state from his loadout, the record taken whole as the field spawns, and his hand's size."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-22T16:53:46Z }
---

# His NaviCust, compiled by BN6

BN6 makes a NaviStats' abilities from the NaviCust's board in its menus:
its reload of MegaMan's stats (`BN6_NAVICUST_RELOAD`, bn6f
reloadCurNaviStatBoosts_813c3ac, Gregar's `0x0813E18C`) puts his NaviStats
back to his own, runs each program's routine over them
(applyNavicustPrograms_813C684, `0x0813E464`) and the bugs after. A
lead's are made so on his own GBA on the way in, from the board the Arena
writes into it. A partner has no GBA, so his own client runs the same
routine (`src/online/arena_team_navicust.c`, 22 October 2026): his
checkpoint's state loaded into a core of the plain ROM, his loadout
written in as the Arena writes a lead's (`arena_loadout_install`), the
routine called once in the game's update (the game's own call, as
`gamecall.c`'s and the Arena's boot's), and the NaviStats it leaves read
out, his HP whole. No rule of ours: what BN6 computes is what he is. That
record, the same on every client, is what crosses (with his loadout, once
the wire carries partners); under the dev flag his client is this one.

- **His record** takes it whole as the field spawns: BN6 copies a link
  battle's NaviStats whole from the menu's into its start packet and from
  there into the battle's (bn6f sub_800B144), but for two bytes the start
  sets for every MegaMan of the battle alike (his start form,
  `BN6_NS_START_FORM`, and the BeastOutCounter, `BN6_NS_BEAST_TURNS`),
  kept from his side's record. Measured: each GBA's own menu NaviStats and
  its side's battle record the same, all 0x64 bytes, at the spawn. His
  abilities (SuperArmor, the shoes, UnderShirt, the buster's levels, the
  Custom level) are then his own wherever BN6 asks by object.
- **His hand's size** is his own: BN6's Custom screen deals as many chips
  as its side's battle NaviStats' Custom level says (`0x0802A440`), so his
  screen copy gets his record as his side's before BN6 sets the screen up
  (`arena_team_screen.c`), and deals his number.

Under the dev flag side 0's partner's NaviCust is compiled wherever the
data folder has a checkpoint (he carries the run's MegaMan), and
`teamcust=1` gives side 1's partner the checkpoint's MegaMan's NaviCust
too; `teamcust=-1` leaves the records their sides' copies on the second
GBA alone, the mutation. Proved with a playtest's checkpoint whose
MegaMan runs Custom1, UnderShirt and HP+100 (base 360), both ROMs,
`--dev arena,arenaboth,arena2v2=1,teamcust=1,arenadelay=1,quitend`, this
build (the checkpoint copied into each run's fresh data folder):

| Run | End | Hashes | The two GBAs |
| --- | --- | --- | --- |
| `arenalag=4` (290 rollbacks) | over at frame 3530 | 58, all the same as `arenalag=0,arenawindow=8`'s (0 rollbacks) | 2,798 iterations, 0 apart |
| `arenalag=4,arenaroll=8` | over at 3526 | all 58 the same as the delay alone's (`arenawindow=0`) | 2,787, 0 apart; 440 rollbacks of 8 frames, the game's state the same after all 440 |
| `teamcust=-1,arenalag=4` (the mutation) | | | apart from iteration 1 |

The log: "side 1's partner's NaviCust compiled by BN6 (0813E18C) on his
own core: HP 460 (base 360), Custom 6, buster 1 1 1, ... UnderShirt 1".
Side 1's partner is set up with 460 HP beside his lead's 100 (the poison
of the field takes him from 460 at iteration 62); his HP box on his seat
reads 460 and 451 (`teamseat=3`); his own Custom screen deals six chips,
five on the top row and the sixth under them, where his lead's deals
five. The compile costs a core made, a state loaded and a frame or two
run, once a run of the game.

**Over one state every game has** (issue #181's review, 22 October
2026). BN6's reload keeps part of the NaviStats from the state it runs
on: `0x08138A00` (called from the reload's body) reads +0x0E, +0x21,
+0x22, +0x2C, +0x2D (the folder in use), +0x2E, +0x2F (Folder2's Regular
chip), +0x30, +0x3E, +0x40 and +0x56-+0x5B, resets the NaviStats and
writes them back, and the loadout writes only some of them. Compiled
over each game's own checkpoint, as the first build did, a partner's
record took the rest from that game's run: four players of different
runs built four different pairs. So the compile runs over the way in's
own state instead (`arena_seats_compile`, from `arena_boot.c`'s
make_pair): the game's start the Arena boots on every game and spectator
alike (NEW GAME, the warp to Seaside Area 1), before either lead's
loadout is written into it, each partner's loadout written over it and
BN6's reload run, his record read out. No byte of it is any game's own
save; a compile that fails (no core, the reload not reached in 120
frames) fails the way in with its reason, on every game alike, and no
game falls back to another record. Under the dev flag the stood-in
partners' MegaMen are compiled the same way (`arena_seats_partner`);
`teamcust=-2` compiles over this game's own checkpoint instead, as
before, the mutation. A bout two a side never goes in from the last
one's kept pair (`arenaagain=N` under the dev flag goes the whole way
in, as over the network, where a 2v2 keeps none), so the compile runs
for every bout.

Proved over the server, 22 October 2026, both ROMs: four games of
different runs (`drive.py --diverse`: seeds 3, 11, 29 and 47, depths 7
and 9, folders 1-3, threats 0-2), then three of their checkpoints
swapped as they stood in the Square for other runs' (a playtest's with
Custom1, UnderShirt and HP+100; a run with Souls and thirty DarkPlus;
another), and a spectator whose run's files were removed (no checkpoint
at all):

| Build | The ready hashes | End |
| --- | --- | --- |
| Before (each game's own checkpoint) | Beta's `1d33c68692f9c1c4` against the others' `3cf71bb612a0215c` | "match 2 stopped: the two games went apart before the battle" |
| This build | `3cf71bb612a0215c` on all four | over at frame 1255 on all four, 20 hashes agreed, 949 iterations 0 apart on all five; the spectator without a checkpoint 20 hashes checked to the end |
| This build, `teamcust=-2` on Beta's game (the mutation) | Beta's `1d33c68692f9c1c4` against the others' `3cf71bb612a0215c`, as before | "match 2 stopped: the two games went apart before the battle" on all four |
