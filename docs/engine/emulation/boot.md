---
type: Engine Mechanism
title: "A new run and boot"
description: "NEW GAME's start in the town (first layer built, Lan warped to his start, jack-in flag 0x1727 cleared) and `emu_boot`, which presses through BN6's title to Seaside Area 1 and saves `boot-3.state`."
tags: [engine, emulation]
generated: { by: claude-code/unrecorded, at: 2026-09-30T00:25:15Z }
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
Seaside Area 1 and ends the intro cutscene, then saves `boot-3.state` in the
data directory. Later runs start from that state. It is made on the device
and never shipped.

The boot's frames are never shown, so the core draws no picture for them;
nor for a frame the main loop plays unshown to catch up with the GBA's pace
(`emu_frame` sets the core's frame-skip counter for that one frame, where
its struct reads back as expected).
