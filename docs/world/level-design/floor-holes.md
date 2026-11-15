---
type: Design
title: "Holes in the floor"
description: "The safety net under a generated layer's tiles (`src/map/voids.c`): once its tile map is whole, every tile's top is held to the pixels its two entries draw, and a hole where the background shows through a walkable panel is filled from behind with a tile of the area's own maps from that place in a panel, or the tile laid again with one of their pairs."
tags: [world, level-design, tiles]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-15T01:42:00Z }
sources:
  - id: playtests-s76
    resource: ".build/play/kai6, kai8 (session 76's playtests, not in git)"
    title: "Holes seen on RoboDog Comp's layer 1 (kai8) and BN4's ACDC Area's roads (kai6)"
---

# Holes in the floor

A generated layer's tiles are picked by rules of their own
([the generator](generator.md), Tile picking): the classes' picks, the
original's panels laid whole, the pieces drawn apart and laid over each
other on the two tile layers ([roads](roads.md) among them), and what is
pasted over them (pads, stairs, props, [side pieces](side-pieces.md)).
Each holds to its own rules, and where they meet as none of them foresaw,
a tile on the floor can draw nothing over part of its top: the background
shows through a panel MegaMan walks on. Four playtesters saw it on
15 November 2026, and none of the checks before had a name for it: the
tile test's breaks are held to the strips the originals show at their
tiles' edges, and against a tile that draws nothing no strip is asked
after ([the tile test](../../devtools/tile-test.md), Breaks).

- BN4 Red Sun's ACDC Area (kai6, run seed 0x19CE, layer 1; kai7's layers 2
  and 3): navy squares cut into its cyan roads, the background's navy, and
  roads ending in steps of it.
- BN6's RoboDog Comp, layer 1 (kai8, run seed 0x2269): what looked like a
  walkway's panel between a 2 x 2 platform and a room, its lower half the
  pale background. Drawn again in the atlas (`r2269,1,8`), it is no hole
  in the tiles but one in the layout: a void panel with floor on its four
  sides at a junction, walled off as void is, whose upper half, where the
  faces of the panel above hang, the pick drew in walkway tiles. The net
  leaves it: it fills the top of the floor, and a pit is none. A shape
  Capcom's maps never make is the generator's to keep out
  ([the generator](generator.md), Navigation).

## The net

Once the map is whole, before its colour banks are set and its scenery
placed (`pick_map` in `src/map/netmap.c`), `voids_mend` holds each tile's
top to what its entries draw:

- **What a tile draws** is known without drawing it: each of the 1024
  tiles its tile set holds draws the pixels whose colour is not 0
  (`area_src_opaque`, a 64-bit mask a tile, kept while the layers are the
  same area's), flipped as the entry says. A tile's two entries are two
  lookups.
- **Its top** is its pixels on the floor (the layout's panel under each,
  as the pick reads it) with floor 4 pixels to either side and 2 above
  and below: the floor's outline, where a tile's own edge may fall short
  and a rim's corner be cut off or notched (Robot Control's platforms, a
  red stud in a notch at each corner), is not its top. Only a tile that draws less than all of
  its pixels and lies by the floor is looked at so.
- **A hole** is more than 2 pixels of its top left undrawn
  (`VOID_SLACK`), unless the area's own maps leave them undrawn too where
  they set a tile in the very same neighbourhood (a pair of its class,
  the panels round it the same, drawing at most 2 of them): then it is
  their art. Robot Control's platforms set a red stud in a notch at each
  corner, which read as 14 holes over its atlas's layers.

A hole is mended from the area's own maps, the books the pick reads (a
road's from its roads' books): of the pairs seen at the tile's place in a
panel (its phase), the one whose neighbourhood lies nearest the tile's
own, that covers the hole and draws nothing where the floor must not be
(the pick's pixel test, `tl_expect`):

- **behind the tile's own**, a pair that is one tile: the tile's front
  entry stays in front, on the second layer, and the hole shows the floor
  of its place. A room's rim laid over a walkway's end, a prop's stamp,
  keep their art.
- **in its stead**, a pair whole, where none behind it covers the hole;
  dearer, as the tile's own art goes.

A hole nothing covers is left and counted. Invisible paths and Rush's
gaps are void in the layout itself, so no hole: the net reads the floor
the pick reads ([invisible paths](invisible-paths.md), [Rush's gaps](rush-gaps.md)).

It changes no layout and no placement, only the entries of tiles that
showed a hole: what a layer seed makes is the same (no `LAYER_MAKE`). It
allocates the floor's panels as bytes and, while it reads them, the tile
set's 64 KB as VRAM takes it; a hole's mend is a look through its
phase's pairs.

## Measured

The atlas counts the holes each layer's tiles left (`voids`, framed white
where the net mended one, blue where it left one: `holes left`), and
`CYBERWORLD_VOIDS=0` draws the layers without the net
([the atlas](../../devtools/atlas.md)).
