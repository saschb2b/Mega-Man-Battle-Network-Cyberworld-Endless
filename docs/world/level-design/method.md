---
type: Design
title: "Method"
description: "How BN6's internet maps (groups 0x90-0x96) were decoded from the ROM and measured, with a table of each map's panels, box, one-wide share, dead ends, loops, bridges and platforms."
tags: [world, level-design]
generated: { by: claude-code/unrecorded, at: 2026-09-24T10:21:27Z }
---

# Method

Every internet map of groups 0x90-0x96 was decoded from the player's ROM:
its floor panels (a panel is floor when the front tile layer draws its centre
at the height coordinate section 1 gives there) and a render of the whole map.
The lab scripts are `panels.py` (grids), `stats.py` (the numbers below) and
`render_levels.py` (pictures), kept outside the repository with the other ROM
tools.

| Map | Panels | Box | 1-wide | Dead ends | Loops | Longest bridge | Platforms (panels) |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Central 1 | 213 | 28x20 | 21% | 4 | 0 | 5 | 83, 32, six 3x3 pads |
| Central 2 | 201 | 32x24 | 48% | 24 | 1 | 12 | 42, 40, 9, 6, 4, 4 |
| Central 3 | 277 | 24x27 | 10% | 8 | 0 | 6 | 225 (with a crater), 12, 9, 4 |
| Seaside 1 | 238 | 24x22 | 13% | 10 | 0 | 5 | 190, two pads |
| Seaside 2 | 390 | 26x27 | 14% | 34 | 1 | 5 | 316, two pads |
| Seaside 3 | 429 | 29x35 | 11% | 10 | 4 | 8 | 363, two pads |
| Green 1 | 401 | 30x34 | 38% | 26 | 4 | 14 | 240, pads |
| Green 2 | 384 | 35x36 | 60% | 53 | 2 | 13 | 62, 36, six pads |
| Underground 1 | 287 | 31x30 | 47% | 37 | 6 | 8 | ten, 9-34 each |
| Underground 2 | 351 | 27x37 | 28% | 33 | 3 | 11 | fifteen, 10-65 each |
| Sky 1 | 307 | 32x28 | 18% | 38 | 0 | 5 | eleven, 8-74 each |
| Sky 2 | 335 | 30x33 | 13% | 14 | 4 | 5 | 149, 65, 37, 16 |
| Sky 3 | 394 | 33x24 | 26% | 15 | 5 | 7 | 168, 36, 16, 16, 16 |
| Undernet 1 | 417 | 37x34 | 39% | 30 | 8 | 13 | twelve, 9-72 each |
| Undernet 2 | 498 | 32x41 | 28% | 28 | 0 | 8 | eighteen, 14-96 each |
| Undernet 3 | 272 | 35x29 | 43% | 34 | 3 | 13 | 67, 58, 20, 10 |
| Graveyard | 509 | 29x34 | 10% | 6 | 10 | 11 | 232, 57, 40, 40, 33, 30 |

(1-wide: panels in no 2x2 block of floor. Loops: holes in the floor, not
counting the 2x2 blocks. Platforms: groups of panels that are in a 2x2
block.)

The generator this replaces built 6-14 rectangles of 3-5 panels joined by
L-shaped walkways: about 100 panels in a 14x16 box, every area alike.
