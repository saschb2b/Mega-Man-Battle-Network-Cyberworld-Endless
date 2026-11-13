---
type: Measurement
title: "What the layers take from a map"
description: "The atlas counts, per area, how much of each original map its generated layers ever draw (two thirds of the maps' tiles, two fifths of their different tile pictures) and draws each map with the unused tiles tinted; what stays unused, by kind."
tags: [devtools, level-design]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-13T23:50:00Z }
sources:
  - id: tool
    resource: "src/dev/lent.c, src/dev/atlas.c"
    title: "The count and the atlas that runs it"
  - id: run
    resource: "python3 build.py atlas all 6"
    title: "The run the table is from, 13 November 2026"
---

# What the layers take from a map

The owner, 13 November 2026, on BN4's Park Area: the learner "seems to not
fully scan and reuse all details of the original maps, e.g. the yellow
parts"; it should "analyze it fully to fill the generator with all DNA
from the original map". Before changing the learner, this measures what
it leaves behind.

## The measure

The atlas ([the tile test](tile-test.md) runs it) keeps every tile picture
an area's generated layers draw, over all its layouts and seeds, and the
mirror image of each. Then it goes through each of the area's original
maps, tile by tile and layer by layer, and asks whether a layer drew that
picture anywhere (`src/dev/lent.c`). A tile's picture is its 64 pixels as
the game shows them, so the count does not depend on tile numbers or
flips.

```bash
python3 build.py atlas all 6     # .build/atlas/lent_bNN_GG_N.bmp and lent.txt
```

Each map comes out as `lent_bNN_GG_N.bmp` (area, map group and number):
the map as the game draws it, the tiles no layer used tinted red. Its
line in `lent.txt` gives two shares:

- **tiles drawn, used:** of the map's tiles that draw something, how many
  show a picture some layer draws. A plain floor repeated a thousand times
  counts a thousand times.
- **different, used:** of the map's different tile pictures, how many a
  layer draws. This is the share of the map's art that reaches a run.

## What it found

Six seeds per layout, every area (39 areas, 68 maps): the layers draw
**67%** of the maps' drawn tiles and **42%** of their different tile
pictures (20926 of 49974).

| Area | Maps | Tiles drawn | used | Different pictures | used |
| --- | --- | --- | --- | --- | --- |
| Central | 2 | 10077 | 74% | 1898 | 56% |
| Seaside | 3 | 29247 | 77% | 2053 | 58% |
| Sky | 2 | 14042 | 68% | 3012 | 40% |
| Green | 1 | 8146 | 87% | 937 | 32% |
| Graveyard | 1 | 9201 | 63% | 631 | 41% |
| Undernet | 2 | 19622 | 66% | 2189 | 37% |
| Secret | 1 | 12272 | 79% | 673 | 51% |
| Nest | 2 | 15403 | 66% | 1943 | 28% |
| Comp | 1 | 2220 | 100% | 131 | 100% |
| Homepage | 1 | 1592 | 100% | 139 | 100% |
| Comp B | 1 | 1592 | 100% | 139 | 100% |
| Robot Comp | 3 | 19063 | 44% | 2690 | 40% |
| Aquarium Comp | 4 | 36134 | 75% | 3194 | 35% |
| Judge Comp | 4 | 24971 | 76% | 3623 | 21% |
| Weather Comp | 4 | 36874 | 45% | 2889 | 22% |
| CopyBot Comp | 1 | 4859 | 59% | 794 | 58% |
| ACDC HP | 1 | 1592 | 100% | 139 | 100% |
| Green HP | 1 | 1592 | 100% | 146 | 100% |
| Sky HP | 1 | 1592 | 100% | 139 | 100% |
| BN5 ACDC | 2 | 8584 | 83% | 1328 | 67% |
| BN5 SciLab | 3 | 15226 | 74% | 1933 | 51% |
| BN5 End | 3 | 25382 | 68% | 2920 | 44% |
| BN5 Nebula | 2 | 19007 | 82% | 1967 | 49% |
| BN5 Oran | 2 | 14478 | 71% | 2177 | 42% |
| BN5 Undernet | 2 | 17041 | 91% | 1625 | 75% |
| BN5 dark 1 | 1 | 4888 | 38% | 708 | 29% |
| BN5 dark 2 | 1 | 7206 | 52% | 603 | 33% |
| BN5 dark 3 | 1 | 9102 | 52% | 642 | 30% |
| BN5 dark 4 | 1 | 8233 | 49% | 600 | 32% |
| BN5 dark 5 | 1 | 8830 | 51% | 497 | 40% |
| BN5 dark 6 | 1 | 8987 | 53% | 365 | 61% |
| BN5 dark 7 | 1 | 7740 | 52% | 537 | 41% |
| BN5 dark 8 | 1 | 8445 | 55% | 317 | 66% |
| BN5 dark 9 | 1 | 7862 | 51% | 391 | 52% |
| BN4 ACDC | 3 | 13331 | 57% | 2134 | 35% |
| BN4 Town | 2 | 10893 | 48% | 1178 | 32% |
| BN4 Park | 1 | 6259 | 63% | 837 | 35% |
| BN4 countries | 1 | 7447 | 76% | 677 | 36% |
| BN4 Undernet | 2 | 14757 | 84% | 1179 | 46% |

The home pages and the two plain comps use their one small map entirely.
Everywhere else more than a third of the art never appears, and in the
Judge and Weather Comps four fifths of it.

## What stays unused, by kind

Read off the tinted maps of Park, Central, Sky and Town Area 3:

- **Floor of another kind.** The largest share. The learner reads a map
  as one floor (two materials at most) and skips the rest by hue: Park's
  yellow paths with their bends, stripes and arrow heads; Sky's pink hubs
  and its lower path; all of Town Area 3 but its squares (roads, hubs and
  the lower level, left out on purpose: [the tile test](tile-test.md), A
  border kept whole). None of it has a place in a layer today, because a
  layer has no third kind of floor to put it on.
- **What crosses over or under.** Park's overpasses and the ramps up to
  them, on the map's second layer.
- **Set pieces.** A map's warp pads beyond the one kind a layer sets, its
  gates, the arrows where it leaves for the next map, Central's building.
- **The outside of a turn and of an end.** A rounded bend's outer rim, a
  round end's far side: tiles the learner turns down because the art runs
  past the line a tile may draw up to. The same cause as the notch under a
  Park platform's right corner.

## Where the unused tiles lie, every area

The owner's concern is variety across the whole board, not one map at a
time (13 November 2026). So the measure also says where in its map each
tile picture no layer drew lies (`lent.c`, `place`; `tiles_src_panels`):
by the panel its middle is in as the area's tiles are learned, and by the
walls' heights, since floor at a height is drawn that much higher. Over
all 39 areas, two seeds a layout, layers draw 40% of 49974 different
pictures; of the 29986 they leave:

| Where | Pictures | Share |
| --- | --- | --- |
| Away from any floor | 8357 | 28% |
| Beside the layer's own floor (a panel away at most) | 6721 | 22% |
| On the layer's own floor | 6118 | 20% |
| On floor at another height | 3379 | 11% |
| On floor of another kind | 1790 | 6% |
| On its pads (small platforms on spurs) | 1633 | 5% |
| On ramps between two heights | 1494 | 5% |
| Beside floor of another kind | 494 | 2% |

No one place holds most of it. What the tinted maps show in them is a
few families that come back in area after area, under different places
of this table:

- **A second look of walkway**, a road: Park's orange, BN4's ACDC Area's
  red, BN5's SciLab's at its exits, BN5's End Area's raised highway with
  its ramps, Sky's. It counts as other floor where the area's settings
  skip its hue, as the layer's own floor where they do not, and as raised
  floor where it runs at a height. A road mostly ends in an arrow's head
  at an exit.
- **Pads and hubs of other looks**: BN4's ACDC Area's round red hubs,
  Sky's cross-shaped pads, End Area's domed ones. A layer's pads all take
  the one look.
- **Marks on the floor**: one-way arrows, caps at crossings, Mr. Weather
  Comp's belts of lamps across its field.
- **Things**: the Aquarium's tanks and stepping stones, Mr. Weather Comp's
  clouds. Free-standing scenery (`decor.c`) is a small part: three areas
  have any (Robot Control, the Aquarium, CopyBot). A wider rule for it
  (what is joined to the floor told pixel by pixel) gained 0.3 points and
  took pads' leg labels for scenery; it was dropped.

| Area | Used | Left | On its floor | On its pads | On other floor | Raised | Ramps | Beside a floor | Away |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Judge Comp | 19% | 2947 | 740 | 144 | 565 | 0 | 0 | 1122 | 376 |
| Weather Comp | 21% | 2284 | 952 | 115 | 64 | 139 | 103 | 316 | 595 |
| Aquarium Comp | 35% | 2081 | 87 | 0 | 0 | 360 | 283 | 86 | 1265 |
| Sky | 37% | 1902 | 37 | 11 | 83 | 792 | 204 | 168 | 607 |
| BN5 End | 36% | 1867 | 398 | 82 | 66 | 301 | 127 | 475 | 418 |
| Robot Comp | 38% | 1665 | 318 | 403 | 199 | 0 | 0 | 559 | 186 |
| Undernet | 34% | 1439 | 145 | 2 | 44 | 344 | 106 | 373 | 425 |
| Nest | 26% | 1432 | 155 | 17 | 111 | 380 | 94 | 356 | 319 |
| BN4 ACDC | 34% | 1402 | 791 | 12 | 0 | 0 | 0 | 592 | 7 |
| BN5 Oran | 36% | 1386 | 326 | 71 | 0 | 181 | 126 | 349 | 333 |
| BN5 Nebula | 49% | 996 | 119 | 105 | 165 | 107 | 83 | 181 | 236 |
| BN5 SciLab | 49% | 995 | 510 | 121 | 0 | 0 | 0 | 341 | 23 |
| Seaside | 54% | 951 | 393 | 63 | 1 | 137 | 0 | 181 | 176 |
| Central | 56% | 842 | 49 | 98 | 150 | 204 | 0 | 199 | 142 |
| BN4 Town | 35% | 762 | 142 | 9 | 0 | 5 | 16 | 255 | 335 |
| BN4 Undernet | 46% | 642 | 102 | 36 | 0 | 0 | 25 | 290 | 189 |
| Green | 32% | 636 | 27 | 31 | 177 | 186 | 23 | 111 | 81 |
| BN5 dark 1 | 28% | 507 | 32 | 5 | 0 | 0 | 0 | 56 | 414 |
| BN5 ACDC | 63% | 490 | 216 | 106 | 12 | 0 | 0 | 149 | 7 |
| BN4 Park | 44% | 472 | 59 | 32 | 39 | 0 | 51 | 131 | 160 |
| BN5 dark 3 | 30% | 449 | 18 | 19 | 0 | 0 | 0 | 73 | 339 |
| BN5 Undernet | 73% | 446 | 137 | 87 | 0 | 6 | 18 | 132 | 66 |
| BN4 countries | 36% | 431 | 40 | 15 | 108 | 0 | 40 | 145 | 83 |
| BN5 dark 2 | 32% | 409 | 34 | 0 | 0 | 0 | 0 | 85 | 290 |
| BN5 dark 4 | 32% | 405 | 39 | 0 | 0 | 0 | 0 | 40 | 326 |
| Graveyard | 37% | 400 | 67 | 0 | 6 | 120 | 78 | 61 | 68 |
| CopyBot Comp | 56% | 352 | 34 | 8 | 0 | 59 | 53 | 82 | 116 |
| Secret | 50% | 334 | 19 | 10 | 0 | 58 | 64 | 89 | 94 |
| BN5 dark 7 | 41% | 318 | 26 | 8 | 0 | 0 | 0 | 62 | 222 |
| BN5 dark 5 | 40% | 300 | 32 | 0 | 0 | 0 | 0 | 45 | 223 |
| BN5 dark 9 | 51% | 190 | 34 | 0 | 0 | 0 | 0 | 38 | 118 |
| BN5 dark 6 | 61% | 144 | 31 | 0 | 0 | 0 | 0 | 43 | 70 |
| BN5 dark 8 | 66% | 109 | 9 | 23 | 0 | 0 | 0 | 29 | 48 |

(The comps and homepages that draw all of their one map are left out.)

What it says for the work: [Roads on the way](../world/level-design/roads.md)
is the mechanism for the first family, and it runs in one area, by a hue
named for it. Across the board it has to find each map's second look of
walkway by itself. The second family wants a layer's pads drawn from all
of its maps' pads, by the seed.

## What follows

The first and the last kind share a root: the learner does not know which
piece of a map a pixel belongs to. Told apart (each floor its own piece,
with the art that hangs on it), a rim past the line can be kept with its
floor, and another kind of floor becomes something a layer can be given.
The first step is taken ([the tile test](tile-test.md), The art told
apart by its floor): each pixel of a floor's art now carries the panel it
belongs to, and floor of another kind has art of its own.
Whether a layer should have it is a question for
[the generator](../world/level-design/generator.md) and the owner. Park's
orange and yellow paths are plain floor in the map's data, and assumed so
in play ([Sources](../research/sources.md), BN4's Park Area): a second
look of walkway, which BN4 lays as long straight runs with rounded bends
and striped joints, mostly towards its exits, changing to pink end to end.
So a layer can carry them without a new rule of play, and without a new
layout: some of its walkways drawn as roads. Which ones is the design:

- **The way.** The walkways of the layer's way from arrival to exit as
  roads, the side paths pink: BN4's own habit, and a hint to the way that
  costs no marker.
- **By the seed.** Long straight runs as roads, wherever they lie: variety
  alone.
- **The lanes.** The arrow lanes' panels as roads
  ([Arrow lanes](../world/level-design/arrow-lanes.md)): a road that
  carries MegaMan is ours, not BN4's, as long as BN4's are plain.

The owner chose the way (13 November 2026), and it is built for Park Area:
[Roads on the way](../world/level-design/roads.md). In the tiles, a road
is a piece drawn apart with books of its own, as a platform is: the same
map learned a second time with the road's colours as its walkway and the
pink skipped. Its ends where it meets a walkway are not learned yet.
