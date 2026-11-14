---
type: Engine Mechanism
title: "A new run and boot"
description: "NEW GAME's start in the town (first layer built, Lan warped to his start, jack-in flag 0x1727 cleared) and `emu_boot`, which presses through BN6's title to Seaside Area 1 and saves `boot-4.state`, its number bumped when what NEW GAME gives changes."
tags: [engine, emulation]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-14T22:27:59Z }
---

# A new run and boot

## A new run: the town

NEW GAME builds the run's first layer and the town ([The overworld](../../world/overworld.md)),
warps Lan to where the town starts him (his front door in Central Town,
the Metroline's stairs in ACDC Town) and clears the flag that stops
jacking in (`0x1727`); the PET's Save stays off. While Lan walks the town
the director waits: the town's jack-in cells and jack-in table send the
game's own jack-in to the first layer, and when MegaMan has arrived there
the run goes on as after any layer's warp (the flags set again, a
checkpoint). A NEW GAME deletes the last run's save at once, so CONTINUE
always resumes a run in the net.

## Boot

`emu_boot` resets the core, presses through the title and NEW GAME, warps to
Seaside Area 1 and ends the intro cutscene, then saves `boot-4.state` in the
data directory. Later runs start from that state. It is made on the device
and never shipped.

Every later build loads the state the first one kept, so its number
(`BOOT_STATE` in `src/emu/boot.c`) is bumped whenever the boot's sequence
changes or what NEW GAME gives in the core's copy does. 4 came with
ten-bit chip ids ([the census](../../rom/chips-code.md), Ten-bit chip ids),
which widen BN6's preset folders in the core's copy: a `boot-3.state`, kept
by a build before them, held the Standard folder as BN6's ROM holds it,
nine-bit. On an install that had kept one before 0.21.0, every new run
on the Standard folder from 0.21.0 to 0.21.2 read those entries as other
chips: Cannon B as BN4's CloudMn (513) in A, AirShot * as AirShot in N,
MiniBomb B as BN4's 566 in A. A code the chip's record lacks is dealt
blank, and so is a chip of BN4's where its ROM is not read: no icon, and
used, no effect. A New 3DS, which reads BN5's ROM beside BN6's and not
BN4's, showed it first (November 2026: 26 of its folder's 30 entries).
The new state is made at the next start, and the old one is deleted once
it is written. A run begun from the old state is mended as it continues
(`own_folder_chips`, `src/director/director_folder.c`): an entry of the
Standard folder's nine-bit form that the pack counts in its nine bits
and not in its ten is widened, as GiveFolder had counted it, and the log
says how many (`folder: N of the Standard folder's entries widened`).

The boot's frames are never shown, so the core draws no picture for them;
nor for a frame the main loop plays unshown to catch up with the GBA's pace
(`emu_frame` sets the core's frame-skip counter for that one frame, where
its struct reads back as expected).
