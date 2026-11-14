---
type: Tool
title: "Profiling: where a start's instructions go"
description: "How the time to the first layer is profiled (Valgrind's callgrind on the host build, in the build image, counting each function's instructions), what the profile of 14 November 2026 found, what was made faster, and the hot spots left."
tags: [devtools, performance, layers]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-14T16:36:50Z }
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
new, `--atlas OUT:all:1-4`, the pictures compared byte for byte and the
reports with their `built in N ms` taken out.

## What the profile of 14 November 2026 found

Instructions to the first layer, the second start (its boot state made),
`game_main`'s count:

| | BN6 alone | BN6, BN5 and BN4 |
| --- | ---: | ---: |
| Before | 2.54 billion | 4.09 billion |
| The floor's fix and the SHA-1 | 1.80 billion (-29%) | 3.15 billion (-23%) |
| And the installs | 1.78 billion (-30%) | 2.58 billion (-37%) |

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
  layers the same byte for byte (139 layers, 1,077 pictures, seeds 1-4).
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

## What is left

The same start, BN6 alone, after: 1.78 billion; with BN5 and BN4, 2.58.

- **Picking the tiles** (`tiles_pick`, 0.38 billion over 11,851 calls)
  and the look each tile needs (`tl_expect_look`, `src/map/tiles.c`, 0.27
  billion of its own: every pixel of a tile and its window asks the
  floor beneath it, memoised per pixel but asked many times).
- **Learning the area's original maps** (`nm_learn`, 0.36 billion), done
  again when a run comes back to an area (`3ds/README.md`, "learned
  again").
- **mGBA's CRC32 of the ROM copy** as the core loads it (0.13 billion
  for its 16 MB), in mGBA's own code.
- **The SHA-1** (0.24 billion a ROM): a hash kept beside the data folder
  for a file whose size and time have not changed would save it at every
  start but the first.

The game's frames themselves are measured by the [frame log](frame-log.md),
not here.
