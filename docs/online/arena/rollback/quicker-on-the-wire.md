---
type: Engine Mechanism
title: "Quicker on the wire"
description: "Issue #167's netcode quick wins: BN6's link battle's own three frames of input delay measured, the delay's floor at 1, no Nagle or worker sleep, sounds through a rollback, the time sync's waits split evenly."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-21T14:22:21Z }
---

# Quicker on the wire (issue #167)

Epic #167's fifth step, its netcode's quick wins (21 October 2026), each
its own commit, from the research's ranking of what other rollback
netcodes and their write-ups do (GGPO's developer guide among them);
nothing of theirs is copied.

**Measured first.** BN6's own link battle, in the pair (`--dev
arena,arenabusy`, delay 1, lag 0, window 0: every key known, no guess),
with a probe kept out of git that read both MegaMen's `BN6_T1_ACTION` and
panel on both GBAs every pair frame: a direction the second player
pressed on pair frame *N*, MegaMan ready, turned his action into a step
on frame *N* + 4, on his own GBA and on the other on the same frame (47
of 47 steps). The same press in a single battle on the game's own core
(`--scene emu --run-depth 1 --seed 3`, a `foes 200@52` battle, the probe
reading the same fields after each frame) turned it into a step on frame
*N* + 1 (24 of 24). So BN6's link battle itself applies a press three
frames later than its single battles, on both GBAs alike: three frames
(50 ms) every NetBattle pays, the cartridge's own over its cable, that no
netcode of ours takes away. A hook on both pair cores could shorten it
(the research's item 10): a reverse-engineering task, and a question of
fidelity for the owner (a NetBattle that answers faster than the
cartridge's), not done here. The network thread's gap, read in
`online_transport.c`: one message taken a pass, then `SDL_Delay(2)`, so a
key waited up to 2 ms in the sender's worker and as long again in the
receiver's (more where several came at once).

**The delay's floor 1, not 2.** The server added a tick for a key to be
taken on top of the lockstep's frame and the latency asked for, so a pair
whose windows covered the whole trip still played at delay 2. With
rollback that tick is one more frame the window covers: `delay_for` now
gives the lockstep's frame and the longer latency said, 1 at least, and a
game says its latency less its window *but a frame* (`arena_ask_ms`; a
game with no window says a frame more). Past the window's edge the delay
is what it was; within it, 1. Every own press, step and chip comes a
frame (16.7 ms) sooner on such a pair, and the other MegaMan's guesses run
a frame deeper. No wire change: v10 games take delay 1
(`ARENA_DELAY_MIN`), spectators read it from WATCH_STATE. The server's
tests say it (`test_delay_covers_the_trip`: 0 and 0 give 1, 16 ms 2, 40
and 100 7); `--dev arena` with the local link's 50 ms one way and window
7, at the delay the server now gives such a pair (1) and the old floor
(2):

| Delay | Over at | Hashes | Rollbacks, frames run again, deepest | Stalls | Worst tick |
| --- | --- | --- | --- | --- | --- |
| 2 (before) | frame 2,691 | 44 agreed | 12, 24, 2 | 0 | 3.0 ms |
| 1 (after) | frame 2,690 (the delay alone's battle, the same last hash) | 44 | 12, 36, 3 | 0 | 3.6 ms |

**No send held back.** Every client socket now asks for no Nagle where
the system lets it: curl's (Linux, macOS, PortMaster, the 3DS) says
`CURLOPT_TCP_NODELAY` (curl's default since 7.50.2, said for older ones),
and Android's Java socket had it already (`setTcpNoDelay`). WinHTTP's
WebSocket (Windows) and `NSURLSessionWebSocketTask` (iOS) offer no such
option and do not document what they do; whether they hold small sends
back is unverified (a capture on a real Windows PC and an iPhone, the
research's step 0, is still to do; Wine's WinHTTP is its own). The
worker no longer sleeps 2 ms between looks: it takes in every message
that came, then rests until a message comes in or one is put to go out
(`online_socket_wait`, `online_socket_wake`), 10 ms at most: on curl's
systems a `poll` on the socket and a pipe, on Windows a semaphore that
WinHTTP's read completions post, on iOS one that each message's
completion posts; Android's and the 3DS's streams, which the worker
cannot wait on, are looked at every millisecond. A key goes out the moment
the game puts it in the outbox, and one that comes in is taken at once.
The browser has no worker and is unchanged. CPU, the transport's probe
(`tests/test_online_transport.c`, its exchange against a local server,
5.3 s, three runs each; its own main loop still sleeps 2 ms a pass):

| Build | CPU | Voluntary context switches |
| --- | --- | --- |
| Before | 86 to 94 ms (1.6 to 1.8%) | 4,850 |
| After | 62 to 72 ms (1.2 to 1.3%) | about 3,000 |

**The other player's sounds through a rollback.** The frames a rollback
ran again were silent, so a sound that began in them (his buster, his
chip, a hit) lost its start, and one the wrong guess began went on. Now
the pair's sound goes into the ring a frame at a time, each frame's place
on the ring's timeline kept (`emu_audio_mark`); a rollback cuts the ring
back to the first frame run again (`emu_audio_rewind`): what is not yet
played is replaced by the frames run again, and where some was played
already as much of the new sound is skipped, so the sound stays on its
timeline. Every system that plays the battle's sound takes it (the ring
is `emu.c`'s). The ring holds about 21 ms on a desktop (1,024 samples),
so a rollback of a frame or so is replaced whole and a deeper one in part;
a few frames more of the ring in battle would cover more (not done: it
would put the battle's sound further behind its picture, a call for the
owner). `tests/test_emu.c` holds the ring's marks to it (a mark cut back
to, what was heard skipped, the timeline kept, with a guest core's tone).
Not yet heard on a device: headless runs check that the game is the same
(below), not how it sounds.

**The battles, before and after** (this machine, the host build headless,
`--dev quiet,arena,arenaboth,quitend` and a fresh data folder each):
every run over at the same frame with the same hashes after all three
changes as before them.

| Run | Over at | Hashes, the last | Rollbacks, frames run again, deepest |
| --- | --- | --- | --- |
| `arenadelay=1,arenalag=0,arenawindow=0` | 2,690 | 44, `86d2f837337a1494` | none |
| `arenadelay=2,arenalag=3,arenawindow=7` | 2,691 | 44, `ab7854f3d4ebe5a1` | 12, 24, 2 |
| `arenadelay=1,arenalag=6,arenawindow=7` | 2,690 | 44, `86d2f837337a1494` | 12, 72, 6 |
| `arenabusy,arenadelay=1,arenalag=5,arenawindow=8` | 3,288 | 54, `31087ba7ecf2a8a9` | 489, 2,445, 5 |
| `arenadelay=1,arenalag=0,arenaroll=6` | 2,690 | 44 and the game's 44, `86d2f837337a1494` | (the rollback tried every 6 frames) |

`--dev rollbench` after them: a pair frame 0.84 ms (the worst 1.31), a
rollback of 8 frames 6.2 ms, the game's state the same in 11 of 11, the
Arena's window 8 from the way in and 10 from the battle's.

**The time sync's waits split evenly, where nobody sees them** (protocol
v11). Each game held its own lag at its own round trip and a tick
([Smoothed delay](../smoothed-delay.md)), so the two lags summed to the two
round trips but the game with the worse link kept the larger one, and
with it the deeper rollbacks. Now each game says its least lag of the
other's keys over the last second once a second (ARENA_HASH's lag note,
frame 0xFFFFFFFD, `arena_link_net.c`; never kept for the stands), and the
one whose lag is two ticks or more past the other's holds a tick
(`arena_sync.c`): a hold takes a tick from its lag and gives one to the
other's, so the two meet within a tick, and after a hold it waits 180
ticks for the other's next note to show it. Before the first note it
holds by the old rule. And a hold waits for a tick nobody sees: my GBA on
the Custom screen, or my keys unchanged for half a second (as GGPO holds
only on idle input), two seconds at most before it holds anyway. The
local link carries no notes, so `--dev arena` keeps the old rule (its
battles above the same). The ROM-free test runs two clocks four ticks
apart through a server one tick from the first and three from the
second: the old rule left their lags at 3 and 5 ticks, the notes at 4
and 4, nothing late, and with the keys never quiet the holds still come.

**Through a local server** (this machine, two host builds headless, the
Net Square's challenge and Yes, both fighting by `arenaboth`,
`arenahp=60`, `arenawindow=9`; a relay holding each chunk back a set
time and up to the jitter, in order, each way between a game and the
server; kept in `.build/`). Before is this step's first three changes on
a v10 server; after, the v11 build and server:

| Each way | Build | Delay | Rollbacks, frames run again, deepest (Alpha; Beta) | Holds | Stalls | Hashes |
| --- | --- | --- | --- | --- | --- | --- |
| 20 ms and up to 8 | `origin/dev` (v10) | 2 | 39, 140, 6; 32, 59, 2 | 4 (Alpha) | 0 | 28 agreed |
| | the delay's floor, the worker and the sound (v10) | 1 | 39, 82, 3; 39, 192, 5 | 0 | 0 | 28 |
| 40 ms and up to 15 | `origin/dev` (v10) | 2 | 39, 160, 5; 39, 274, 8 | 1 | 1 (Beta) | 28 |
| | the delay's floor, the worker and the sound (v10) | 1 | 39, 269, 8; 39, 234, 7 | 0 | 0 | 28 |
| none (the relay's own) | `origin/dev` (v10) | 2 | 39, 76, 5; 0 | 4 | 0 | 28 |
| | the delay's floor, the worker and the sound (v10) | 1 | 0; 39, 137, 7 | 5 | 0 | 28 |
| Beta 40 ms, Alpha 8, up to 4 | the delay's floor, the worker and the sound (v10) | 1 | 39, 173, 8; 35, 121, 4 | 5 | 2 (Alpha) | 28 |
| | v11 | 1 | 39, 168, 6; 39, 137, 4 | 3, all quiet | 0 | 28 |

The 20 and 40 ms pairs ran side by side, two battles at once (ticks past
a 60th of a second there, the machine's, not the link's). With the time
sync's notes the two games' least lags ended at 3 and 4 ticks (Alpha's
and Beta's), the three holds on the Custom screen. With no relay the
Square's round trips said dropped from 3 ms to 1 ms: the worker's 2 ms
rests gone. Every battle was over at its frame on both games, every hash
agreeing.

**Not yet, of this step.** The Nova's CPU with the new worker; a capture
of WinHTTP's and iOS's sends; the rollback's sound heard on a device.
