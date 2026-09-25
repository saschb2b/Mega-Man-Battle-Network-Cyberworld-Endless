# The overworld

How BN6's real world is stored, and how the engine builds the town where
a run begins (`src/world/`). Lan starts there, walks across the square and
jacks MegaMan in at a public port; the game's own jack-in takes him to the
run's first layer.

## BN6's real world

Groups `0x00`-`0x06` are the real world: ACDC Town (2 maps), Central Town
(5: the town, Lan's house, his room, the bathroom, Aster Land), Cyber
Academy (11), Seaside Town (5), Green Town (5), Sky Town (4) and the Expo
Site (5). Their tables sit beside the internet's (docs/ROM_DATA.md) and
hold the same formats, with three differences:

- **256 colours.** Real-world tile sets are 8 bits per pixel: 64 bytes a
  tile, one 256-colour palette, the map entries' palette bits unused.
  While the game runs, colours 208-255 belong to its interface.
- **The layers the other way round.** The tile map's first layer is the
  ground (drawn behind), the second what stands on it (drawn in front):
  posts, signs, cones. On the internet the first layer is in front.
- **Buildings and trees are objects.** A map's ground is one slab with
  faces on its lower edges; its trees, statues and many props are map
  objects (20-byte spawn records whose first byte, 5, picks the
  map-object spawner), people are NPC scripts.

`python3 build.py world` draws all 37 maps with their walls, heights and
triggers marked, and warps Lan through each in the game.

Central Town (`0x01:0`), 132 x 72 tiles, is the town's source. Per
8-unit world cell its ground is one of seven materials, each with its own
colour indices: road (10), sidewalk (4), cobblestone (5-8), brick (80-84),
grass (32-39 and the flowers), curbs and markings (1-3), and the sky (0).
The walls ring the walkable ground; planters are walled grass. Material
edges fall on a 16-unit lattice offset by 8, and Central Town shows only
some of them:

- Roads sit in sidewalk on every side.
- Its brick plaza reaches the slab's north-east edge; brick meets sidewalk
  on its other three sides only.
- Cobblestone runs up to the slab's south-west face and meets planters
  through a curb on their south-west side.
- The slab's north-east edge is sidewalk, a curb and a one-cell grass
  strip; its south-west face is white, its south-east face dark. Its
  north-west edge lies under buildings.

## The town

`townsrc.c` learns Central Town, and its mirror image (world x, y become
-y, -x): the mirror turns the north-east planting strip into a north-west
one. Every cell gets a material from its colours and walls: off the
walkable ground a cell that shows sky is sky (the slab's faces hang into
it), else a building. It then records, along both axes, every edge the
source shows: a run of one material, up to three cells of others (curbs,
strips), a run of the next, and where on the 32-unit period the first one
ends.

`town.c` plans the town as rectangles of those materials: the sidewalk
slab, a cobbled court on its south-west edge with a planter in its far
corner, the brick plaza on its north-east edge, and (four runs in five) a
road across. Sizes vary in steps of four cells, the period of the source's
edges. Each side of a rectangle takes the edge the source shows most often
between the two materials, moved by up to two cells to where it shows it;
the mirror's edges serve only the slab's north-west side. The port stands
in the plaza's middle, Lan starts at the square's south corner, trees
stand on the paving in rows (the plaza's corners, along the road, beside
the strips), and up to four townsfolk wait by the plaza, the court and the
road, each with a line about the net.

The tiles are picked like this, for each 8 x 8 tile of the map:

1. **Key**: the plan's materials under 16 points of the tile, 8 points
   above it at both corners (a face hangs below the floor's edge, and
   which side the floor is on tells the white face from the dark one), and
   the tile's phase against the cell lattice.
2. **Candidates**: the source tiles of that key, the original's before the
   mirror's (the mirror's faces are lit from the other side); with no such
   key, those of the key of the same phase and faces that agrees best.
3. **Coherence**: the tile after the left neighbour's source tile, or below
   the upper one's, when it fits as well as the best: the source is copied
   in runs.
4. **Fit**: fewest pixels in colours the plan's material does not show
   inside itself (a curb's line on plain sidewalk, a house's wall in a
   planting strip), counted half near another material; then the same
   texture phase; then the most common tile.

Tiles in the interface's colours are never picked. A plan whose tiles do
not all match is planned again with the next seeds (up to eight), keeping
the one with fewest misses; a town has about 1,500 tiles and 10-15 of them
at corners without an exact match. The map is 110-120 x 60-70 tiles, wide
enough that the camera never shows past it.

The town takes over Central Town's own map (`0x01:0`), in the game's
tables only: its tile map pointer, coordinate data (walls around the
walkable cells and the port's jack-in trigger), NPC list, map scripts
(none), map objects (the trees, the game's own tree object `0x7D`),
sprite list, jack-in table and music (Central Town's theme, `0x03`). Its
data lives apart from the layers' (docs/EMULATION.md), since the first
layer is built while Lan still walks the town.

## The jack-in

Pressing R in the real world, the game reads the trigger cell under Lan:
a value `0x40` + n is jack-in point n, unless event flag `0x16D0` + n is
set. The map's jack-in table (per map, in GameState `+0x64`) turns n into
one of 43 20-byte destinations (WarpData: group, number, departure,
facing, x, y, z; then the index of Lan's "Jack in!" line), and the game
plays its jack-in cutscene to it. The town's trigger is a 7 x 7 ring of
cells around the port (value `0x40`), its table points n = 0 at
destination 42, and the engine rewrites destination 42 (a comp the run
never visits) to the first layer's arrival. When MegaMan arrives there,
the run goes on as from any layer (docs/EMULATION.md).

`CYBERWORLD_AUTOPILOT` walks Lan to the port and presses R.
`python3 build.py town` draws the town for a few seeds, tiles without a
match marked red, and shows the game around the first.
