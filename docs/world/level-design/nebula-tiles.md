---
type: Feature
title: "Nebula Area's tiles"
description: "BN5's Nebula Area drawn on generated layers: its maps' purple plates on one-wide cobbled paths, the breaks its layers showed (rooms wider than a plate drawn as cobbles round a plate), the drawing taken (plates as plain platform floor, lay 31 | 64: 273 to 211 breaks per 100 panels) and what is left."
tags: [world, level-design, tiles, bn5]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-15T15:11:16Z }
sources:
  - id: bn5-maps
    resource: "BN5 Team Colonel (USA), Nebula Area 2 (0x90:13) and Nebula Area 4 (0x90:14), drawn by `--atlas DIR:a3:2`"
    title: "Nebula Area's two maps beside the atlas's six layers, looked at side by side"
---

# Nebula Area's tiles

BN5's Nebula Area dresses an area of BN6's in a run with BN5's ROM beside
BN6's (`src/core/rom.c`, its `bn5_areas` entry). In the
[tile test](../../devtools/tile-test.md) it counted 273 breaks per 100
panels on 15 November 2026, among the three worst areas left after
[Oran Area's pass](oran-tiles.md).

## What its maps draw

Looked at by eye beside the atlas's six layers:

- **Plates**: purple squares of 3 x 3 panels, a star of lines across them,
  purple to their rims, a magenta stone hanging at each corner. Nebula
  Area 2's carry an emblem in their middles, Nebula Area 4's are plain.
- **Paths**: cobbles one panel wide, their stones teal and purple by turns,
  joined to a plate at the middle of its side.
- No floor wider than a path but the plates.

## The breaks it showed

Its plates were learned in their pads' look, so a room of the layout that is
no pad (any room past a plate's 3 x 3, and floor two wide) took its
panels from the paths: a field of cobbles with a plate's middle somewhere
inside, its rims in stones cut off at every tile. A room's edge was the
worst place (1479 of 2969 breaks on 315 of 1086 panels), then where a path
meets a room (496), a room's corners (371) and notches (244).

## What it draws now

Two settings of its entry, measured over the atlas (`--atlas DIR:a3:2`,
six layers, 1086 panels):

| Laying | Breaks per 100 panels |
| --- | --- |
| As it was (its pads' look, lay 31) | 273.4 |
| Lay 63 (a room's rim takes no path's piece) | 272.7 |
| Drawn apart, lay 31 | 287.8 |
| Without its pads' look, lay 31 | 224.0 |
| **Without its pads' look, lay 31 \| 64** | **210.9** |

- **Its plates are platform floor** (`TILES_NO_PAD_LOOK`). Learned so, a
  room of any size is drawn as a plate: purple to its rim, stones at its
  corners, the star's lines across its panels. Floor two wide on the way is
  a plated way too, where it was cobbles.
- **`lay` 31 | 64**: the rules beside panels laid whole, but with its pads'
  panels alone laid whole, and it has no pads now, so its floor is picked
  tile by tile and what lies round it (faces, the stones under its
  corners) is laid from its maps' panels. Against lay 31 the pictures are
  the same but for fewer scraps at the joins (224 to 211).

Every layer counts fewer breaks (2969 to 2290); the nicks rose (845 to
973) and the seams on four of the six layers, the lines of two plates'
stars meeting where no map sets two plates side by side. The layouts are
as they were: each layer's coordinate data hashes the same. Its six rows
in `tests/atlas_baseline.txt` were written again.

## Not mended

- **A plate's star where rooms are wider than a plate**: each panel of a
  wide room takes a plate's panel, so the stars' lines repeat and cross
  where two plates' panels meet. BN5 has no plate larger than 3 x 3 to lend
  a wide room's middle.
- **A room's edge** (1084 breaks on 315 panels, from 1479): a plate's rim
  where a layout's room is no plate's size, a stone or half a star's line
  cut at a tile.
- **Where a path meets a room** (520 on 224, from 496): a path's cobbles
  end in steps against a plate's rim, which BN5 joins at a plate's side
  middle only.
