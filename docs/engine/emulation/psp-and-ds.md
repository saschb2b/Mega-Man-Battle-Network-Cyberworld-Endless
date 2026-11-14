---
type: Measurement
title: "The PSP and the DS, measured"
description: "Whether the game could run on a PSP or a Nintendo DS, measured in PPSSPP at 333 MHz (14 November 2026): mGBA takes 19 to 25 ms of a 16.7 ms frame on BN6, libretro gpSP's MIPS dynarec 6 to 7.4, and a DS has neither the speed nor the memory for any GBA core."
tags: [engine, emulation, platforms]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-14T02:11:11Z }
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
- **Memory**: a PSP-2000, 3000 or Go gives homebrew about 52 MB with
  large memory; a PSP-1000 about 24, too little for the ROM, the core's
  copy and a run. gpSP takes its ROM buffer in 1 MB blocks, up to 32 as
  many as it finds (41 MB of the heap in use here, the harness's own
  copy of the ROM with it); sized for the game's 16 MB copy it would
  hold BN6 alone, with no room for BN5's or BN4's ROM beside it.
- **Making a layer** took about 20 seconds on the New 3DS for a run's
  first layer ("Building the net...", `3ds/README.md`); a PSP's CPU at
  333 MHz with no L2 cache would take two to three times that, not
  measured.

What a PSP build would ask of the engine: gpSP as a second core beside
mGBA, behind what `src/emu/` takes from mGBA today (the hooks' BKPT,
which gpSP's dynarec would have to call out on and then run the
replaced instruction, [Hooks](hooks.md); writes to the ROM copy, which
must drop gpSP's translations of it; raw reads and writes; the states the
checkpoints and `boot-3.state` are, in gpSP's own form, which a save
moved from another system would need converted); one core only, so no
NetBattles or Arena and nothing that runs a spare core; BN6's net alone;
the online parts left out; the 480x272 screen filled at a sharp-scaled
1.7x, as the 640x480 rule does (AGENTS.md). gpSP is GPL-2.0: a PSP
binary linking it would be distributed under the GPL's terms, the
project's own code staying MIT.

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
