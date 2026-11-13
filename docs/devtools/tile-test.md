---
type: Tool
title: "Tile test: clean tiles in every area"
description: "`python3 build.py tiles [SEEDS]` runs the atlas over every area and layout, lists each area's breaks in its picture, tiles drawn off and seams per 100 panels with reasons, and fails where a layer got worse than the baseline."
tags: [devtools]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-13T16:30:00Z }
---

# Tile test: clean tiles in every area

```bash
python3 build.py tiles [SEEDS] [--baseline]
```

The atlas of every area and layout (2 seeds each by default, about three
minutes), and in the areas that have stairs (Sky, the Undernet) as many
layers that climb one, then a table per area: its layers and panels, the
breaks in its picture, the tiles drawn with other floors than they show and
the seams per 100 panels, and why the tiles were drawn off
(`src/map/tiles.h`, `TILE_WHY_*`):

- `unseen`: no pair of the area's maps at that place in a panel shows what
  the tile shows. A shape its maps never draw: change the layout, or learn
  from more maps (`net_area.more`).
- `pixels`: pairs that show it cover the floor wrongly here (the pixel
  test: faces and legs of another height or length).
- `plain`: they do not look like the area's plain floor well inside it.
- `ranked`: one would have done and lost to a nearer neighbourhood or a
  seam.

The seams count what the classes pick; what a player sees best is the
pictures against the area's own maps (`src_bNN_*.png`). A sweep of them
found what the numbers missed: arenas paved with walkway tiles
(`TILES_ARENA_FLOOR`; `TILES_ARENA_DRAWN` where the arena's layout must
stay as the legalizer made it: the Undernet's), edges whose own rim tiles failed the plain test and
were stood in for by inner panels (`TILES_RIMMED`), decorated panels
whose pieces turned up elsewhere (`skip_styles` with `SKIP_ANY_PIXEL`,
`SKIP_PALE`), and two floors sharing a hue told apart by shape
(`TILES_WALK_NARROW`).

How much of each original map the layers draw at all is counted beside
these: [what the layers take from a map](map-use.md).

## Breaks: the picture itself

The off tiles and the seams say how far a tile's pair was seen from its
place. Neither says whether the picture holds together: BN4's Park Area
read 0.0 tiles off with blocks of walkway floor all over its platforms (12
November 2026, the owner's captures of seeds 3 and 6), each tile right for
its panels and wrong beside its neighbour. So the finished picture, what
is pasted over the picks with it, is held to the originals' edge by edge
(`src/map/breaks.c`, kept only by the atlas): across every tile edge lie 8
pixels on each side, a strip, and the area's maps show a set of them at
their own tile edges, as drawn and as their first layer alone draws them.
A strip of the layer's is held to the nearest of those with the same
pixels on one side; the pixels of its other side that differ by more than
a shade (3 steps of a 5-bit channel) are what breaks there.

- **break**: half the edge or more (4 of 8 pixels). A block of another
  floor or shade against this one, a rim cut off.
- **nick**: fewer. A panel's line or light cut short.

Both are listed per layer in the report and held by the baseline; the
breaks are yellow in the `seams_*.png` pictures, and `breaks_*.txt` lists
each tile's (x, y, bits: 1 right, 2 below, 4 and 8 a nick there). Strict
equality of strips read 368 per 100 panels in Central, every one a pixel
or two of its panels' lights, so the count takes the nearest strip and
tells the two sizes apart.

What they found, and what the pick does about it
([the generator](../world/level-design/generator.md), Tile picking): inside
a floor two whole tiles were never held to each other, so each took its
place's most common look; the pick now pays for a strip no map shows.
Over the atlas's 278 layers the breaks fell from 218 to 116 per 100
panels and no area rose: Central 124 to 56 (its fields' lights no longer
cut), Sky 228 to 107, the Graveyard 107 to 40, Robot Control 134 to 25,
the Aquarium 235 to 56 (its glass pads stand on their legs again, not on
the water's wall), BN5's ACDC Area 318 to 142, BN4's ACDC Area 637 to
249. What is left is mostly where two floors join as no map joins them
(a pasted pad's rim, a walkway's mouth) and the areas below.

Still far from clean, each a call of its own rather than the pick's:

- **BN4's Park Area** (304): its one platform never touches a walkway
  (a conveyor crosses it, and the wall rings count the larger half as a
  hole), and its upper edges carry a wall 8 pixels above the floor's
  edge, which the pixel test takes for something drawn off the floor. Its
  platforms are drawn apart, as BN4's ACDC squares are
  (`NET_APART_PLATFORMS`, the owner's call of 13 November 2026): a
  rectangular room comes out as the platform itself, whole, the walkways
  ending at its rim, where flush its rim was walkway floor in steps
  against its field (285 breaks then: two tiles laid over each other at
  every such end count more than the steps did, and read better). Not
  clean: a room three panels wide has a field one panel wide, which the
  platform never shows, and where the way runs two wide beside a room it
  is walkway floor, a block's edge against the room's field (seed 6's
  first room). Tried and dropped: its walls' inner rings as floor
  (`TILES_INNER_WALLS`: the conveyor's pieces came along), a tile let draw
  the wall's height over the floor's upper edges (the walkways' round
  studs came along, and no wall).
- **BN4's net countries** (195): its green floor is clean now, but a
  cream platform's frame gives way to plain cream in places: plain floor
  continues anything plain. Taking a tile's own neighbourhood first,
  whatever it meets, kept the frames and cost more than it won (116 to
  139 breaks per 100 panels over all areas, the seams 84 to 111).
- **BN4's Town Area** (488), **BN5's Oran Area** (529), **CopyBot Comp**
  (288), **Seaside** (279), **BN4's and BN5's Undernet** (303, 248),
  **Nebula Area** (248): not looked into here.

## A floor drawn by hand, and the original as the bar

`CYBERWORLD_FLOOR=FILE build/host/cyberworld --headless --rom-dir ROMS
--data-dir DIR --atlas OUT:hN` draws the floor FILE describes (rows of
`.` void, `#` floor, `P` a pad's floor; at most some 24 by 18 panels, the
tile map's size) in area N's tiles as it stands, no cell changed, as
`hand_bNN.bmp`, its breaks marked in `seams_hand_bNN.bmp`
(`src/dev/atlas.c`, `hand_floor`). It answers what an area's art can draw
at all: a sheet of rectangles of every size, a sheet of walkways with their
turns, ends and joins. The atlas also writes each original map's first tile
layer alone (`src1_*.bmp`) beside the map as the game draws it.

The originals themselves count no break: the strips are theirs. So the bar
for a generated layer is 0, and what a layer counts is what it draws that
no map of its area shows. Two things keep layers from it (13 November
2026, measured on hand-drawn floors of BN4's Park and Town Areas):

- **The learner's reading of a map.** BN4's Park Area's one platform
  lies whole on the first tile layer, but a conveyor crosses over its
  middle and the walls give it as two halves: each half ended, under the
  conveyor, in an edge that is none, so "a room's end" was learned as plain
  floor cut off, and the wall standing 8 pixels over its upper edges failed
  the pixel test. Read as one platform (`TILES_UNDER`, `TILES_INNER_WALLS`)
  and laid panel by panel ([whole panels](../world/level-design/generator.md)),
  a rectangle four panels wide or more comes out as BN4's platform, three
  of its four corners exact. Town Area 3's lower paths pass under its
  floors the same way, and its two maps measure side faces of 17 and 10
  pixels, so the walkway lattice of Town Area 1 lends its panels nothing.
- **The layouts.** Park's platform is four panels wide; a room three wide,
  a way two wide beside a room, an L of two rooms have no panel in BN4
  (the first has its tiles all the same: Panels lent tile by tile, below).
  Town's maps have walkways one wide and squares of three by three joined
  at the middles of their sides; a walkway's dead end, a square of another
  size and two squares side by side have none. Laying whole panels changes
  nothing there: over all 39 areas it left the breaks where they were
  (116.8 per 100 panels with it on everywhere), which is the measure of
  how much of the rest is layout.

Whole panels were first on for Park Area (breaks 304 to 263 per 100
panels) and BN5's Oran Area (529 to 438) alone; switched on everywhere
they left the total where it was, the Graveyard's slabs losing their upper
rims (40 to 59), BN4's ACDC Area gaining breaks (249 to 272).

The layouts stay as they are (the owner's call, 13 November 2026: no
area's rooms are held to the shapes its art draws). So what is left is the
tiles'.

## Panels lent tile by tile

A panel laid only where a map has its whole 3 x 3 leaves most of a map
unused, and three readings were wrong besides (13 November 2026, on
hand-drawn floors again, each cause traced to the map panel laid or left
out):

- **Park's rim under its conveyor was void.** `TILES_UNDER` took a panel
  passing under the conveyor for floor only in the platforms' hue; the
  platform's rim, a walkway's by its hue, was a hole there, so both long
  sides ended in mid-air and their panels lent nothing.
- **A panel was lent whole or not at all.** Park's left corner draws 4
  pixels past where a tile may, in one of its sixteen tiles; its lower-left
  end has a conveyor one panel off. Both were left out, and a room's left
  corner was a plain wedge. A tile shows only the panels it lies on or
  beside, so panels are now kept in part and laid piece by piece
  ([the generator](../world/level-design/generator.md), Tile picking):
  every rectangle, three wide, three by three or five wide, comes out as
  BN4's platform with its four corners.
- **What lies round a floor was laid by its own neighbourhood.** A void
  panel above an upper edge has the same 3 x 3 under a platform's wall as
  under a walkway's rim; Park's upper edges were scalloped with walkways'
  rims. It is laid now as a map lays it beside the floor laid there.
- **Park's small rooms were pads**, drawn in the look of its map's one
  pad, its Navi's round hub (`TILES_NO_PAD_LOOK` now).
- **The Graveyard's upper rims.** Its maps set the rim of some upper edges
  on the second tile layer. A tile that draws nothing on the first passed
  as a tile of nothing, and laid over a slab's upper edge it took the rim
  away. Such a tile is taken from both layers now.
- **Town Area 3's lower paths were its walkways.** They pass under its
  roads inside their walls' rings, three rings deep, which reads as floor.
  `TILES_OUTER_FLOOR` leaves them out. The 17 pixels its side faces
  measure are no error: they are its squares', which stand on a base, its
  walkways' in Town Area 1 measure 10; panels are lent whatever the faces.

Breaks per 100 panels, the atlas's 2 seeds: Park Area 263 to 172, BN5's
Oran Area 438 to 341, and with whole panels now on there, BN4's countries
195 to 115 (small rooms platforms too), BN4's Undernet 303 to 262, BN5's
Undernet 248 to 195. Town Area 488 to 468, its look about the same.

Where they stay off, each looked at with them on:

- **The Graveyard** (40, 52 with them): the upper rims are back, but its
  slabs gain breaks where a pad's room or a walkway joins them, shapes its
  maps do not have.
- **Sky** (107, 88 with them): fewer breaks and the wrong picture. Its
  glass catwalks come out as its fields' framed squares.
- **BN5's ACDC Area** (142, 165): its circuits' walkways lose their rims
  to panels from its wide circuit field.
- **BN4's ACDC Area** (249, 306): its squares come out complete, both
  marks and the base, but the walkways take an end stud or a connector at
  every other panel.

What BN4's Park map has no tiles for, drawn tile by tile as before: the
tip under a platform's bottom corner (its one bottom corner carries a
walkway's end, so a room shows two tiles of it there), the inner corner of
an L of two rooms, a room two panels wide (no field: two rims).

Park's walkways changed with them: a dead end is BN4's round end pad now,
and some straight runs repeat BN4's connector piece. A way's end at a room
took that pad or an arrow head too, since rooms are drawn apart and the
walkway's own view sees a dead end there; the room then covered half of it
(the owner, 13 November 2026: "its circle is not round and cut off"). Such
an end is no dead end, and no panel is laid there now (A border kept
whole).

## A border kept whole

With panels laid, the layers' borders showed teeth (the owner, 13 November
2026: "jigsaw edges around the areas, and inside open areas"): a wall or a
slab's band present in one tile and missing in the next, all along an
edge. The cause is one and general. A border's art runs past the line a
tile may draw up to: 3 pixels beside a floor's edge, 8 over an upper edge
where a wall is let stand. BN4's Park Area's wall, drawn on a slope, is 8
pixels over the edge at one end of a tile and 12 at the other; its Town
Area's squares stand on a blue slab 11 pixels wider than they are, all
round. Each tile was judged alone, so the tile holding the overhang was
turned down and its neighbour kept: a border cut at a tile's edge.

Two rules mend it, one in the learner and its twin where panels are laid
([the generator](../world/level-design/generator.md), Tile picking):

- **The learner follows the floor's own art.** From the pixels the
  floor's tiles must draw, to every pixel of the same colour beside one of
  its own, and its outline; a tile may draw that art wherever it lies. So
  the tiles that hold a border's overhang are learned with the border, each
  marked at its place in its map.
- **A laid tile takes the next one along.** Where a laid tile draws up to
  its edge and nothing is laid beyond, the marked tile its map sets there
  is laid too, and so on from it.

Measured on a square drawn alone in Town's tiles: 34 breaks, 9 with both
rules (its slab whole all round, the steps left are under its lower sides,
where BN4 always joins a road or a hub). Park's upper edges carry BN4's
wall as one band.

Town Area needed its reading put right first. Hue could not tell Town
Area 3's roads and lower paths from Town Area 1's walkways (both pale
blue in a panel's middle), and a layer's walkways took the paths' pieces
at every end and turn. So Town Area 3 lends its squares alone
(`TILES_PADS_ALONE`) and Town Area 1 the walkways (`TILES_MORE_COLOURS`:
their colours are none of the squares'); and a tile of Town Area 3 draws
its squares' own art and nothing else, which keeps out the hub BN4 sets
against a square's corner and the road joined to its side, though they
lie where the square's faces may. Its squares' colours are taken from
what their tops show well inside and what their sides show in most of
their tiles, counted by tiles: a side's outline is a pixel or two in every
tile, a hub many pixels in a few. Tried first and dropped: telling other
levels' art by how often a colour shows far from the floor against on it
(the squares' own colours are the lower platform's too, and Town Area 1's
walkways are the upper network's in Town Area 3), and by a tile being
nearly one colour.

A walkway's end at a room drawn apart is no dead end. Its own view takes
the room for void, so with panels laid it took its map's dead-end piece,
Park's round pad or arrow head, half under the room. Where an area has no
joint piece to stand at such an end (the Undernets' bridges end in theirs),
the walkway's panel beside the room is marked (`TILE_JOINS`) and no panel
is laid on it or round it: the end is picked tile by tile, plain, at the
room's rim. Tried first and dropped: the walkway drawn as running on under
the room (it came out over the room's rim, and Oran's circuits lost their
ends at its hubs).

Two more of the owner's points on one Park room (13 November 2026), each
mended by a rule of its own:

- **Teeth on a small room's upper-right edge** ("weird rounded zigzags").
  The map's own panels over that edge have a walkway right behind them,
  whose face reaches the wall's upper tiles, so those are not lent; the
  mirror image's are clean, but a panel off the floor was laid only beside a
  rim from its own map, and a small room's rim there is the map's. A map
  and its mirror image set the very same tile where the art is the same on
  both sides, a platform's rim: so a panel also counts as lying beside what
  is laid where its map's neighbour and the laid one hold the same tiles
  in every slot along the edge the two share (`same_edge`), whatever map
  each is from. Park's breaks fell from 158 to 142 by it, Oran's from 340
  to 331; Town's rose by 10, the countries' by 9.
- **The overhang under a platform's bottom corner** ("tile bleed
  overhang"): two tiles of the walkway BN4 joins to its one bottom corner.
  The tip's clean halves exist, the left in the map and the right in its
  mirror image, each within the 3-pixel margin of the walkway's panel on
  its own side and so not lent. A panel laid in pieces and still short of
  tiles now takes more of the pieces it has (`lay_more`): their tiles that
  lie beside a panel that differs, not on it, where the map has something
  there and the floor has not; and a tile beside its own mirror image
  counts as meeting it, no map setting the two together. Only in a piece
  drawn apart, whose every neighbour in its map is away by design: with
  floors that join, a panel's own art turns with its neighbour, and Town's
  walkways took the lattice's junctions (Town 363 to 385). Taking tiles
  that near anywhere (a margin of 1 pixel) lost a corner's mark elsewhere.

- **A walkway's end at a room's upper side** stopped in steps beside the
  room's wall, a dash of its outline on the room's rim: its end is picked
  tile by tile, and tiles end at tiles' edges. There the walkway now runs
  on one panel under the room (`pieces_floor`): its own view takes the
  room's panel just below its end for more walkway, so the end is drawn
  straight, and of its tiles in the room's panel only those that reach
  back over the room's edge are set, behind the room's; a floor's top just
  behind another's upper edge counts as behind the wall that may stand
  there (`tiles_in_front`, `TileGrid.wall`). From below a walkway ends
  under the room's side face as before. Tried first: every tile of the
  walkway's in the room's panel set behind the room's (its hidden end came
  out on top of the room: Park 127 to 182).

- **A border cut off at a room's corners** ("the blue border around: they
  are sometimes cut off or still jagged", the owner, 13 November 2026). On
  a Park room drawn alone: a wall missing its last tile at the upper
  corners, a step under the bottom corner's right half, a notch under the
  right corner. The first two had one cause: a panel off the floor is laid
  only beside one laid before it, and in one pass the first by a corner
  had none yet and stayed empty, while one laid early never took what a
  later neighbour could have handed it. So **what lies off the floor is
  laid in rounds**, while any round lays a tile, and a panel in pieces
  takes the tiles it still misses on a later round (the bottom corner's
  right half, from the mirror image's face once that lies beside it). Park
  111 to 94, a room alone 13 breaks to 6. And **what is drawn off the floor
  hangs on it**: a tile laid there that no row of laid tiles joins to the
  floor's panels is taken up again (`lift_loose`), since a map's panel
  beside a floor also holds what another floor near it draws there. Town
  363 to 356.

  The notch under the right corner is the learner's and stays (Not
  mended). Tried against the first two and dropped: a panel counted as
  lying beside a neighbour in pieces by the piece that touches it (Park's
  tip came out, Oran 331 to 345 and BN5's Undernet 190 to 208); one tile
  the same along a corner contact (nothing changed).

Breaks per 100 panels against the state before, whole panels now on in
Town too: Town Area 468 to 356, Park Area 172 to 94, BN5's Oran Area 341
to 332, BN4's countries 115 to 105, BN5's Undernet 195 to 190, BN4's
Undernet 262 to 261; the other 33 areas identical. Laying any tile of the
border's art that meets its neighbours, where the map's own is missing,
set pieces where none belonged (Town 353 to 396) and is not done.

Not mended:

- **The notch under a Park platform's right corner**, three tiles. BN4's
  platform ends 13 pixels past its floor's right corner, where a tile may
  draw 3, in bands a pixel thick (a highlight, two shades, an outline)
  that no colour of their own joins across a step: the learner's own art
  stops short of them and the three tiles are turned down. Learned as the
  floor's art (what is drawn straight under it, and an outline of 3
  lines), the corner comes out as BN4 draws it, Park 94 to 90, BN4's
  Undernet 261 to 209, the countries 105 to 90, Town 356 to 334: but BN5's
  Oran Area then takes a walkway's grey base under a room's rim for the
  room's own and lays spikes off every room (its count barely moved, 332
  to 337: the picture shows it, the breaks do not), and BN5's Undernet its
  lava. Bounding it by depth (2 to 12 pixels) or to colours no top draws
  loses either the corner or Oran. What it needs is the art told apart by
  the floor it belongs to, which the learner does not know yet (kept on
  the branch `wip/learner-under`, commit 044a017d, not pushed).
- **A seam at the very tip** under a Park platform's bottom corner, where
  the two halves meet, and a pixel of the walkway's pink.
- **Town:** where a square joins a walkway or another square, and rooms
  that are no square of three by three, which take a square's panels in
  pieces.

## The seams, and what was tried against them

The seams are counted only where a tile is partly drawn: two whole tiles
side by side never make one. Tried against a floor's inside mixing two of
the original's looks and dropped, each over the seam count, before the
breaks were measured: taking a panel's tiles from one panel
of the original (its most typical for the neighbourhood: the bridges read
whole, the seams doubled at every panel border, the original alternating
two), the same by the panel's parity (no better), costing an unseen pair
of whole tiles 1 (2570 seams to 6098 over three areas: the edges paid for
the insides; the strips cost only what shows, and the seams stayed where
they were, 81 to 84 per 100 panels), laying straight walkway runs by their period (the
originals have too few runs five panels long to learn one), and for
CopyBot Comp's plateaus a rim context (a walkway panel beside platform
floor is a rim: its rims took the plateaus' stone, but the seams grew by
60% where they met the walkways' lips and legs), and for the Undernet's
bridges their gem joints as a floor of their own (platform floor to the
tiles, at a generated bridge's ends, turns and crossings: the gems'
pieces left the straight runs, but brown notches came where a stripe
lies beside a joint corner to corner, the seams up a quarter and the
near picks by half). What holds instead keeps both one floor and gives
the joints the pads' look (`joint_hues`, `TILE_PAD` at a generated
bridge's ends, turns and crossings while the tiles are picked): the
straight runs are plain planks, and the count rose a tenth on three
layers, where two stripes' posts meet side by side, which the originals
rarely set (a joint stands between); the baseline records it.

The close-ups in `.build/atlas/defects_bXX.png` show each spot. It fails as
the atlas does, where a layer got worse than the baseline; `--baseline`
writes the new numbers.

The baseline keys a layer by its area, layout, depth and seed, not by
what was built there: a change to what a seed makes (`LAYER_MAKE`) builds
other layers under the same keys, and the comparison then holds one
layer's numbers against another's. On 4 November 2026 dev read 33
layers worse than it, all of BN6's and BN5's areas; the atlas draws the
same report twice and with any data folder, but at the commit that wrote
the baseline (#164, 26 October) 83 of its 172 layers already differed
from it and 10 read worse, and since then 179 of the 278 layers had
changed floor (`LAYER_MAKE` 106-109, services and gates rolled apart):
24 of the 25 flagged layers were other layers, the 25th flagged at that
commit already. A stale baseline, written again with BN4's areas'
polish (their BN6 and BN5 layers the same as before, byte for byte).
After a change to what seeds make, write it again (`--baseline`) once
the areas' pictures are looked over.

The check sees the classes' picks, not what is pasted over them (pads,
emblems, stairs, props), so look over the pictures too. To see why a tile
was picked, `CYBERWORLD_TILE_AT="x,y;x,y" python3 build.py atlas BIOME
SEEDS` prints every pair weighed for those tiles (tile x, y of the
uncropped layer: pixel over 8, plus the offset in its `.box` file), after
a `layer bXX_...` line for each layer.
