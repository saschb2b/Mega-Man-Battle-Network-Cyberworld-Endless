---
type: Feature
title: "Oran Area's tiles"
description: "BN5's Oran Area drawn on generated layers: what its maps draw (octagon hubs, never a bare side), the kinds of breaks its layers showed, the drawing taken (hubs one floor with their circuits, lay 11, hub cells worn to circuits drawn as circuits: 332 to 241 breaks per 100 panels) and what is left."
tags: [world, level-design, tiles, bn5]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-15T18:30:00Z }
sources:
  - id: bn5-maps
    resource: "BN5 Team Colonel (USA), Oran Area 2 (0x90:3) and Oran Area 1 (0x90:2), drawn by `--atlas DIR:a4:2`"
    title: "Oran Area's two maps, their panels read by the learner (src_b23_90_3.txt)"
  - id: bn5-walls
    resource: "BN5's coordinate data for 0x90:3, each panel read with area_src_walled_floor"
    title: "Which of Oran Area 2's panels are floor MegaMan stands on"
  - id: playtest
    resource: "a persona playtest of dev at e2005dac6, 15 November 2026 (seed 8808, layer 7)"
    title: "The orange octagon's left edge jagged, as if tiles were cut"
---

# Oran Area's tiles

BN5's Oran Area dresses Green Area in a run with BN5's ROM beside BN6's
(`src/core/rom.c`, its `bn5_areas` entry). In the
[tile test](../../devtools/tile-test.md) it was the worst of all 39 areas:
332 breaks per 100 panels, where BN6's own comps count about 2. A
playtester saw it on a layer: the orange octagon's left edge cut into
jagged blocks.

## What its maps draw

Oran Area 2 (0x90:3) lends nearly every tile; with Oran Area 1 (0x90:2)
left out, the atlas's six layers count the very same breaks. Its pieces,
surveyed by eye beside the generated layers:

- **Hubs**: orange octagons on a 3 x 3 pad, framed in grey, a round knob at
  the top and bottom corners and a block ear at the left and right.
- **Circuits**: olive walkways a panel wide in grey lips.
- **Something at every side of a hub**, in its middle. Of Oran Area 2's
  eleven hubs' 44 sides, 40 meet a circuit or a dock, a pale stone plate
  one panel long that ends there; the other four meet brown planks or an
  arrow panel (looked at in its picture). The docks are floor: BN5's
  coordinate data puts every dock panel on floor, the same as its circuits
  (verified by reading the walls of 0x90:3 at each panel, 15 November
  2026). So no map of Oran's draws a hub side with void beside it.
- A circuit meets a hub in one piece of art: its lips run into the hub's
  frame.
- Brown planks, blue and purple arrow panels (conveyors) and the green
  warp pads, which lend no tiles off the hubs (`skip_styles`).
- Grey sprigs behind the planks' upper edges, which no layer can set
  ([side pieces](side-pieces.md), What Oran Area sets round its floors).

## The breaks it showed

Six layers, two seeds a layout, 809 panels, 2683 breaks, counted by
[where they lie](../../devtools/tile-test.md) (breaks / panels):

| Place | Before | After |
| --- | --- | --- |
| A circuit at a hub | 677 / 130 | 289 / 130 |
| A pad (a 3 x 3 hub) | 497 / 180 | 435 / 180 |
| A circuit's straight run | 380 / 150 | 181 / 150 |
| A room's edge | 351 / 102 | 378 / 102 |
| A room's corner | 326 / 100 | 250 / 100 |
| A circuit's end | 200 / 29 | 207 / 29 |
| A circuit's fork | 135 / 42 | 124 / 42 |
| A circuit's bend | 92 / 17 | 48 / 17 |
| A room's notch | 25 / 9 | 41 / 9 |

The kinds, looked at in the worst windows of each layer:

1. **Joins.** The hubs were drawn apart from the circuits
   (`NET_APART_PLATFORMS`): a hub as if no circuit met it, a circuit
   ending plainly at the hub's rim, and between them pieces of docks,
   ears and orange floor. The most breaks a panel, and what the
   playtester saw.
2. **Bare sides.** A generated hub with no circuit at a side has a shape
   its maps never draw. Its corners were laid in pieces, and slivers of
   docks stand off the side: the jagged edge.
3. **Straight runs.** A circuit's panels taken up for how they met their
   neighbours were picked again tile by tile, the circuit's pattern broken
   every few panels.
4. **Hubs worn to circuits.** The legalizer changes some of a layout's
   cells, and a hub's cell it leaves in no 2 x 2 block of floor still had
   the pads' look: it took a hub's panels, orange middles in the mouth of
   a circuit.
5. **Shapes BN5 never draws**: two hubs a panel apart, a circuit one
   panel long between them, rooms bigger than 3 x 3 drawn as hubs.

## What it draws now

Two settings of its entry, measured each way, and a rule:

| Laying | Breaks per 100 panels |
| --- | --- |
| Drawn apart, as before | 331.6 |
| Drawn apart, rules 15 | 303.6 |
| Drawn apart, rules 11 | 303.2 |
| One floor | 315.8 |
| One floor, rules 15 | 276.6 |
| One floor, rules 11 | 271.1 |
| **One floor, rules 11, worn hubs drawn as circuits** | **241.4** |

- **The hubs one floor with their circuits** (no `.apart`). A circuit
  meeting a hub's side is laid from the map's own joins, its lips running
  into the frame as BN5's do.
- **`lay` 11**: a panel taken up is laid again, and an end's panels lend
  no other panel art ([the tile test](../../devtools/tile-test.md), Each
  area's laying). Straight circuits come out whole. Rule 4 (beside a
  panel laid whole, its map's own tiles) drew a little worse here.
- **A hub's cell worn to a circuit's shape is drawn as one** while the
  tiles are picked (`pad_at`, `src/map/netmap.c`): on a floor drawn as
  one and told by shape, a pad's cell in no 2 x 2 block of floor loses
  the pads' look. Measured over every area: it changes Oran's alone,
  where it holds; on floors drawn apart it changed three areas, all for
  fewer breaks (BN4's Town Area 264 to 222 per 100 panels, its ACDC
  Area 199 to 190, SciLab 170 to 169), but in BN4's the worn cells take
  the way's road look, which those areas' own passes should judge, so
  it holds on floors drawn as one alone.

Every layer counts fewer breaks, nicks and seams (2683 breaks to 1953,
seams 2114 to 1845). More tiles are picked one at a time from a nearby
neighbourhood, since hubs and circuits are one floor now (10.5% to 13.4%
of tiles on average over the six layers, 18.9% at most), so the area's
rows
in `tests/atlas_baseline.txt` were written again. The layouts are as they
were: the cells and the coordinate data's hash are the same for every
layer, and no seed makes anything else.

## Not mended

- **Bare sides.** The slivers stay where no circuit meets a hub. The one
  drawing that mends them is BN5's own: a dock at every bare side. Tried
  with the hubs drawn apart, it took the pads from 497 breaks to 298 and
  the area to 270 per 100 panels, and looked like BN5's hubs. But BN5's
  docks are floor MegaMan steps onto, and here a dock would be drawn over
  void he cannot step onto. That is the owner's call, and it is not done.
- **Orange blobs** where two hubs nearly touch: told by shape, a hub's
  rim with floor closing round it reads as the hub's middle. Keeping
  each pad room's own rim as rim was tried (270.1 before the rule above,
  the blobs stayed).
- **Rooms bigger than 3 x 3**: drawn as hubs, their inner edge scalloped
  (tiles drawn off), their frame repeated along a side.
- The [roads' rule](roads.md) found none here: Oran's brown planks lie
  between its hubs, in scraps.
