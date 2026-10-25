---
type: Measurement
title: "Measured"
description: "Rollback measured through a local relay against the smoothed delay (delay 2 against 8 to 11, no tick past a 60th, every hash agreed) and on the Nova, where a 7-frame rollback fits a tick."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-20T00:24:51Z }
---

# Measured

**Through a local server** (20 October 2026, this machine): three host
builds, #143's three-game harness (Alpha challenges Beta, both pick a
chip, OK and fire charged shots with a step between them, `arenahp=150`;
Gamma, BN6's ROM alone, asks the referee and watches from the way in)
behind #142's relay, which holds each chunk back a set time and a random
part, in order, each way between a game and the server (`.build/`, not
committed). Before is `origin/dev` (ca33edef, v7: the delay alone);
after this build (v8). Two runs went side by side, so the frames cost
more than alone (1.4 to 1.6 ms). Per game, Alpha (the cable's first) and
Beta:

| Each way | Build | Delay | Window | Rollbacks, deepest | Waits per 100 frames | Worst tick | Hashes agreed | Gamma |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 20 ms and up to 8 | before | 8 | | | 0, 0.03 | | 54, 54 | 54 checked |
| | after | 2 | 7, 7 | 93 (2.2 a second), 5; 98, 2 | 0, 0 | 9.6, 6.1 ms | 43, 43 | 43 checked |
| 40 ms and up to 15 | before | 11 | | | 0.09, 0 | | 53, 54 | 54 |
| | after, a first battle (window 4 asked) | 5 | 7, 7 | 93, 4; 96, 2 | 0, 0 | 10.4, 6.8 ms | 42, 42 | 43 |
| | after, a frame's cost kept (1.0 ms: 9 asked) | 2 | 8, 8 | 93, 5; 96, 8 | 0, 0 | 12.7, 14.3 ms | 42, 42 | 42 |
| 20 ms and up to 40 | before | 11 | | | 0, 0.09 | | 53, 53 | 54 |
| | after, a first battle | 6 | 7, 7 | 92, 4; 13, 2 | 0, 0 | 8.6, 5.1 ms | 42, 42 | 43 |
| 20 ms and up to 8, full rules (all three with BN5's ROM) | after | 2 | 8, 8 | 101, 3; 104, 3 | 0, 0 | 8.0, 9.7 ms | 53, 54 | 54 |

No tick past a 60th of a second in any battle; every battle ran to its
end (Alpha's win), the spectator's straight run agreeing with every hash
the players sent. A battle's frame count differs before and after (3,249
and 2,588 frames at 20 ms) as the scripts' keys land at other frames
with another delay. A capture of Alpha's game with rollback on (delay 2,
window 9) shows the Custom screen and charged shots taking Beta from 150
to 90, 80, 60, 40 and 20, as without. The hashes still catch two games
apart: with Beta's copy of Alpha's keys changed from frame 1,200
(`arenadesync=1200` on Beta, delay 2, window 9), both stopped at frame
1,267, "the two games went apart by frame 1259", and Gamma's watching
ended with the battle.

**On the Nova** (the final build, headless, `--dev arena,arenabusy`
with delay 1 and the local link's lag making every frame's other keys a
guess, the window its own):

| Run | Window | Rollbacks, frames run again, deepest | A pair frame | Worst tick | Game hashes |
| --- | --- | --- | --- | --- | --- |
| BN6's rules, lag 4 ticks | 7 | 360, 1,440, 4 | 1.13 ms (the worst 1.39) | 7.5 ms | 40, the same as this machine's run and as the delay alone |
| BN6's rules, lag 7 | 7 | 360, 2,520, 7 | 1.13 ms (1.41) | 11.4 ms | the same |
| Full rules, lag 5 | 7 | 360, 1,800, 5 | 1.15 ms (1.64) | 9.0 ms | the same as this machine's full-rules run |
| The delay alone (`arenawindow=0`, lag 0) | 0 | none | 1.14 ms (1.43) | 2.3 ms | the same |

No tick past a 60th there either: a rollback of 7 frames fits the Nova's
tick with room.
