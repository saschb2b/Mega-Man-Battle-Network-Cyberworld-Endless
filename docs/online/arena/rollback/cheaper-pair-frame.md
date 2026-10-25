---
type: Engine Mechanism
title: "A cheaper pair frame"
description: "How the pair frame got cheap enough for rollback: whole run loops under mGBA's lockstep credits (`arena_pair_step`) and both cores halted at BN6's frame wait, the Nova's frame from 3.7 to 1.05 ms."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-20T00:21:12Z }
---

# A cheaper pair frame

Before, a pair frame cost 2.8 ms here and 3.7 ms on the Nova, so the
Nova could roll back 2 frames at the means and 1 at the worst (the
measurement below). The cost was the pair's emulation. Two levers,
measured one at a time with `--dev rollbench` (this machine: Intel Core
Ultra X7 358H, the host build; the Nova: ROCKNIX, aarch64, the PortMaster
build headless over SSH in a scratch folder), a pair frame's mean and
worst under BN6's rules (full rules within a tenth of a millisecond):

| The pair's stepping | This machine | The Nova | The window a tick holds on the Nova (means / worst) |
| --- | --- | --- | --- |
| Before: the cores an instruction at a time in turn (the research harness's) | 2.78 / 3.29 ms | 3.72 / 4.13 ms | 2 / 1 |
| Whole run loops under mGBA's lockstep credits | 1.27 / 1.75 ms | 1.70 / 1.82 ms | 6 / 5 |
| ... and both cores halted at BN6's wait for a frame | 0.82 to 0.97 / 1.1 to 1.4 ms (with the machine's other work) | 1.05 / 1.30 ms | 10 / 8 |

- **Run loops** (`arena_pair_step`). The research stepped each core one
  instruction and then the other, through mGBA's `step`: a function call,
  an event check and the frame counters read for every instruction, and
  the two cores apart by whatever their instructions cost (a core could
  run two frames ahead of its partner). Now each core runs mGBA's run
  loop (its instructions up to its next event) and the cable's own
  credits pace them, as mGBA's frontend paces its two threads
  (`src/platform/qt/MultiplayerController.cpp`): the first GBA posts the
  cycles it ran, the second runs until it has used them and then sleeps
  until more are posted, and the first sleeps while a transfer waits for
  the second. A frame begins in an event, where a run loop ends, so each
  core still takes its own frame's keys. The two stay within a few
  thousand cycles: a core is never more than one frame ahead of its
  partner (measured through way ins and whole battles), so a frame needs
  the keys of the one after it, not the three after it
  (`ARENA_LOOKAHEAD` 2), and the least delay is 1, not 3.
- **The idle halt** (`idle_on`). Both pair cores take the game's own
  core's hook on BN6's wait for a frame (`src/emu/idle.c`), on their own
  hook tables, under both rules: half of a battle's cycles were that
  loop. With an instruction at a time it did not work (a halted core ran
  to its VBlank in one step, past the cable's waits: "the cable stopped
  in BN6's menus"); with run loops it does.
- **Kept deterministic.** Every client and spectator runs the same
  scheduling, so their pairs stay alike; the snapshot keeps who sleeps.
  Whole battles by `--dev arena,arenaroll=W` (W = 1, 6, 8), under BN6's
  rules and full rules: every battle over at frame 2,738 as run
  straight, all 45 of the game's hashes the same; rollbench's 11
  rollbacks of 8 exact for the game on both machines; the same battle on
  the Nova and here (`--dev arena,arenabusy`, below) gives the same 40
  hashes. The pair's hashes changed with the scheduling (and so did the
  ready's), one more reason for protocol v8.
- A profiler was not to be had here (the kernel's `perf_event_paranoid`
  is 4, and stays so); the levers came from reading the stepping loop and
  mGBA's run functions. A pair frame now costs about two of the game's
  own core's (0.37 to 0.43 ms here, 0.68 on the Nova).
