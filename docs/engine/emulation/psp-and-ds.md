---
type: Measurement
title: "The PSP and the DS, measured"
description: "Whether the game could run on a PSP or a Nintendo DS, measured in PPSSPP at 333 MHz (14 November 2026): mGBA takes 19 to 25 ms of a 16.7 ms frame on BN6, libretro gpSP's MIPS dynarec 6 to 7.4; the PSP build on gpSP, its ROM held once and its memory cut, plays a run at 60 frames a second in a 12.6 MB heap; a DS has neither the speed nor the memory for any GBA core."
tags: [engine, emulation, platforms]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-14T02:38:23Z }
---

# The PSP and the DS, measured

A player asked for a DS version, and a PSP is the same kind of question:
handhelds older and slower than the New 3DS, which is the slowest system
the game runs on. An old 3DS or 2DS was left out because the GBA core
alone takes longer than a frame on its 268 MHz ARM11 (`3ds/README.md`);
on the New 3DS's 804 MHz ARM11 with its L2 cache it takes 11 ms of the
16.7 ([The core's time](time.md), with the idle hook). Cutting the run's
features does not change that sum: the time is BN6's own code
interpreted, so the question is whether a core runs BN6 in a frame's time
on the hardware at all. It was measured, not guessed.

## How it was measured

`tools/psp_bench.py` builds `tools/psp_bench/bench.c` for the PSP with
pspdev's toolchain (GCC 15.2, in Docker) twice: against mGBA 0.10.5, the
core the game drives, with the game's 16 MB ROM copy and its idle hook
(`src/emu/idle.c`); and against libretro gpSP, whose MIPS dynarec was
written for the PSP. Each EBOOT runs in PPSSPP's headless runner (built
from commit `1aebad2`, 13 November 2026), sets the PSP's clock to its
highest, 333 MHz (`scePowerGetCpuClockFrequency` reads it back), and
times every frame with the PSP's own clock, which in PPSSPP is emulated
time:

- **battle**: romlab's state of BN6's plain ROM in a battle against
  BlastMan ([tools/romlab](../../devtools/romlab.md)), MegaMan standing,
  played on to GAME OVER;
- **intro**: romlab's state at NEW GAME's opening text;
- **boot**: BN6 from power-on through the title to that text, by
  scripted input, with no state.

gpSP cannot read mGBA's states, so the harness converts one: gpSP's own
state taken, mGBA's memory and I/O written over it and loaded through
gpSP's loader (its palette and dynarec caches made again), then the CPU's
registers and banks set and the timers', sound's and DMAs' control
registers written again through gpSP's own handlers. The pictures it
dumps (`--dump`) show the battle going on as in mGBA, BlastMan's attacks
landing and MegaMan's HP falling to the GAME OVER screen, on the host and
in PPSSPP alike.

PPSSPP charges about one cycle for each MIPS instruction and nothing for
a cache miss or a stall, so its times are the least a PSP would take: a
core over a frame here is over a frame on the hardware; one under it has
the headroom shown to spend on what PPSSPP does not count.

## The PSP

On 14 November 2026, 900 frames a scene (the battle's GAME OVER comes
about 720 frames in), in emulated milliseconds a frame against a frame's
16.74:

| Core | Scene | Mean | Median | p90 | p99 | Max | Of a frame |
| --- | --- | --- | --- | --- | --- | --- | --- |
| mGBA | battle | 23.84 | 24.01 | 26.55 | 38.02 | 65.48 | 142% |
| mGBA | intro | 20.69 | 18.48 | 30.32 | 30.34 | 30.65 | 124% |
| mGBA | boot | 19.12 | 17.18 | 31.63 | 38.83 | 65.77 | 114% |
| gpSP | battle | 7.10 | 7.08 | 7.46 | 8.14 | 13.45 | 42% |
| gpSP | intro | 7.37 | 7.23 | 7.29 | 7.30 | 10.75 | 44% |
| gpSP | boot | 6.02 | 5.53 | 7.63 | 7.76 | 9.26 | 36% |

mGBA's heap held 17 MB (the game's 16 MB copy of the ROM), gpSP's 41 MB
(below).

The first 300 frames of the battle alone, every one in the fight: mGBA
25.3 ms (p90 28.1), and 35.9 without the idle hook; gpSP 7.3 (p99 8.4).
On this computer the same harness takes 0.50 and 0.25 ms on the battle:
gpSP's lead grows from twice to three and a half times on the PSP, its
dynarec written for that CPU.

- **mGBA cannot be the PSP's core.** Under PPSSPP's most generous count
  it takes a frame and a half in battle, before the engine's own work,
  the picture's present and the sound; the hardware is slower still.
  mGBA's own PSP port stopped there (its `PORTING.md`: "Make it faster",
  a MIPS dynarec). The PSP's second processor, the Media Engine, shares
  no coherent cache with the first and is reached from kernel mode:
  drawing the picture there, as the New 3DS draws it on its other core,
  would be a project of its own and would still leave mGBA's CPU over a
  frame.
- **gpSP would leave room.** 6 to 7.4 ms of the 16.7 in battle and on
  the title and text, under half a frame (13.5 at the worst one), which leaves the hardware's cache
  misses, the engine's frame and the present more than PPSSPP counts.
  Whether the PSP itself keeps a whole frame needs one run on a PSP
  (the harness's EBOOT, as `psp_bench.py` builds it, with the ROM beside
  it in `ms0:/PSP/GAME/cwgpsp/`).
- **Memory** is the PSP's harder limit, below. A PSP-2000, 3000 or Go
  gives homebrew about 52 MB with large memory; a PSP-1000 about 24.
- **Making a layer** took about 20 seconds on the New 3DS for a run's
  first layer ("Building the net...", `3ds/README.md`); a PSP's CPU at
  333 MHz with no L2 cache would take two to three times that, not
  measured.

## The PSP's memory

The game itself, first built for the PSP with mGBA as its core
(`build.py psp`), booted in PPSSPP to the title, and there its
heap is full: 26.9 MB in use of 26.9 (`frame_log`'s heap line), BN6's
ROM and the core's 16 MB copy 24 of it. Its code and statics take the
rest of the 52 MB: 4.6 MB of code and 19.9 MB of static arrays (the
host's are 18.3, of which 5.5 MB are the dev tools' own, the economy
report's 3.2 MB the largest, and 1.2 MB the second screen's). NEW GAME
then found nothing to make a layer with, and the screen stayed black.

What a run needs was measured on this computer with the 3DS's heap
model ([The 3DS's memory](3ds-memory.md), `tools/heap3ds.py --cases bn6
--seeds 1-6 --frames 60000`): BN6 alone peaks at 60 to 65 MB of heap,
42 MB still in use at a run's end. The 3DS page's breakdown says where:
the ROMs (24 MB), the town's source kept for the next visit (some 14),
the layer's area's map being learned and the learning itself (some 20 at
once), the installs' log (2). With the PSP's statics that is about 37 MB
more than a PSP holds, before the core's own needs. A PSP build had
to take the static arrays only a dev tool or a second screen uses
from the heap when used (some 7 MB, on every system), page the ROM from
the memory stick rather than hold it twice (gpSP pages a ROM in 32 KB
pieces; the engine's reads of `R.data` would need the same), keep no
town between visits and learn an area's maps in half the memory. Each
is a change to the engine every system shares.

## The PSP build

The memory was cut where every system gains (PRs #533 and #535: the dev
tools' static arrays taken from the heap when used, 6.2 MB; a learned
map's pixels held as colour indices, 3 bytes a pixel where they took 10,
a BN6 run's heap peak from 60-65 MB to 45-48), and by the PSP's own
rules (`CW_HEAP_PSP`: one area's learned maps kept, no town between
visits, no installs' log; `tools/heap3ds.py --cases psp`, 39 to 43.5 MB
with mGBA's copy still in it). The rest came with gpSP as the PSP's core
(`src/emu/emu_gpsp.c`, [psp/README.md](../../../psp/README.md)):

- **The ROM held once.** gpSP maps its ROM by 32 KB pages; the PSP's
  points them at the player's own bytes (`R.data`), copies a page before
  the engine writes it, and points the free space at one shared page of
  0xFF until a page of it is written (`docker/gpsp-cyberworld.patch`'s
  `cw_rom_pages`). mGBA's builds hold a 16 MB copy beside the 8 MB ROM.
- **Hooks without a BKPT.** gpSP's translator asks a table at each
  instruction it translates and, at a hooked one, emits a call out to
  the engine's hook before it, the ARM registers spilled for it, and a
  branch on where the hook answers an address (HOOK_RETURN, HOOK_JUMP):
  written for the MIPS dynarec and for x86-64's, so the host built on
  gpSP (`CORE=gpsp`) plays it too ([Hooks](hooks.md)). A test of its own
  hooked BN6's `main_awaitFrame` once a frame on both: every frame of
  1500 the picture the plain run drew, as when the hook sent the CPU
  back to its own instruction every other time.
- **States** are gpSP's own with the cart's save RAM after them, so a
  checkpoint saved on another system does not load on a PSP; one core
  alone, so no NetBattles, no Arena and no second core for Options'
  scene; gpSP skips BN6's wait for VBlank itself, the idle hook off.

Measured in PPSSPP at 333 MHz on 14 November 2026, the autopilot from a
run's first layer (`args.txt`'s `--scene emu`, `env.txt`'s
`CYBERWORLD_AUTOPILOT=1`, ten emulated minutes, `frame_log = on`): two
battles won, the act's guardian reached, the run's end screen; 60 frames
shown in 499 of the 560 seconds' lines and 57 to 61 in the rest, a
frame's update 10.2 ms (the GBA 9.8 of it, its picture with it), drawing
1.5 ms, the worst frames 75 and 94 ms as a layer was made; the heap at
most 12.6 MB, BN6's ROM in it, of the some 27 MB the 23 MB of code and
statics leave (gpSP's 2.4 MB of translation caches among them). The
title's own drawing takes 21 ms a frame there (27 frames a second), the
one screen under its pace. The picture fills the screen at 1.70x,
smoothly from 1x: the PSP's 2 MB of video memory holds no sharp texture
beside the canvas. On a PSP itself the times will be longer than
PPSSPP's count; one has not yet been run.

The old 3DS is the same question: mGBA alone takes longer than a frame
on its 268 MHz ARM11 (`3ds/README.md`), and gpSP has an ARM dynarec for
it that would need the same call out written; its 64 MB would hold what
the PSP's 52 do.

## The DS

No GBA core runs on a DS, and none was built for one. A DS's ARM9 runs at
67 MHz with 4 MB of RAM; a DSi's at 133 MHz with 16 MB. In PPSSPP the
battle frame took mGBA about 8.4 million cycles of its count (25.3 ms at
333 MHz, roughly an instruction each) and gpSP about 2.4 million; a DS
frame holds 1.1 million cycles and a DSi's 2.2 million, before a wait on
memory, so an ARM core as lean as gpSP's would still miss the frame. BN6's ROM is 8 MB, twice
a DS's RAM, and the engine's 16 MB copy all of a DSi's. gpSP's ARM
backend is for ARMv6 and later (its README), and the DS's ARM946E-S is
ARMv5. Nothing was run in melonDS: no DS program could hold the ROM, and
its DSi mode needs the console's own firmware and NAND images.

GBA games do run on a DS another way: GBARunner2 and GBARunner3 have the
ARM9 run the game's own code directly, as a hypervisor, the GBA's hardware
emulated around it. A run on that would be another engine, not a port:
no states (no checkpoints, no `boot-3.state`), no second core for BN6's
own screens, the director sharing the CPU with the game, and 4 MB for the
ROM's pages, the generator and the run. It was not pursued.

## Sources

- mGBA's `PORTING.md` (0.10.5's tarball): the PSP port incomplete, on a
  branch, "Make it faster" its open item.
- libretro gpSP's README and Makefile (commit `5819380`): the MIPS
  dynarec, the `psp1` platform, its ARM targets.
- GBARunner2 and GBARunner3, on the Emulation General wiki's GBA page: a
  hypervisor on the DS, GBARunner2 GPLv2.
