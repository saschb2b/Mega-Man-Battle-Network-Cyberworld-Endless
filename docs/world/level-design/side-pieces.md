---
type: Design
title: "Side pieces"
description: "What an original map hangs on its floor's sides (CopyBot Comp's claws, the Undernets' flames, the Aquarium's pillars), cut whole from its second tile layer with the panel it grips (`src/map/sides.c`) and set on a generated floor's panel of the same neighbourhood; the survey of what layers still miss (issue #505)."
tags: [world, level-design]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-15T18:30:00Z }
---

# Side pieces (built for five areas, issue #505)

The originals do not stop at their floors. CopyBot Comp's walkways are
gripped by purple claws at their bends, flames stand beside the catwalks of
BN4's Undernet, the Judge Tree hangs planters on its walls. A generated
layer drew its floor from the maps' tiles and none of that: a survey of 24
areas (13 November 2026, issue #505) found the layers drawing 41% of the
maps' different tile pictures, some 6,800 of the rest lying beside a floor.
The free-standing kind, which touches no floor, is the scenery
[the generator](generator.md) sets in the void (`decor.c`); this page is
the kind fixed to the floor.

## What a side piece is (`sides_learn`, verified on the maps)

A map's second tile layer holds what it sets in front of its floor. A group
of its tiles joined eight ways is a side piece where it both grips ground
floor and hangs off it: at least 8 of the pixels it draws lie on a ground
panel's top, and three tenths or more lie off every ground panel's. That
tells the three kinds on that layer apart:

- what lies on the floor alone is a pad's centrepiece or an emblem, which
  marks a warp and is set by rules of its own ([Props](props.md));
- what touches no floor is scenery;
- what does both is a claw, a flame, a lamp on a rim.

A group is refused where the map's edge cuts it, where it is smaller than 4
tiles or larger than 160 (20 by 14 at most), where a tenth of it lies over
floor of another height (a bridge, a raised floor's own tiles) and where the
panel it grips is an [arrow lane](arrow-lanes.md)'s (the lane's frame goes
with the lane).

It is kept with the panel it grips, the one most of its floor pixels lie
on, and that panel's neighbourhood in its map: which of the nine panels
round it are ground floor. With it go the panels it draws over (6 pixels or
more, 16 at most), each as ground floor or none; floor of another height
under it is left out. Each look is kept once, 24 an area at most.

## Where a layer sets one (`nm_paste_sides`)

On a ground panel of the layer whose neighbourhood is exactly a piece's, so
a claw cut at a bend stands at a bend and reaches into the same void, and
where every panel it draws over is what it is in its map: ground floor
where it lay on floor, nothing where it hung free. The Undernet's flames
come in twos and threes that reach past the nine panels, and without that
a set of three burned over a room's floor (seen on a layer). Not
on raised floor, a pad, a lane or the arena nor beside them, and not within
a panel of anything that stands on the layer (Mystery Data, a Navi, a
warp). Of the panels that fit, half are taken by the layer's seed, none
within two panels of another, 12 a layer at most; a piece goes down whole
or not at all (on the map, on its tile lattice, over nothing the second
layer already holds, nor over a pad's or a stair's art). It is the second layer's tiles alone: the floor under
it is the layer's own, and nothing about walking changes.

## Who is in front (`priorities`, `nm_sides_extra`; verified in the game)

BN6 draws MegaMan behind a claw on the panel it grips: its maps hold layer
priorities (section 2 of the coordinate data) there, 27 to 48 cells within
a panel of each of CopyBot Comp's claws, of the map's 434. A piece is cut
with the cells on its panel and the eight round it and they are written
with the layer's own, so he passes behind a claw on a generated walkway as
he does in the comp (placed on a claw's panel in a CopyBot layer, the claw
covers his legs; a panel back, he stands clear of it).

## Which areas (`net_area.sides`)

Taken per area as looked at, as every laying rule is
([the tile test](../../devtools/tile-test.md), Each area's laying):

| Area | What the rule cuts | Taken |
| --- | --- | --- |
| CopyBot Comp | its claws, 3 looks; behind its plateaus, the dark crystal spikes (below) | yes, both |
| BN4's Undernet | the horns in front of its catwalks, 8 looks; behind them, the rows of horns on the first layer (below) | yes, both |
| The Undernet (Undernet 1-3) | its fire bowls beside floors and bridges, alone, in twos and threes: 9 looks, and Undernet Zero's 4 | yes |
| Undernet Zero | its fire bowls in threes, 4 looks, and the Undernet's 9 | yes |
| The Nest (Underground 1-2) | its crystals and their rubble beside its rims, on the second layer: behind upper edges and under lower ones (below), 17 looks; its lone spires fit no layer | yes |
| Central Area | the Net Dealer's capsule counter, which means a shop | no |
| Aquarium Comp | one piece on the second layer, nothing to see; under its pads, the pillars (below) | the pillars |
| BN5's Oran Area | behind its floors, 4 looks that are no thing and its sprigs, which fit no layer (below) | no |
| BN4's ACDC Area | nothing: on its second layer a lid on a hub alone; behind its floors, 6 looks of floor art (dead ends' discs, an arrow's head); no scenery (below) | no |
| Robot Control Comp | nothing on its second layer; its robot figures and its wires with red lights are scenery, cut against its own faces (below); the bulbs on springs under its white platforms and the fists past its arms' ends are not placed (below) | the scenery |
| every other area | nothing | - |

## The Nest's crystals (`sides_learn_under`; verified on the maps and in the game)

The Cybeast Nest's two maps (0x93:0 and 0x93:1, the Underground) stand
crystals round their rims: squat stumps and tall spires, with loose
rubble about them. They lie on the second tile layer, as the Undernet's
bowls do (the maps' two layers drawn apart: the crystals and rubble on
the second, the floor, its rims, tabs and spikes on the first), and
none of their pixels lies on a floor's top where they stand in front of
a lower edge. So the side-piece cut takes, in this area, a group that
touches no floor's top where a ground panel lies straight above its top
row within 48 pixels, kept with that panel (bit 3 of `net_area.sides`).
With the ones behind upper edges that the cut took already, the maps give
17 looks.

Set as they were cut, a piece behind an upper edge stood over a rim's tab
its map had not had there, and each added about three breaks to the
[tile test](../../devtools/tile-test.md). So each of these looks keeps
the first layer's tiles under it (`SidePiece.over`; an entry that draws
no pixel counts as none) and goes down only where the picture round it is
the same: where the layer's first-layer tile differs from its map's, both
must lie wholly behind the crystal's own pixels (the tiles' pixels,
`voids_entry_px`). That is rare at any one panel, so these looks try
every free panel of the floor rather than half, still three apart, and a
panel takes any of the looks that fit there rather than the one its hash
names. Over 18 of the atlas's layers that sets 2.8 a layer (0 to 6, one
layer without), in three looks (a stump with its rubble in front of a
lower edge or behind an upper one, a spire behind an upper edge), and no
layer's breaks grew.

The lone spires (8 tiles) never fit: in their maps they stand in front of
a room's corner, over the stem of the tab that hangs there and the rim's
face (92 tries refused over six layers), which a layer's corner rarely
draws in the same tiles. Set without them, the stem would be cut off at
the spire's edge. The rubble alone in the void (a tile to three) is
smaller than a piece, the rubble on the floor's top is no side piece, and
the crystals along the maps' bridges stand by raised floor, beside no
ground panel.

The maps hold no layer priorities round them (none within a panel of any
look), and none are needed: BN6 draws MegaMan in front of the second
layer there, so he walks in front of a stump behind an upper edge, and
stands behind one in front of a lower edge, his feet on the floor above
it (captured on a Nest layer, seed 4). A Glitch flip into the Nest
draws them too: they are set in the tiles' own pass (`pick_map`) from
what the area learned, and need nothing but their tiles.

## Pieces the game animates (verified in the game)

The Undernet's fire bowls burn: their twelve tiles are the ones the map's
animation script writes over every 8 frames (tiles 726 to 737;
[the ROM map](../../rom/graphics.md), Net maps' own tile animations), and
the tile set as stored holds numbered placeholders there, which is how
the [tile test](../../devtools/tile-test.md)'s pictures show them
("1 2 3 4 / 5 6 7 8 / 9 10 11 12"). A layer runs on a map of its area's own
group, whose script BN6 starts by itself, so a piece set with those tile
numbers burns on a generated floor with nothing added (captured on an
Undernet layer: bowls of flame beside a bridge and a room's corner, their
flames changing between frames). Five of Undernet 1's lone bowls draw
nothing on ground floor and are no side piece; the ones that touch a rim
are.

The Undernet's maps hold more in the second layer than what hangs on a
floor's side. In Undernet 1 the rest of it joins into one group of some
3,000 tiles, which the rule refuses for its size, and every piece it cuts
there holds a bowl (9 of 9, Undernet Zero's 4 of 4).

The Undernet's four maps (0x95:0 to 3) share one tile set and one palette,
byte for byte (compared: 23,232 and 5,472 bytes of tiles, the second block
loaded at tile 726, where the bowl's twelve begin). A piece is its tile
entries, so either area takes the other's too (`net_area.side_maps`):
Undernet Zero's own come in threes alone, which fit a generated floor about
once a layer, and with the Undernet's lone bowls and pairs it gets four.

## What stands under a raised floor (`hangs_learn`; verified on the maps and in the game)

Aquarium Comp's glass pads stand on pillars, short stubs with bubbles
rising under each and long ringed columns. They are no second-layer
piece: they lie on the first layer, under the pads' lower edges, and its
pads are raised (its maps hold the water lanes at height 0 and the pads at
16; Aquarium Comp 2, 0x81:0: 6,177 and 984 height cells). A layer's pads lie
on the ground, so a cut under ground panels found none.

They are cut in the view that brings the pads' level down to the ground
(`area_src_raise_in`, as the tiles of a raised floor are learned). A
first-layer tile belongs to such a piece where it draws, lies off that
level's floor at its four corners, and lies more than 8 pixels under any
edge of it, past the pad's own rim. Tiles joined eight ways are one piece
where no pixel of it lies on that floor and a panel of it lies straight
above its top row, within 48 pixels; the piece is kept with the panel most
of that row's columns meet, its neighbourhood and the panels it draws
over, as a side piece is. The Aquarium's four maps give more than the 24
looks kept: a lone stub or column (11 to 18 tiles) and whole sides of a pad
(27 to 55).

A layer sets one under each panel of a pad that fits: the same
neighbourhood, nothing where it hung over nothing. It begins in the tiles
of the pad's rim, so up to a third of it may go over tiles the first layer
already holds there, in its two top rows alone. No seed thins them, as none of the maps' pads lacks
its pillars: 15 to 20 a layer in the [tile test](../../devtools/tile-test.md)'s
pictures. The stubs' bubbles are the map's other animation (tiles 1 to 12,
[the ROM map](../../rom/graphics.md)) and rise on a layer as the
Undernet's bowls burn (captured: a pad on six pillars, the bubbles under
its stubs changing from frame to frame; the fish there are the
backdrop's). Nothing about walking changes: the panels under a
pillar are void in the layer as in the map.

## What stands behind a floor (`behind_learn`; verified on the maps and in the game)

BN4's Undernet sets horns along both sides of its catwalks. The ones in
front lie on the second layer and are side pieces; the ones behind, in rows
along a catwalk's upper sides, lie on the first, where the floor's own art
covers their feet (its maps 0x94:2 and 0x94:4: some two dozen, 10 or 11
tiles each; the scenery rule refused every one as touching other art).

They are cut as what stands under a raised floor is, the other way up: a
group of first-layer tiles off the floor is such a piece where the floor
lies straight below its bottom row within 16 pixels and none straight
above it (a pad's corner knob over a walkway behind it is its pad's, and
came along as a scrap until that was asked), 24 tiles at most (an exit's
arrow is larger). It is kept with the panel under it, its neighbourhood and
the panels it draws over.

A layer sets them on free floor panels as it does side pieces, but in
stretches: of the squares of four panels a side, one in three by the
layer's seed takes a piece behind every panel of it that fits, as the maps
set theirs in rows; the rest take none. 0 to 9 a layer in the
[tile test](../../devtools/tile-test.md)'s pictures. Seen in the game: a row
of five behind a catwalk, the horn in front of it as before.

CopyBot Comp's dark crystal spikes stand the same way behind its plateaus'
upper edges, alone and in rows. Its plateaus are raised, so the cut is
made in the views of their heights too (as the pillars' is): 11 looks,
11 set over the atlas's eight layers. A piece behind a floor may go over
the floor's own tiles in its two bottom rows alone, where its foot meets
that floor's edge; anywhere else a tile it would cover is another floor's,
and a spike first stood in front of a plateau's underside, over the void. BN5's
End Area and Oran Area were tried and left: the cut finds 4 looks in each
and none fits a layer, and what End Area's maps set beside its floors is
larger than this rule takes (its pagoda, hundreds of tiles). Oran Area's
are below.

## What Oran Area sets round its floors (none set; measured on the maps and the atlas)

BN5's Oran Area (its maps 0x90:3 and 0x90:2, the atlas's `a4`) sets one
thing of these kinds round its floors: sprigs, a few grey pixels forking
once or twice, standing behind a walkway's upper edge, about twenty in
Oran Area 2 (counted by eye). None can reach a layer:

- Almost all stand behind its brown planks, the floor of another kind its
  layers leave out ([Oran Area's tiles](oran-tiles.md)). A sprig has a tile
  of its own in ten places (its left fork, 14 pixels, 7 looks); the rest
  of it lies in the tiles of the planks' own lip. Cut alone (the behind
  cut taking pieces of one tile, tried) and set behind a generated circuit
  or hub, it fit nowhere over the atlas's six layers: in 26 of 34 tries
  its tile was the generated floor's own rim, the circuits' and hubs' lips
  standing a tile higher than the planks', and the other 8 lay off the tile
  lattice.
- The few behind its green circuits stand behind a raised one, drawn on
  the second tile layer (no ground panel lies under it), and the area's
  layers have no raised floor.
- The behind cut's 4 looks are no thing: two are pieces of a circuit's own
  rim behind a bend, one the knob under the raised warp hub, one an
  escalator's top.

The rest of what its layers never draw (1386 tile pictures of its two
maps, [what the layers take](../../devtools/map-use.md)) is of the kinds
left to the owner (issue #505's 3 to 5) or no side piece: its brown planks,
among them the junction the survey took for a hub's piece (`b23_90_2` at
(544, 336)); its conveyors and escalators; its exit arrows; its warp
centrepieces and raised hubs; and a machine on one of Oran Area 1's hubs,
on its second tile layer over the floor alone, which this page's rule
takes for a centrepiece (whether it blocks the way in BN5 was not checked).

## What BN4's ACDC Area sets round its floors (nothing; measured on the maps and the atlas)

BN4's ACDC Area (its maps 0x90:2, 0x90:0 and 0x90:1, ACDC Area 3, 1 and
2; the atlas's `a15`) sets no thing on, beside, behind or under its
floors. Every cut on this page was run on its three maps (15 November
2026, each switched on for the area in turn and taken back):

- **Its second tile layer** holds one piece in the three maps: a
  half-open green lid on a hub of ACDC Area 2 (0x90:1, at (1014, 301) in
  the atlas's picture), over the hub's floor alone, which this page's rule
  takes for a centrepiece. The side-piece cuts, over a floor's side and
  under its lower edge, take nothing.
- **The cut behind a floor** takes 6 looks, and none is a thing: five are
  the upper halves of dead ends' round discs (5 to 18 tiles), drawn past
  the walls of the panel they end, and one is an exit arrow's head. Set on
  a layer, they put a dead end's rim behind a walkway's panel (0 to 5 a
  layer in the atlas's).
- **Scenery** (`decor.c`): none; nothing in the maps stands clear of the
  floor's art. Nor do the maps raise any floor, so nothing stands under one.

What its layers never draw beside a floor (455 tile pictures of the
three maps, [what the layers take](../../devtools/map-use.md); marked tile by tile on the
maps and looked over) is the floors' own art: the rims and sides of its
round hubs, its cyan roads and their exit arrows, some 130 of them (the
hubs and roads lend no tiles, so their edges count beside the walkways;
the arrows' edges are also all 7 pictures away from a floor); and the
green and white of its walkways: the outsides of their turns and the green filling inside
a bend, the rims of the dead ends' discs, and the rounded ends of the
deeper green skirt under some stretches of walkway. None is a piece to
set; the hubs, roads, arrows and the hub's lid are of the kinds left to
the owner (issue #505's 3 to 5).

## Scenery a face's depth from the floor (Robot Control Comp; verified on the maps and in the game)

Robot Control Comp sets its robot figures, alone and in twos to fours, and
its wires with red lights, alone and in twos and threes, in the void beside
its walkways and pads, on the first tile layer. They touch no floor, and
the scenery cut ([the generator](generator.md), `decor.c`) is theirs: but it
counts 44 pixels under every panel's lower edges as the floor's own, room
for the tall faces and legs of other areas, and Robot's faces are 17
pixels deep. A figure standing closer than that was cut in two, its lower
half refused as touching the floor's art. The area sets its own depth
(`scenery_reach`, 20 pixels: the faces and 3 more), and the cut takes 11
looks from its three maps where it took 3 (a robot and two wires): the
figures alone and in pairs, the wires in their groups. A layer sets 4 to
10 as it sets any scenery. The layers draw 47 to 52% of the three maps'
different tile pictures where they drew 46 to 49% (two seeds a layout). In
the game the figures glow as the maps' do: their colours are the palette
the area's own map cycles.

Left as they are:

- **The bulbs on springs** under its white platforms' lower faces (Comp 2
  and the Pavilion's map). They share their tiles with the face they hang
  from, so they come only with a stretch of it. The cut under a floor
  (`hangs_learn`) at the ground's level takes them so, but also flat
  scraps where the platforms' inner walls ring their cubes, bare face
  segments and a fist, and sets what it cuts under pads alone; a layer's
  pads here are its teal ones, which hang none. A rule of their own would
  set a face-and-bulb stretch under a room's lower edge.
- **Figures and wires touching a walkway or an arm**, behind its upper
  edge or against its face, share a tile with its art and are refused by
  the scenery cut (a tile of the map holds both). The cut behind a floor
  (`behind_learn`) finds 7 looks, robots and wires behind one-wide
  walkways, and none fits a layer over three layers.
- **The fists past its arms' ends** (issue #505: a dead end's art, not a
  piece), the robot's door and its striped conveyor (a set piece), its
  pads' centrepieces and exit arrows (issue #505's kind 4).

## Not built

- **The first tile layer's other pieces.** Robot Control's pipes ending in
  red fists (above), the Judge Tree's planters, BN5's Undernet's dark figures and Nebula Area's dripping pods
  lie on the first layer, beside or under the floor (probed on the maps),
  and Mr. Weather's maps hold their background in the second, so its
  pieces lie on the first too. (The Nest's were counted here as well; its
  crystals lie on the second layer, below.) Scenery refuses a
  piece that touches other art; the rule for these is theirs to get.
- The other kinds the survey found (floor marks, the exit arrows and warp
  discs, stairs) are listed in issue #505.
