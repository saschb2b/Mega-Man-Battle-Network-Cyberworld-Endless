---
type: Feature
title: "Roads on the way"
description: "Roads: in an area whose map has a second look of walkway, found as it is learned (BN4's Park Area's orange roads, its ACDC Area's cyan, its Town Area's yellow), the walkways of a layer's way from its arrival to its exit are drawn in it, the side paths in the area's walkways; a look alone, no rule of play (`mark_roads`, `TILE_ROAD`, `learn_roads`)."
tags: [world, level-design, tiles]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-13T20:30:00Z }
sources:
  - id: owner-call
    resource: "the owner's answer in the session, 13 November 2026"
    title: "Which walkways a layer draws as roads: the way to the exit"
  - id: park-map
    resource: "docs/rom/bn4.md"
    title: "Park Area 1's coordinate data: its orange paths plain floor"
---

# Roads on the way (built: BN4's Park, ACDC and Town Areas)

BN4's Park Area 1 has two looks of walkway: pink, and orange roads a panel
wide in a pink and blue rim, with rounded bends and striped joints. In the
map's data both are plain floor ([BN4 Red Sun's tables](../../rom/bn4.md),
Park Area 1's coordinate data; in play assumed so,
[Sources](../../research/sources.md), BN4's Park Area). Three of its five
exits lie at the end of an orange road, in an arrow's head. The
[measure of what a layer uses of a map](../../devtools/map-use.md) named
them as the largest kind of floor a layer left out. BN4's ACDC Area has
cyan roads and its Town Area yellow ones, the same motif.

## Which areas have roads

None is named: a road is found in an area's own map as it is learned
(`road_hues`, `src/map/netmap_learn.c`). Its walkways on the ground (floor
a panel wide, in no 2 x 2 block) are counted by their hue. The area's own
walkway hue is the one its settings name, or the one most of its maps'
walkways show; a road is another hue, none of its platforms' and none
next to its walkways' own, shown by a dozen panels, five of them in one
straight run, and mostly in the open (a quarter of its panels at least
with no other floor beside them).

| Area | Its walkways | Its road | Panels, longest run |
| --- | --- | --- | --- |
| BN4's Park Area | pink | orange | 30, 11 |
| BN4's ACDC Area | white | cyan | 15, 8 |
| BN4's Town Area | pale cyan (Town Area 1's) | yellow (Town Area 3's) | 20, 5 |

Every other area's map was surveyed the same way (13 November 2026) and
has none on the ground. What the rule turns down, and why it is right to:

- **A platform's hue a panel wide**: Central's green strips, Robot
  Control's grey.
- **A shade of the walkway's own**: a dark area's blue beside its cyan (16
  panels, a run of 9), which as a road drew no different.
- **A joint's or a pattern's hue**: BN4's Undernet's steel joints (42
  panels, never three in a row), the Undernet's bridges of three hues.
- **A look that lies between other floor**: BN5's Oran Area's brown planks
  between its hubs (34 panels, a run of 6, but an eighth of them in the
  open): as a road it was laid in scraps.

BN6's and BN5's second systems of floor run on raised levels (End Area's
highway, Sky's paths, Seaside's): 84 panels a panel wide at most on any
one level, and no road a layer's ground could take as it stands.

## What a layer does with them

The owner's call (13 November 2026), of three put to him (the way, by the
seed, the arrow lanes): **the walkways of the layer's way from its arrival
to its exit are roads, the side paths stay pink.** Nothing else changes:
the layout, the walls, the battles, what stands where.

The design, by the game-design skill:

- **The dialectic** it serves is the layer's own: push on to the exit, or
  look round for what the side rooms hold. A road says which is which
  without a marker; the choice stays the player's.
- **The loop layer** is the walk across a layer, moment to moment. It
  feeds nothing new: no reward, no cost, no state.
- **The pattern** is the golden path signposted in the world's own colour,
  the way BN4 itself lays its roads towards its exits. It is diegetic and
  costs no HUD.
- **The smell watched** is the one the statistics already show: players
  leave half a layer's data behind. A road makes the short way plainer.
  It is one area of the BN4 set, so the dose is small, and the statistic
  on data taken per layer is the thing to read before another area gets
  roads.

Rooms on the way stay rooms: a road is the look of a walkway, a floor one
panel wide. Where the way was widened to two panels (the generator's cap
on one-wide walkways, [the generator](generator.md), Navigation) it is
platform floor and no road. A pad is no road either.

## How

- **The way** is the generator's (`ng_mark_way`, the shortest walk from
  the arrival to the exit or the guardian), handed to the map as
  `NetLayout.way`. The legalizer then changes cells, among them a lane of
  a way widened to two, so the map walks it again over the floor as it
  stands, between the same two ends, keeping to the generator's cells
  where it can (`mark_roads`, `src/map/netmap.c`). It reads the layout
  alone, so a layer is the same from its seed.
- **The look** is learned from the area's own map a second time
  (`learn_roads`, `src/map/netmap_learn.c`): the roads' hues as its
  walkways, its walkways as the floor that lends no tiles, no platform
  floor, its panels laid whole whatever the area's own are. Park Area 1 gives 46 whole
  panels and 55 in part, per mirror image.
- **Drawn apart.** A walkway cell on the way is marked `TILE_ROAD` while
  the tiles are picked; the legalizer reads walkway floor as before. Each
  run of road is a piece of its own, after the rooms and before the rest
  of the floor (`src/map/tilemap.c`), picked from the roads' books.
- **Its ends.** BN4 ends a road in an arrow's head or at a pink walkway,
  never at a platform, and a road's rim is wider than a tile picked alone
  may draw. So a road is laid as if it ran on two panels under whatever
  floor it meets (`pieces_floor`), and only its own panels' tiles are
  kept (`road_draws`), behind the room's (`stack_road`): its rim runs
  whole up to a room's edge.
- **Thin books** (`TileGrid.thin`, `src/map/panels_lay.c`). One map's
  roads are a few dozen panels, where an area's walkways are hundreds, and
  a road's rim lies past where a tile picked alone may draw: a panel that
  does not meet its neighbour cannot fall back on single tiles, the rim
  would be a gap. So:
  - **A straight run is laid from one plain panel** (`lay_plain`). Park's
    map has two to six panels between a joint and the next, a bend or an
    end, and all but the middle ones carry the start of what follows.
    Laid on by its map's neighbours and then by whatever suited, a run
    longer than the map's took a panel that did not meet the last at every
    step, and the joint's panel, which meets anything across its gap, at
    each: a ladder of joints. The plain panel is the one of its shape that
    meets itself on both sides, of those one in the open (no other floor's
    art in the tiles round it).
  - **Beside a panel laid whole, its map's own tiles** (`keep_to_whole`),
    set before any tile is judged by how it meets its neighbour: a rim
    tile from another place did not meet the plain panel, and every second
    panel of a run was taken up.
  - A panel taken up is laid again from another that meets what is laid
    round it, or in pieces (`lay_again`, `lay_thin`); what is still open
    round the floor is filled from any panel alike.
  - A road's end lies two panels under what it meets and is never drawn:
    how a panel meets it takes none up (`hidden_end`), and an end's panels
    (an arrow's head, wider than its panel) and those round it lend
    nothing where the floor has no end (`ends_near`).
  - Nothing is picked alone off the road's floor or on a corner's tip
    (`tilemap.c`, `on_floor`).

## Not mended

- **Bends.** Park's map has each kind of bend once or twice, some only
  beside other floor, which lends its tiles in part. A bend is laid whole
  where its map's panel fits and in pieces where not: then with a nick at
  its inner corner, a tile past its tip or a gap at it. Two bends in a
  row (a jog of one panel) show most.
- **Joints.** A run is plain from end to end. BN4 sets a striped joint
  every few panels; a layer's road has one only where a panel outside a
  run happens to carry it.
- **The tile test's counts.** Park's layers count more breaks with roads
  (per 100 panels 90 before, 96 now): no map of BN4 shows a road at a
  room, so each such join counts, as a walkway's does.
  `tests/atlas_baseline.txt` holds the new counts.
- **Where a road meets a pink walkway** side on, the walkway ends plainly
  at the road's rim; BN4's own change of look end to end (a line across)
  is not learned, as each look's learning skips the panels beside the
  other.
- **The New 3DS's heap** with two more books is not measured
  (`tools/heap3ds.py`).
- **Town Area's roads** are the roughest of the three: its map has 22
  whole panels of road, and its layers count 7% more breaks with them
  (per 100 panels 329 to 352).
- **Raised roads.** A second look of floor on a raised level is not found:
  a layer has no place for it.
