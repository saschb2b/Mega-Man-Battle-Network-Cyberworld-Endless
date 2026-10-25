---
type: Engine Mechanism
title: "The core on its own thread"
description: "The GBA core on its own thread on the New 3DS (`emu_threaded`, `CYBERWORLD_EMU_THREAD=1`): `emu_frame` starts a frame and returns, every other access waits for it, and mGBA's threaded video draws alongside."
tags: [engine, emulation]
generated: { by: claude-code/unrecorded, at: 2026-10-12T21:51:29Z }
---

# The core on its own thread

On the New 3DS the GBA core takes most of a frame, so it runs on the third
core where the system gives it one (`emu_threaded`). `emu_frame` then starts
the frame and returns, and every other access to the core (`emu_read*`,
`emu_write*`, states, the sound's rate) waits for that frame first: the
director's logic takes in each frame whole, as on the main thread. The emu
scene's update takes the frame done in and then starts the next, which runs
while the frame done is drawn from a copy of its picture; keys, frames and
the logic keep their order, and the autopilot's run log comes out the same.
`CYBERWORLD_EMU_THREAD=1` runs it so on a computer, to test it. With the
core on its own thread, mGBA's threaded video draws its picture on one
more (`threadedVideo`), beside the emulation; the worker waits for it at
each frame's end, so the picture copied is whole. On the 3DS that thread
is on the main core, which waits most of a frame for the emulation.

A run has one core, BN6's, on every build since issue #82's last round:
BN5's guest core, which booted on a thread of its own (the browser's
frames' spare time) from the game's start on, left runs ([Guest battles](../../multirom/guest-battles.md),
Guest battles), and the differential's boots it on the main thread.
