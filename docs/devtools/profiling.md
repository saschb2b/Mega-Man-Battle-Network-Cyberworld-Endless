---
type: Tool
title: "Profiling: where a start's instructions go"
description: "How the time to the first layer is profiled (Valgrind's callgrind on the host build, in the build image, counting each function's instructions), what the profile of 14 November 2026 found, what was made faster, and the hot spots left."
tags: [devtools, performance, layers]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-14T17:28:30Z }
---

# Profiling: where a start's instructions go

A run's first layer took about 20 seconds to make on the New 3DS
("Building the net...", `3ds/README.md`), and CONTINUE as long. The slow
systems' time follows the instructions run more than anything this
machine's clock says, so the start is profiled by counting them:
Valgrind's callgrind, each function's own count and what it calls.

`perf` needs `kernel.perf_event_paranoid` at 1 or below, which the
machine's owner sets; callgrind needs nothing, in a container.

## Running it

The host build runs in an image made from the build image with Valgrind
added (kept out of git, in `.build/`):

```sh
python3 build.py host
mkdir -p .build/prof/img
printf 'FROM cyberworld-build\nRUN apt-get update -qq && apt-get install -y -qq valgrind\n' > .build/prof/img/Dockerfile
docker build -q -t cw-layer-prof .build/prof/img
# a start that makes the boot state first, then the one measured
build/host/cyberworld --headless --rom-dir ROMS --data-dir .build/prof/d --scene emu --frames 2
docker run --rm --name cw-layer-prof --user $(id -u):$(id -g) -v "$PWD":/src -v ROMS:/rom:ro -w /src cw-layer-prof \
  valgrind --tool=callgrind --callgrind-out-file=/src/.build/prof/cg.out \
  build/host/cyberworld --headless --rom-dir /rom --data-dir /src/.build/prof/d --scene emu --frames 2
docker run --rm -v "$PWD":/src -w /src cw-layer-prof callgrind_annotate --inclusive=yes .build/prof/cg.out | head -60
```

`--frames 2` ends the run once the first layer stands: the scene's
`enter` makes it before the first frame. `ROMS` is a folder with BN6
alone, or with BN5 and BN4 beside it (their installs and hashes count).

A change that is to leave every layer as it was is checked by the
[atlas](atlas.md): every area's layers drawn by the old binary and the
new, `--atlas OUT:all:1` (its last number the seeds a layout), the pictures compared byte for byte and the
reports with their `built in N ms` taken out.

## The lab

One start measures one layer of one area, with the boot, the ROMs' hashes
and the installs around it. The lab measures every area's layers, without
the game, and checks that a change leaves them as they were:

```sh
python3 tools/layer_bench.py                        # origin/dev against this tree, every area, a seed a layout
python3 tools/layer_bench.py --base HEAD~1 --biomes 8 --seeds 3 --callgrind
python3 tools/layer_bench.py --biomes a5            # another game's area 5 (NET_AREAS + 5: BN5's Undernet)
```

`tools/layer_bench.py` builds the base commit once (a worktree in
`.build/layer-bench/src`) and this tree, has each draw every area's layers
([the atlas](atlas.md)), and fails where any picture, any text file, any
report line but its times or the coordinate data's hash differs. Then it
sums each part of making a layer over the layers, the base's against the
change's: learning an area's maps, the floor made drawable (`legal_fix`),
the tiles picked (`tilemap_pick`), the coordinate data (`coords_write`).
Milliseconds come from the atlas's report (this machine's, and noisy);
`--callgrind` counts instructions as well, exact, in the build image with
Valgrind (`cw-layer-prof`, made the first time), on fewer seeds since it
runs some fifty times slower. The atlas makes each layer twice, as the
game does and again measuring its tiles, so its counts hold both.

On a player's system the same parts are in `log.txt`, a line a layer:
`net: layer made in 47 ms: maps learned 13, floor 14, tiles 18,
coordinates 2` (`src/director/director_layer.c`; the maps learned only
where their area's were not kept).

Over every area (a seed a layout, 139 layers; 14 November 2026, this machine)
the picture is not the one a start in one area gave: the tiles' pick took
13.2 of the 14.5 seconds of making the layers, learning the areas' maps
11.0 more (once an area), the floor made drawable 1.1 and the coordinate
data 0.2. The tiles' pick and the learning are where the time is.

The lab's first find there: another game's areas pick far slower than
BN6's (BN5's Undernet 483 ms a layer here, a BN6 Comp 18), and in them
the pick spends its time laying the maps' whole panels
(`src/map/panels_lay.c`), above all finding a book's panel by its place
(`panel_at`): 45 million searches of a book sorted by place, 9.6 billion
instructions of BN5's Undernet's 45.9. Each book now keeps a grid of its
panels by place (`src/map/panels.c`; 3 KB for the largest book, the 3DS's
modelled heap the same to 0.01 MB, `tools/heap3ds.py`), which the search
reads: over every area the pick 13.3 to 7.8 seconds and making the layers
14.5 to 9.1, BN5's Undernet 35.7 to 27.0 billion instructions; every layer
the same.

Then the pieces' search (`lay_pieces`): for each of the floor's panels, for
each of its slots still open and round after round, it tried every panel
of every book for the slots it suits (`slots_like`), which hangs on the
panel's shape alone. Each shape's suiting panels are now listed once a
map's laying, in the books' order (`likes_of`), and the search tries those:
over every area the pick 7.8 to 7.2 seconds (13.0 before the grid), BN5's
Undernet 27.0 to 24.4 billion instructions; every layer the same, the
3DS's modelled heap the same.

Learning the areas' maps, after the pick the larger part over every area
(11.1 seconds): in BN4's area 34 it took 3.1 seconds, 67.6 of its 74.4
billion instructions in following its floors' own art (`tl_own_art`), and
63.0 of those in `tl_skipped`, whether a panel's tiles are left out,
which counts the panel's pixels and was asked 2.6 million times. It is now
asked once a panel and kept with the map's view (`Src.skipped`, asked
again once the map's pads are found): over every area the learning 11.1
to 4.5 seconds, area 34's 3.1 to 0.25; every layer the same, the 3DS's
modelled heap 0.02 MB more while a map is learned.

## What the profile of 14 November 2026 found

Instructions to the first layer, the second start (its boot state made),
`game_main`'s count:

| | BN6 alone | BN6, BN5 and BN4 |
| --- | ---: | ---: |
| Before | 2.54 billion | 4.09 billion |
| The floor's fix and the SHA-1 | 1.80 billion (-29%) | 3.15 billion (-23%) |
| And the installs | 1.78 billion (-30%) | 2.58 billion (-37%) |
| And the tiles' looks | 1.67 billion (-35%) | 2.46 billion (-40%) |
| And the walls | 1.50 billion (-41%) | 2.29 billion (-44%) |

Where it went before, by what each part runs:

- **Making the floor drawable** (`legal_fix`, `src/map/legal.c`): 0.96
  billion, the most of any part. For every panel not drawn exactly it
  tried each cell and each 2 x 2 block near it, and for each asked the
  look of every panel around it again on a grid that had not changed,
  and counted the whole grid's pieces of floor and void twice, a flood
  fill each time. Now each panel's look is asked once between two edits
  (the clean callback reads the cells alone, `legal.h`), the grid's
  pieces are counted once an edit, and the flood fill runs in a copy
  ringed with walls, without bounds: 0.30 billion, and every area's
  layers the same byte for byte (139 layers, 1,077 pictures, a seed a layout).
- **Each ROM's SHA-1** at the start (`src/core/rom.c`): 0.33 billion a
  ROM, its 8 MB hashed. The rounds unrolled with the words'
  roles turned by name: 0.24 billion, the same hashes (`test_sha1`, and
  each ROM still recognised).
- **The installs** with BN5 and BN4 (`xinstall_core`): 0.76 billion, 0.28
  after. The cartridge's bytes are read and written in place rather than
  a byte at a time through mGBA's bus (`src/emu/emu.c`, `cart_read` and
  `cart_write`: what GBALoad8 reads and GBAPatch8 leaves, the copy made and
  the size grown by the write's last byte, written mGBA's way); the claims'
  check (`src/emu/claims.c`) is one pass, the installers' names compared
  only where two ranges meet (four million comparisons before); and the
  code walk's "is this halfword code" is answered from marks made once a
  block (`xvirus_is_code_in`, `src/emu/xvirus_recipe.c`) rather than by
  scanning the family's blocks and data for each. Checked by the ROM copy
  after the installs, its 32 MB dumped by the old build and the new and
  compared, the same byte for byte with BN6 alone and with all three, and
  an autopilot run's log the same line for line.
- **A tile's look** (`tl_expect_look`, `src/map/tiles.c`), which both the
  tiles' pick and the learning of an area's maps ask for every tile: 0.27
  billion of its own, 0.11 after. Each of its 14 columns asked every row
  of the reach above the tile (up to 40 pixels) for floor; now only where
  the tile's top row has none, upwards and no further than the first. And
  a pixel's floor beside and below it is read from those columns'
  distances rather than asked again. The atlas's 1,077 pictures the same
  byte for byte.
- **The coordinate data's walls** (`walls`, `src/map/coords.c`), written
  for a layer's floor and again with its pads: 0.11 billion each time, 0.02
  after. It asked each cell of a 252 x 252 square for floor up to fourteen
  times through `netmap_floor_cell`; now each cell is asked once into a
  map (floor, the ground's floor raised over it, a stair). The ROM copy
  after the first layer, its 32 MB dumped by the old build and the new,
  the same byte for byte over 30 starts (six areas, BN5's among them, and
  seeds 1-12 at depths 3 and 5).

## What is left

The same start, BN6 alone, after: 1.50 billion; with BN5 and BN4, 2.29. Of a layer's 1.0 billion: the tiles' pick 0.35, the floor's fix 0.30, learning the area's maps 0.29.

- **Picking the tiles** (`tiles_pick`, 0.32 billion over 11,851 calls),
  `best`'s own comparisons the most of it; in another game's areas,
  laying the maps' panels (`panels_lay`: `suits`, the neighbours a
  suiting panel would meet, after the grid and the shapes' lists).
- **Learning the area's original maps** (`nm_learn`), done again when a
  run comes back to an area (`3ds/README.md`, "learned again"): 4.5
  seconds over every area since `tl_skipped` is kept, BN6's Seaside
  (area 2) the slowest at 0.47.
- **mGBA's CRC32 of the ROM copy** as the core loads it (0.13 billion
  for its 16 MB), in mGBA's own code.
- **The SHA-1** (0.24 billion a ROM): a hash kept beside the data folder
  for a file whose size and time have not changed would save it at every
  start but the first.

The game's frames themselves are measured by the [frame log](frame-log.md),
not here.
