---
type: Engine Mechanism
title: "The core on its own thread"
description: "The GBA core on its own thread on the New 3DS (`emu_threaded`, `CYBERWORLD_EMU_THREAD=1`): `emu_frame` starts a frame and returns, every other access waits for it, and mGBA's threaded video draws alongside."
tags: [engine, emulation]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-25T10:55:00Z }
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

## Nothing drawn reads the game

A frame's drawing runs beside the next frame of the game only while it
reads nothing of the game: `emu_read*`, `emu_memory` and `emu_sync` wait
for the frame in flight, and one read made while drawing puts the whole
drawing behind the GBA. The frame then costs the core's time and the
drawing's, where it cost the longer of the two. So what a picture needs of
the game is read after the game's frame, as the update runs, and kept for
the drawing: the director's in `seen` (`director_see.c`: MegaMan's place,
the map's marks and its way, what stands over the field for
`director_cover`), a liberation's in `lib_see`, the PET menu's in
`pet_menu_see` (the game's mode, BN6's menu's state and, while it is up,
its tiles, palettes and icons), the dive's divers' camera in
`dive_people_frame`.

0.24.0 broke it on every frame, and the New 3DS ran slower than before
0.22.0 (issue #897): the PET menu's picture read the game's mode on every
frame drawn, its menu up or not, and behind it `director_cover`, asked by
the dive's band every frame, read the game's fade and chat box. Measured on
a computer with `CYBERWORLD_EMU_THREAD=1`: 0.22.0, 0.22.2 and 0.23.0
waited at no frame, 0.24.0 at every one. The same frames ran as many
instructions in all of them (24.6 to 25.0 million a frame on a layer, the
core's own 7.27 million in each, callgrind), and every release's 3DS build
came from one image of devkitARM, so the waits were the whole of it.

The look for the rest found older ones, which waited only where their
picture was drawn: the layer's map read the game's flags and MegaMan's HP
for its way, and the flame of darkness's flag for its mark, each time it
was drawn, on the bottom screen and under SELECT's map, which from 0.24.0
stays open; a liberation's team's HP and a blow's harm read MegaMan's HP
and place; the divers' names, rings and signs read the camera and
MegaMan's place.

Three things hold it now:

- **The count.** `emu_draw_waits` counts the reads that waited while a
  frame was drawn (from the scene's draw to the frame's present,
  `frame_log_drawn`; a quit's save or a test's state read after it is no
  drawing), and the [frame log](../../devtools/frame-log.md) names the part
  the last one waited in. Since the start it is the state's `drawwaits`.
- **The scenario.** `tests/scenarios/draw_waits.py` plays a layer, SELECT's
  map left open, the PET's menu, a battle and the town with the core on its
  own thread and holds `drawwaits` at 0
  ([Scenarios](../../devtools/scenarios.md)).
- **A whole look.** The count sees a frame's first read alone (the ones
  after it find the frame done), so one fixed can uncover the next, as the
  PET menu's uncovered `director_cover`'s. To find them all at once, every
  scenario is run with the core on its own thread and each read made while
  drawing printed with where it came from; the second screen is drawn each
  frame for it, which a computer otherwise never draws. That look, on 25
  November 2026, found the ones above, in three rounds: each round's fixes
  uncovered reads the round before could not reach.
