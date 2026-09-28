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
yellow boardwalks, Sky's darker glass, Green's orange planks, Graveyard's
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
(`TILES_RIMMED`). The walls inside Comp 2's platforms ring the grey cubes
that stand on them, not holes; learned as holes, the panels beside them
taught plain white next to the void, which the inner corners of generated
platforms took. The tiles are learned with those panels as floor
(`TILES_INNER_WALLS`), while the neighbourhoods legal.c asks after keep
them as holes, so floors do not change. Comp 2's one small platform is the
striped conveyor before the robot's door, which drew the pads half striped:
the pads take the look of Comp 1's and the Pavilion's raised white
platforms alone (`TILES_MORE_PADS`).

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

### Pads' centrepieces (built)

The originals' 3x3 pads carry a walkable ornament on their middle panel,
second-layer art over the floor: the red gem (16 of them over the surface
areas), the link ring and the cube on its round base, each 28 tiles, the
same tiles in Central, Seaside, Green and Sky in each area's colours.
`props_learn_ornament` finds them in an area's own maps by their tiles
(0x379, 0x372, 0x375) and cuts each on the corner of the panel it lies
on; `paste_ornaments` sets one on the middle of every pad at ground level
with nothing standing there, the red gem three times in five, from the
layer's seed (the floor and the layout unchanged).

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
