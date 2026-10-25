---
type: Engine Mechanism
title: "Why an emulator"
description: "Why BN6 runs on mGBA rather than as a native port like Ship of Harkinian or Zelda64Recomp: no C decompilation, no display list, no shipping Capcom's code, other ROMs as data, free save states."
tags: [engine, emulation]
generated: { by: claude-code/unrecorded, at: 2026-10-02T00:49:45Z }
---

# Why an emulator

Ship of Harkinian runs Ocarina of Time with no emulator, and Zelda64Recomp
runs Majora's Mask so too. Both stand on work BN6 does not have:

- [Ship of Harkinian](https://github.com/HarbourMasters/Shipwright) compiles
  [zeldaret/oot](https://github.com/zeldaret/oot), a matching decompilation:
  C written by hand that builds back into the original ROM byte for byte.
  [libultraship](https://github.com/Kenix3/libultraship) stands in for the
  N64's hardware, its Fast3D renderer turning the game's display lists into
  OpenGL, Direct3D or Metal; the assets come from the player's ROM at first
  start. [2Ship2Harkinian](https://github.com/2ship2harkinian/2ship2harkinian)
  does the same for Majora's Mask.
- [Zelda64Recomp](https://github.com/Zelda64Recomp/Zelda64Recomp) translates
  the game's MIPS code to C, function by function, with
  [N64Recomp](https://github.com/N64Recomp/N64Recomp), which finds the
  functions by the decompilation's symbols; [RT64](https://github.com/rt64/rt64)
  renders. The translated code ships in the executable.

Neither runs a CPU emulator, and both replace the GPU at the display-list
level, which the N64 allows: its games hand the GPU lists of commands. BN6
keeps mGBA because:

1. **No C decompilation of BN6 exists.** [bn6f](https://github.com/dism-exe/bn6f)
   is a matching disassembly of Falzar, almost all of it assembly.
2. **The GBA has no display list.** BN6 writes tiles, maps, OAM and video
   registers itself, some of it timed to the scanline, so a native build
   would still emulate the GBA's video and sound. Only the ARM interpreter
   would go, the core's cheapest part everywhere but the 3DS.
3. **A decompiled or recompiled BN6 ships Capcom's code.** The project ships
   none of it, which is what keeps it on PortMaster, Flathub, the 3DS and
   GitHub Pages.
4. **Other games' ROMs lend their content as data** ([Other games' ROMs](../../multirom/overview.md)); a
   native build would need each game's code ported too.
5. **Save states are free checkpoints** (`run.state`) on every target.

What the native ports do have is worth taking: hooks, where the game's code
calls the port's at named points. The engine's hooks (below) are that, on the
emulator.

The question is worth asking again when a C decompilation of BN6 covers most
of its battle and overworld code **and** a run needs changes deep in battle
logic (new chip behaviour, enemy AI) that hooks make awkward; or when the New
3DS's measurements (issue #35) show that the ARM interpreter, not the
renderer, holds its frame rate back. Even then a decompilation helps first as
a map for hooks, as Majora's Mask's does for Zelda64Recomp.
