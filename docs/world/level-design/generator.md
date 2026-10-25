---
type: Design
title: "The generator"
description: "The layer generator's rules, measured from BN6's maps: a cap on the way's one-wide walkways (`net_way.c`), each area's layouts (`net_layouts.c`), the guardian's arena, walkway joins, tile picking and the floor legalizer (`legal.c`)."
tags: [world, level-design]
generated: { by: claude-code/unrecorded, at: 2026-10-06T13:05:29Z }
---

# The generator

## Navigation

MegaMan walks as in BN6, and always will ([Fidelity](../../game/fidelity.md)): a one-wide
walkway is entered only from its own lane, and a direction held into its
mouth half a panel off that lane stops him at the edge. In Capcom's hand
made maps that costs little; in generated ones, with every room joined by
one-wide walkways and each layer new, a playtester spent a third of his
inputs lining up (sessions 55 to 58). So the maps carry the navigation:
how many mouths a platform has, where they sit, and whether a way through
keeps its lane are the generator's rules, measured from the originals.

Measured on the floor the walls give (14 of BN6's net maps against 58
generated layers; a walkway stepping corner to corner across the screen
counts as one, as MegaMan walks it with one direction):

| | Capcom's | Generated, before |
| --- | --- | --- |
| Mouths per 100 floor panels | 10.0 | 5.1 |
| One-wide walkways that are dead-end spurs, per map | 12.5 | 0.9 |
| One-wide links between the two farthest big platforms (12 panels or more) | 1.2 | 1.9, up to 4 |

Capcom's maps have twice the mouths, but seven in ten of their one-wide
walkways are spurs off the way (pads, comb teeth, stubs), and their way
across runs over wide floor: Central's fields and Seaside's, Sky's rings;
the Undernet crosses one bridge between plateaus, the Graveyard three
ramps. A cap on mouths would be wrong; the rule caps the way:

- **The way crosses few one-wide walkways.** From the arrival to the
  guardian's antechamber or the exit, the way crosses at most its area's
  count of one-wide walkways between big platforms, as its originals do
  (`net_way.c`, `way_cap`): one in Central, Seaside, Sky, Green, the
  Undernet, the Secret Area and most comps and homepages, two in the
  Nest and Robot Control, three in the Graveyard and the Judge Tree's
  catwalks. A pad's spur does not count: it leaves from the middle of
  the pad's side, where MegaMan stands already.
- **Past it, the way widens.** Its walkways become two panels wide,
  the shortest first (the long bridges are an area's own), where the new
  side touches nothing but the walkway and its two platforms and meets
  both whole; the floor draws them in the platforms' floor, a wide way.
  The guardian's own bridge stays one panel wide: his gate. The
  legalizer reads the layer as before.
- Over the tests' 1140 layers, the way's one-wide walkways between big
  platforms fell from 1.65 to 0.86 on average, and those past their
  area's count from 573 to 32 (`test_way_links` holds each area to 90%);
  the tile test's seams fell from 63.0 to 61.7 per hundred panels.
- **Every one-wide walkway on the way counts too** (3 October 2026). The
  cap above counts walkways between big platforms; a playtester lines up
  at every mouth (session 59: three on one of Central's layers). Counted
  the same way on BN6's own maps (`--nav-study`, `src/dev/navstudy.c`:
  the floor their walls give, a panel at a time, and the walkways the
  walk between a map's two farthest panels crosses, of any platforms),
  and on the layers over their way across:

  | Area | BN6's maps | Layers, before | Layers, now |
  | --- | --- | --- | --- |
  | Central | 3.3 (2, 6, 2) | 2.8 | 2.6 |
  | Seaside | 1.0 (0, 3, 0) | 3.8 | 1.8 |
  | Sky | 3.7 | 2.7 | 2.2 |
  | Green | 1.5 (2, 1) | 4.3 | 2.6 |
  | Undernet | 2.3 | 2.7 | 2.3 |
  | Secret Area | 1.0 | 3.3 | 1.8 |
  | Nest (Underground) | 3.0 | 4.2 | 3.7 |
  | Mr. Weather Comp | 0.7 | 2.5 | 1.6 |

  So past its originals' count (`narrow_cap`: Central 3, Seaside 1, Sky
  4, Green 2, the Undernet 2, the Secret Area 1, the Nest 3, Robot
  Control 3, the Aquarium 3, the Judge Tree 4, Mr. Weather 1, CopyBot's
  3), the way's walkways widen as above, before those between big
  platforms (one widened joins its platforms into a bigger one). Central's
  layers cross fewer than BN6's Central maps already: its glass bridges
  are its own. The comps' and homepages' originals are single small
  rooms, nothing to hold a layer to, and the Graveyard's slabs cross
  eight: those keep the cap between big platforms alone. `test_way_links`
  holds each area to its count on eight ways in ten.
- **Rungs meet a field square on** (issue #98). A comb's rung back to a
  ragged field that met its edge on a tab beside a bite was a mouth two
  panels wide on one side, the field going on past it, where the arrow's
  way held walked MegaMan over the lane (`test_arrow_mouths`, a Seaside
  layer); the panels beside a rung's landing are filled, so the edge runs
  straight past it. Seaside's ways past its count fell from 12 of the
  test's 60 to 8.

Still apart from the originals: their dead ends. Capcom's maps have about
twenty-five dead-end panels each (comb teeth on boardwalks, stubs, pads
on spurs), ours about three: the layouts ask for stubs off platform
edges, but a random cell of the window is floor one time in eight, and
few get placed. Placing every stub asked for, off any platform edge, was
tried and set aside: stubs beside rooms took the services' and
landmarks' places and blocked the way's widening, and placed last, two
panels from all that stands, they were too few (2.8 to 3.4 a layer) for
their tiles' cost (the seams 61.7 to 62.4). The originals' dead ends are
mostly teeth on their boardwalks, off the platforms: that is where to
start.

A layer is built from those parts, not from rectangles. Each area has its
own layouts (`src/net/net_layouts.c`), all made of the same pieces
(`src/net/net_shapes.c`); an act's three layers take them in a shuffled
order, so an area does not repeat one while it has others, and each is
built round its signature, the room it is remembered by (Identity,
below).

| Layout | Areas (weight) | Built from |
| --- | --- | --- |
| Route | Seaside 40, Sky 45, Green 45, Secret 25, the comps and homepage 30 | a winding chain of platforms on bridges, pad spurs, stubs, a shortcut |
| Field | Central 35 (around a crater), Seaside 60 (ragged), Comp 40 | one big field, comb boardwalks with teeth on two or three sides, pads |
| Ladder | Green 55 | four or five parallel planks joined by rungs, teeth on the outer ones, grass blocks at the ends; one line of rungs straight across every plank near its middle, the way's crossing, and one or two more a pair at random |
| Hub | Sky 55, Secret 25, Homepage 40 | an octagon centre, four mirrored spokes to pods, a ring between them |
| Slabs | Graveyard 65, Nest 40 | a chain of big slabs with punched holes, long bridges between |
| Web | Graveyard 35, Undernet 45, Secret 50, second Comp 30 | plateaus kept far apart, bridges crossing between them, many stubs |
| Crosses | Undernet 25, Nest 60, Comp 30, Homepage 30, second Comp 40 | plus-shaped platforms grown over a lattice from a big middle block; the lattice runs along the window's diagonal, as the window is narrow across the screen |
| Catwalks | Undernet 30, Aquarium and Judge Tree comps | a maze of 1-wide turns with some walls knocked through, plazas at its ends |
| Trail | Central 35 | after Central 1: a path two or three panels wide winding down the window in legs along x and y by turns (each leg's length steered back towards the middle), a bulge a panel wider here and there, pads hung off its sides on short catwalks; a room on every second leg, never raised |
| Comb | Central 30 | after Central 2: five 1-wide catwalks three apart, long and short in turn, hung off a 2-deep walkway; pads on the short ones, a rung closing one loop, the middle one leading to a plaza, the arrival behind the walkway's end |

Every layout an area draws must build as planned on most seeds (`build.py
test` checks 40): one that does not fit the window falls back to another
layout each time, and its share of the area is silently lost. Crosses did
so everywhere until its lattice was turned to the window.

MegaMan arrives on the pad nearest the layer's side of the screen, facing
along the way on ([Arrivals](arrivals.md)); the exit is the room farthest from it
by walking, past the layer's signature where rooms lie beyond it
(Identity). On a guardian's layer the exit is an
arena instead (`src/net/net_arena.c`): a 5x5 platform at the end of a single
bridge of 3-5 panels, attached where it lies farthest from the arrival
(past the signature too),
drawn in the area's second floor so it reads apart from the platforms.
Pads (the 3x3 rooms on spurs) are drawn with tiles learned on the
original's own pads, where those are square (not the Judge Tree's round
stumps or Sky's round pods; Robot Control's are its other maps' raised
white platforms), and the originals' free-standing scenery is
set in the void beside the floor (`src/map/decor.c`). The
guardian holds its middle and the exit waits on its far side; the room the
bridge leaves from gets the heal and the Net Dealer ([Guardians and super bosses](../../game/bosses/overview.md)). Services go to the bigger platforms, the
better Mystery Data to pads, and most other Mystery Data to dead ends. A
layer has 120 to about 250 panels (depth grows the layouts) and fits a
window of 29 x 53 panels along the grid's diagonals, the screen rectangle
the game's tile map buffer holds (0x13FF4 bytes, up to the coordinate
data); `netmap.c` centres it on that rectangle.

Walkways, the floor panels in no 2x2 block of floor, are drawn in the
area's second floor, learned from the same source map by its hue
(`walk_styles` in `src/core/rom.c`): Central's blue catwalks, Seaside's
yellow boardwalks, Sky's cyan glass catwalks (not its lavender ones, whose
pieces turned up in the cyan ones' bends; its fields of framed squares share
their hue, so there the hue's panels in 2 x 2 blocks are platform floor and
those a panel wide the catwalks, `TILES_WALK_NARROW`), Green's orange planks, Graveyard's
purple bridges and the Undernet's red striped bridges, which turn and meet
the plateaus in joints with a yellow gem. The tile classes
tell platform, walkway and void apart, so the joins between the two floors
come from the places the original maps join them. The Nest's source map
(Underground 2) has no second floor.

So the generator joins them as the originals do (`src/net/net_layouts.c`):
a bridge between two rooms runs straight across where their sides face each
other, else leaves one square on and bends once into the other's near side,
else jogs halfway; either way it meets both in the middle of a side, where
floor lies on both sides of it. Spurs and stubs leave the floor square on
too. An L that ran along a platform's edge into its corner was a join no
map draws, and its tiles came out as wedges of one floor cut into the
other. In the comps and homepages (`TILES_CROSSING`), a walkway entering a
platform square on goes on straight in its own floor for as long as floor
lies on both sides of it (`netmap.c`, after the floor is made drawable),
out the far side only into another walkway, else a panel short of the edge:
a stripe ending on the rim was another join no map draws. The comp's pads,
which its map has none of, are crossed as well; pads with a look of their
own keep it.

Each tile is also held to what it shows: per pixel, which panel's top or
side face it draws and in which floor. A pair costs a point for every pixel
where its own neighbourhood would have shown otherwise, beside the
neighbourhood's distance, so of the pairs near enough the one drawing the
right floors wins (most tiles drawn with a wrong floor or edge had a right
one passed over for a nearer neighbourhood). The plain look inside the
floor may come in another shade of the same colours: the Green HP draws its
walkways a shade darker than its platforms, in tiles of their own, and
every walkway tile had failed the test. Still, a pair showing what the
tile shows beats a plain-looking one that shows other floors: where a
bridge plugs into a Graveyard slab, the originals' own join lightens the
slab around it, and the plain slab's tile left a notch there.

Some areas never set one floor flush with another. Green's planks reach
its raised grass by stairs, CopyBot's walkways reach its plateaus, which
stand on pods, by ladders, and the Aquarium's glass pads stand on legs
above its water, reached by stairs, with glass walkways ending at their
rims. There each of those pieces (Green's and CopyBot's platforms, CopyBot's
red pads among them, and the Aquarium's pads) and the rest of the floor
are picked apart, each as if the others were not there, and laid over each
other on the game's two tile layers (`net_area.apart`, `src/map/tilemap.c`).
Where two draw the same tile, the one whose top covers the other's side
face goes on the second layer, which the game shows in front: a platform
keeps its whole rim, a walkway coming from below on screen ends over its
side face and one coming from above ends behind its rim. The Aquarium's
guardian arena, wider than its water ever gets, is a pool as well.

The originals draw on their second tile layer what stands in front of a
floor: Seaside's boardwalks and Central's bridges where they cross a
platform, Green's and Central's pad ornaments, CopyBot's spikes and claws,
the corners of platforms that overlap others on screen. A tile keeps its
first layer alone where that draws the floor, and its second too only where
the floor needs it (the Judge Tree draws its panels' middles there), so
none of these comes along in pieces. Surfaces that share a floor's colours
but not its look are left out of its tiles, still counting as floor around
them: the Judge Tree's flat red courts (`skip_styles`).

An area's other maps (`more`), at each of their heights, fill in the
neighbourhoods its own map never shows, in the same tiles and palette. A
tile of theirs that draws a colour the own map never shows on its floors
(at any height) belongs to another surface and is left out
(`tiles_other_colours`): Central Area 2's raised plateau is yellow-green
where Central's fields are green, and its corners fitted the inner corners
where two of Central's floors meet, which Central Area 1 never shows,
better than any green tile; so did an Undernet map's raised court at
bridge ends, and stray pieces of other maps at the joins of Sky, Robot
Control Comp and Mr. Weather Comp. Their neighbourhoods still count as
seen (legal.c), so floors do not change: the nearest tile in the area's
own colours draws them. Seaside keeps its other maps' colours
(`TILES_MORE_COLOURS`): its second floor forms fields only in Seaside 2
and 3, as their yellow panels, and its guardian's arena is drawn in them;
elsewhere their pieces cost more than its own (outside a field of its
second floor they had drawn yellow panels' scraps at boardwalk joints).

Well inside the floor, a tile must look like the area's plain floor, from
about 5 pixels inside its edge. Robot Control Comp's white platforms are
framed by two light bands half a panel deep (16 pixels under their upper
edges), so its own tiles for an edge failed that test wherever they
reached that far in, and plain white or a pad's piece stood in: the
platforms read as flat white fields with ragged edges. Its platforms'
edges, like CopyBot's rims, are not held to the plain look
(`TILES_RIMMED`). So are the Graveyard's, whose slabs' rims carry a clip
at every join of two panels, and Mr. Weather Comp's: their own edge tiles
failed the test, and full panels stood in, which poked out past the rim
in a sawtooth (the Graveyard, its holes slots with straight sides) or
hung stepped blocks under the field's edge (Weather). The walls inside Comp 2's platforms ring the grey cubes
that stand on them, not holes; learned as holes, the panels beside them
taught plain white next to the void, which the inner corners of generated
platforms took. The tiles are learned with those panels as floor
(`TILES_INNER_WALLS`), while the neighbourhoods legal.c asks after keep
them as holes, so floors do not change. Comp 2's one small platform is the
striped conveyor before the robot's door, which drew the pads half striped:
the pads take the look of Comp 1's and the Pavilion's raised white
platforms alone (`TILES_MORE_PADS`).

Well inside a platform (floor all round), a tile is taken from the
original's own field as it lays it, not by its neighbourhood, which is the
same everywhere there: by the field's period, the smallest (up to 6 x 6
panels) that explains 95% of its middle (Seaside's 2 x 2, the Nest's
cracked 5 x 2), else by the largest stretch of its middle (up to 8 x 8
panels) laid whole and repeated. Mr. Weather Comp's solar panels carry
lights that cross the tiles' edges at random: picked tile by tile, its
fields came out as plain bands the length of the field, their lights gone.
Its fan belts, which run across its fields from edge to edge, and the
clouds lying on them lend no tiles (`SKIP_PALE`: panels whose tops are
pale in places, but for those beside a walkway, which its joins need):
a field cuts them anywhere, and cut, they stood as half domes at the
ends of its bands.

Before its tiles are picked, a layer's floor is made drawable
(`src/map/legal.c`): every panel whose 3x3 neighbourhood of platform,
walkway and void no original map shows is a place the tiles can only
approximate, so single floor cells are added into notches or taken away,
and walkways' bends and branches are widened into a 2x2 platform (the
originals join their walkways on platforms), where that leaves fewer such
panels around. An edit may not join, split or enclose anything (a
simple-point test, and for the 2x2 blocks a count of the floor's and
void's pieces), nor touch objects and the eight panels around each (a cell
changed beside a service could make it a walkway's mouth, where the
generator had kept it off one), rooms' anchors, pads, the arena, stairs
and raised floors; straight walkways keep their width. The originals'
neighbourhoods are recorded while their tiles are learned. It removes about
a third of the approximated tiles; mazes of 1-wide walkways (Central's
catwalks, the Undernet's webs) have shapes no original has and stay
approximate.
