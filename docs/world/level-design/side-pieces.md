---
type: Design
title: "Side pieces"
description: "What an original map hangs on its floor's sides (CopyBot Comp's claws, the Undernets' flames, the Aquarium's pillars), cut whole from its second tile layer with the panel it grips (`src/map/sides.c`) and set on a generated floor's panel of the same neighbourhood; the survey of what layers still miss (issue #505)."
tags: [world, level-design]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-13T23:03:31Z }
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
| Central Area | the Net Dealer's capsule counter, which means a shop | no |
| Aquarium Comp | one piece on the second layer, nothing to see; under its pads, the pillars (below) | the pillars |
| Judge Tree Comp | nothing on the second layer; on its walkways' edges, in the floor's own tiles, the tufts on its back rims and the planters stacked at its walls' feet, 139 looks over its four maps (below). Not placed: its stumps with their tree houses, doors, bridges and roots, and its tree's red court with the tree and the terminal on it, floor of another kind (the stumps its pads' look, the court skipped on purpose, `skip_styles`; issue #505's kinds 3-5) | what grows on the edges |
| every other area | nothing | - |

## What grows on a floor's edge (`edges_learn`; verified on the maps and in the game)

The Judge Tree's walkways carry two things the rules above cannot cut, as
its four maps were looked over beside the tile test's tinted copies (its
unused tiles in red; 15 November 2026). Both lie on the first layer in the
very tiles that draw the floor's rim and wall, so no group of tiles off
the floor holds them:

- **tufts of grass** on the back rims, a little mound with green blades
  rising 4 to 6 pixels over the rim, at the back corners and along the
  back edges, some 30 to 40 a map (2 to 3 tiles each, the rim's tiles
  among them);
- **planters stacked at the walls' feet**, two to four brick boxes of
  green, the upper on the wall, the lower hanging below its foot, some 9
  to 12 stacks a map, and lone planters whose box pokes below the wall.

The planters on the walls themselves are the floor's own art and were
already drawn.

A first-layer tile grows on the floor where it draws 4 pixels or more past
the floor's body: off every ground panel's top, more than 3 pixels over
it (a plain back rim's outline stands 2 over its top, the tufts' blades 4
to 6) and more than the walls' height under it (25 pixels on the Judge
Tree's maps, the book's `face`). Tiles that do, joined eight ways, are one
piece, 24 at most, cut whole with the ring of tiles round it that draw the
floor's art, 8 pixels of it or more (the mound under a tuft's blades, a
planter's top on the wall; a planter's corner that only lies where a wall
would is no wall), 48 at most. It is a tuft where more of its floor's art is rim than wall,
gripping the panel straight under it; else a planter, gripping the panel
straight over it. With it go the panels of its neighbourhood, as a side
piece's, and every panel whose top, rim or wall could draw in its tiles
(the walls of the panels over it, the rims of those under it), 32 at most,
each as ground floor or none: a wall of a walkway further back reaching
down to a tuft cut a notch in it on a layer until that was asked. A piece
is kept only where its panel and every floor round it is the area's own
walkway floor (`tiles_src_panels`), so nothing is cut from its stumps or
its court. Each look is kept once: 74 tufts and 65 planters over the four
maps, which is more than the book's 24 its other kinds keep, so a book
grows as looks come (an area without any holds none).

A layer sets one on two in three of its free ground panels of the same
neighbourhood, by its seed (on every one, a long straight edge took a tuft
every other panel), where every panel it reaches is what it is in its map,
none a panel from another; of the looks that fit there, the first from the
one the seed picks whose tiles go down cleanly. Its tiles that drew the
floor's art too go over the layer's own there, which is the same art (the
same panels round it), the rest over nothing, and each tile at its edge
must meet the layer's tile beside it as the area's maps set those two side
by side (`seams_seen`): a planter stack's top beside a planter the layer's
wall already shows cut that one in two, 31 more breaks on a layer of the
[tile test](../../devtools/tile-test.md), until that was asked. Its layers
show the same breaks with them as without, and 12 to 17 a layer, where its
maps carry some 45 on as many panels: mostly tufts and lone planters, the
stacks where the wall beside them is plain.

Nothing about walking changes: its tiles are the first layer's, and the
panels under them are what they were. MegaMan stands in front of them as
he does of the floor's rim and walls (captured on a layer: a tuft's blades
show behind his helmet as he stands on the rim's panel); none stands where
he could pass behind it, a tuft behind a rim over the void and a planter
under a wall. They are set in `pick_map`'s pastes from the area's
`Learned` alone, so a [Glitch layer](../../game/glitch-layer.md)'s flip into
the Judge Tree draws them too; they need no layer priorities.

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
and none fits a layer, and what their maps set beside their floors is
larger than this rule takes (End Area's pagoda, Oran Area's hubs, hundreds
of tiles).

## Not built

- **The first tile layer's other pieces.** Robot Control's pipes ending in
  red fists, BN5's Undernet's dark figures and Nebula Area's dripping pods
  lie on the first layer, beside or under the floor (probed on the maps),
  and the Nest's and Mr. Weather's maps hold their background in the
  second, so theirs lie there too. Scenery refuses a
  piece that touches other art; the rule for these is theirs to get.
- The other kinds the survey found (floor marks, the exit arrows and warp
  discs, stairs) are listed in issue #505.
