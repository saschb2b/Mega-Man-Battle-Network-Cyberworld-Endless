---
type: Measurement
title: "The measurement: --dev rollbench"
description: "`--dev rollbench` (`src/dev/rollbench.c`): what a rollback costs where the game runs, the pair's 797 KB snapshot, frame and rollback times here and on the Nova, the window a tick holds, whole battles rolled back exactly."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-20T18:18:06Z }
---

# The measurement: `--dev rollbench`

`--dev rollbench` (`src/dev/rollbench.c`, [Dev tools](../../../devtools/overview.md)) runs on the
player's ROM wherever the game runs and says what a rollback costs there:
the game's core alone from power-on, then the pair through
the way in to a Practice NetBattle under BN6's rules and 600 frames of
scripted keys on both sides (the Custom screen's OK, then steps, shots and
the buster from a number of the frame), then again under full rules where
BN5's ROM is read (its installs made on the game's core first, as a run's
start makes them). Each frame it saves the pair's
snapshot and runs the frame, no load among them; every 50th frame it rolls
back eight frames, runs them again (each saved, as a rollback would),
compares the state with the one run straight and goes on from there; then
it times a hundred loads alone. From the means (and the worst) it gives
the largest window a tick holds: one load, then each frame resimulated and
the new one, each run and saved, within three quarters of a 60th of a
second (a quarter kept for the drawing, the picture's present and the
system). At delay *D* the pair's frame needs the other player's keys a
frame on: a window *W* covers a lag of his keys of *W* + *D* − 1 frames.
It says the way in's frames too, the window the Arena would pick from
them and from the battle's, and keeps the battle's cost for the delay
the next challenge asks.

| | This machine, before (19 October) | After | The Nova, before | After |
| --- | --- | --- | --- | --- |
| One core's state (mGBA's, raw) | 397,312 bytes | the same | the same | the same |
| The pair's snapshot | 797,040 bytes under BN6's rules (both cores, then 2,416 of the cable's, the pair's own and each GBA's outside its state, its registers as they read the most); 797,080 under full rules (40 more: what BN5's hooks keep, 20 a core) | the same | the same | the same |
| The game's core alone: a frame (mean / worst) | 0.40 / 0.81 ms | 0.43 / 0.87 ms | 0.69 / 1.6 ms | 0.68 / 1.6 ms |
| The pair: save, load | 0.035, 0.042 ms | 0.04 to 0.05, 0.05 ms | 0.027, 0.049 ms | 0.027, 0.048 ms |
| A pair frame, BN6's rules / full rules | 2.93 / 3.5 ms, 2.94 / 3.6 ms | 0.94 / 1.35 ms, 0.97 / 1.41 ms | 3.69 / 4.1 ms, 3.70 / 4.1 ms | 1.05 / 1.30 ms, 1.06 / 1.31 ms |
| The way in's pair frames | | 0.67 / 1.6 ms | | 0.81 / 2.9 ms |
| A rollback of 8 frames | 23.8 / 25.1 ms | 7.9 / 9.0 ms | 30.0 / 32.0 ms | 8.7 / 9.9 ms |
| Came back the same (11 rollbacks of 8, each rules) | the game's state 11 of 11; every byte 0 of 11 (the sound's, below) | the same | the same | the same |
| The window a tick holds (means / worst) | 3 / 2 | 11 / 7 | 2 / 1 | 10 / 8 |
| The window the Arena picks (from the way in / the battle's) | | 9 / 10 | | 7 / 9 |

What the numbers say:

- **The snapshot is cheap everywhere**: about 0.05 ms to save or load the
  whole pair (800 KB) on the Nova, and the size is no problem for any
  system that runs the pair (a ring of ten is 8 MB). BN5's hooks under
  full rules cost the pair's frame next to nothing.
- **The pair's emulation was the cost**, and is now about two single
  cores' ([A cheaper pair frame](cheaper-pair-frame.md)). (An earlier cut
  of the measurement loaded the snapshot before every frame, which put
  back mGBA's idle loop detection each time and made the frames look
  faster; the straight path has no loads, as a battle's.)
- **The restore is exact for the game, not for the sound's bytes**: the
  game's state (CPU, RAM, IO, video, timers, DMA, the cable's and the
  pair's own) came back byte for byte in every rollback, on both machines,
  and so did whole battles. mGBA 0.10's PSG keeps channels 1 to 4 up to
  date lazily, and a state loaded and run is not byte-equal in their
  timing fields and the samples rendered (`GBASerializedState`'s `audio`
  and `samples`, offsets 0x130 to 0x17B and 0x390 to 0x3CF) to the same
  frames run straight. The game never reads them back, so the pair's hash
  leaves them out (`arena_pair_hash`; `arena_pair_hash_whole` takes them,
  rollbench's "whole state"), and `arena_pair_snap_diff` says which bytes
  differ.
- **What mGBA's state leaves out.** A state holds each register as a read
  gives it and loads it through a write, and keeps no copy of the SIO's
  own control word: loaded, `SIOMLT_SEND` (the word a GBA sends down the
  cable next) and the SIO's busy bit came back as the frames after had left
  them, and the cores ran a few cycles apart from the first frame on (seen
  as each core's master cycles and next event apart, then whole battles
  apart). The snapshot keeps each GBA's registers as they read and the
  SIO's words, and mGBA's idle loop detection, the open bus and a DMA under
  way, and puts them back after the state (`arena_pair_load`).

**Whole battles rolled back** (`--dev arena,arenaroll=W`, the script of
[Run it](../run-it.md) with steps between the shots, `arena_roll.c`): the
pair's snapshot kept before each frame, and every W frames the last W
rolled back and run again from the first's, the battle going on from
there. Under BN6's rules and under full rules, W = 1, 6 and 8: every
battle over at the frame run straight, all of the game's hashes the same
(20 October: 2,313 frames and 38 hashes with the old stepping; 2,738 and
45 with run loops and the halt). Before the registers and the SIO's
words were kept, each of them came apart at the first frame.
