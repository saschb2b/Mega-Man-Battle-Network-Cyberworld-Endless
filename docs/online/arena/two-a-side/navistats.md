---
type: Feature
title: "The partners' own NaviStats"
description: "The partners' own battle NaviStats (`arena_team_navi.c`): each record in one of BN6's effect object slots, a hook at `BN6_NAVI_STATS_ADDR` answering by object for the five routines that ask so, and the proofs."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-22T14:54:41Z }
---

# The partners' own NaviStats

BN6 keeps one battle NaviStats a side (`BN6_BATTLE_NAVI`, `0x64` bytes:
HP, the buster's levels, abilities, mood, form), and every reader and
writer goes through one routine with a side (`BN6_NAVI_STATS_ADDR`). Five
routines take that side from the object in r5 (`BN6_NAVI_BY_OBJECT`:
330 calls in bn6f, a MegaMan's HP as he is set up, his forms, his mood);
the rest pass a side (the HP box, the Custom screen, the battle's end)
and stay the side's lead's. `src/online/arena_team_navi.c` answers the
five by object (21 October 2026):

- **Each partner's record**, a copy of his side's made as the battle
  spawns its field, his own max HP in it where `teamhp=N` gives one (as
  his own loadout will).
- **Their room is BN6's own.** The RAM BN6 leaves alone (docs/
  [Memory the engine writes](../../../engine/emulation/memory.md#bn6s-ram), BN6's RAM) has `0x70` bytes left past the partners'
  words (`RAM_TEAM_LINK`), room for one record of `0x64`, not two. So each record takes an
  object slot of BN6's fourth kind (the effects: 32 of `BN6_T4_SIZE`,
  `0xC8` bytes), slots 30 and 31, taken in its pool (`BN6_T4_ACTIVE`,
  slot i bit 31 - i) as the field spawns, the pool just set up. BN6 runs
  and frees its objects through their run order alone (bn6f
  `object_freeMemory`: an object frees itself), and reads the pool's
  bits only to find a free slot as it spawns (the lowest first) and to
  clear them all as the next battle sets up
  (`InitializeStructsOfObjectType`): a slot taken and never linked is
  never run, never freed, never zeroed until then. The battle keeps 30
  effects. A slot's `0xC8` bytes hold his record and his hand (`0x50`,
  below).
- **By object**: an answering hook at `BN6_NAVI_STATS_ADDR` returns a
  partner's record where the ask comes from one of the five (its return
  address), for his side, and r5 is his object, or one he spawned, or
  one they spawned in turn: every spawn (`BN6_SPAWN_COMMON`, r5 the
  spawner) marks the object it makes (`BN6_SPAWN_MADE`) a partner's or
  no one's, for each of the three kinds' 32 slots. The marks are kept a
  core between calls (`hook_keep_slots`), so a rollback puts them back;
  the records are core RAM, in the pair's hash.

Proved with `--dev arena,arenaboth,arena2v2=1,teamhp=150,arenadelay=1`
(the partners' records and hands as they stand since, their own Custom
screens and the leads' wait among them), this build:

| Run | End | Hashes | The two GBAs |
| --- | --- | --- | --- |
| `arenalag=4` (169 rollbacks) | over at frame 1850 | 30, all the same as `arenalag=0,arenawindow=8`'s (0 rollbacks) | 1,299 iterations, 0 apart |
| `arenalag=4,arenaroll=8` | over at 1846 | all 30 the same as the delay alone's (`arenawindow=0`) | 1,292, 0 apart; 230 rollbacks of 8 frames, the game's state the same after all 230 |
| `teamhp=-150` (the records' HP on the second GBA alone) | over at 1850 | | apart from iteration 1 (1,299 of 1,299) |

With the records their sides' copies (`arena2v2=1` alone, before the
partners took chips) the battle's 194 changes of HP were those without
the hooks, iteration for iteration. Both partners are set up with 150 HP
from their records while their leads keep 100: the field's poison takes
both from iteration 62; side 1's partner's charged shot takes side 0's
from 100 to 90 at iteration 407 and his Cannon from 84 to 43 at 454,
while side 0's player, whom nothing hits, stays at 100 and side 1's
falls by his own hits alone (100 to 90 at 184, 132 left on his partner
then); side 0's partner is deleted at iteration 685 and side 1's at
1035. Each GBA's screen shows its enemies' own HP under them: on the
first, side 1's player 100 to 67 and his partner 144, 123, 101 at the
battle's frames 300 to 600; on the second (`arenaother`), side 0's
player 100 throughout and his partner 144 to 37 by frame 700. The
records answered some 40,000 to 80,000 asks each a battle, all of them
the partners' own objects': of the 64 to 86 objects marked theirs
(their buster's shots, their Cannons and the effects) none asked by
object, so an attack's inheriting his record is built and its marks
counted, but not yet seen to matter. The HP box on a partner's own GBA
(BN6 draws its lead's) and a partner's own loadout (his NaviCust's
programs, his Cross) wait on his own GBA's screen and the wire.
