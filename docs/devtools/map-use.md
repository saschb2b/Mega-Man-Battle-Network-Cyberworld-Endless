---
type: Measurement
title: "What the layers take from a map"
description: "The atlas counts, per area, how much of each original map its generated layers ever draw (two thirds of the maps' tiles, two fifths of their different tile pictures) and draws each map with the unused tiles tinted; what stays unused, by kind."
tags: [devtools, level-design]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-13T21:30:00Z }
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

## What follows

The first and the last kind share a root: the learner does not know which
piece of a map a pixel belongs to. Told apart (each floor its own piece,
with the art that hangs on it), a rim past the line can be kept with its
floor, and another kind of floor becomes something a layer can be given.
Whether a layer should have it is a question for
[the generator](../world/level-design/generator.md) and the owner: what
Park's yellow paths do in BN4 is not looked up yet, and a path that moves
MegaMan there is more than tiles.
