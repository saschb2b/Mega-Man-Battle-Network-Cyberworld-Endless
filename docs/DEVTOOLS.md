# Dev tools

Tools for checking generated layers and a run's difficulty without playing
it out: the atlas and the tour look over every area at once, the pacing
report checks every act's battles and guardians, the run log records real
runs, and the dev menu shortens a run on the handheld.

## Atlas: every layer drawn

```bash
python3 build.py atlas [BIOMES] [SEEDS]
```

Builds each area's layers without running the game (`src/dev/atlas.c`): every
layout the area uses at depth 2, plus its guardian's layer at depth 3, drawn
with the area's own tiles and palettes, the second tile layer over the first
as the game shows them. All 19 areas take about 35 seconds.

Output in `.build/atlas`:

- `sheet_bXX.png`: one sheet per area, each layer small next to a 2x crop of
  its densest part.
- `src_bXX_GG_N.png`: the area's original maps (group, number), the second
  tile layer over the first as well, to hold the layers against; with
  `_zZ`, the view of its floor at height Z brought down to the ground, in
  which the tiles of a raised floor are learned.
- `bXX_lL_dD_sS.png`: each layer whole, at 1x, for zooming in.
- `seams_bXX_...png`: the same with the seams marked in red: tile edges
  along the floor's edges where two tiles meet as no original map shows
  them, or where one draws
  floor up to its own edge above or below an empty tile (the steps along a
  platform's lower edges). Framed magenta, the tiles drawn with other floors
  than they show (their pair's neighbourhood would have shown, over more
  than 3 of their pixels, another panel's top or side face, or another
  floor: a wedge of one floor in the other, an edge cut short), and yellow,
  those off by 3 pixels or fewer. A cyan dot in a tile's corner: set whole
  over the classes' picks (a pad's stamp, an emblem, a stair).
- `defects_bXX.png`: close-ups of the magenta spots of all the area's
  layers, most tiles first, each as drawn and as marked, named by layer and
  place.
- `cells_bXX_...txt`: the layer's floor as text, a row per grid y: `a`
  platform, `b` walkway, `s` walkway floor across a platform, `p` a pad,
  upper case (`*` on void) where a tile drawn off lies; `src_bXX_...txt`
  the original's panels the same way. `offs_bXX_...txt` lists the magenta
  tiles (pixel x, y before the crop, and why, below).
- `report.txt`: a line per layer with its panels, rooms, how many tile picks
  were near misses or fallbacks, how many seams are left, the floor cells
  changed to be drawable and the panels whose neighbourhood no original
  shows (docs/LEVEL_DESIGN.md), the tiles picked in colours the area's own
  map never shows on its floors (other colours: pieces of another surface,
  0 but for Seaside's yellow panels, `TILES_MORE_COLOURS`), the scenery
  placed, whether a guardian layer has its arena, its stairs, and how long
  the layer took to build as the game builds it. Seams, other colours,
  `off near` and `off edge` count the map as drawn, not the tiles a pad's
  or a stair's stamp covers; `off near` and `off edge` are the magenta and
  yellow tiles, `at mouths` the services and navis
  beside a panel-wide stretch of the floor as drawn (the atlas fails on
  any: they stand in the way on).

The build prints the report and flags layers that were not built, guardian
layers without an arena and fallbacks above 1%. Objects are marked: blue the
arrival, green the exit, red the guardian, yellow shops and program
traders, pink the heal pad, white Mystery Data.

`BIOMES` is `all` or a comma list of area numbers (`0,5,13`); `SEEDS` the
number of seeds per layout (default 1). `all` also draws the areas another
game lends a run, where its ROM sits beside BN6's (docs/MULTIROM.md),
numbered after BN6's 19 in the order `src/core/rom.c` lists them (BN5's
from 19), each laid out by the rules of the BN6 area it dresses, its
guardian's layer too; `aN` draws another game's area N alone. `gNN` draws
every map of group 0xNN instead (`src_gNN_N.png`), for choosing the maps
an area learns from.

The atlas is also the tiles' regression check: each layer's near misses,
fallbacks, seams, inexact panels, tiles in other colours and tiles drawn off
are compared with `tests/atlas_baseline.txt`, and the build fails (exit 1)
listing every layer that got worse than it by more than a little (2 points
of near misses, 0.15 of fallbacks, a tenth more of the rest). After a change
that improves the tiles, `python3 build.py atlas --baseline` writes the new
numbers; commit them with the change. The baseline holds counts only,
nothing from the ROM.

## Tile test: clean tiles in every area

```bash
python3 build.py tiles [SEEDS] [--baseline]
```

The atlas of every area and layout (2 seeds each by default, about three
minutes), and in the areas that have stairs (Sky, the Undernet) as many
layers that climb one, then a table per area: its layers and panels, the tiles drawn
with other floors than they show and the seams per 100 panels, and why the
tiles were drawn off (`src/map/tiles.h`, `TILE_WHY_*`):

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

The seams are counted only where a tile is partly drawn: two whole tiles
side by side never make one, so a floor's inside can mix two of the
original's looks unseen by the count (the Undernet's bridges, a stripe
from one panel beside a gem's corner from another). Tried against that
and dropped, each over the count: taking a panel's tiles from one panel
of the original (its most typical for the neighbourhood: the bridges read
whole, the seams doubled at every panel border, the original alternating
two), the same by the panel's parity (no better), costing an unseen pair
of whole tiles 1 (2570 seams to 6098 over three areas: the edges paid for
the insides), laying straight walkway runs by their period (the
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

The check sees the classes' picks, not what is pasted over them (pads,
emblems, stairs, props), so look over the pictures too. To see why a tile
was picked, `CYBERWORLD_TILE_AT="x,y;x,y" python3 build.py atlas BIOME
SEEDS` prints every pair weighed for those tiles (tile x, y of the
uncropped layer: pixel over 8, plus the offset in its `.box` file), after
a `layer bXX_...` line for each layer.

## Nav study: the way across BN6's own maps

`build/linux/cyberworld --headless --rom-dir DIR --data-dir DIR --atlas nav`
reads each area's original net maps (the maps whose random battles it
fights, `NetAreaDef.battles`), lays the floor their walls give out a panel
at a time, walks between each map's two farthest panels, and prints the
one-wide walkways that walk crosses, of any platforms and between big
ones, and its length: per map, then the area's average
(`src/dev/navstudy.c`, the count `grid_way_narrows` in `net_way.c`, which
the unit tests run on the layers too: `CW_WAY_STATS=1 build/host/test_core`
prints them per area). The originals' counts set `narrow_cap`
(docs/LEVEL_DESIGN.md, Navigation).

## Pacing report: every act's battles and guardians

```bash
python3 build.py pacing
```

Rolls the random battles of every area in every act it can visit, on the
first two cycles, without running the game (`src/dev/pacing_report.c`):
300 rolls per layer, the run's opening battles and a Server challenge. For
each it prints the act's band (docs/PROGRESSION.md), the viruses' HP
together (lowest, median, highest), the strongest hit and how many battles
reached each version, and marks OVER where a battle lies past the band or
its damage cap. A challenge that meets an SP Navi is counted apart. Then it
draws 500 runs and lists the guardian each act met, with version and HP,
marked OUTSIDE when one lies past the act's band, and the Net Dealers'
answers: per act and element, the chips 300 layers list first, with a `+`
on those over the act's cap (stocked one, not two). The whole report takes
about a minute.

Where BN5's ROM is beside BN6's, each area one of its areas dresses
(docs/MULTIROM.md) is followed by that area's battles in BN5's own engine:
for every layer of the act, the records its first battle (`opening`, the
band's lower half) and its others (`guest`) are picked from, as the
director picks them (`guest_pool`, `src/emu/guest.c`: the layer's map's
records that fit the band, else the area's other maps', else those of
BN5's other areas, else the map's weakest alone), each as likely: their
viruses' HP together (lowest, median, highest), the strongest hit, how many
records and where they came from, and OVER on any past the band.

Output: `.build/pacing.txt`. It needs the ROM, and prints how many
battles and guardians were past their band, the territories' included;
after a change to the bands, the areas or the encounter code, that number
should stay 0.

## Run log

Every battle and every finished run is appended to `runlog.txt` in the data
folder (`src/director/runlog.c`), on the handheld too: the seed, depth,
area, what kind of battle, MegaMan's HP and max HP before, each foe as
family.version, the foes' HP together, and MegaMan's HP after (or
"deleted"). A battle in BN5's engine (docs/MULTIROM.md, Guest battles) has
`guest` and its record in BN5's ROM after MegaMan's HP, its viruses as `x`
and BN5's enemy id, and after MegaMan's HP at its end the reward as the
run got it: `... battle hp 300/300 guest 08121188 x1 x1 x1 foehp 120 ->
won hp 260 reward 30 zenny` (a chip as BN6's of its name, `reward Wind *`;
`reward HP+50`, which the HP after counts; `left` for an escape). A last
line gives where the run ended. Past 512 KB the log moves to
`runlog.old`. Collected from real runs, it shows where runs are lost.

## Frame log: pacing on a player's machine

`CYBERWORLD_FRAME_LOG=1` prints a line a second of how frames reached the
display: how many were shown, the game frames played so far, and the gaps
between shown frames (shortest, longest, and how many fell under 12.5, 20
and 30 ms and over), then a frame's update, drawing and present, and the
longest update and drawing of the second (a hitch the means hide: BN5's
boot, docs/MULTIROM.md, ran beside BN6's frames without one). 60 shown
with gaps near 16.7 ms is the GBA's pace;
fewer shown with more played is the loop catching up (a slow machine, a
missed refresh). On the Steam Deck OLED in Desktop Mode it showed 60 shown
and played a second, gaps 15-18 ms, after the first layer's first seconds
(27 shown, 52 played, while the layer is made).

## Town and world: the real world

```bash
python3 build.py town [SEEDS]
python3 build.py world
```

`town` plans the towns runs of seeds 1, 2, ... start in (docs/OVERWORLD.md)
and draws each with its original's tiles, tiles without a matching source tile
marked red, Lan's start blue, the jack-in green, trees green, other
objects blue and people yellow (`.build/town/town_sNN.png`), then starts a
run in the town and shows the game at six places around it (`tour.png`).
`world` draws the real world's 37 original maps as they are stored
(`.build/world/world_GG_N.png`, and `_coords` with walls red, raised floor
blue and triggers yellow) and warps Lan through each in the game
(`tour_GG_N.png`). `CYBERWORLD_TOWN_STYLE` and `CYBERWORLD_TOWN_VARIANT`
fix the town's plan, `CYBERWORLD_TOWN_DEBUG=1` prints it and marks the
tiles hints picked, `CYBERWORLD_TOWN_TILE=X,Y` prints how tile (X, Y) was
picked.

## Tour: the game shows every room

```bash
python3 build.py tour [BIOMES]
```

Runs the game headlessly and warps MegaMan through each area's layer
(`src/dev/tour.c`): one layer per area at depth 2, every room in turn, the
frame saved once the map has settled. Random battles are off and MegaMan
cannot be deleted. Two areas take about 6 seconds.

Output: `.build/tour/tour_bXX.png`, the rooms of one area at 2x, four to a
row. This is what the game itself draws: objects, NPCs, decoration and
collision problems show up here that the atlas cannot show.

## Dev menu: shorter test runs

Hold **Select** and press **R**, in battle or on the net, a battle in
BN5's engine too (docs/MULTIROM.md, Guest battles). The game holds still
while the menu is open; **B** or Select+R closes it.

| Item | Effect |
| --- | --- |
| Can't die | MegaMan's HP stays full, in battle and out |
| One-hit enemies | enemies keep 1 HP: any hit deletes them |
| Random battles | off: no random battles (guardians and challenges still fight) |
| Speed | 1x, 2x, 4x or 8x the game's speed |
| Win this battle | every enemy's HP to 0; they are deleted once the turn runs |
| Heal | MegaMan's HP full |
| +10000 zenny | through the game's own give-zenny script |
| Next layer | the next layer down, at once |
| Next guardian | the layers down to the next guardian, arriving before its room |
| Area | the area of the next layers (Next layer to go there); "the run's" to return to the run's own |

Switches reset when the game restarts. The same switches start on from the
command line:

```bash
python3 build.py shot --scene emu --dev god,onehit,quiet,speed=4
```

Eight more have no menu entry: `fragile` (MegaMan keeps 1 HP in battle, so the
first hit ends the run), `powers` (the five Crosses and BeastOut open from
the first battle on, for a capture of them: `tools/trailer.py` plays one),
`gem` (every random battle with a green Mystery Data on the field, to
check it and its reward), `veteran` (a profile that has met seven
guardians, found two Spins and won three of the rival's duels, where it
has none: the PET's mails for `build.py screenshots pet`) and `duels=N`
(the rival's wins made N: `duels=2` brings ProtoMan's netbattle, or his
words naming the third act before it, `duels=1` opens the official gates
of level 1; docs/RIVAL.md), and `hp=N` (MegaMan's max HP made N, and his
HP with it whenever the game moves the max: a later act's guardian, which
a headless start meets at 100 HP, fought at a playtester's HP; `hp=N/H`
holds his HP at H through the run's first 600 frames, a battle begun
hurt), and
`pieces=MASK` (the set pieces in MASK on every layer whose area has them,
where they find room: 1 purple data, 2 a Rush gap, 4 teleports, 8 a Link
Navi obstacle, 16 a security cube, 32 an arrow lane, 64 an invisible
path; `pieces=32` to look at an area's arrow lane and ride it;
docs/LEVEL_DESIGN.md, Set pieces), and `mapall` (the layer's map whole,
as if every panel were seen: a capture of the map and its way, as
`tools/before_after.py` compares two builds' with it). BN5's battles
have four of their own (docs/MULTIROM.md, Guest battles, For tests):
`darkchips=MASK`, `worried`, `souls=MASK` (a new run holding BN5's
Souls, bit k Team Colonel's Soul 7 + k) and `slowboot=N` (BN5's first
boot at N of its frames a frame on the main thread, the same frames every
run, so a fresh data dir's first BN5 battle waits behind the older net's
screen: with `CYBERWORLD_AUTOPILOT=1` paging the arrival's words,
`--net-biome x0 --dev quiet,slowboot=12 --input "300:battle"` opens it at
frame 370 and the battle at about 640). `darkchips=MASK` holds BN6's
own DarkChips of those kinds too (0x2 DrkSword, 0x4 DarkInvs, 0x40
DarkThnd, 0x80 DrkRecov: 0xC6 all four), which the first layer puts in
BN6's Pack (docs/META.md, BN6's own DarkChips). `folder=ID` makes the
run's folder thirty of chip ID in * (`folder=286`: every hand DrkSword,
for a capture of one chip in BN6's battles), `folder=ID/N` its first N
entries alone; `pack=N` puts a copy of each of the first N chips in the
Pack, in its first code (the folder editor's Pack and its scroll on the
second screen, issue #75); `--talk bugfrag:FRAME` gives one BugFrag, as
`bugfrags` gives fifty (a DarkChip's last).

Can't die, One-hit enemies, `fragile`, Win this battle and Heal work the
same in a battle on the guest core: on BN5's battle objects, laid out as
BN6's (`src/emu/bn5.h`), after each of its frames.

## A battle on demand

```bash
CYBERWORLD_AUTOPILOT=1 python3 build.py shot --scene emu --net-biome x0 --dev quiet --input "300:battle" --shot 1100:/src/.build/a.bmp
python3 tools/play.py do NAME "battle; wait 200"
```

The `battle` step (`--input`'s `0:battle`, which `300:battle` waits 300
frames before; play.py's `battle`, beside `place` and `flags`) starts the
layer's next random battle at the first moment MegaMan stands free on its
map: no chat, the director's words or a scene, no warp, no guardian's
staging or challenge under way. It waits that long: an act's arrival
words must be paged first (the autopilot pages them above; A in the
script does too).
On a layer whose battles are BN5's (its territory, docs/MULTIROM.md) it is
a battle in BN5's engine on the guest core; elsewhere BN6's own, the
record the roll would have handed out next, forced past the roll as the
guardians' are (with `--dev quiet` too). `--talk guest:FRAME` begins a
guest battle at a frame of the layer instead. With
`CYBERWORLD_EMU_DEBUG=1`, a guest battle prints the record it starts from,
its folder, a line every 30 of its frames (its game mode and sub-mode, the
battle's phase, MegaMan's HP, the Custom gauge, the battle's clock, his
mood) and its end, and with Souls held whether UNITE stood on its first
Custom screen and each unison; play.py's state says `battle (the older
net's)` while it runs.

## Sprite gallery

L and SELECT together on the title screen (SELECT alone opens the
controls screen), or `--scene gallery`: every ROM sprite's animations, L
and R the category, Left and Right the sprite, Up and Down the animation,
A the palette, B back to the title.

## A controller in a test

```bash
python3 build.py shot --scene title --pad xbox --input "60:,4:START,60:,4:pad.back,20:,4:pad.dpdown" --shot 200:/src/.build/a.bmp
python3 build.py shot --scene emu --run-depth 1 --pad nintendo --input "420:,6:pad.a" --shot 460:/src/.build/a.bmp
python3 tools/play.py start NAME -- --pad playstation
python3 tools/play.py do NAME "press pad.back; press key.J"
```

`--pad KIND` (xbox, playstation, nintendo or generic) attaches one of
SDL's virtual controllers, named and numbered as that kind is (an Xbox 360
pad, a PS4 pad, a Switch Pro Controller), so the game opens and reads it
as a player's (`pads.c`): its family and labels, the controls screen,
`pad.ini`, which the data folder then holds headless too (written with
the defaults where missing, and when the controls screen keeps a map).
Steps of `--input` and play.py's commands take `pad.NAME`, that pad's input
by SDL's name held for the step (`pad.a`, `pad.dpup`, `pad.lefttrigger`,
`pad.-lefty`), and `key.NAME`, a key (`key.J`, `key.Return`), both through
SDL's events as a player's, past any map; the GBA's own names (`A`,
`UP+RIGHT`) still inject the buttons themselves. It needs SDL 2.24 (every
build but the handhelds' PortMaster binary, linked against 2.0.14, where
`--pad` says it has none).

## How the switches work

- **Random battles** sets event flag 0x1700 every frame, one of the flags
  `checkThenStartBattle` (0x08005A8C) returns early for. The game clears it on
  entering a map, which is why it is set again each frame. Flag 0x173E, tested
  just after it, is the SlipRun state and is cleared by the player's own
  update each frame, so it cannot hold battles off. Forced battles (guardians,
  challenges) branch past these tests.
- **Can't die** writes the Navi stats' HP (0x020047CC +0x40 from +0x42) and,
  in battle, the HP of the T1 battle objects on MegaMan's side (0x0203A9B0,
  0xD8 bytes each: +0x16 alliance, +0x24 HP, +0x26 max HP).
- **One-hit enemies** and **Win this battle** lower the HP of the enemy side's
  T1 objects.
- **Speed** runs several emulator frames per frame shown; the director and
  the dev tools step after each.
