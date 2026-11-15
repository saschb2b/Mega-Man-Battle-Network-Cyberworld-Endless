---
type: Design
title: "Side pieces"
description: "What an original map hangs on its floor's sides (CopyBot Comp's claws, the Undernets' flames, the Aquarium's pillars), cut whole from its second tile layer with the panel it grips (`src/map/sides.c`) and set on a generated floor's panel of the same neighbourhood; the survey of what layers still miss (issue #505)."
tags: [world, level-design]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-15T19:30:00Z }
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
| BN4's countries | nothing; its one thing off the floor, two sockets on platforms' rims joined by a cable through the void, is a set piece no cut takes (below) | no |
| The Undernet (Undernet 1-3) | its fire bowls beside floors and bridges, alone, in twos and threes: 9 looks, and Undernet Zero's 4 | yes |
| Undernet Zero | its fire bowls in threes, 4 looks, and the Undernet's 9 | yes |
| BN5's SciLab Area | nothing; its wire tufts behind its circuits' upper edges are drawn into a rim no layer draws (below) | no |
| BN5's ACDC Area | nothing; its wire tufts behind its blue walkways' upper edges are drawn into a rim no layer draws, its layers laying no blue walkway (below) | no |
| The Nest (Underground 1-2) | its crystals and their rubble beside its rims, on the second layer: behind upper edges and under lower ones (below), 17 looks; its lone spires fit no layer | yes |
| Central Area | on its second layer the Net Dealer's capsule counter, which means a shop; behind its floors one look, the overpass's shadow on a field; nothing else of these kinds (below) | no |
| Aquarium Comp | one piece on the second layer, nothing to see; under its pads, the pillars (below) | the pillars |
| BN5's End Area | no side piece (below, End Area's survey); in a walled hole, its pagoda, the layer's landmark (below) | the pagoda |
| BN5's Oran Area | behind its floors, 4 looks that are no thing and its sprigs, which fit no layer (below) | no |
| BN4's ACDC Area | nothing: on its second layer a lid on a hub alone; behind its floors, 6 looks of floor art (dead ends' discs, an arrow's head); no scenery (below) | no |
| Robot Control Comp | nothing on its second layer; its robot figures and its wires with red lights are scenery, cut against its own faces (below); the bulbs on springs under its white platforms and the fists past its arms' ends are not placed (below) | the scenery |
| Seaside Area | none: its maps set no thing on, beside, under or behind their floors; the one look a cut takes is an arrow field's rim (below) | - |
| BN4's Park Area | nothing: its second layer holds its two overpasses with their stairs and its Navi's hub's centrepiece; behind its floors, 21 groups that are no thing (below) | - |
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
and none fits a layer. End Area's are the pins along its roads' upper rims
and a raised warp pad (below, End Area's survey); Oran Area's are below.

## What stands in a walled hole (`hole_learn`; verified on the maps and in the game)

End Area 2 (0x90:8) stands a pagoda on one of its grey pads: an orange
plinth under a purple roof of three tiers, 163 second-layer tiles, 16 by
17. It is no side piece, too big for one and gripping no rim: its plinth
fills a block of 2 by 2 panels the map's walls leave out of the floor (a
hole, MegaMan walks round it), ground floor all round the block, and its
roof reaches back over the pad's floor behind it and over the void past
the pad's back corner. The map's layer priorities (92 cells round it) put
MegaMan behind the roof there. It is the map's one landmark, and none of
the other two maps has one.

A second-layer group of 400 tiles at most is such a piece where the panels
it covers half of or more that are no floor and lie inside the floor
(floor within three panels of each, along world X and Y both ways: the
roof over the void past a rim is not) make a block of three a side at
most with ground floor all round it. It is kept with that block's lowest
panel, the block's size, the panels it draws over (floor or none) and the
map's layer priorities over those and one round them: one piece an area
(`Learned.hole`).

A layer stands it as its landmark (`ng_landmark_hole`, where the area sets
no landmark of BN6's): in the best room that holds it, the signature
first, then big rooms far from the way, not the arrival's, the exit's, the
arena or a pad. Its hole lies in the room, off the way, with floor all
round it, cutting nothing off; every cell its roof hangs over that is void
in its map is void on the layer (a roof over floor where its map has
none would show MegaMan standing on it), and a cell over floor is floor
or void. So it stands in a room's back corner, as in its map. The hole is
walled off (`C_SOLID`, drawn as the floor round it) and the cells it draws
over are kept clear of objects and of set pieces' floor (`ng_reserved`).
Its tiles go on the second layer whole, its priorities with the side
pieces' (`nm_sides_extra`). It stands on about one End Area layer in
six: the first layer of 7 runs in 40 (seeds 1 to 40, `--net-biome x2`),
2 of the atlas's 16 (four seeds a layout); the way through the rooms'
middles keeps it off most of the rest.

A Glitch layer's flip swaps tiles and colours alone: a flip into End
Area draws no pagoda (its hole and priorities are the layer's own), and a
flip out of one shows its hole as the other area's floor.

## End Area's survey (15 November 2026)

What End Area's three maps (0x90:7-9) set on and round their floors that
no layer drew, from the atlas's tinted maps (`build.py atlas a2 2`,
`lent_b21_*`) grouped tile by tile:

| What | Where it stands | Size, how often | Taken |
| --- | --- | --- | --- |
| The pagoda | in a walled hole of a pad, End Area 2 | 163 tiles, once | yes, above |
| The raised highway, its ramps and the posts under it | second layer, over the void and the roads | 1,200 to 2,000 tiles a map, one group | no: a road, issue #505's kind 5 |
| Raised pads, the diamond one with a warp disc | raised floor (height 32) | 25 to 48 tiles each, one or two a map | no: raised floor and a warp's centrepiece |
| Its 3x3 pads' centrepieces: a green warp disc, a purple one, a red console block | second layer, on a pad's middle alone | 24 to 32 tiles, one a map | no: pads' centrepieces, kind 4 |
| Exit arrows | at the maps' edges | about 100 tiles each, End Area 2's two | no: kind 4 |
| Arrow panels | on the roads | 15 or 16 tiles, three to eight a map | no: floor of another kind |
| Pins along the roads' upper rims | first layer, in the rim's own tiles and the row above | 2 or 3 pixels each, dozens a map | drawn already with the rims; the variants left are floor tiles |
| The rings' lower faces, road ends, bends | first layer, the floor's own | 4 to 12 tiles | floor tiles the pick does not choose, no piece |

The cut behind a floor (`behind_learn`) finds 4 looks: three rows of pins,
11 to 13 tiles, and the raised warp pad seen at the ground's level. Each
fits a layer's panels by its neighbourhood, but on every one the layer's
floor already holds all of its tiles (13 of 13, probed): the pins are
drawn in the rim's tiles, which the floor's own art lays.

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

## What Seaside Area sets round its floors (nothing; measured on the maps)

Seaside Area's three maps (0x91:0 to 2, the atlas's `1`) set no thing
round their floors: nothing in the sea, nothing gripping a rim, nothing
under a boardwalk or behind an edge. Every rule this page has was run on
each map, its two tile layers drawn apart (15 November 2026): the
side-piece cut, with and without what stands under a lower edge, the cuts
under and behind a floor at the ground and at Seaside 1's raised floor
(64), and the scenery cut with the floor's faces taken 0, 22 and 44 pixels
deep. The scenery cut and the cuts at the raised floor take nothing. The
side-piece cut takes one look, a 50-tile stretch of the rim of Seaside 3's
arrow field where it meets the yellow field, which goes with that field.
The cut under a floor, run at the ground, takes 8 groups of the first
layer in Seaside 2 and 3: two exit arrows, and yellow and blue field
panels lying under the second layer's arrow fields and yellow field. None
is a thing.

What the layers never draw (951 tile pictures, [what the layers take](../../devtools/map-use.md))
is floor and the kinds left to the owner (issue #505's 3 to 5):

- **Floor art.** The yellow fields' and boardwalks' rims, faces and turns'
  outsides in the variants no layer's floor happens to need: the small
  yellow and blue lamps along the fields' faces, the posts on the
  boardwalks, and the boardwalks' cleats, a pair of orange (Seaside 1) or
  green (Seaside 3) brackets some 3 by 5 pixels, every panel or two along
  a boardwalk's front face. A cleat shares its 8 by 8 tiles with the face
  round it, so it is no piece to cut; the layers draw some with the faces
  they lay. The blue field's panels Seaside 1's boardwalks cross over (on
  the second layer) are drawn part covered there, the same.
- **Floor of another kind.** Seaside 2's and 3's arrow fields, the
  currents (150 and 250 panels, a floor of their own,
  [Arrow lanes](arrow-lanes.md)); the boardwalks of Seaside 2 and 3 in their
  own colours.
- **Raised floor and its stairs.** Seaside 1's two framed platforms, 64
  above the field, each reached by a ramp of striped steps (kind 5).
- **What marks a warp or an exit.** The round plate with a gold cube on
  Seaside 2's blue field, the violet ring on Seaside 3's framed pad, and
  the exit arrows, two in Seaside 1 and 2 and one in Seaside 3 (kind 4).

Seaside's geysers and cyclones are the Link Navi obstacles a layer already
sets as objects ([Link Navi obstacles](link-navi-obstacles.md)), not tiles
of its maps.

## What Park Area sets round its floors (none; measured on the map and the atlas)

BN4's Park Area (its one map 0x92:0, the atlas's `a17`) sets nothing on,
beside, behind or under its floors that is not floor or a set piece. Its
layers draw 39% of the map's 837 different tile pictures (two seeds, six
layers); what they leave, looked at on the tinted map and cut by each rule
in turn (the cuts switched on for the area and every group they weighed
listed, 15 November 2026):

- **Its second tile layer** holds three groups: the two overpasses with
  the stairs up to them (553 and 665 tiles, past the 160 a side piece may
  have; heights and stairs, issue #505's kind 5) and the centrepiece of
  its Navi's round hub (21 tiles, on the floor alone: a centrepiece,
  kind 4). None hangs off a floor's side.
- **Under a floor** there is nothing to cut: the map's only floor above
  the ground is its overpasses', and no view at their height has enough
  cells to be learned.
- **Behind its floors** the cut weighs 21 groups of the first layer and
  takes none, and none is a thing: the far halves of its dead ends' round
  discs (four, 17 and 18 tiles), its walkways' S-bends under the left
  overpass (five, on floor at the overpass's height), a road's rounded end
  and a stair's foot (7 and 6 tiles), and its rims, roads, exit arrows and
  the platform's frame (24 to 759 tiles, larger than the cut takes: 24
  tiles, under 20 wide).
- **The scenery cut** takes nothing: every group off the floor touches
  other art.

The rest is floor art or the owner's kinds: its orange and yellow roads'
rims, bends and stripes ([Roads on the way](roads.md) draws the roads
themselves), the outer rims of its walkways' turns and forks, a walkway
broken off where it nears the platform (at (792, 456)), its two round
hubs, its exit arrows, and the overpasses and their stairs.

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

## What BN4's countries set round their floors (none set; measured on the map and the atlas)

BN4's countries (their area's map 0x93:2, the one of the four looks it
learns; the atlas's `a18`) set one thing off their floor: a pair of
sockets, cyan half-discs in a green rim, joined by a blue cable. One
stands in front of a platform's lower edge (map pixels (800, 150) to
(870, 200)), the other behind another platform's upper edge ((985, 335)
to (1055, 375)), and the cable between them runs through the void along
the panels' two ways, 33 tiles one way and 6 the other, turning at
(1105, 318); along each run it is the same five tile pictures a row,
two tiles further on each row. The map holds that one pair. No cut
takes it (each cut switched on in the area's row, `.sides = 1 | 4`, and
what it refused printed; reverted):

- The cut behind a floor (first layer): the sockets and the cable touch
  the platforms' faces, and with them make one group of 1,695 tiles, far
  past its 24. Its other groups (2 to 45 tiles) are corners of faces with
  no floor straight below.
- The side cut (second layer): the map's second layer holds only its
  orange arrow conveyors (groups of 24 to 420 tiles: too large, over floor
  of another kind, or lying on the floor alone) and the green shading
  along some walkways' sides (7 to 69 tiles, over floor of another kind or
  not off the floor). It holds nothing that hangs off a floor.
- Scenery: none (the learn report's "0 pieces of scenery"). The sockets
  touch the rims, and the cable alone is 33 tiles across, past scenery's
  20.
- What stands under a raised floor: the map's floor is all at one height
  where it is not a stairway.

A layer could only set it as a set piece: two platforms whose edges face
each other across the void, a socket on each, and the cable drawn along
the void panels between them from its runs and its turn. That is a rule
of its own for one pair in one map; not built.

The rest of what its layers never draw is no side piece: the two exit
arrows, the pink warp hub, the arrow conveyors, the stairway's steps,
the cyan joints where a walkway meets a platform's rim, and the rims'
pieces at walkways' turns (floor art; issue #505's kinds 3 to 5).

## What SciLab Area sets round its floors (none set; measured on the maps and the atlas)

BN5's SciLab Area (its maps 0x90:5, 0x90:4 and 0x90:6, the atlas's `a1`)
sets one thing of these kinds round its floors: wire tufts, a cyan fork of
a few pixels standing behind a green circuit's upper edge, 18, 16 and 20 in
its three maps. They lie on the first tile layer, and a tuft is three tiles
in a row: one of its own (tile 0x014, mirrored 0x414), and one or two of
the circuit's rim with the fork drawn into them (0x015 and 0x016, whose
plain rims are 0x002 and 0x001: the same pixels but the fork's). None can
reach a layer:

- The cut behind a floor takes nothing in SciLab. A tuft's own tile is one
  tile where the cut takes 4 at least, and the area's circuits run a panel
  or two apart, so another floor stands within the cut's 48 pixels above
  most of them.
- Cut whole with its rim's tiles, each kept with the plain rim it goes over
  and set only where a layer draws that rim (tried: 17 looks from the three
  maps), it fit nowhere on the atlas's layers. In 42 tries over its three
  layouts the layer drew a cyan lip there (tile 0x03C in 28, 0x019, 0x0D1
  and 0x0AB in the rest), never the green edge: a layer's circuits are
  wide, edged in its hubs' lip, and none of the six layers of two seeds
  draws the green edge's tiles 0x001 and 0x002 at all, which the maps draw
  along every long circuit's upper side. They are among the tiles beside the floor that
  [what the layers take](../../devtools/map-use.md) counts as never drawn.
- Set over the lip anyway, a tuft's rim tile would cut the lip off; set
  alone, its own tile is the fork's two smallest prongs.

So the tufts come with the green edge: where a layer draws its circuits'
upper sides as the maps do, a cut of the tuft with its rim's tiles can set
them. That edge is floor art, the tiles' matter, not a piece's.

The rest of what its layers never draw is no side piece: the green edges
and the rounded rims where its circuits cross (floor art); its orange exit
roads, their dark faces and the arrows at their ends; the warp discs on two
of its hubs; and the orange-striped blocks standing on 0x90:6's circuits,
on the floor alone (issue #505's kinds 3 to 5).

## What Central Area sets round its floors (none set; measured on the maps and the atlas)

Central Area's two maps (0x90:0 and 0x90:1, the atlas's `0`) set nothing
on, beside, behind or under their floors that a layer can carry. Its
layers draw 65% and 48% of the maps' different tile pictures (two seeds a
layout), and what is left is floor or one of the kinds left to the owner:

- **Its second layer** holds the Net Dealer's capsule counter (Central
  Area 2), its pads' emblems and warp discs, a bench and one arm of a rail
  on Central Area 1's yellow plaza, and all of Central Area 2's overpass.
  Of these the side-piece cut takes the capsule alone (24 tiles: it grips
  its floor and hangs off it). Switched on for a probe, it set the capsule
  bare, no dealer behind it, on 7 of the atlas's 8 layers: a counter that
  means a shop, so the area stays out.
- **Behind its floors** the cut finds one look: a field's panel drawn
  darker where the overpass's shadow falls on it (Central Area 2), the
  field's own art. No layer set it.
- **Under its raised floors** nothing: the overpass and its raised field
  stand on no legs, and no free-standing piece is left for the scenery cut
  (none learned).
- **The plaza** by Central Area 1's exit to Central Area 2 is floor of
  another kind (yellow, sunken in its middle). Its rail, an L along its
  back edges (one arm on each tile layer), and its bench on a raised slab
  stand on it alone and go with it.
- **The rest is floor art**: where a one-wide catwalk meets a pad's rim
  (its lip with a pink stripe), a catwalk's end caps (a tile each), and
  the catwalks drawn again under the overpass.

The overpass (a raised walkway over the catwalks, its raised field and two
raised pads), the plaza, the warp discs and the two exit arrows are issue
#505's kinds 3 to 5, left to the owner; the overpass also crosses over
other floor, which no layer does ([what the layers take](../../devtools/map-use.md)).

## What BN5's ACDC Area sets round its floors (none set; measured on the maps and the atlas)

BN5's ACDC Area (its maps 0x90:0 and 0x90:1, ACDC Area 1 and 2, the
atlas's `a0`) sets one thing of these kinds round its floors: wire tufts,
a teal fork of a few pixels standing behind a blue walkway's upper edge,
some twenty in ACDC Area 1 and twice that in Area 2 (counted by eye). None
stands behind its green walkways or its cyan platforms. A tuft lies on the
first tile layer, as SciLab's do: two tiles of its own over the void, and
one or two of the walkway's rim with the fork drawn into them. None can
reach a layer:

- **Cut alone**, a tuft's own tiles are two where the cut behind a floor
  takes 4 at least; the cut weighs 12 groups in the two maps and keeps
  none: eight are entries that draw no pixel, four are the side faces
  under a walkway's lower edge (7 to 18 tiles).
- **Cut with its rim's tiles**, it could go down only where a layer draws
  the same plain rim, as SciLab's test asked. No tile a tuft is drawn into
  is such a rim: of the 377 tile pictures the atlas's eight layers (two
  seeds a layout) never draw in the two maps, 8 are a picture some layer
  draws with up to 20 pixels added over its empty ones, and all 8 are
  joins of a platform's rim to a walkway or an exit's end, none a tuft
  (compared pixel by pixel on the atlas's pictures). The layers lay the
  area's walkways in its green circuits' tiles, and a blue walkway's upper
  rim is tinted in both maps' pictures wherever a tuft stands.
- Set over a platform's or a green walkway's edge instead, a tuft would
  stand where neither map sets one.

So the tufts come with the blue walkway's own upper edge, which is the
floor's tiles' matter, not a piece's.

The rest of what its layers never draw (86 tile pictures beside the floor,
5 away from it, [what the layers take](../../devtools/map-use.md)) is no
side piece:

- **Floor art**: the blue walkways' upper rims and their comb and zigzag
  turns (`b19_90_1` at (600, 80)), and the crest where a green walkway
  bends (`b19_90_0` at (472, 96)).
- **On the floor alone**, its second layer's every group: the warp pads'
  centrepieces (three in each map; `b19_90_0` at (456, 192)), and in ACDC
  Area 2 a purple machine on a platform (`b19_90_1` at (760, 192), 24
  tiles), which the cut takes for a centrepiece (whether it blocks the way
  in BN5 was not checked).
- **The exit arrows** at the maps' edges (`b19_90_0` at (72, 200),
  `b19_90_1` at (272, 96) and (1088, 408)).

The area has no raised floor to cut under, and the scenery cut sets none
on its layers. The centrepieces, the machine and the arrows are issue
#505's kind 4, left to the owner.

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
