---
type: Design
title: "Set pieces"
description: "Set pieces and detours (issue #49): blue Mystery Data at the detours' ends (`net_detours.c`), long detours never left empty, the layer's RegUp, and the act's plan of which pieces a layer holds (`layer_pieces` in `net_pieces.c`)."
tags: [world, level-design]
generated: { by: claude-code/unrecorded, at: 2026-10-06T11:29:31Z }
---

# Set pieces

BN6's net areas hold more than floors and props: Rush's bone gaps, purple
Mystery Data, the Link Navis' obstacles, arrow panels, security cubes,
teleport pads. Each area repeats two or three of its own, the main route
stays free, and each lock guards one thing on a spur, its reward following
the detour. The research behind them (what each area holds, how the ROM
runs each, the rules Capcom places them by) and the plan are in the epic,
issue #49; what is built is below.

The design is the run's dialectic, prepare or press on ([The meta layer](../../game/meta/overview.md)), at
the scale of a spur: a detour costs steps, and BN6's step counter rolls
battles; a lock's key costs zenny or a Cross; what it pays is power for the
act's guardian, or a shortcut that saves battles. Value that has nothing
to do with its cost asks no decision, so the generator ties them together.

## Detours and their data (built)

`ng_measure_detours` in `net_detours.c` walks from the way (the layer's shortest
walk from the arrival to the exit or the guardian, `layer_on_way`) over
the floor: each panel's detour is the panels walked to reach it from the
way (`layer_detour`), and its branch the way's panel that walk leaves
from, so a spur, a room off the way and everything past them share one.
The guardian's arena is no detour (the way ends at him).

Mystery Data follow BN6's colours, which say what a walk is worth:

- **Blue data where the detours end**: 2 a layer, one more past depth 6
  and in the Undernet; each the farthest free panel of a branch three
  panels off the way or more, the farthest branches first, on a pad's
  middle where the end is a pad (`detour_end`). The farthest is of the
  best quality (a chip three tiers up, or an HPMemory) where it lies eight
  panels off or more; the others three times in ten. A layer with too few
  detours puts the rest in its rooms. The run's Spin lies in the best of
  them.
- **Green data loose**, 2-4 a layer (up to two more on bigger layouts), most at
  dead ends: the commonest quality.

Over 3000 layers of a first cycle the old roll gave every quality the same
places, 7 panels off the way on average and a quarter of them within two
panels of it, all drawn green; now the green data lie 6.1 panels off on
average, the blue ones 8.2 and the best 11.8, 1-2% of them within two
panels, and about as much in all (fewer of the middle quality, a fifth
more of the best). The tests check that nine blue data in ten stand three
panels off the way or more.

## No long detour left empty (built)

Once all stands, a detour five panels or more off the way that holds
nothing, no object and no set piece's stand, takes a green data at its
end, the farthest first (`fill_empty_detours`): one lying loose (on the
way, or on a shorter detour) is moved there, so a layer holds as many; and
a detour that runs two panels wide or more, a band that reads as the way,
takes a new one where the map has room. A playtester walked a two-wide
band to an empty end; over the tests' 300 layers 321 of 1589 such
detours had been empty, 106 of them wide; 215 and 40 now, the rest where
the map's sixteen are spent or the end has no free panel. A narrow spur
to nothing stays, as BN6's maps keep some. The signature's open floor is
no end (issue #98): a plaza's corners and a ring road's far side are seen
from the way that crosses or rounds them (`layer_detour_open`). Over the
same layers 586 of 1877 such branches end there; of the other 1291, 122
are empty, 30 of them wide.

## The layer's RegUp (built, issue #51)

Each layer before a guardian holds one of BN6's RegUps ([Reg memory and TagChips](../../game/meta/reg-memory-and-tagchips.md), Reg
memory and TagChips): in the first Mystery Data a set piece keeps (behind
an obstacle, a cube or a door, on a Rush or teleport island, at an
invisible path's end: `NetObj.prize`), else in the best blue data where a
detour ends; never in the Spin's or a purple one, whose rarest chip is its
own. The lock and the walk pay the same thing the folder's consistency
grows by, so a skipped spur is a planned opener left behind
(`layer_regup`, `regup_md` in `layer_objs.c`).

## The act's plan (built)

`layer_pieces` in `net_pieces.c` decides which set pieces a layer holds,
from the run's seed and the depth alone, so a Net Dealer earlier in the act
knows which keys to stock (`layer_pieces_ahead`):

- **Budget by the act's rhythm.** None on the run's first layer or in the
  Secret Area; one on an act's first layer (half of them), two on its
  middle one (the dealer's: a choice of where to spend), one on the
  guardian's (four in ten), one on a dark warp's layer (six in ten).
- **Each area its own,** by weights from what its original maps hold
  (purple data everywhere but the Secret Area, the most in the Graveyard
  and the Undernet; Rush in Central, Seaside, Green, Sky, the Undernet and
  ACDC's homepage), with a weight of nothing beside them, so an area with
  few of BN6's pieces keeps them rare, as its maps do; never one piece
  twice on a layer, and no Rush before a guardian.
