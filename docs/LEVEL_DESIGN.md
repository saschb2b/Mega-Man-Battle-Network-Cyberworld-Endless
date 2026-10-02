# Level design

How BN6's own net areas are built, measured from the ROM, and how the layer
generator (`src/net/`) follows them.

## Method

Every internet map of groups 0x90-0x96 was decoded from the player's ROM:
its floor panels (a panel is floor when the front tile layer draws its centre
at the height coordinate section 1 gives there) and a render of the whole map.
The lab scripts are `panels.py` (grids), `stats.py` (the numbers below) and
`render_levels.py` (pictures), kept outside the repository with the other ROM
tools.

| Map | Panels | Box | 1-wide | Dead ends | Loops | Longest bridge | Platforms (panels) |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Central 1 | 213 | 28x20 | 21% | 4 | 0 | 5 | 83, 32, six 3x3 pads |
| Central 2 | 201 | 32x24 | 48% | 24 | 1 | 12 | 42, 40, 9, 6, 4, 4 |
| Central 3 | 277 | 24x27 | 10% | 8 | 0 | 6 | 225 (with a crater), 12, 9, 4 |
| Seaside 1 | 238 | 24x22 | 13% | 10 | 0 | 5 | 190, two pads |
| Seaside 2 | 390 | 26x27 | 14% | 34 | 1 | 5 | 316, two pads |
| Seaside 3 | 429 | 29x35 | 11% | 10 | 4 | 8 | 363, two pads |
| Green 1 | 401 | 30x34 | 38% | 26 | 4 | 14 | 240, pads |
| Green 2 | 384 | 35x36 | 60% | 53 | 2 | 13 | 62, 36, six pads |
| Underground 1 | 287 | 31x30 | 47% | 37 | 6 | 8 | ten, 9-34 each |
| Underground 2 | 351 | 27x37 | 28% | 33 | 3 | 11 | fifteen, 10-65 each |
| Sky 1 | 307 | 32x28 | 18% | 38 | 0 | 5 | eleven, 8-74 each |
| Sky 2 | 335 | 30x33 | 13% | 14 | 4 | 5 | 149, 65, 37, 16 |
| Sky 3 | 394 | 33x24 | 26% | 15 | 5 | 7 | 168, 36, 16, 16, 16 |
| Undernet 1 | 417 | 37x34 | 39% | 30 | 8 | 13 | twelve, 9-72 each |
| Undernet 2 | 498 | 32x41 | 28% | 28 | 0 | 8 | eighteen, 14-96 each |
| Undernet 3 | 272 | 35x29 | 43% | 34 | 3 | 13 | 67, 58, 20, 10 |
| Graveyard | 509 | 29x34 | 10% | 6 | 10 | 11 | 232, 57, 40, 40, 33, 30 |

(1-wide: panels in no 2x2 block of floor. Loops: holes in the floor, not
counting the 2x2 blocks. Platforms: groups of panels that are in a 2x2
block.)

The generator this replaces built 6-14 rectangles of 3-5 panels joined by
L-shaped walkways: about 100 panels in a 14x16 box, every area alike.

## What the originals do

- **Two materials.** Each area pairs a floor for its platforms with a second
  one for its walkways: Central green fields and blue catwalks, Seaside blue
  sea floor and yellow boardwalks, Green grass and orange planks, Undernet
  pink plateaus and red bridges, Graveyard slabs and purple ramps.
- **Walkways are 1 panel wide and straight.** Bridges run 3 to 13 panels
  without a turn and bend at right angles; 2-wide paths are rare.
- **Walkways meet platforms square on,** in the middle of a side, with
  platform floor on both sides of the join; never at a corner or along an
  edge. The comps and homepages go further: a walkway runs on across the
  field it enters as a stripe of its own floor (the orange comp's field is
  crossed by green traces from both its walkways), ending a panel inside
  the field's edge or leaving it as another walkway.
- **3x3 pads on spurs.** The most common platform is a 3x3 square on a short
  bridge off the route (Central, Seaside, Sky, Green): the places for Mystery
  Data, Mr. Progs, shops and warps.
- **Plenty of dead ends.** 10 to 50 per map: pads, stubs and the teeth of
  comb-shaped boardwalks. Exploring a side way usually pays with an item.
- **Few loops.** Most maps have 0 to 5; the route is mostly a tree, with one
  or two shortcuts. Graveyard's loops are holes punched in its slabs.
- **One landmark per map.** A giant plaza, a crater in a field (Central 3), a
  ring road around coloured fields (Sky 3), a raised block (Underground), a
  symmetric hub of pods and stairs (Sky 1).
- **Each area has its own shape language.**
  - Central: fields of 2-4 panel wide paths winding across the map
    (Central 1), with combs of 1-wide catwalks hung off them (Central 2:
    long and short in turn, pads on their ends) and satellite pads.
  - Seaside: one huge field with ragged edges, framed by comb boardwalks.
  - Green: long parallel planks (ladders with rungs) beside grass blocks.
  - Sky: pods and platforms joined by stairs, often mirrored.
  - Graveyard: few huge slabs with holes, joined by long ramps.
  - Undernet: a web of long bridges crossing between scattered plateaus.
  - Underground: plus-shaped platforms on a lattice around a raised block.
- **Exits sit far out.** Arrows at the ends of walkways on the map's edge,
  away from where MegaMan arrives.

## The generator

### Navigation

MegaMan walks as in BN6, and always will (docs/FIDELITY.md): a one-wide
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
order, so an area does not repeat one while it has others.

| Layout | Areas (weight) | Built from |
| --- | --- | --- |
| Route | Seaside 40, Sky 45, Green 45, Secret 25, the comps and homepage 30 | a winding chain of platforms on bridges, pad spurs, stubs, a shortcut |
| Field | Central 35 (around a crater), Seaside 60 (ragged), Comp 40 | one big field, comb boardwalks with teeth on two or three sides, pads |
| Ladder | Green 55 | four or five parallel planks joined by rungs, teeth on the outer ones, grass blocks at the ends |
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

MegaMan arrives on the pad nearest the top of the screen; the exit is the
room farthest from it by walking. On a guardian's layer the exit is an
arena instead (`src/net/net_arena.c`): a 5x5 platform at the end of a single
bridge of 3-5 panels, attached where it lies farthest from the arrival,
drawn in the area's second floor so it reads apart from the platforms.
Pads (the 3x3 rooms on spurs) are drawn with tiles learned on the
original's own pads, where those are square (not the Judge Tree's round
stumps or Sky's round pods; Robot Control's are its other maps' raised
white platforms), and the originals' free-standing scenery is
set in the void beside the floor (`src/map/decor.c`). The
guardian holds its middle and the exit waits on its far side; the room the
bridge leaves from gets the heal and the Net Dealer (docs/BOSSES.md). Services go to the bigger platforms, the
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

## Props

The originals furnish their floors: a Net Dealer behind his capsule counter,
NetCafe desks with their staff behind them, avenues of cybertrees framing a
giant tree, a statue between two braziers, rows of gravestones. They are what
makes a map read as a place, and none of it is scattered. Each prop does a
job, and the generator sets one only where it does that job:

- **A signifier.** A counter says "a service stands here" before a word is
  read, as a shop's awning does. The navi behind it is spoken to across it.
- **A landmark.** One set piece per layer that the eye finds and the way can
  be told by ("past the statue"): the originals' hubs, craters and giant
  trees.
- **The area's identity.** Each area furnishes itself from its own maps:
  Green's trees and potted bushes, the Undernet's statues and braziers, the
  Graveyard's stones, Sky's field pictures and pods.

The pattern is the hand-authored unit in a generated layout (the game-design
skill's handcrafted-pcg-hybrid): the units are Capcom's own, cut whole from
the ROM with their tiles, walls and the navi's place, and the generator
decides only where they go, by rules measured on the originals.

### How the originals place them

Measured over the internet maps and their object lists (the research's
counts, maps 0x90-0x96):

- **Counters face the camera** (6 of 7): their front towards the screen's
  foot, the navi behind on the far side. A dealer's capsule sits one panel
  in from a platform's back edge, the aisle behind it on the rim, or closes
  a dead-end stub; a NetCafe desk sits at its room's far end, facing the way
  in, with the WELCOME sign beside the entrance.
- **Tall props stand at the back** of their platform, in its back third
  (signs 6 of 7, statues 8 of 11, boards 8 of 10, braziers 5 of 6).
- **Clear of the way**: decoration keeps two panels or more from a
  walkway's mouth (trees 25 of 25, signs 7 of 7, statues 10 of 11) and three
  or more from the shortest route between the warps (trees 16 of 25,
  statues 8 of 11, boards 10 of 10). What stands in a mouth is a story
  obstacle, a gate.
- **Symmetry and rhythm**: braziers flank a statue as a mirrored pair;
  Green Area 2's nineteen cybertrees form a V along its field's rim, mirrored
  about its axis (18 of 19 have their mirror), framing the giant tree in the
  back corner; gravestones and Sky's trees stand in rows of three a panel
  apart; Green's potted bushes repeat every second panel in the gaps between
  its planks.
- **Edge or hole**: signs and braziers stand in the void just past the
  floor's edge (11 of 13); gravestones, boards and statues fill walled holes
  of one panel inside platforms (33 of 39).

### Counters (built)

The Net Dealer stands behind a counter wherever the area has one:
Sky Area 3's capsule in Central, Seaside and Sky (the four surface areas
share one tile set, each drawing it in its own colours) and Green Area 2's
NetCafe desk in Green (`RomLayout.net_area[].counter`, docs/ROM_DATA.md).
`src/map/props.c` cuts it: its second-layer tiles from the floor it stands
on, its ring of walls, its navi one cell behind the ring's back row and the
navi's talk centre 8 towards the front; the map's mirror image gives the
other way it can face. The generator (`counter` in `net_gen.c`) puts it one
panel in from a room's back edge, grid -x or -y, facing the camera: the
aisle behind it on the platform's rim (void beyond) is walled off
(`C_SOLID`), its own panels keep their floor under the ring (`C_PROPPED`),
the floor before it stays free, and nothing it closes off may be cut from
the rest. It is centred along the edge, trying outwards from the middle.
Where the dealer's room has no such place, the next room that has one takes
him; the room before a guardian's arena keeps its dealer, standing if need
be. Over 400 layers per layout the dealer stands behind a counter on 97% of
Central's routes, all its fields and 16% of its catwalks (their mazes have
few rooms with a rim), 99% of Seaside's, 79% of Sky's routes and all its
hubs, 99% of Green's routes and 75% of its ladders. The navi is drawn under
the second layer (script 0x1B) so the counter's art covers its legs, and the
director aims A at the talk centre (OverworldNPCObject +0x11-0x13).

### Sets (built)

Each area furnishes its layers from its own maps' objects (`RomLayout.
net_area[].looks`): the originals' map objects by their ids, each at its
panel's spot 14 and 18 in from the corner as the originals set theirs,
spawned with the layer's map (docs/ROM_DATA.md, Map objects). The
generator composes them after the services and before the loose Mystery
Data and bystanders, as a map is built before it is filled
(`landmark`, `rows`, `signs` in `net_gen.c`):

| Area | Landmark | Rows and signs |
| --- | --- | --- |
| Green | the giant cybertree in a walled hole one panel in from a room's back rim, cybertrees past the rim on both sides of it, mirrored, up to four pairs | rows of three cybertrees past rims; the WELCOME sign by the counter |
| Undernet, Secret Area | the statue past a back rim, a brazier two panels to either side, mirrored | |
| Graveyard | the monument past a back rim | rows of three gravestones in one walled hole of three panels in a big slab |
| Central, Sky | | rows of three cybertrees past rims; the WELCOME sign by the counter |
| Seaside | | a BBS past a big room's rim; the WELCOME sign by the counter |

The rules, from the originals' numbers above:

- **The landmark** takes the best room for it: not the arrival's, the
  exit's or the arena, no pad, twelve panels or more, no counter in or
  beside it, a back rim (grid -x or -y, the top of the screen) of three
  panels (five for the statue's pair) with void two panels deep past it,
  its middle off the way between the warps; bigger rooms, longer rims and
  more distance from the way first. The set slides along the rim from its
  middle until it fits.
- **A walled hole** (the giant tree, the gravestones) has floor all round
  it, keeps a cell clear of stairs, rooms' anchors, walkway mouths and
  objects, stands off the way, and cuts nothing off.
- **Past a rim** a prop stands in the void a panel out, with void a second
  panel out, so never in a gap between two platforms.
- **Rows** of trees go past the rims of up to two other rooms, trees two
  panels or more from the way; rows of gravestones go in slabs five panels
  or more across, up to two.
- **Signs**: the WELCOME sign in the void at the end of a counter's aisle,
  as the originals set theirs by a NetCafe's way in; a BBS past a big
  room's rim.

Over the tests' 300 layers with every look: 2348 props, 255 of them in
walled holes, a landmark on 162 layers (the areas' own looks give fewer).
A map loads at most 12 compressed sprites and 0x8800 bytes of them (the
game's loader); a prop whose sprite no longer fits is left out whole, not
shown as noise.

### Pads' centrepieces: warps only (built)

The originals' 3x3 pads carry art on their middle panel, second-layer tiles
over the floor: the red gem, the link ring and the cube on its round base,
each 28 tiles, the same tiles in Central, Seaside, Green and Sky in each
area's colours. `props_learn_ornament` finds them in an area's own maps by
their tiles (0x379, 0x372, 0x375) and cuts each on the corner of the panel
it lies on. None of them is decoration: all 29 in BN6's net maps stand on a
warp pad (the gem on a teleport within the map, always one of a pair; the
cube on a link to a homepage; the ring on a link to a comp or another
area). They were set on the middle of every pad, so most layers showed warps
that did nothing, which players take for warps; the owner ruled it out. Now
only a teleport pair's two pads carry the gem (`paste_ornaments`, Teleports
below), and a pad without one keeps its plain floor, or its frame's empty
recess, as BN6's pads without a warp do.

### Central's, Seaside's and Sky's framed pads (built)

Central Area 1's pads are one piece of art over their 3x3 panels: a pale
frame with violet corners, a cyan band and a blue recess that holds the
centrepiece, the catwalks meeting the frame's edges; Seaside's the same in
yellow with green corners, Sky Area 2's in lavender with pink ones. The
tile classes drew a layer's pads in Central's green floor (the framed
pads' blue lies in the walkway material, and classed as walkway their
pieces came apart). `props_learn_pad` takes the 3x3 islands of floor in the
area's maps whose middle has the recess's hue (`net_area.pad_hues`:
Central's blue, Seaside's yellow, Sky's lavender) and lays them over each
other: the first layer's tiles over their panels, the faces under them and
the rim their upper edges reach past the diamond, each tile from an island
where it draws the island alone, the one with the fewest bridges beside
its lower sides first (every island has a bridge somewhere, and a bridge's
start came along in the tiles beside it: Seaside's pads hung a scrap of
boardwalk off their lower edge, and where the rim was left to the classes
Central's showed tile-wide steps along its upper edges). `paste_pads` sets
it on every 3x3 pad at ground level away from the stairs, each tile only
where it covers the pad and the void alone (a catwalk's last panel came
out cut where the stamp's edge lay over it), and the centrepieces go on it
(never art that draws mostly black: Sky's gem tile is the pit in one of
its platforms). A pad itself is walkway floor in these areas, as the
originals' are, so the tiles the stamp leaves at a catwalk's join are the
originals' join of a catwalk into a pad (as platform floor they drew the
catwalk meeting a green field). Green's are not cut: its pads stand
raised on the originals, and would sink MegaMan into them.

### Green's potted bushes (built)

Green's maps set potted bushes, a 4x4 block of second-layer tiles, in the
void gaps between parallel planks, one every second panel, plain and in
flower in turn (32 in Green Area 1, 25 in Green Area 2). `paste_bushes`
does so in a void panel with floor on both sides along one axis and void
on the other two, every second one along the gap, never over the floor's
own art.

### The Graveyard's and the Undernet's emblems (built)

The Undernet's plateaus carry the same cross in magenta, alone or four in
a diamond; the Graveyard's slabs carry cyan crosses drawn in the floor
itself (the first layer, not second-layer art): one on each small platform's middle
panel, and on its big slabs in rows alongside the line of holes through
their middle, two panels to either side, at even steps, never at a rim or
beside a hole. `props_learn_floor_emblem` finds them by their colour
(BGR555 0x56C0, which the floors show nowhere else) and cuts the tiles
that draw one where it lies on a panel with floor all round; the
generator sets `PROP_EMBLEM`s by that rule in every room, on ground floor
with floor all round and nothing standing there (`emblems` in
`net_gen.c`), and `paste_emblems` draws them in place of the floor there.
The slabs' holes follow the originals too: a line along the middle rather
than a grid, three panels apart where the originals keep two (the floor
between two holes one panel apart reads as a walkway to the tiles, and
takes a bridge's link look).

### Next

The field pictures and a hub's centrepiece where the layout has a centre;
each a stamp cut like the counters, set by the same rules.

## Set pieces

BN6's net areas hold more than floors and props: Rush's bone gaps, purple
Mystery Data, the Link Navis' obstacles, arrow panels, security cubes,
teleport pads. Each area repeats two or three of its own, the main route
stays free, and each lock guards one thing on a spur, its reward following
the detour. The research behind them (what each area holds, how the ROM
runs each, the rules Capcom places them by) and the plan are in the epic,
issue #49; what is built is below.

The design is the run's dialectic, prepare or press on (docs/META.md), at
the scale of a spur: a detour costs steps, and BN6's step counter rolls
battles; a lock's key costs zenny or a Cross; what it pays is power for the
act's guardian, or a shortcut that saves battles. Value that has nothing
to do with its cost asks no decision, so the generator ties them together.

### Detours and their data (built)

`measure_detours` in `net_gen.c` walks from the way (the layer's shortest
walk from the arrival to the exit or the guardian, `layer_on_way`) over
the floor: each panel's detour is the panels walked to reach it from the
way (`layer_detour`), and its branch the way's panel that walk leaves
from, so a spur, a room off the way and everything past them share one.
The guardian's arena is no detour (the way ends at him).

Mystery Data follow BN6's colours, which say what a walk is worth:

- **Blue data where the detours end**: 2 a layer, one more past depth 6
  and in the Undernet; each the farthest free panel of a branch three
  panels off the way or more, the farthest branches first, on a pad's
  middle where the end is a pad (`detour_end`). The farthest is of the
  best quality (a chip three tiers up, or an HPMemory) where it lies eight
  panels off or more; the others three times in ten. A layer with too few
  detours puts the rest in its rooms. The run's Spin lies in the best of
  them.
- **Green data loose**, 2-4 a layer (up to two more on bigger layouts), most at
  dead ends: the commonest quality.

Over 3000 layers of a first cycle the old roll gave every quality the same
places, 7 panels off the way on average and a quarter of them within two
panels of it, all drawn green; now the green data lie 6.1 panels off on
average, the blue ones 8.2 and the best 11.8, 1-2% of them within two
panels, and about as much in all (fewer of the middle quality, a fifth
more of the best). The tests check that nine blue data in ten stand three
panels off the way or more.

### The act's plan (built)

`layer_pieces` in `net_pieces.c` decides which set pieces a layer holds,
from the run's seed and the depth alone, so a Net Dealer earlier in the act
knows which keys to stock (`layer_pieces_ahead`):

- **Budget by the act's rhythm.** None on the run's first layer or in the
  Secret Area; one on an act's first layer (half of them), two on its
  middle one (the dealer's: a choice of where to spend), one on the
  guardian's (four in ten), one on a dark warp's layer (six in ten).
- **Each area its own,** by weights from what its original maps hold
  (purple data everywhere but the Secret Area, the most in the Graveyard
  and the Undernet; Rush in Central, Seaside, Green, Sky, the Undernet and
  ACDC's homepage), with a weight of nothing beside them, so an area with
  few of BN6's pieces keeps them rare, as its maps do; never one piece
  twice on a layer, and no Rush before a guardian.

### Purple data (built, issue #41)

BN6 sets none or one purple Mystery Data on a map (20 in Gregar), locked
until an Unlocker opens it, holding what no shop sells: ElecSword E,
Muramasa M, DreamAura U, a Spin; Central Area 3's lies beneath the statue.
A layer does the same (`layer_purple`, `place_purple` in `net_gen.c`):

- **Where:** at the landmark's foot, the panel in front of the giant tree,
  the statue or the monument, else where the longest detour ends, first of
  the layer's data.
- **When:** from the run's seed and the depth alone, so a dealer earlier in
  the act knows: on six in ten of an act's middle layers and three in ten
  of its guardians', a quarter more in the Graveyard and the Undernet,
  which hold the most of BN6's, and on half the dark warps' layers; never
  on an act's first.
- **What it holds:** a chip of the rarest tiers (`roll_chip`'s bonus 4, a
  Mega now and then) in one of its letters, never one the layer's dealer
  lists. The run's Spin lies in it where its layer holds one, as BN6 hid
  SpinBlue in a purple data.
- **The key:** BN6's own, the Unlocker. The Net Dealer stocks one for each
  purple data the act holds from his layer on, at 600 zenny in act 1 and
  300 more an act (BN6's 4000 is out of a run's reach: a key priced past
  its use is never bought), and says so: "Word is, there's purple data
  locked on this layer". Three layers in ten with a purple data hide an
  Unlocker in one of their blue data instead, a lock and its key on one
  layer. The game's own text asks for it and takes it.

Over the tests' 300 layers, 46 hold a purple data, never on the way.

### Rush's gaps (built, issue #14)

BN6 lays 13 bone panels over 9 gaps (Central 2, Seaside 1, Green 1 and 2,
Sky 1 to 3, Undernet 1, ACDC's homepage), 1 to 3 panels each: six are
shortcuts, three lead to one Mystery Data. Press A at the edge with RushFood
held, as many as the gap's panels, and Rush comes, eats one, and lies in
the gap for good. The game runs all of it (docs/ROM_DATA.md, Rush's gaps);
a layer gives it what its own maps do.

- **The place** (`plan_gap`, `carve_gap` in `net_gen.c`): a walkway's tip
  (else a platform's edge) off the way, at ground height, aimed across the
  gap at a pad of its own, 1-3 panels on (the area's longest: Sky's 3,
  Central's and the Undernet's 2, the rest 1), the gap's panels and the
  pad void with void a panel round them and no prop, inside the camera's
  window; the farthest off the way first. The stand and the panel behind
  it are kept clear of what is placed after; the pad is carved last, as a
  room of its own, and holds one blue Mystery Data, of the best quality
  past one panel (two or three bones' worth).
- **The map** (`netmap.c`, `coords.c`): for the walls the gap is a
  walkway, walled along its sides; across its mouths the floor's edges
  stand again with the flag byte 0xFF (the second gap's 0xFE), which Rush
  lying there switches off. Its trigger strip is the lane's floor 24 units
  in from the near mouth, where the engine's A probe lands (BN6's strip is
  the mouth's row alone, its probe 8 ahead). Its tiles stay void.
- **The objects** (`rush.c`): per panel Rush (handler 0x25, shown once
  bridged) and the bones (OverworldMapObjects 0xD5 + g, shown until then).
- **The key:** RushFood, sold by the Net Dealer where the act holds gaps
  from his layer on (one for each gap, and as many more as the longest
  needs held), at 300 zenny in act 1 and 100 more an act (BN6's seller asks
  3000). The dealer names the gap, and MegaMan names it when he first
  comes near its stand: what calls Rush and how many he needs, or, with
  enough held, to press A at the edge (BN6 answers A without them with a
  sound alone).

Over the tests' 300 layers, 40 hold a gap, 55 panels in all (the act's
budget shared with more kinds of piece since: 62 and 91 before the
obstacles); the way never needs one, and each island is reached only
across it, holding its one Mystery Data.

### Teleports (built, issue #44)

BN6's gem marks a teleport pad: step on it and MegaMan beams to its pair
within the map, the camera scrolling along (warp-list departure 12; 18 gems
in the surface maps, all in pairs: Green Area 1's four pairs, Sky's three,
Central's one). A layer's pair (`plan_teleport`, `carve_teleport_island` in
`net_gen.c`) is one of two things, in Green, Sky and Central, where the
act's plan calls for one (`net_pieces.c`) and the area's maps give the gem's
art:

- **A quick way back:** one gem where a long detour ends (7 panels off the
  way or more), the other 2 to 6 off the way, the walk between them 16
  panels or more, so the long way in has a short way out.
- **Else an island of its own,** as ACDC's homepage keeps an isolated square
  reached by its teleport alone: a 3x3 pad past the void (two void panels
  round it, inside the camera's window, the nearest such to the gem near
  the way, so it is seen from there), one blue Mystery Data on its corner
  away from its gem.

Each gem stands on a panel with floor all round it (its art draws its own
diamond over them) and never on or beside the way (its trigger reaches a
cell into the panels round it): no one is warped walking by. The pads'
triggers take warps 2 and 3, each leading to the other's middle
(`mapslot_teleport`, the layer map's warp list). Over the tests' 300
layers, 12 hold a pair, 11 of them to an island (21 before the obstacles
shared the act's budget).

### Link Navi obstacles (built, issue #42)

BN6 sets 25 obstacles in its nets, each in the mouth of a pocket that holds
one thing (a SubMemory, Attack+1, HP+100, Sky's purple data): a geyser of
cyberwater, a cybertree, a pillar of flames, a cyclone or a cloud, which two
of Gregar's Link Navis clear each. A layer sets one the same way:

- **Its kind is its area's own** (`layer_block_kind` in `net_pieces.c`):
  Seaside's geysers and cyclones, Green's trees, Sky's clouds, flames and
  geysers, the Underground's clouds, cyclones and flames under the Nest,
  all five in the Graveyard; none in Central, as in BN6.
- **The run's Crosses are its keys:** the Cross brought (alone) or each
  won from a guardian before this layer clears the obstacles its Navi
  clears in Gregar. Four pockets in five take a kind the run can clear,
  else one it can't, a hint for the next run's Cross; a run that can clear
  none of the area's meets one two layers in five, else none (a pocket a
  run can never open is dead weight). So the setup's Cross is a bet on the
  net's locks as well as on its battles.
- **Its pocket** (`plan_obstacle` in `net_gen.c`): a walkway's first panel
  off a wider floor, off the way, whose closing cuts off 6 to 40 panels and
  nothing the way, a service or the guardian needs; seen from the way
  (its mouth near it). Nothing else is placed in it; its one thing, a blue
  data of the best quality, lies where it ends.
- **The map** (`blockers.c`, `netmap.c`): BN6's own obstacle (handler 3)
  where BN6 sets one in a walkway's mouth, a line of wall cells across the
  walkway flagged with its slot, and its check cells where the engine's A
  probe lands from the floor before it; the check runs the layer's talk
  from the map's own archive: MegaMan names it, and with the Cross, the
  Navi's mugshot says "Leave it to me!" over his sound, and the obstacle
  opens, for good (its present flag is kept in the run's state).

Over the tests' 300 layers (their guardians Cross navis), 13 hold one.

### Security cubes (built, issue #45)

BN6's security cube (handler 3, sprite 7:0x03, its records looking the four
ways) stands across a walkway and vanishes when its condition is met. A
layer sets one where the act's plan calls for it, in every comp and
homepage, Central and Seaside (`layer_cube_kind` in `net_pieces.c`), in a
pocket's mouth as an obstacle stands (`plan_obstacle`), its one thing a
blue data of the best quality:

- **A P-Code** (comps, homepages, Central): four digits from the layer's
  seed, which one bystander knows (`place_teller` in `net_gen.c`). He
  stands a walk of eight panels or more from the cube and apart from
  anything else, so the key lies on another way than the lock: at the cube
  without it, MegaMan says somebody on this layer must know it, ask
  around; told it, A at the cube enters it and the cube opens. A layer
  with no such place for him takes a toll instead (a lock no one can open
  would close its pocket for good).
- **A toll** (Seaside): 200 zenny in act 1 and 100 more an act, half the
  Net Dealer's answer chip: the run's dialectic, prepare or press on, in
  zenny. The game's own zenny check takes it, or MegaMan says they don't
  have that much.

Two rules came from the cube and hold for every piece spoken to:

- **No navi within three panels of a piece's A** (a cube's or an
  obstacle's mouth, a Rush stand; `navi_near`, `hush`): the engine's A
  turns to a navi within 52 units before a map's check, and a teller two
  panels from his cube took the cube's A. A piece is planned away from the
  navis standing, and the bystanders and the teller placed after it keep
  off the panels round it.
- **A mouth stays one panel wide:** the map's floor fix (`legal.c`), which
  toggles cells for the tiles, widened a cube's mouth on a comp to three
  panels, and MegaMan walked round the cube. The mouth, the void beside it
  and the floor before and after it are locked (`lock_pieces` in
  `netmap.c`).

Over the tests' 300 layers, 10 hold a P-Code cube, each teller eight
panels' walk or more from his, and 9 a toll; no navi stands within three
panels of a piece's A.

### Arrow lanes (built, issue #43)

BN6's arrow panels carry MegaMan one way, input held, four units a frame,
until he is past them: Seaside Area 2 is a field of them, Sky Area 3 rings
a platform with one-way lanes, Green's and Seaside Area 1's are single
panels set into a walkway. A layer's lane is the quick way back
(`plan_lane`, `carve_lane` in `net_gen.c`):

- **Where:** from a free ground panel five or more off the way (where a
  long detour ends, by its data) straight across one to five void panels,
  the void beside each, to free ground floor three or less off the way,
  the walk it saves five panels or more, the most saved first. It is
  carved last, once the rest stands: neither end on an island past a gap
  or a teleport's void, in a pocket a lock closes or in the arena. So the
  walk in is long and the ride out short; walked against its arrows, its
  first panel carries MegaMan back, so it is never a way in.
- **Which areas:** by the act's plan, Seaside's the most, then Green's,
  the Undernet's, the Underground's under the Nest and the Judge Tree
  comp's; Sky's seldom (its layers seldom leave a short gap from a far
  floor back to the way: one in ten found room), none in Robot Control's
  (two ways drawn, its layers too close).
- **The art** (`props_learn_arrow` in `props.c`): each area's own arrow
  panel for each way, cut from its maps where one closes a lane a panel
  wide (the end cells on its far edge): the tiles of both layers whose
  middle lies on its diamond. An area whose maps draw a way's panel alone
  gets lanes that way only (`LayerKit.arrows`); Sky's and Green's also
  come from Sky Area 3 and Green Area 2 (`net_area.arrow_maps`).
- **The ride** (`lanes_place` in `netmap.c`) is BN6's own: three start
  cells across the lane's near edge and three end cells across its far one
  (docs/ROM_DATA.md, Arrow panels), as Robot Control Comp 1's lane has
  them; the game's player update carries MegaMan from one to the other.
  The lane, its ends and the void beside it are kept as generated
  (`lock_pieces`).
- **The way on** (`layer_step_ok`): the guide's walk and the autopilot's
  step onto and along a lane only the way it runs.

Over the tests' 300 layers, 20 hold a lane (of 36 planned), 35 panels in
all.

### Invisible paths (built, issue #46)

BN6 hides floor drawn as void in Seaside Area 1, Sky Area 2, Underground 1
and Undernet 2, from a stub's tip to a lonely pad: an HPMemory, MegaCannon
S, AirShoes. A layer hides one the same way (`place_hidden`,
`carve_hidden` in `net_gen.c`):

- **Where:** a Rush gap's site (`plan_gap`: a walkway's tip, else a
  platform's edge, off the way, aimed across one to three void panels at
  a 3 x 3 pad of its own), planned once the islands stand; in Seaside,
  Sky, the Undernet and the Nest, from the second act on, by the act's
  plan. Its pad holds a blue data of the best kind.
- **Drawn as void, walked as floor** (`path_panel` in `netmap.c`): its
  panels stay void in the layout, so the tiles draw the void there, and
  count as floor to the map's walls, as a Rush gap's do once Rush lies in
  it, with no wall across its mouths; the legalizer leaves them as they
  are (`lock_pieces`). The guide and the maps never show it.
- **Its cue, never none:** the tip aimed at a lonely pad, and a navi two
  to eight panels from it, off the way, who says he saw a Navi walk out to
  that pad over nothing; where no navi may stand, there is no path.

Laying it turned up a bug in `cuts`, which keeps a navi from cutting the
floor: it counted every floor panel, so once any island stood (a Rush
gap's, a teleport's, this one's) every navi placed after looked as if it
cut the floor, and a P-Code's teller gave way to a toll. It now counts the
floor reached before.

With every layer made to hold one (`--dev pieces=64`), 59 of the tests'
300 layers find a site and a navi for it.

### The Undernet's doors (built, issue #47)

BN6's Undernet locks its spurs with its own doors, the security cube's
records with their own words (docs/ROM_DATA.md, Link Navi obstacles and
security cubes): skull doors, which only a WWW-ID passes, and number
doors, which ask a count the map itself holds ("count the flames of
hatred": Undernet 2's braziers). A layer of the Undernet sets one where
the act's plan calls for a cube (`layer_cube_kind` in `net_pieces.c`):

- **A number door** on an act's first layer and on a dark warp's, else
  half the time: it asks how many braziers burn on the layer, three
  numbers in a row to pick from (the right one first, second or third, as
  the layer's seed has it). MegaMan warns that a wrong number seals it;
  B steps back to count. Wrong, it seals itself for the layer. The
  Undernet's layers line rows of two or three braziers past their rooms'
  rims (`brazier_row`), with the pair beside its statue six to nine in
  all; a layer with fewer than two asks a P-Code instead.
- **A skull door** otherwise: it opens for BN6's WWW-ID (key item 0x44),
  one of which opens every skull door of a run. The act's Net Dealer
  stocks it where a skull door lies ahead (at 2200 zenny in the Undernet's
  act, two layers' worth) and says what it opens; never on an act's first
  layer, before his. The keys now come before the SubChips on his list of
  eight: an Unlocker had taken the last place and left the WWW-ID off.

### Teaching them (issue #48, in part)

A player meets each set piece without a manual, the lock and its key said
together:

- **At the piece:** a Link Navi obstacle, a cube and the Undernet's doors
  name themselves and their key at A; Rush's bones are named when MegaMan
  first comes near (`rush_hint`); an invisible path has its navi.
- **From the dealer:** he names each key on his list and where its lock
  lies (`dealer_keys`: the Unlocker, RushFood, the WWW-ID).
- **From L** (`status_words`): on a layer's first L, MegaMan senses its set
  pieces with its services ("I sense a Net Dealer, purple Mystery Data and
  bone panels here!"), and the first time a profile meets a kind he says
  what it is and what opens it (`piece_lessons`, `profile.pieces_taught`),
  as he explains the map's violet marks once. Never an invisible path.

Not built yet: the first layer of a run holding a kind holding its key
too, and the 3DS map's marks on locks seen.

