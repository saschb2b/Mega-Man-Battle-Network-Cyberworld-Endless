# The overworld

How BN6's real world is stored, and how the engine builds the town where
a run begins (`src/world/`): Central Town or ACDC Town, cut into pieces
and set out again. Lan walks to the town's landmark (the blue bird statue,
the squirrel statue) and jacks MegaMan in; the game's own jack-in takes
him to the run's first layer.

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
- **Buildings are tiles, split across both layers.** Walls and the lower
  part of a building are in the first layer, over cells Lan cannot enter;
  roofs, signs and upper walls in the second, and coordinate section 2
  marks the cells behind them, where the game draws Lan, people and
  objects behind the second layer. Trees, statues and a few props are map
  objects (20-byte spawn records whose first byte, 5, picks the
  map-object spawner, then x, y, z in 16.16 and the object's id; the id
  indexes OverworldMapObjects at `0x0A4F24`, 16 bytes each: category,
  sprite, animation, ...). People are NPC scripts.

`python3 build.py world` draws all 37 maps with their walls, heights and
triggers marked, and warps Lan through each in the game.

Central Town (`0x01:0`), 132 x 72 tiles, is one of the town's two
sources. Per 8-unit world cell its ground is one of seven materials, each
with its own colour indices: road (10), sidewalk (4), cobblestone (5-8),
brick (80-84), grass (32-39 and the flowers), curbs and markings (1-3),
and the sky (0). The walls ring the walkable ground; planters are walled
grass. Material edges fall on a 16-unit lattice offset by 8, and Central
Town shows only some of them:

- Roads sit in sidewalk on every side.
- Its brick plaza reaches the slab's north-east edge; brick meets sidewalk
  on its other three sides only.
- Cobblestone runs up to the slab's south-west face and meets planters
  through a curb on their south-west side.
- The slab's north-east edge is sidewalk, a curb and a one-cell grass
  strip; its south-west face is white, its south-east face dark. Its
  north-west edge lies under buildings.

## The town

A run's town is one of two styles, picked by its seed: Central Town
(`0x01:0`) or ACDC Town (`0x00:0`), both 132 x 72 tiles. A style's town is
its original cut into pieces and set out again, in the original's own map
(its tables in the game, its music): the generator arranges Capcom's
buildings and does not make new ones. `town.c` plans a town as a list of
pieces over 8-unit world cells:

- **Copied pieces**: a rectangle of the original moved as a whole. It
  brings its first layer's tiles where a tile's middle lies in it, and
  its second layer's art where that art stands in it: the second layer
  is cut into 8-connected runs of tiles, each standing on the cell under
  its lowest pixels (its foot), so a roof goes with its house, not with
  the street behind it. It also brings its walkable cells (floor its
  walls ring, less the wall cells; walls that an event flag lifts do not
  count), its section-2 cells (its own and those under its art), its
  jack-in cells and checks (section 3), and its trees, statue and Chip
  Trader (objects `0x7D`, `0x7E`, `0x16`, `0xBC`; never the story's
  gates and barriers).
- **Tiled pieces**: a strip of the original repeated, to stretch ground
  that is the same all along the strip: a road, the plaza's brick, the
  houses' cobbled court. Its tiles are picked (townsrc), each hinted the
  source tile a few whole steps back along the strip: 8 cells in brick
  and cobblestone (their pattern's period), else 2 (the tile grid's).

A move keeps the tile grid when its cells add up even (dx + dy), and the
brick's and cobblestone's pattern when both are multiples of 4 and differ
by a multiple of 8. The pieces meet only along cuts through uniform ground,
and a cut must stay a cell clear of anything drawn across it (a parking
box's lines, a building's back wall), since tiles straddle the cells.

**Central Town** is the original widened by 0 or 8 cells: the Expo gates
move away from the Academy along the north-west edge (and the verge
between them gets a row of trees), the plaza's brick and the main road
grow wider, and so does the houses' court. Lan steps out of his house
door where the game puts him (`0x01:1`'s warp back), facing away from it,
and jacks in before the blue bird statue on the plaza.

**ACDC Town** is a row of blocks between roads: the park with the
squirrel statue and Higsby's shop, the L of houses round the Metroline
with Dex's block in its notch, the Ayano mansion at the end. Half the
time the first two trade places, the road between them going with them;
below the avenue's near curb the cuts run clear of the crossings, so a
crossing and its landing stay at the corner of their block. Lan comes up
the Metroline's stairs, and jacks in beside the squirrel statue or at the
doghouse (the original's second point, made the first's).

Once Lan is out, he and MegaMan talk over the PET: on the very first run
Dad calls about the new net under the town, later runs have a word of
their own (and, after the Nest has fallen, of the net rebuilding). In
both, the townsfolk stand where they belong, each with their own face in
the chat box and saying something
about the place (the game's generic people, never its story's, each out
one run in four; a Mr. Prog explains the jack-in), a few
pace a sidewalk up and back as the game's own walkers do (NPC command
`0x38` sets a direction, speed and number of steps, `0x39` walks on or
back), and everything answers A: the original's checks (section
3 triggers `0xF0` + n in front of houses, shops, signs and statues) keep
their cells and say the town's own lines. Standing on one and pressing A,
the game skips it if event flag `0x16C0` + n is set (the town clears them),
then runs text script `table[n]` of the map's per-map check table
(`0x0803461C`: per group, per map, 16 bytes, `0xFF` none) from the map's
text archive, which it decompresses to `0x02033400` on entering the map
(the archive proper at `+4`, after a word holding its size). The town
writes both.

The tiles townsrc picks for the stretched ground: `townsrc.c` learns the
original, and its mirror image (world x, y become -y, -x). Every cell gets
a material from its colours and walls. Central Town: road (colour 10),
sidewalk (4), cobblestone (5-8), brick (80-84), grass (32-39 and the
flowers), curbs and markings (1-3). ACDC Town: road (6, its joints
182-186), crossings (10), sidewalk (1-3), lawn and bushes (11-15, 89-92),
the park's dirt (121-123). Both: the sky (0); off the walkable ground a
cell that shows sky is sky (the slab's faces hang into it), else a
building. For each 8 x 8 tile of the map:

1. **Taken whole**: a copied piece's tile is the original's.
2. **Key**: the plan's materials under 16 points of the tile, 8 points
   above it at both corners (a face hangs below the floor's edge), and the
   tile's phase against the cell lattice.
3. **Hint**: the hinted source tile, if its key is the same, it fits as
   well as the best of that key, its own pixels show the key's grounds as
   well as any, and the original shows it more than once.
4. **Coherence**: the source's own neighbour of a neighbour's source tile
   (a tile taken whole on any side first, then the picked tile on the
   left or above), if its key is the same, it fits as well as the best and
   the original shows it more than once.
5. **Fit**: fewest pixels in colours the plan's material does not show
   inside itself (colours of at least 1 in 400 of its pixels), counted
   half near another material; then the same texture phase; then the most
   common tile.

Tiles in the interface's colours (208-255) are never picked, and the
second layer of stretched ground stays empty (no posts or cones made up).
The map holds the floor with room for the camera (136 pixels either side,
104 above and below) and every copied tile, and the whole plan moves by
a pattern-keeping step to sit in the map's middle (the camera's bounds are
symmetric around it): Central Town 142 x 82 tiles (152 x 84 widened),
ACDC Town 156 x 88.

The town takes over its original's own map, in the game's tables only:
its tile map pointer, coordinate data (walls around the walkable cells,
section 2, the jack-in cells and checks), NPC list, map scripts (none),
map objects, sprite list, text archive and check table, a warp list whose
every entry leads back to where Lan starts, jack-in table and music
(Central Town's theme `0x03`, ACDC Town's `0x24`). Its data lives apart
from the layers' (docs/EMULATION.md), since the first layer is built while
Lan still walks the town.

## Further towns (planned)

The roadmap's more starting towns (docs/META.md) would come from the other
real-world groups, each first as its original whole (ACDC Town's first
variant is that), then cut like the two. Found so far (`build.py world`,
and the section 3 values dumped per cell):

- **Seaside Town** (`0x03:0`, song `0x06`): jack-in points 0x40 (cells x
  -21..-14, y -22..-21, before the fish shop) and 0x41 (a ring, cells x
  2..6, y -10..-6, round an object at the plaza's east edge, whose check is
  f7); the mermaid fountain is a check ring (f0, cells x -19..-8, y
  -17..-6), a landmark as the statues are. The plaza and the fish shop lie
  at height 0, the walkway over the whale and the station at 64, the pier
  at -32: Lan should start at height 0, since the town's warp entry holds
  no height.
- **Green Town** (`0x04:0`, song `0x08`): one jack-in point, 0x40, a ring
  of 56 cells (x -56..8, y -192..-128 in world units) round a large object.
- **Sky Town** (`0x05:0`, song `0x07`).

All three stand on more than one height (Seaside's walkway and station at
64 and its pier at -32, Green Town's stumps, Sky Town's decks), which the
town's plan does not model: it rings the walkable cells with walls on one
floor. A trial of Seaside Town copied whole, with those walls, drew right
(the copy must reach cells -62..62: its roofs' and the whale's art stands
on cells far up the picture) but typed its chat ten times slower than
Central Town, and its raised floors had no heights. A third town wants
the original's own walls and heights (sections 0 and 1) carried with the
copied pieces first.

The songs are the per-map bytes of the map music lists
(`0x080360E4`: a list pointer per chapter byte, the later chapters' from
index 0x10 on, docs/ROM_DATA.md), in the chapters where the town is open.

## The jack-in

Pressing R in the real world, the game reads the trigger cell under Lan:
a value `0x40` + n is jack-in point n, unless event flag `0x16D0` + n is
set. The map's jack-in table (per map, in GameState `+0x64`) turns n into
one of 43 20-byte destinations (WarpData: group, number, departure,
facing, x, y, z; then the index of Lan's "Jack in!" line), and the game
plays its jack-in cutscene to it. The town keeps the style's points
(Central Town's 4 x 4 cells on the plaza, not the story's second one in
front of Aster Land; ACDC Town's 2 x 5 beside the squirrel statue and
4 x 2 at the doghouse) and makes the landmark's whole front one too: the
original's points lie a few cells from the statue, with the statue's own
check between, where a player walks up and presses R. Every cell is value
`0x40`; its table
points n = 0 at destination 42, and the engine rewrites destination 42
(a comp the run never visits) to the first layer's arrival. When MegaMan
arrives there, the run goes on as from any layer (docs/EMULATION.md).

A new run plans the other town than the last run did (`town_style_for`,
with the last run's first area and guardian in `run_new_varied`): a
playtester began on Central Town's street by Lan's house four or five runs
running.

`CYBERWORLD_AUTOPILOT` walks Lan along the streets to the landmark's front
(a breadth-first path over the town's walkable cells) and presses R.
`python3 build.py town` draws the towns runs of seeds 1, 2, ... start in,
tiles without a match marked red, objects and people dotted, and shows
the game around the first; `CYBERWORLD_TOWN_STYLE` (0 Central, 1 ACDC) and
`CYBERWORLD_TOWN_VARIANT` fix the plan, `CYBERWORLD_TOWN_START=x,y` where
Lan starts (the original's world units), `CYBERWORLD_TOWN_DEBUG` marks
hinted tiles and trigger cells and prints the plan,
`CYBERWORLD_TOWN_TILE=x,y` how one tile was picked.
