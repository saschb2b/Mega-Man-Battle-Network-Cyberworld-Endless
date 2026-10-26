---
type: Engine Mechanism
title: "Smoothed delay"
description: "Smoothed delay (issue #128's first step): a wait moves the game's clock, the leading game holds a tick (`arena_sync.c`), the server's `delay_for` fits the delay to both games' pings, measured through a relay."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-21T14:31:15Z }
---

# Smoothed delay

Issue #128's first step (the owner's decision of 19 October 2026), on
every system the Arena runs on. It serves a NetBattle that feels like
the player's own presses: the same short wait on every frame, never a
frame that sticks now and then.

**Why the leading game waited.** Each game starts its ticks when the
other's ready has come, so the game ready last starts first, ahead by up
to the link's one way. The lag of the other's keys there (its tick as a
key comes, less the tick the other game sampled it on) is two one ways,
the other game's none. At the server's delay (3 frames, the one way and
2 more) the leading game ran at the edge: each late key was a frame that
waited, the tick after ran two to catch up, and the next late key found
it at the edge again, about one frame in eight on the deployed server.
The server's one way was also short: it took half the larger round
trip, where a key goes up from one game and down to the other, half of
each game's.

**What changed** (no message on the wire):

- *A wait moves the clock.* A frame whose keys have not come still waits,
  but the tick after it samples no key and runs that frame instead of two
  (`arena.c`): the game's clock is a tick later, the wait is one frame
  shown twice and none skipped, and a game that led waits only as many
  times as it led.
- *The leading game slows down.* The two games' lags add up to their two
  round trips through the server, so each holds its own at its own round
  trip and a tick (`arena_sync.c`): it keeps the lags of the other's keys
  of the last second, and once half a second's have come, where even the
  least is past that, it holds a tick (samples nothing, runs nothing) at
  most one tick in 20 (5% slower for a moment) until it is not. A game's
  round trip is the least its transport measured during the battle
  (`arena_rtt`); the local link's is twice its one way, so `--dev arena`
  holds none.
- *The delay fitted to the pair.* Each game keeps the round trips its
  transport measures in the Square (one a ping, every two seconds, the
  last eight) and says their mean and twice their spread as ARENA's
  latency, in milliseconds as before (`arena_pings_latency`). The server
  (`delay_for`) gave the lockstep's 3 frames, a tick for a key to be
  taken, then the longer of the two latencies in frames: 5 at least, 30
  at most (0 and 0 gave 5, 20 and 10 gave 6, 40 and 100 gave 10). Since
  each game holds its lag at its own round trip, the longer covers both.
  An older game says one ping, without its jitter. (Protocol v8 makes the
  lockstep a frame and has each game say its latency less what its
  rollback window covers: [Rollback](rollback/overview.md).)

A game that truly falls behind still waits as before, and its picture
says so (the link's badge, then after a second a note naming whose PET
it waits for: [The way in, the battle and the result](from-square/way-in.md));
the 128-frame cap on its keys still holds.

**Measured** (19 October 2026, this machine): two host builds through a
local server behind a relay that holds each chunk back, in order, a set
time plus a random part each way between a game and the server
(`.build/`, not committed), both games with BN6 alone, `--dev
quiet,arenahp=60`, Alpha's challenge, both OKing the Custom screen and
firing charged shots to the end. Before is `origin/dev`'s build and
server. The leading game is the one that waited (which one leads depends
on whose ready came last); after, the other waited on none in any
battle, before it did in two rematches. Each run fought a rematch as
well (the scripts go on pressing), listed where it ended before the run.

| Each way, game to server | Build | Delay | The leading game's waits | Per 100 frames | After frame 300 | Hashes |
| --- | --- | --- | --- | --- | --- | --- |
| 20 ms and up to 8 (as the deployed server: delay 7) | before | 7 | 265 of 1,617 frames; 341 of 1,610 and its rematch 502 of 2,055 | 16.4, 21.2, 24.4 | throughout | 26, 26, 34 agreed |
| | after | 8 | 1 of 1,611; its rematch 0 of 2,055 | 0.06, 0 | 0 | 26, 34 |
| 40 ms and up to 15 | before | 8 | 320 of 1,606; its rematch 370 of 2,053 (and the other game 84) | 19.9, 18.0 | throughout | 26, 34 |
| | after | 11 | 3 of 1,606; its rematch 0 of 2,051 | 0.19, 0 | 0 | 26, 34 |
| 20 ms and up to 40 | before | 8 | 516 of 1,600; its rematch 658 of 2,050 (and the other game 301) | 32.3, 32.1 | throughout | 26, 34 |
| | after | 11, its rematch 12 | 1 of 1,605; its rematch 1 tick held, no wait | 0.06, 0.05 | 0 | 26, 34 |

A wait before cost two uneven ticks (the frame shown twice, then two
frames in one); after, one. With the holds alone (an earlier cut, the
waits still caught up) the leading game settled within 270 frames but
waited up to 56 times on the way there; moving the clock at each wait
settles it in as many waits as it led. The delay is what the link
needs: a frame more than before on the first link (133 ms), three on the
second and the third (183 ms; 200 for the third's rematch, its pings
asked for more), where the old one was short of the trip. Not yet
measured on the deployed server or between two devices.
