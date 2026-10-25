---
type: Design
title: "The generator"
description: "The Liberation generator (`tools/liberation.py`): BN5-measured bands per act, six steps from floor to traps, the guarantees and command rule that reject boards, and the par from a planner (`liberation_plan.c`) scaled by BN5's ratio."
tags: [game, liberation, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-23T18:41:43Z }
---

# The generator

## What a mission is made of

The board's shape and its dark follow bands measured on BN5's own boards
(`tools/liberation_bn5.py`): **fields**, the share of panels in a 2x2
block of floor; **fill**, the floor's share of its box; **dense**, the dark
panels around a dark panel on average, which sets how often a liberation
is won in one turn. BN5's missions 1-3 stand at 72-82% fields, 22-28% fill
and 3.5-3.9 dense; missions 4-9 at 88-99%, 43-48% and 4.1-6.0: its later
boards are wide fields thick with dark.

| Act | BN5 like | Panels | DarkHoles | Team | Barriers | Dark | Fields | Fill | Dense | Items | Bonus | Guardians | Darkloid |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 2 | 1 | 90-112 | 2 | 2 | none | 28-45% | 68-86% | 26% | 3.3-4.3 | 7 | 2 | BigBrute | 300 HP, reach 1, 30 |
| 3 | 2-3 | 165-215 | 3 | 3 | 1 (sealed) | 45-58% | 70-88% | 27% | 3.4-4.5 | 8 | 3 | BigBrute | 600, 2, 50 |
| 4 | 4-5 | 210-245 | 4 | 4 | 2 | 52-66% | 86-100% | 42% | 4.6-6.2 | 12 | 3 | + TinHawk, roaming | 900, 3, 80 |
| 5 | 6-7 | 215-255 | 5 | 5 | 2 | 58-68% | 86-100% | 44% | 4.6-6.2 | 16 | 3 | all | 1200, 2, 80 |
| 6+ | 8-9 | 220-260 | 5 | 5 | 3 | 55-68% | 86-100% | 45% | 4.2-6.2 | 14 | 3 | all | 1500, 3, 100 |

The steps (`tools/liberation.py` `generate`):

1. **The floor:** platforms (3-5 by 3-4 at act 2, up to 8 by 6 from act 4)
   in a box the act's fill sizes, joined by walkways bending at right
   angles (a spanning tree, then a loop or two), one wide early and two
   wide in six of ten from act 4, as BN5's fields run; a start zone at the
   left edge; the Darkloid's 3x3 at the right; spurs ending in 3x3 pads or
   one-wide stubs. In the engine the floor is the generated layer's own,
   taken at 32 world units a panel.
2. **DarkHoles** on platforms with a whole ring of floor, past a third of
   the walk from the start, six apart and four from the Darkloid; their
   rings dark, from act 4 a second ring where the floor runs on; a guardian
   on each ring (BigBrute first, the act's kinds after); roaming guardians
   from act 3.
3. **Dark panels** thickening with the walk from the start; from act 4 the
   Darkloid's approach dark three deep. Then **the density rule:**
   majority passes until a dark panel has as many dark around it as BN5's
   boards of the act have (the dense band), so a one-turn win is no sure
   thing and a command that frees several panels in one battle is worth
   its Order Point. The first two panels of the walk stay clean.
4. **Barriers**, by the staircase (The quality bar, 3): from act 3 the
   sealed region's, a wall of one or two panels (a two-wide way walled
   across) that cuts five to sixteen panels off by itself (the prototype's
   smaller boards allow thirty), nothing of the board's in them, its key in
   a dark panel two or more off the way and the route at the region's far
   end; from act 4 one across a cheapest way, a way round it kept, its key
   off the way; at act 6 a second across the way, its key past the first
   where the floor gives one. **BonusPanels** at stub ends.
5. **Items** in the dark, corners and dead ends first: one Order Points
   item always, then hearts and Order Points; the act's chips and zenny
   off the way (a detour's worth). From act 5 **traps**: dark panels on a
   cheapest way that a way round, a dark panel or two longer, avoids, shown
   as traps (BN5 hides them in item panels).
6. **The par** from the planner, then **the command rule** (below).

## Guarantees, and what is rejected

A board is rejected and the next seed tried when: its size is off; the
Darkloid is nearer than 60% of the farthest walk; the DarkHoles cannot be
spread; its dark misses the act's dense band; a barrier finds no wall or
its key no near spot; a key lies behind its own lock; a DarkHole is cut
off; nothing can be liberated in the first phase; a guardian stands within
three of the start; a teammate's command finds no situation; the planner
wins it less than 60% of the time; or a teammate's command does not earn
its Order Point.

The quality bar's rules add theirs: a board is rejected where more than
55% of its phases on the way are dead, 60% stripped of its rewards, where
its sealed region's route costs the route's player fewer than 2 or more
than 10 phases over the par's (the prototype: 1 to 8, scaled as the par),
where LongSwrd goes unused or wins more than 40% of the phases, where the
act's barrier or traps find no place, or where a unit reaches the start.

**The command rule.** The run knows which Souls it holds when a layer is
made, so the generator makes the board for its team. First it gives each
teammate's command its situation (`situations`): a roaming guardian on the
way to a DarkHole for SneakAttack; a cache of four item panels in a 3x2 of
dark, a clean panel beside it to stand on (from act 4 one of them a trap)
for NumberCheck; a dark line of five on the way for LifeMelody; at least
three spots of three dark across, or two by three, for ScrenDiv and
TomahawkSwing. Then the planner plays the board again with each teammate
and no command, the same random numbers as before, and the board is kept
only where the planner uses every command (half a time a play at least)
and where:

- a liberating command (LongSwrd, ScrenDiv, TomahawkSwing, LifeMelody)
  saves phases (`EARN`: no worse than a tenth of a phase, scaled as the
  par, within the plays' noise);
- a utility command (SneakAttack, NumberCheck) costs the team no more than
  a quarter of a phase (`UTILITY_EARN`): it pays in safety and loot, a
  guardian weakened or items taken with no battle, which phases do not
  show, so its yield is reported beside it.

KnightMan's StoneBody is passive and always on.

## The par

A planner plays each board 12-24 times by BN5's rules, abstracted: each
Navi moves over clean panels and takes the best of liberating, his command,
fighting a guardian in his way (the guardian's HP carried from fight to
fight; one standing on the way on is worth the detour he forces), or
resting; support acts first (LifeMelody, SneakAttack), then
MegaMan; battles won in one turn, won later or lost by the dark around;
the guardians' and the Darkloid's phase; the Darkloid healing by his dark
ring. On BN5's nine boards, with their own teams, it needs more phases than
BN5's par, by a ratio that changes with the missions (BN5's par over the
planner's median: 0.50 at mission 1, 0.58 at 2-3, 0.59 at 4-5, 0.79 at
6-7, 0.75 at 8-9). A generated board's par is the planner's median times
its act's ratio (`PAR_SCALE`): BN5's own yardstick at that stage.

The engine's planner (`src/director/liberation_plan.c`, MegaMan alone)
plays each board 12 times the same way, with BN5's battle odds (the one-turn
win 75% less 5 a dark panel round it, the failure 4% plus 3.5 a dark panel),
the act's virus harm (14 to 40) and a run's HP for MegaMan (300 at act 2, a
hundred more an act: its HPMemory, [Economy](../economy/overview.md)), resting where his HP
weighs more than a phase, never risking a battle whose worst harm would end
the run. The par is the median of its plays won (BN5's ratio is for teams;
a lone MegaMan is the planner's own yardstick until stage 3 brings the
teams): an act 2 board's par is 8 to 19, act 4's 12 to 31.

## How it reads as BN5's own

BN5's units and numbers throughout: its panel kinds and their placement,
its 3x3 Darkloid block and ringed DarkHoles, its guardians, Order Points 3
to 8, its commands' shapes, its par tiers. The board stands on the net
area the act is dressed as (a BN5 area's maps where BN5's ROM dresses it),
its dark panels and units drawn from BN5's ROM at run time, never kept.
