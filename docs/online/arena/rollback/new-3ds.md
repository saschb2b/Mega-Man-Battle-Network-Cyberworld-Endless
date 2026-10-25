---
type: Engine Mechanism
title: "The New 3DS"
description: "Why the New 3DS builds the Arena but offers no NetBattle: a pair frame takes 25.8 ms on one ARM11 core; the shared ROM copy that fits memory, and the two-core cable driver still needed."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-20T18:18:06Z }
---

# The New 3DS

Measured on a New 3DS (20 October 2026, the Homebrew Launcher's
netloader, `3dslink -s` with `--dev
statistics=off,rollbench,arena,arenaboth,arenabusy,arenadelay=1,arenalag=4,quitend`,
a data folder of its own beside the game's, both ROMs on the SD card):

| | The New 3DS | This machine |
| --- | --- | --- |
| The game's core alone, from power-on (its thread on the third core, waited for) | 15.98 ms a frame (worst 41.0); save 1.18 ms, load 1.60 | 0.40 ms |
| The pair's snapshot: save, load | 3.11 ms (worst 7.70), 4.40 ms (worst 5.10) | 0.03, 0.04 ms |
| A pair frame with my GBA's picture / none / both | 25.79 / 20.71 / 32.55 ms (worst 31.7 / 24.8 / 34.5) | 0.79 / 0.70 / 0.94 ms |
| A rollback of 8 frames | 200.2 ms (worst 224.2); the game's state the same in 11 of 11 | 5.95 ms |
| The way in's pair frames | 22.5 ms (worst 65.9) | 0.64 ms |
| The window a tick holds (means / worst), the Arena's | 0 / 0, 0 | 16 / 9, 9 or 10 |
| The heap with the pair and ten snapshots made | 62,542 KB in use, 63,867 KB at most of 98,304 (BN6's ROM, BN5's, the game's core with BN5's installs, the pair's shared ROM copy and the snapshots); the linear heap 9,341 KB free of 14,404 | |

Then `--dev arena`'s battle there, playing itself (`arenaboth`), delay 1,
the local link's lag 4, the window its own (0): over at frame 3,404 with
56 hashes agreed, a pair frame 29.1 ms (worst 35.2), a tick at worst
53.9 ms, 3,403 of 3,408 ticks past a 60th: the battle played at about
34 frames a second.

- **Memory fits.** The two pair cores share one ROM copy on the 3DS
  (`arena_core_new`): mGBA's 3DS build (FIXED_ROM_BUFFER) maps a core's
  ROM as it is and patches it in place, and both cores take the same
  bytes and the same hooks at the same places; a hook put where another
  core's BKPT already stands takes that hook's original
  (`hook.c`, `original_of`). The boot core takes the copy first and lets
  it go before the pair is made. With it the pair and ten snapshots
  leave about 33 MB of the 96 MB heap free. The record of BN5's
  installs is kept on the 3DS only for these two dev tools, so a run's
  heap is spared its 2 MB.
- **Time does not.** Two GBAs on one ARM11 core at 804 MHz take 20.7 ms
  a frame with no picture, about 10.4 a core (the game's own core takes
  11 ms a frame in play, [Running the game's own logic](../../../engine/emulation/overview.md)), and my GBA's picture 5 ms
  more: past a 60th of a second before any rollback, so the window is 0
  and even the delay alone runs the battle at about 34 frames a second,
  slowing the other player with it (his game waits for my keys). A
  smaller window, the delay alone, one picture or none, and the Square's
  core held (it already is, and costs no time) were each measured or
  weighed: none brings a frame under 16.7 ms.
- **What is left: the two GBAs on two CPU cores.** The New 3DS's third
  core, where the game's own core runs and which is idle while the
  Square's core is held, could run one GBA while the main core runs the
  other. mGBA's cable cannot be split so as it stands: the second GBA's
  next cable event is scheduled from the cycles it has left
  (`_GBASIOLockstepNodeProcessEvents`, `useCycles`), and both read the
  cable's transfer state directly, so two threads leave the cores'
  timing (their next events, in the state and the hash) as their
  interleaving fell, and the pair's hash apart from a desktop's. It
  needs a cable driver of our own whose scheduling is the same however
  the threads interleave (the first GBA's cycles posted in chunks, the
  cable's writes held back until the partner reaches them), a new pair
  hash on every system and a protocol version. Even then a frame is at
  the edge: 10.4 ms a core and 5 for the picture on one of them, 15.4 of
  the tick's 16.7 before the threads meet and the frame is shown.

The 3DS so builds the Arena and offers neither NetBattles nor watching;
its NetPass holds the Arena stamp with BN5's ROM, as the server's rule
has it, and says "Every game in! No NetBattles on this PET."
([The NetPass](../../netpass.md)).
