---
type: Feature
title: "Link Navi obstacles"
description: "Link Navi obstacles (issue #42): BN6's geysers, cybertrees, flames, cyclones and clouds in a pocket's mouth, each area's own kind, opened by the run's Crosses (`layer_block_kind`, `ng_plan_obstacle`, `blockers.c`)."
tags: [world, level-design]
generated: { by: claude-code/unrecorded, at: 2026-10-07T08:43:35Z }
---

# Link Navi obstacles (built, issue #42)

BN6 sets 25 obstacles in its nets, each in the mouth of a pocket that holds
one thing (a SubMemory, Attack+1, HP+100, Sky's purple data): a geyser of
cyberwater, a cybertree, a pillar of flames, a cyclone or a cloud, which two
of Gregar's Link Navis clear each. A layer sets one the same way:

- **Its kind is its area's own** (`layer_block_kind` in `net_pieces.c`):
  Seaside's geysers and cyclones, Green's trees, Sky's clouds, flames and
  geysers, the Underground's clouds, cyclones and flames under the Nest,
  all five in the Graveyard; none in Central, as in BN6.
- **The run's Crosses are its keys:** the Cross brought (alone) or each
  won from a guardian before this layer clears the obstacles its Navi
  clears in Gregar. Four pockets in five take a kind the run can clear,
  else one it can't, a hint for the next run's Cross; a run that can clear
  none of the area's meets one two layers in five, else none (a pocket a
  run can never open is dead weight). So the setup's Cross is a bet on the
  net's locks as well as on its battles.
- **Its pocket** (`ng_plan_obstacle` in `net_set_pieces.c`): a walkway's first panel
  off a wider floor, off the way, whose closing cuts off 6 to 40 panels and
  nothing the way, a service or the guardian needs; seen from the way
  (its mouth near it). The pocket lies ahead of the mouth, the floor
  before it the arrival's: BN6's walls hold MegaMan back from that floor
  alone, and a pocket measured behind its mouth (8 of 37 obstacles and
  cubes over the tests' layers) let him walk in and never out (issue
  #111). Nothing else is placed in it; its one thing, a blue
  data of the best quality, lies where it ends.
- **The map** (`blockers.c`, `netmap.c`): BN6's own obstacle (handler 3)
  where BN6 sets one in a walkway's mouth, a line of wall cells across the
  walkway flagged with its slot, and its check cells where the engine's A
  probe lands from the floor before it; the check runs the layer's talk
  from the map's own archive: MegaMan names it, and with the Cross, the
  Navi's mugshot says "Leave it to me!" over his sound, and the obstacle
  opens, for good (its present flag is kept in the run's state).
- **Another game's look**: on Nebula Area's layers (BN5's, [Other games' ROMs](../../multirom/overview.md))
  the pillar of flames is BN5's wall of dark flames, standing across the
  walkway either way, "A wall of dark flames" in MegaMan's words; the same
  Crosses clear it, and it burns out as BN5's does.

Over the tests' 300 layers (their guardians Cross navis), 13 hold one.
