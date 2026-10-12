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
  any: they stand in the way on), `signature` the room the layer is
  remembered by (docs/LEVEL_DESIGN.md, Identity; `none` where its layout
  found it no place), and `arrives` the side of the screen MegaMan arrives
  at and the way he faces there (Arrivals).

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

`--atlas arrivals` reads where BN6's own maps put MegaMan as he arrives
(the same file, `navstudy_arrivals`): every jack-in destination and every
warp of another map (the real world's groups and the internet's, but a
teleport within one map and a jack-out) onto each area's original maps,
each place and facing once, the side of the map's floor on the screen it
lies at (its edge within a quarter of the floor's box, else the middle) and
whether the way it faces MegaMan points into the map; per map, per area,
and over the net areas' own maps (groups 0x90-0x96) once each. They set the
layers' arrivals (docs/LEVEL_DESIGN.md, Arrivals).

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
on those over the act's cap (stocked one, not two). Last, the Net's clock
(docs/HOME.md, going back): each act's median guardian HP at 0 to 6
notches, beside the next act's band. The whole report takes about a
minute.

Where BN5's ROM is beside BN6's, each area one of its areas dresses
(docs/MULTIROM.md) is followed by that area's battles in BN5's own engine:
for every layer of the act, the records its first battle (`opening`, the
band's lower half) and its others (`guest`) are picked from, as the
director picks them (`xrecords_pool`, `src/core/xrecords.c`: the
layer's map's records that fit the band, else the area's other maps',
else those of BN5's other areas, else the map's weakest alone), each as
likely: their viruses' HP together (lowest, median, highest), the
strongest hit, how many records and where they came from, and OVER on
any past the band.

Output: `.build/pacing.txt`. It needs the ROM, and prints how many
battles and guardians were past their band, the territories' included;
after a change to the bands, the areas or the encounter code, that number
should stay 0.

## Run log

Every battle and every finished run is appended to `runlog.txt` in the data
folder (`src/director/runlog.c`), on the handheld too: the seed, depth,
area, what kind of battle (`battle`, `challenge`, `duel` for ProtoMan's,
`guardian`), MegaMan's HP and max HP before, each foe as
family.version, the foes' HP together, and MegaMan's HP after (or
"deleted"). A battle in BN5's engine (docs/MULTIROM.md, Guest battles) has
`guest` and its record in BN5's ROM after MegaMan's HP, its viruses as `x`
and BN5's enemy id, and after MegaMan's HP at its end the reward as the
run got it: `... battle hp 300/300 guest 08121188 x1 x1 x1 foehp 120 ->
won hp 260 reward 30 zenny` (a chip as BN6's of its name, `reward Wind *`;
`reward HP+50`, which the HP after counts; `left` for an escape), and
after it a green Mystery Data's find where one stood on the field to the
end (`reward 200 zenny find 1 BugFrag`, `find Tornado L`). A last
line gives where the run ended (`run over`, or `run won`). Past 512 KB
the log moves to `runlog.old`. Collected from real runs, it shows where
runs are lost. The same moments give the anonymous statistics their
guardian and run-end events (src/analytics/, docs/GUIDE.md's Anonymous
statistics), where the player said yes.

## Statistics: asked, sent and checked

A plain start (no `--scene`, `--seed`, `--dev` or a capture's option, not
headless) is a player's: it asks once and sends where the answer is yes.
Any other start neither asks nor sends, but for these switches:

```bash
# the question, as a first start asks it (a fresh --data-dir: the answer is written there)
python3 build.py shot --data-dir /src/.build/ask --dev statistics=ask --frames 120 --shot 100:/src/.build/ask.bmp
# the controls screen's row (SELECT on the title), the answer held
python3 build.py shot --dev statistics=on --input "30:,4:START,70:,4:SELECT,60:" --shot 150:/src/.build/row.bmp
```

`statistics=ask|on|off` holds the session's answer; such a session sends
nothing but to `statsurl=URL`, a local stand-in for Umami's `/api/send`
(any server that logs what it is posted). `statscheck` sends one event,
`game-dev-check` at `/dev-check` (its system as its data), to Umami itself
whatever the answer, and nothing else: on the systems that send from a
thread of the game's (Linux, the handhelds, Windows, the 3DS) the output
says `statistics: taken (2xx) by ...` or `not taken by ...`. It is the
one way a test reaches the real Umami; keep it to a check of a system's
way there.

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

Nine more have no menu entry: `fragile` (MegaMan keeps 1 HP in battle, so the
first hit ends the run), `powers` (the five Crosses and BeastOut open from
the first battle on, for a capture of them: `tools/trailer.py` plays one),
`gem` (every random battle with a green Mystery Data on the field, to
check it and its reward; BN5's battles too, with BN5's own find), `veteran` (a profile that has met seven
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
`tools/before_after.py` compares two builds' with it), and `xcontrol`
(the control family installed as the core starts: BN6's own Mettaur
moved into the engine's free space as AI 32, ids 0xBB-0xC0, for the foes
step below and `tools/xcontrol.py`; docs/MULTIROM.md, Transplanted
families; it takes the place of BN5's CanGard, which is installed
wherever BN5's ROM is read), and `nofamilies` (no family moved into the
engine, BN5's ROM read or not: BN6's battles as the player's ROM has
them, which `tools/xfamily.py` compares with CanGard installed). BN5's battles
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
second screen, issue #75); `programs=N` gives a copy of each of the first
N NaviCust programs, in the first of its colours the ROM draws, with the
check BN6 lists them by (the NaviCustomizer on the second screen, issue
#76); `--talk bugfrag:FRAME` gives one BugFrag, as `bugfrags` gives fifty
(a DarkChip's last). `clock=N` starts the run with the Net's clock at N
notches (docs/HOME.md, going back): the guardians spawn with its HP, a
tenth more a notch. `CYBERWORLD_AUTOPILOT_BACK=N` has the autopilot take
N trips back from Lan's HP, through its older portals, before a way.

The super bosses (docs/BOSSES.md, Super bosses) have three: `side=N`
starts the run on a side layer at `--run-depth`, 1 the Undernet (its
golden gate) or 2 the Secret Area, Bass's once the title's S is held
(`--marks 08`); `bass=N` and `beast=N` make MegaMan's wins over Bass and
over the Cybeast N, their forms and words by that record (`bass=1` Bass
SP, `bass=2,beast=1` Bass BX). `--talk scrtdata:FRAME` gives the three
ScrtData a golden gate wants. With the debug output on, the music's
state every second shows the approach's fade and the silence after the
battle:

```bash
CYBERWORLD_AUTOPILOT=1 CYBERWORLD_EMU_DEBUG=1 python3 build.py shot --scene emu --run-depth 8 --seed 7 --marks 08 \
    --dev god,quiet,hp=800,side=2 --frames 4000 --shot-range 1700:4000:/src/.build/bass/f
CYBERWORLD_AUTOPILOT=1 python3 build.py shot --scene emu --run-depth 19 --seed 7 --dev god,quiet,hp=800 --frames 4000
```

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

The `foes` step (`--input`'s `0:foes BB@52/01@43`, play.py's `foes
BB@52 01@43`) starts a battle of the foes it names, at the same first
free moment: each an enemy id in hex at its panel, column 1-6 then row
1-3 (`mm@22` puts MegaMan at column 2, row 2, where he stands without),
on a plain field with nothing else on it, in BN6's engine whatever the
layer. It names the families moved into the engine (docs/MULTIROM.md,
Transplanted families) beside BN6's own: with BN5's ROM read, BN5's
CanGard is AI 32 with ids 0xBB-0xC0 (its six versions), and `0:foes
BB@52/01@43` fights CanGard beside a Mettaur; `--dev xcontrol` installs
the control in its place, BN6's own Mettaur moved as AI 32 with the same
ids. A layer dressed as BN5's area has longer arrival words, and its
random battles are BN5's: seed 15's first area is BN6's own.

```bash
CYBERWORLD_ROM_DIR=$PWD/.build/xcontrol/rom python3 build.py shot --scene emu --net-biome 0 --seed 1 --dev xcontrol \
  --input "300:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,0:foes BB@52" \
  --shot 1100:/src/.build/a.bmp
python3 tools/xcontrol.py --story     # the control against BN6's own Mettaur, frame by frame
python3 build.py shot --scene emu --net-biome 0 --seed 15 --frames 1400 \
  --input "300:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,0:foes BB@52/01@43,260:,4:START,10:,4:A,400:" \
  --shot 1378:/src/.build/a.bmp   # CanGard's reticle striking MegaMan, its muzzle's blast
python3 tools/xfamily.py              # CanGard in BN6's battles: fight, statuses, deletions, BN6's own left alone
```

The `xrecord` step (`--input`'s `0:xrecord 811DE10`, play.py's `xrecord
811DE10`; issue #82) starts one of BN5's battle records at the same first
free moment, in BN6's engine whatever the layer, as BN5's territories
will fight them (docs/MULTIROM.md, One net; `src/core/xbattles.c`): the
record's address in BN5's ROM in hex (`build.py xrecords` lists them,
below), then the versions up its viruses go (`0:xrecord 811DE10 1`: V1EX
from V1, BN5's own index). Its foes by BN6's ids (a family both games
have by name BN6's, at the tier of its version; a family moved into the
engine its installed ids), its field as BN6's, its green Mystery Data by
BN5's rule with BN5's finds, its rocks; it prints what it fights, or why
the record cannot be fought yet (a family BN6 lacks, a field with panels
BN6 has none of, a Navi) and starts nothing.

```bash
python3 build.py shot --scene emu --net-biome 0 --seed 15 --frames 1500 \
  --input "300:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,0:xrecord 811DE10,260:,4:START,10:,4:A,400:" \
  --shot 1378:/src/.build/a.bmp   # BN5's ACDC Area record: two CanGards and a Mettaur on BN5's field 0x15, holes and all
CYBERWORLD_AUTOPILOT=weak python3 build.py shot --scene emu --net-biome 0 --seed 15 --frames 1200 \
  --input "300:,0:xrecord 8121898" --shot-range 300:1200:/src/.build/r_   # won, its results: 200 zenny, then the Mystery Data's 2000
python3 build.py xrecords          # every territory record: fightable in BN6's engine, or why not
```

The `status` step (`--input`'s `0:status freeze 90`, play.py's `status
freeze 90`) gives the battle's enemies a status at once, as a hit leaves
it in their collision data for BN6's engine to act on as their next frame
starts: `paralysis`, `freeze` (an ice block round it), `bubble`, `drag` or
`slide` (pushed along a panel), for the frames named (60 where none; drag
and slide take none), and with an enemy id in hex after them only that
id's (`0:status paralysis 60 BB`). It prints how many it struck, and
names none outside a battle. It puts a family moved into the engine
through BN6's generic states, which the fight alone never reaches
(docs/ROM_DATA.md, Statuses as a hit leaves them).

`tools/xcontrol.py` copies BN6's ROM alone into `.build/xcontrol/rom`
(a layer of BN5's fights its battles in BN5's engine) and runs its
specs, `fight` and `status` (`--spec` one of them), three battles each,
with `CYBERWORLD_BTRACE=1` (docs/EMULATION.md, Testing).
`tools/xfamily.py` copies both ROMs into `.build/xfamily/rom` and runs
its specs, `fight`, `status`, `delete` and `native` (`--spec` one of
them), each battle traced and its checks printed, pictures of CanGard's
first shot and of each status in its folders (docs/EMULATION.md,
Testing). The status step prints the battle's frame it struck on, which
the `status` spec reads.

The `sig` step (`--input`'s `0:sig`, play.py's `sig`) puts MegaMan in the
layer's signature, the room it is remembered by (docs/LEVEL_DESIGN.md,
Identity), on the free panel nearest its anchor; play.py's state (with
`CYBERWORLD_STATE_POS`) names the signature and where it lies, in panels
and in the world. The `identity-*` screenshots and the `identity` clip
are made with it.

The `chip` step (`--input`'s `0:chip 204 L 29`, play.py's `chip 204 L
29`; issue #82) puts chip ID (1-511: BN6's, or one of BN5's in BN6's
tables, docs/MULTIROM.md, The chip registry) in code CODE (a letter or
`*`) into the folder's first N entries (one where none), owned as BN6
marks a chip it gives, and gives it to the pack N + 1 times through
BN6's GiveChips (outside the folder editor the pack's counts hold the
folder's copies: one is the pack's own). It prints the chip's name and
its four counts in the pack (an id from 320 counts in the pack's
extension: `0:chip 411 M 1`, MrkCan1, reads "its counts 2 0 0 0"). A
battle then deals the folder as BN6 does, with the chips that sit out
taken out of the deck (`CYBERWORLD_EMU_DEBUG=1` prints how many).

```bash
CYBERWORLD_EMU_DEBUG=1 python3 build.py shot --scene emu --net-biome 0 --seed 15 --frames 1700 --dev quiet \
  --input "300:,0:chip 204 L 29,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,0:foes 01@52,260:,4:A,10:,4:START,10:,4:A,150:,4:A,200:" \
  --shot 1150:/src/.build/a.bmp,1365:/src/.build/b.bmp   # WideSht1 L's card on the Custom screen, BN5's picture; its wave at the Mettaur
```

## BN5's records in BN6's engine

```bash
python3 build.py xrecords          # the areas' lines: how many records fightable, the families and fields holding them back
python3 build.py xrecords --all    # ... and every record's line
```

`tools/xrecords.py` (issue #82; docs/MULTIROM.md, One net) copies both
ROMs into `.build/xrecords/rom`, builds the host binary and runs its
`--xrecords PATH` (`src/dev/xrecords_report.c`, a tool run in the game's
place), which starts a core with the families installed as a run's core
starts, then lists every record of the three maps each of BN5's areas
takes its battles from: as BN5 has it (its field, its enemies by name and
id at their panels, row then column as its own bytes are, its Mystery
Data, rocks and Navis), and as BN6's engine fights it (`xbattle_translate`:
BN6's field, the foes by BN6's ids, the Mystery Data and its chance) or
why it cannot yet; per area how many are fightable, whether all are
(`xterritory_native`), the families still holding records back and the
fields of panels BN6 lacks; and, for an area with any, its random battles
as BN6's roll would pick them (`xterritory_encounter`, issue #82's M3):
six a layer at every depth of the first cycle, each record's HP against
the band, from one seed and again from it, `NOT THE SAME AGAIN` where the
two differ and `OVER` where one lies past the band. The listing is in
`.build/xrecords/list.txt`; it exits 1 where a roll was not the same
again.

## The differential: a moved family against its own game

```bash
python3 build.py xdiff                       # every spec in tests/xdiff: PASS, FAIL or INCONCLUSIVE each
python3 build.py xdiff cangard_v1 --story    # one spec, the battle as each game fought it
python3 build.py xdiff cangard_v1_keys --survey   # every difference of every frame, counted, masked or not
python3 build.py xdiff --mutations           # the comparator's own test: each break must FAIL its spec
python3 build.py shot --data-dir /src/.build/xdiff/data \
  --xdiff "foes=4F@62;mm=22;hp=100;frames=1200;out=/src/.build/xdiff/runs/s1"   # the traces alone
```

Issue #82 moves BN5's virus families into BN6's battle engine by their own
code (docs/MULTIROM.md, Transplanted families). The differential proves a
moved family behaves as it does in BN5's own engine, frame by frame. Local
only: it needs both ROMs (CI runs the unit tests of its parts).

`--xdiff SPEC` (`src/dev/xdiff.c`, a tool run in the game's place, as
`--pacing` is) fights one battle twice and traces both:

- **The reference**: BN5's battle on the guest core, as BN5 has it
  (`guest_dev_battle`, guest.c): BN5's random battles' record with the
  foes named (BN5's ids at their panels), MegaMan at the spec's HP,
  everything else BN5's own: none of what the run's guest battles change
  (the versions scaled to the act, the GAME OVER option and backdrop,
  Souls, DarkChips, the buster, the mood, the run's folder), and BN5's
  own deck compaction, the shelf's jump undone (put back before a run's
  next guest battle).
- **The subject**: BN6's battle with the families installed, the same
  foes by BN6's ids (BN5's 0x4F-0x54 are 0xBB-0xC0), forced as the foes
  step forces one (`emu_battle_force`), from BN6's boot state, with only
  the encounters' hooks and the install's (none of the run's director).
- **Alignment**: each battle's timer (`BN5_BATTLE_TIMER`,
  `BN6_BATTLE_TIMER`), never the cores' frames (BN5's Custom screen
  opens about 56 frames later). Each core runs to its fight's first frame,
  then a frame of each in turn. The Custom screen is passed the same way
  in both: once its window reads open, START 4 frames, nothing 8, A 4 (OK,
  no chip). The fight's keys go by the timer read before the frame
  (`keys=T:BUTTONS:N`, held for timer values T to T+N-1).
- **Seeding**: both RNG words written in both cores after the fight's
  first frame (`seed=P:S`); seeding as the battle begins does not work
  (BN5's opening draws 84 more). With `rng=follow`, BN6's words are
  written from BN5's after each frame both traced: BN5's buster draws one
  more number a shot than BN6's, so without it everything that draws
  later (a hit's spark) would part.
- **The traces** (`OUT.bn5`, `OUT.bn6`, their format in xdiff.c): a record
  each time the timer moves while the battle fights: the objects of the
  first, third and fourth kinds in play (each from its list node on, its
  AI's data and its collision data), the 40 panels, the order the objects
  ran in, the songs the music players began, both RNG words.
- **The field**: `field=XX` fights both on BN5's battlefield XX, BN6's as
  `src/emu/xfields.c` installs it (docs/ROM_DATA.md, Battlefields and
  their panels' types), `field=XX:YY` BN6's own field YY against it; the
  specs `field_poison`, `field_grass` and `field_ice` fight CanGard on
  BN5's poison, grass and ice, every panel's type and flags compared.
- **Breaking it on purpose**: `poke=A:V[:N]` writes into BN6's copy after
  the install (an address of BN5's in a moved block is written to its
  copy) and `call=F:T` aims the family's veneer to BN6's routine F at T.

`tools/xdiff.py` copies both ROMs into `.build/xdiff/rom`, builds the host
binary, runs each spec of `tests/xdiff` (`*.json`: the foes, MegaMan's
panel and HP, the frames, the seed, the keys, `rng`, `natives` with
their `native_attacks` and `native_effects`, and what BN5's battle is
known to do) and compares the two traces (`tools/xdiff_snap.py` the
snapshot and its differences, `tools/xdiff_masks.py` whose each is and
the masks):

- **By role, never by slot**: the first kind's objects by their enemy id,
  the third and fourth by their type (an effect object, the fourth kind's
  type 0, by its effect id in BN6's numbers too, so one spawned with the
  wrong id is a role the other game lacks), each by the order it appeared.
- **In BN6's numbers** (`tests/xdiff/legit.json`, maps, each with why):
  the enemy ids, MegaMan's NameID (0x180 against 0x1A0), CurAction (BN5's
  0-5 BN6's 0-5, 6 BN6's 8, 7 on two up; MegaMan's ready 6 against 8), the
  attack's type (0x25 against 0xD1), the muzzle's effect (0x57 against
  0x6C), the AI index (14 against 32), a sound by its song (the player it
  plays on is each song table's own). The engine's own numbers too, the
  way an enemy id is read: a collision data's alliance words (+0x30 and
  +0x34, set from a table by alliance and region, each game its own) as
  the BN6 entry at the BN5 word's place in BN5's table, and a hit's or a
  touch's word (+0x70, +0x6C: the other's +0x30 ORed in) as the OR of
  those entries, where every way of making it of the table's words gives
  the same (`alliance`); the renderer's tile place and palette slot of a
  family object as their rank among those the family's objects drawn that
  frame take (`renderer`: the renderer counts them over everything drawn
  before, so what else is drawn is no part of the family's; one not drawn
  keeps the rank of its last place); an object's slot (+3) counted from
  its own (`slot`). A difference prints BN5's value as read so.
- **Pointers by what they name**: an object by its role and part (its
  node, its AI's data, its collision data), a panel, ROM that was moved by
  where it came from, a pointer into each game's own ROM by the data there
  (32 bytes the same), a sprite's frames from its sprite's own data.
- **Each structure through its own layout** (legit.json, layouts): the
  collision data (0xA0 bytes in BN5, 0xA8 in BN6, laid out otherwise from
  +0x14) and the panels (0x24 and 0x20), each segment with the paired
  routines that show it. Where BN5 keeps in two places what BN6 keeps in
  one, BN5's parts are held to their rule (`bn5_or`: a hit's result by the
  hitter's side, +0x6C and +0x70, ORed into +0x68; its modifier, +0x18 and
  +0x19, into +0x0F), the word itself compared with BN6's. Any other byte
  outside the segments must be 0 in both (BN6's own +0x74-0x7B, the
  multiplier and elements of a hit, among them). The flag words are
  compared as bytes, never read as addresses.
- **Every difference explained** (legit.json, masks): an engine difference
  that is not the family's code, with why and the evidence (both games'
  routines by address), or it fails the spec at the first frame it shows.
  A mask names its place and may pin it further: its values (`cells`), a
  bit with its direction (`direction`: BN6 has it, BN5 not), a condition
  (`when`), whose records (`whose`). No mask applies to the family's own
  records (its objects, what they own, its attacks and effects): there
  every difference that is left after the maps fails the spec. The tool
  prints the masks it applied, none of them on the family's.
- **Whose a difference is**, from both traces: the family's where the
  reference (BN5) says the object is the family's, where the two traces
  disagree, or where only the subject has it and calls it the engine's
  (the subject's numbers are the transplant's own: a wrong id must not be
  its alibi). An object owned by another (+0x4C) is its owner's.
- **The stop rule**: where the first unexplained difference is MegaMan's
  own state (his object, or one he owns: his shots, his buster's flash,
  their sparks) or an effect of the engine's, the spec is INCONCLUSIVE from
  there, not failed: the battle is no longer the same battle for the
  family. A native enemy's own differences (a spec's `natives`, their
  attacks and effects: each game runs its own code for them) do not stop
  it: the family is compared on while its world (MegaMan, the RNG, the
  songs, the run order, the panels) stays the same, and the spec stops at
  the first difference there. After a stop the family's objects are still
  compared, and the tool says how far they stayed the same.
- **What BN5's battle is known to do** (a spec's `expect`): CanGard's
  CurActions at given frames, its reticle's appearances, path and damage,
  the sound as it fires, MegaMan's HP lost when, the virus hit only in its
  firing state, holding its guard while MegaMan is out of its row;
  checked on both traces.

`--story` prints each game's battle, `--survey` every difference over
every frame by whose and where, `--no-run` compares the last traces,
`--poke` and `--call` break the subject for one run, and `--mutations`
runs `tests/xdiff/mutations.json`, the breaks a transplant bug would make
(an immediate, a state number and an offset left as BN5's, a call aimed at
nothing or at the wrong twin, a table's entry, a number in its data, an
effect id left as BN5's alone and beside a native, a strike's effect id,
a collision region), each of which must FAIL its spec. docs/MULTIROM.md, Transplanted families, has
what they showed.

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
