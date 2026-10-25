---
type: Engine Mechanism
title: "In the browser"
description: "The Arena in the browser build since 20 October 2026: the pair run without threads under `emscripten_set_main_loop`, its window from the page's frame cost, battles against browsers and a native game, the ping fix, watching."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-20T18:18:06Z }
---

# In the browser

The browser build battles since 20 October 2026 (the owner's aim: every
stamp earnable on every system). Nothing in the Arena needed a thread:
the pair runs its two cores one run loop at a time as everywhere, the
way in in slices of the tick, and the battle a tick a frame of the page
(`emscripten_set_main_loop`, at most two game frames a tick); the
transport sends the Arena's messages over the browser's WebSocket at
once and takes them in from its callbacks between frames, the inbox's
ring as large as the native one's (64 KB, a spectator's keys catching
up). WebAssembly's memory grows for the cores (each takes mGBA's 32 MB
ROM copy once a hook patches it). The window comes from the page's own
frame cost as on every system: the way in's frames 1.15 to 1.19 ms in
headless Chrome, so 4 frames; a battle's frame about 2.0 to 2.5 ms,
kept in the page's data folder for the next challenge's ask.

**Measured** (20 October 2026, this machine): a local v9 server (seed
120), the worktree served on 127.0.0.1 with the developer's ROMs for the
page (`/.dev/rom.gba`, `/.dev/bn5.gba`), headless Chrome (agent-browser,
one browser a player, so each page is shown and its frames run), each
page the browser build from home through the gate with both ROMs, `--dev
quiet,arenaboth,arenahp=60`: Alpha challenges Beta, Beta says Yes, both
fight by `arenaboth`'s script (the harness in `.build/`, not committed).

| Pair | Delay | Window (Alpha, Beta) | Rollbacks, deepest | A pair frame | Worst tick | Past a 60th | Over | Hashes |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Two browsers | 2 | 4, 4 | 39, 1 deep (Alpha); 0 (Beta) | 2.40, 2.46 ms (worst 4.0) | 7.0, 5.0 ms | 0, 0 | frame 1,731 on both, Alpha's win | 28 agreed, the same on both |
| A browser (Alpha) against the host build headless (Beta), before the ping fix below | 8 | 5 and 4 (two runs), 8 | 0 | 2.21 and 2.04 ms (browser), 1.37 and 1.49 (native) | 4.0 and 3.37 ms | 0 | frames 1,734 and 1,736 on both, the browser's win | 28 agreed, the same on both, each run |
| The same, after it | 2 | 3, 7 | 39, 2 deep, 51 frames run again (browser); 0 (native) | 2.76 ms (worst 11.0), 1.66 (6.8) | 11.0, 6.8 ms | 0 | frame 1,730 on both, the browser's win | 28 agreed, the same on both |

Both pairs were made with the same installs' record as a native game's
(1,913,709 bytes, 132 hooks, `1efe82b0894607e2`), and the two browsers'
ready was `15974d28b169e19e`. A capture of the browser's picture
mid-battle against the native game shows the Custom screen and both
MegaMen charging, the HP 27 then 6 on both sides (`.build/`).

**The page's pings.** The first mixed runs got delay 8: the browser
asked with 146 and 162 ms of round trips, the native game with 2 or 3.
Its pings, timed on the page's one thread, take their answer in after
the frame they came during, and the first one after the arrival spanned
the Square's building: 193 ms, then 2 to 5 ms each, but one such ping
among the last eight makes their mean and twice their spread 150 ms for
16 seconds. A ping whose answer came after a frame of the page longer
than 50 ms is now left out (`online_transport.c`, the browser's), and
the same challenge asks with 6 ms: delay 2.

**Watching from a browser** (the same day): two host builds headless
fought (Alpha challenged, Beta took it, both by `arenaboth`,
`arenahp=150`, delay 2); a browser tab (Gamma, both ROMs) went to the
referee and said Yes to "WANNA WATCH?". Its pair agreed with the
players' ready at the Custom screen (`f6ed42d99ad29baa`); it caught up
from frame 0, 2,052 frames behind the keys, to 90 behind at frame 2,259,
showed the battle (a capture: the Custom screen, both MegaMen charging,
29 HP each), and the battle was over at frame 3,651 on all three, the
browser's 60 hashes checked against the players' 60.
