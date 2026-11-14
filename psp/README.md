# The PSP build

The game for a PSP-2000, PSP-3000, PSP Go or PSP Street (E1000) with
custom firmware (6.61 PRO or ARK-4): an `EBOOT.PBP` for the memory
stick's `PSP/GAME` folder, with its XMB icon and background. A PSP-1000
has 32 MB, not the 64 the game needs: it is not supported. The build is
new and not yet among a release's files; it has been played in PPSSPP,
not yet on a PSP (docs/engine/emulation/psp-and-ds.md).

Its GBA core is gpSP, not mGBA: mGBA's interpreter takes 19 to 25 ms of
the PSP's 16.7 ms frame, gpSP's dynarec, written for the PSP's CPU,
about 7. gpSP is GPL-2.0, so the PSP's build, which links it in, is
distributed under the GPL's terms; its source is this repository with
`docker/gpsp-cyberworld.patch` over gpSP's pinned commit
(`docker/gpsp.sh`).

## Playing

- Copy the folder `cyberworld-endless` with its `EBOOT.PBP` into
  `ms0:/PSP/GAME/` (on a PSP Go's own memory, `ef0:/PSP/GAME/`). The game
  shows on the XMB under Game, Memory Stick.
- The ROM, Mega Man Battle Network 6: Cybeast Gregar (USA), goes in
  `PSP/GAME/cyberworld-endless/rom/`, or stays where PSP players keep GBA
  games (`ms0:/ROMS/GBA`, `ms0:/ROMS`, `ms0:/PSP/GAME/gpSP`). Any file
  name works: the game reads the header and checks the contents.
- BN6's net alone: a PSP has no room for BN5's or BN4's ROM beside
  BN6's, and does not look for them.
- Saves, `settings.ini`, `keys.ini` and `log.txt` are beside the EBOOT.
- The GBA's buttons are on the PSP's: Cross is A, Circle B, L and R,
  START and SELECT, the D-pad or the analog nub; Options' Controls moves
  them.
- Not on a PSP: online play (the Net Square, NetBattles, trades, the
  DeepNet), the anonymous statistics, the second screen, rumble and the
  voxel views.
- HOME and Exit Game end it at once: the run goes on from its last
  checkpoint (a layer's start, a guardian arena's door).

## Building

```sh
python3 build.py psp    # build/psp/EBOOT.PBP
```

It builds in `docker/Dockerfile.psp`: pspdev's toolchain (pinned by its
digest; GCC 15 for the Allegrex, SDL2 with the GU renderer), gpSP with
the game's additions (`docker/gpsp.sh psp`: libretro gpSP's pinned
commit, `docker/gpsp-cyberworld.patch`), and mGBA's library, whose parts
the rest of the code names (`docker/mgba.sh psp`: the minimal core
without threads, `FIXED_ROM_BUFFER`, its POSIX files and the link
cable's lockstep added). The Makefile builds with `CORE=gpsp`
(`CW_GPSP`), links the game as a PRX (`psp-fixup-imports`,
`psp-prxgen`), writes `PARAM.SFO` with `MEMSIZE=1` (the PSP-2000's and
later's 52 MB for homebrew) and packs the EBOOT with the icon
(`psp/icon0.png`, `tools/app_icon.py`) and the background
(`psp/pic1.png`, `tools/steam_art.py`).

What differs from the other builds:

- **The start** (`src/core/start_psp.c`): the heap all the memory the
  PSP gives but a megabyte, a 1 MB stack, the CPU at 333 MHz; the
  folder beside the EBOOT, `log.txt` and the ROM's places. SDL2main
  names the module and starts the main thread.
- **The GBA core** (`src/emu/emu_gpsp.c`, `src/emu/hook_gpsp.c`): gpSP's
  MIPS dynarec. The ROM is the player's own bytes, read in 32 KB pages:
  a page the engine writes is copied first, the free space past the game
  is one page of 0xFF until written, so the ROM is held once where
  mGBA's builds hold a 16 MB copy beside it. The hooks are a table
  gpSP's translator reads, calling out before a hooked instruction runs;
  no BKPT is written. gpSP skips BN6's wait for VBlank itself (the idle
  hook is off). Its states are gpSP's own with the cart's save RAM after
  them: another system's checkpoint does not load on a PSP.
- **Memory**: the code and statics take 23 MB, gpSP's caches among
  them; in PPSSPP a run's heap went to 12.6 MB at most, BN6's ROM in it.
  The PSP keeps one area's learned maps and no town between visits
  (`CW_HEAP_PSP` builds the host so: `tools/heap3ds.py --cases psp`).
- **The picture** fills the 480x272 screen at 1.70x, smoothly from 1x:
  the PSP's 2 MB of video memory holds no sharp texture beside the
  canvas.

## Testing on a PSP and in PPSSPP

- **PPSSPP**: `tools/psp_bench.py` builds PPSSPP's headless runner at a
  pinned commit into `.build/psp-bench`; run the EBOOT there with
  `--memstick` set to a folder holding `PSP/GAME/cyberworld-endless/`
  (the EBOOT, `rom/`), `--timeout-emulated`, `--press` and
  `--screenshot-at`. Its times are emulated, a cycle an instruction: the
  least a PSP takes.
- **args.txt and env.txt** beside the EBOOT stand in for the command
  line a PSP's start has not: `args.txt`'s words follow argv's (`--scene
  emu` starts in a run's first layer), and `env.txt` sets a variable a
  line (`CYBERWORLD_AUTOPILOT=1`, the autopilot).
- **frame_log** (`frame_log = on` in `settings.ini`): a line a second in
  `log.txt`, its frames, their times and the heap in use and at most.
- On a PSP: the frames' times in `log.txt` at 333 MHz, a run to its
  first guardian with the heap's "at most" under its room, HOME's exit
  and CONTINUE.
