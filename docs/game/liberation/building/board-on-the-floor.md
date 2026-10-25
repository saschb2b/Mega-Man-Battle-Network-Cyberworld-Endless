---
type: Feature
title: "The board on the map's floor, as built"
description: "The Liberation board made on the layer's map once built (`lib_board_make`, `map_floor` in `src/director/liberation.c`), so legal.c no longer drops its panels and every board way is one MegaMan walks; `LAYER_MAKE` 99."
tags: [game, liberation, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-23T19:14:18Z }
---

# The board on the map's floor, as built (23 October 2026)

The board was made from the layer's cells before its map was built, and
the map's build changes them: netmap's legal.c makes the floor drawable
in the area's tiles, writing the layer's cells in place, and drops the
panels it cannot draw (most of them one panel wide, a walkway's beside
another's). So a way of the board's could be no way of the map's: seed
1's act 5 board at depth 14 lost 22 of its panels after it was made
(`off the map's floor` in the debug log), its start's corridor cut from
the rest, and the autopilot stood on its first panel for the whole run.

Now the map is built first (`build_layer`, `src/director/director_layer.c`)
and the board made on its floor: `lib_board_make` takes the floor as a
predicate per panel (`LibFloor`), and the director's (`map_floor`,
`src/director/liberation.c`) reads the map as built, its floor at the
ground's height (`netmap_floor_cell` at the panel's middle, the cell the
fence reads) and no stair. The map's floor goes by panels (a wall cell is
floor where the panel its middle lies in is, and on a panel's edge where
both are), so two board panels side by side are a way MegaMan walks, and
the planner's graph is the map's. What the map's build took from the
board before needs nothing now: the exit moved to the Darkloid's panel and
the units' objects locked their panels' floor against legal.c, and the
board stands on floor already there. A dark layer whose map gives no
board is made plain and its map built again (the second build is the one
cost, on those layers alone). The tests' boards (`test_liberation_board`)
pass no predicate, the layer's flat cells as generated, as before, and
`test_liberation_floor` makes boards on a made-up map's floor (a panel in
29 gone) and finds none of their panels off it. `LAYER_MAKE` 99.

**The measures** (the debug log of 20 seeds an act, `--dev dark`):

| Act | Kept, before | Kept, now | Panels, before / now | Panels lost to the map after the board was made, before | Now |
| --- | --- | --- | --- | --- | --- |
| 2 | 20 of 20 | 20 of 20 | 181 / 161 | 446 (all 20 boards) | 0 |
| 3 | 15 | 14 | 203 / 179 | 429 (all 15) | 0 |
| 4 | 15 | 16 | 192 / 174 | 310 (all 15) | 0 |
| 5 | 9 | 7 | 219 / 196 | 224 (all 9) | 0 |
| 6 | 10 | 6 | 225 / 195 | 191 (all 10) | 0 |

Every board lost panels to the map before, about twenty a board, and the
planner had counted on them; now none does. The boards are smaller by
those panels, and a run's acts (2 to 4) keep about as many; acts 5 and
6, which a run does not meet yet, keep fewer, the cut floor's corridors
failing the planner's dead phases.

**The autopilot** (`--dev dark,god,onehit`, seeds 1 to 5; acts 4 to 6
again with `hp=2000`, as `god` does not hold through a battle that
deletes MegaMan from full: stage 2's battles deleted him on both builds
in every act 4 to 6 board without it): all 18 boards it met played
through, the act's Darkloid deleted and the exit taken (9 in acts 2 and
3; 9 in acts 4 to 6, seeds 1 to 6, with `hp=2000`), and none left
MegaMan standing at a way the map does not have. Before, over the same
seeds, 1 of 20 did: seed 1's act 5 board left the autopilot on its first
panel for the whole run (the rest found ways round the panels they had
lost). On its map's floor no board of that layer's sixteen tries passes
the planner, and it is a plain one.
