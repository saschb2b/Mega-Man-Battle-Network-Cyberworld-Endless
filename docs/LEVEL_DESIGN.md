# Level design

How BN6's own net areas are built, measured from the ROM, and how the layer
generator (`src/net/`) follows them.

## Method

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

## What the originals do

- **Two materials.** Each area pairs a floor for its platforms with a second
  one for its walkways: Central green fields and blue catwalks, Seaside blue
  sea floor and yellow boardwalks, Green grass and orange planks, Undernet
  pink plateaus and red bridges, Graveyard slabs and purple ramps.
- **Walkways are 1 panel wide and straight.** Bridges run 3 to 13 panels
  without a turn and bend at right angles; 2-wide paths are rare.
- **3x3 pads on spurs.** The most common platform is a 3x3 square on a short
  bridge off the route (Central, Seaside, Sky, Green): the places for Mystery
  Data, Mr. Progs, shops and warps.
- **Plenty of dead ends.** 10 to 50 per map: pads, stubs and the teeth of
  comb-shaped boardwalks. Exploring a side way usually pays with an item.
- **Few loops.** Most maps have 0 to 5; the route is mostly a tree, with one
  or two shortcuts. Graveyard's loops are holes punched in its slabs.
- **One landmark per map.** A giant plaza, a crater in a field (Central 3), a
  ring road around coloured fields (Sky 3), a raised block (Underground), a
  symmetric hub of pods and stairs (Sky 1).
- **Each area has its own shape language.**
  - Central: fields of 2-4 panel wide paths, with catwalk mazes of 1-wide
    turns and satellite pads.
  - Seaside: one huge field with ragged edges, framed by comb boardwalks.
  - Green: long parallel planks (ladders with rungs) beside grass blocks.
  - Sky: pods and platforms joined by stairs, often mirrored.
  - Graveyard: few huge slabs with holes, joined by long ramps.
  - Undernet: a web of long bridges crossing between scattered plateaus.
  - Underground: plus-shaped platforms on a lattice around a raised block.
- **Exits sit far out.** Arrows at the ends of walkways on the map's edge,
  away from where MegaMan arrives.

## The generator

A layer is built from those parts, not from rectangles. Each area has its
own layouts (`src/net/net_layouts.c`), all made of the same pieces
(`src/net/net_shapes.c`); an act's three layers take them in a shuffled
order, so an area does not repeat one while it has others.

| Layout | Areas (weight) | Built from |
| --- | --- | --- |
| Route | Central 35, Seaside 40, Sky 45, Green 45, Secret 25 | a winding chain of platforms on bridges, pad spurs, stubs, a shortcut |
| Field | Central 35 (around a crater), Seaside 60 (ragged) | one big field, comb boardwalks with teeth on two or three sides, pads |
| Ladder | Green 55 | four or five parallel planks joined by rungs, teeth on the outer ones, grass blocks at the ends |
| Hub | Sky 55, Secret 25 | an octagon centre, four mirrored spokes to pods, a ring between them |
| Slabs | Graveyard 65, Nest 40 | a chain of big slabs with punched holes, long bridges between |
| Web | Graveyard 35, Undernet 45, Secret 50 | plateaus kept far apart, bridges crossing between them, many stubs |
| Crosses | Undernet 25, Nest 60 | plus-shaped platforms grown over a lattice from a big middle block |
| Catwalks | Central 30, Undernet 30 | a maze of 1-wide turns with some walls knocked through, plazas at its ends |

MegaMan arrives on the pad nearest the top of the screen; the exit is the
room farthest from it by walking. Services go to the bigger platforms, the
better Mystery Data to pads, and most other Mystery Data to dead ends. A
layer has 120 to about 250 panels (depth grows the layouts) and fits a
window of 29 x 53 panels along the grid's diagonals, the screen rectangle
the game's 0x14000-byte tile map buffer holds; `netmap.c` centres it on that
rectangle.

Walkways, the floor panels in no 2x2 block of floor, are drawn in the
area's second floor, learned from the same source map by its hue
(`walk_styles` in `src/core/rom.c`): Central's blue catwalks, Seaside's
yellow boardwalks, Sky's darker glass, Green's orange planks, Graveyard's
purple bridges and the Undernet's red striped bridges. The tile classes
tell platform, walkway and void apart, so the joins between the two floors
come from the places the original maps join them. The Nest's source map
(Underground 2) has no second floor.
