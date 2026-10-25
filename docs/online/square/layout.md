---
type: Feature
title: "The layout"
description: "The Net Square's layout on a 26 x 39 grid: the welcome plaza and its landmark, Trade Central, the Arena and the stage as warped parts, Sky's raised floors, walk times (`tools/square_tour.py`), the unit tests."
tags: [online]
generated: { by: claude-code/unrecorded, at: 2026-10-23T14:25:12Z }
---

# The layout

A 26 x 39 grid (the plaza's middle at 17, 20; its tile map 184 x 104
entries at most of the 20477 the free space holds):

| Part | Where | What stands there |
| --- | --- | --- |
| The welcome plaza | 9 panels a side | The landmark on its middle panel (the nine panels its base spreads over walled off), four props round it; the arrival two panels before it (+x), facing it; beside the arrival the pad home (BN6's return pad) and the welcome sign, the keeper and the greeter either side, the Board past the back rim; off the middle of the other three sides, each warp's gem on its 3 x 3 pad, its part's props either side past the rim and its gold label over it |
| Trade Central (-x, the upper left) | 6 x 5, two void panels past its warp | Its gem where MegaMan lands; BN6's reception desk and its two clerks three panels on, two cybertrees behind it. By its warp in the plaza, Trade Central's BBS: the trade board's offers and who is there |
| The Arena (-y, the upper right) | 7 x 9 | The stands' floor where MegaMan lands, the ring up its stair beyond it, the referee at the stair's foot, braziers at the ring's back corners. Braziers either side of its warp in the plaza |
| The stage (+y, the lower left) | 7 x 8 | The audience's floor where MegaMan lands, the stage up its stair, cybertrees behind it, the GirlNavi pacing the floor. Cybertrees either side of its warp in the plaza |

The landmark is the area's own first: Green's giant cybertree among
cybertrees; in Central and Sky the Undernet's statue or the Graveyard's
monument by the seed, ringed by braziers or cybertrees by the seed. By
the seed too, cybertrees past the plaza's top corner, or past its left
and right ones. Only Sky's maps have stairs to cut (Sky Area's, climbing
towards grid -y, 32 high): there the stage and the ring stand raised as
BN6's raised floors, by the layers' own rules (`net_height.c`: nothing
within a panel of where a raised room is drawn, a panel up the screen).
Central's and Green's maps have none, so their stage and ring are flat
daises in the same places. The props are BN6's map objects through the
layers' own pieces (`layer_prop_objects`), at the stage's or the ring's
height past their back rims. Sprites: six of the twelve the loader
takes, 0x4614 to 0x46BC of its 0x8800 bytes; NPCs: all sixteen BN6 runs
(the warps are tiles and triggers, no NPC).

**The walk, timed** (`tools/square_tour.py`: a local server, three games,
Kei run by the pad as a player holds it, B held, from the arrival round
the landmark to each warp's gem; the same at seeds 120, 121 and 122,
Central, Sky and Green): to Trade Central 396 frames (6.6 seconds), to the
Arena 294 and to the stage 300, each with the warp's beam and ride (about
140 frames of it); back from each part to the plaza 168 to 170 frames
(under 3 seconds). In the old Square the desk stood 24 panels from the
arrival and the referee 27, all of it walked.

`tests/test_online.c` holds, over 64 seeds: every floor panel reached
from the arrival by walking and the warps, a step between heights only
on a stair, and from part to part only by a warp (each part's floor two
void panels or more from another's); each stair from ground floor up to
raised floor, no floor where a raised room is drawn; each gem on ground
floor with its pad all round it, no place nor walker's waypoint on it,
its landing on a screen diagonal, the plaza's a short walk from the
arrival; every place and every walker's waypoint on the floor, no
walker's leg over a pad.
