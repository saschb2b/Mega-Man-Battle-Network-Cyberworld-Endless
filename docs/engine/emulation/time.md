---
type: Engine Mechanism
title: "The core's time"
description: "Where the GBA core's time goes, measured (issue #35): the picture, BN6's busy wait for VBlank halted by the idle hook (`src/emu/idle.c`, about 30% less), and why no routine is done in C."
tags: [engine, emulation]
generated: { by: claude-code/unrecorded, at: 2026-10-20T00:21:12Z }
---

# The core's time

Measured on a computer (issue #35), with a build that sampled the GBA's PC
every 1000 cycles and timed each frame with its picture drawn and, every
other frame, not, over autopilot runs: a walked layer, battles, the town.

| Scene | Emulation, ms a frame | With the picture | Busy-waiting for VBlank | With the idle hook |
| --- | --- | --- | --- | --- |
| A layer | 0.52 | 0.64 | 48% of the GBA's cycles | 0.36 (0.48) |
| A battle | 0.53 | 0.67 | 57% | 0.35 (0.49) |
| The town | 0.51 | 0.59 | 44% | 0.37 (0.45) |

- **The picture** is about a fifth of the core's time on a layer and in
  battle. On the New 3DS mGBA's threaded video draws it on the main core,
  beside the emulation, so the core's own thread pays the interpreter.
- **The wait for VBlank** was half of the interpreter's work. BN6's main
  loop waits for each frame by reading DISPSTAT until its VBlank flag is
  set (bn6f `main_awaitFrame`), and mGBA's own idle-loop removal leaves a
  loop that reads DISPSTAT alone, since the flag changes with no
  interrupt to wake a halted CPU. A hook (`src/emu/idle.c`) halts the CPU
  there while the VBlank interrupt is on, which ends the halt where the
  flag would have ended the loop: the autopilot's logs and run logs are
  the same line for line, at about 30% less of the core's time. The
  Arena's two cores take the same hook on their own tables (`idle_on`),
  run a run loop at a time ([Rollback](../../online/arena/rollback/overview.md)).
- **What is left** is spread out: on a layer the hottest routine is
  bn6f `checkOWObjectInteractions` (`0x080037F4`, a sixth of the busy
  cycles, each object's interaction area against the others'); BN6's
  memory copies through the BIOS's CpuSet and CpuFastSet, which mGBA's
  own BIOS runs as interpreted ARM (a sixth); MP2K's sound mixer,
  copied to IWRAM at `0x03005700` (a tenth); the sprite and OAM code in
  IWRAM; in battle no one routine passes 3.3%.
- **No routine as C.** An answer hook that does a routine's work in C
  takes the routine's GBA cycles away, and with them BN6's own lag
  frames, where a frame's work runs past VBlank and the game takes two
  VBlanks for it. Tried on the object interactions (7-11% less of the
  core's time on a map, on a computer): the game ran one frame more in
  a layer's first second, its RNG a call ahead from there on, and the
  random battles came at other steps. The wait for VBlank is the one
  place the GBA's time goes without the game noticing: no work is
  skipped there, only the wait.

The sound is not the same sample for sample as 0.6.0's: the hooks (and
before them the stubs they replaced, whose own instructions took the
GBA's cycles) move the game's writes to the sound registers by a few
cycles, which shifts the phase of the GBA's tone and noise channels. Its
loudness, 20 ms at a time over 50 s of an autopilot run, follows 0.6.0's
at a correlation of 1.0000, the mean the same.

On a Retroid Nova (ROCKNIX), the same 90 s of an autopilot run (`--seed 7
--dev god,onehit`, `tools/device_run.py` with `frame_log = on`) took the
GBA 3.98 ms a frame with its picture on 0.6.0 and 3.09 ms with the hook
(the median 4.00 and 2.80).

On a New 3DS (`frame_log = on`, a minute or so of the town, a layer and a
battle each), the core's own thread took 15.9 ms a frame with 0.6.0 and
11.0 with the hook (the median), and the main thread's wait for it fell
from 10.4 ms to 1. The frames shown are paced by the refresh now: the
present waits for it where the core has its own thread
(`present_3ds.c`). Before, a timer paced them, and the long core frame
had steadied that timer by accident: with the core faster, 6 frames a
second came early and 8 late (gaps under 12.5 ms or over 20); with the
refresh's pace, 0.1 and 1.3, against 0.6.0's 0.1 and 2. An old 3DS or
2DS has no third core: the core and its picture share the main one,
and there the title stuttered and the net went black after a few
frames, as before (3ds/README.md).
