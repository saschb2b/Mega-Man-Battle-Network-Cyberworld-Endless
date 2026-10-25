---
type: Measurement
title: "Measured"
description: "The Arena's first measurements on the Linux host, 18 October 2026: `--dev arena` battles at 50, 83 and 150 ms one way finishing with every hash agreed, stalls only where the delay fell short."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-18T12:27:56Z }
---

# Measured

On the Linux host in the build image, 18 October 2026, the player's
Gregar ROM, a fresh data folder, the script above (player 1 picks OK and
fires twelve charged shots; player 2 the local script):

| Delay / one way | Way in | Battle | Stalls | Hashes | Result |
| --- | --- | --- | --- | --- | --- |
| 6 frames / 3 ticks (50 ms) | 1,817 frames, 2,858 transfers | over at frame 1,881 | 0 | 31 agreed | 1 and 2 (won, lost) |
| 12 / 9 (150 ms) | the same | over at frame 1,887 | 0 | 31 | 1 and 2 |
| 4 / 5 (83 ms, short of 3 + 5) | the same | over at frame 1,879 | 4 | 31 | 1 and 2 |

The 6-frame runs printed the same hashes at every 60th frame on every
run (`18a9a8c5beb9b412` at frame 59), before and after the engine's
pacing changed. The whole headless run, the way in included, takes
about five seconds. With a checkpoint in the data folder its MegaMan was
carried (`arena: side 0: HP 300 ...` with `arenahp=300`), and the battle's
picture showed 300 HP against the second player's 100. The pictures stay
in `.build/`.
