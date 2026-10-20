# The Arena

The Arena (epic #120) is a NetBattle of BN6's own between two players.
Each client runs **both** linked BN6 cores, joined by mGBA's own cable
(its `GBASIOLockstep`), and only the players' controller keys cross the
network. This is the research's design ([ONLINE_FRAME.md](ONLINE_FRAME.md)):
raw serial over a network stalls ([ONLINE_LINK.md](ONLINE_LINK.md)), while
keys sent ahead by a few frames let BN6's Practice NetBattle finish
identically at 0 to 150 ms one way.

This page is the engine (the pair, the way in, a carried MegaMan,
lockstep play over a link, `--dev arena` with a local stand-in for the
peer) and its network half: the link over the presence server, and the
way in from the Net Square, where one player's MegaMan asks another's for
a Practice NetBattle in BN6's own chat boxes ([From the Net
Square](#from-the-net-square), below).

## Run it

```sh
python3 build.py run -- --dev arena                       # a window: you are the first GBA
python3 build.py shot --dev arena --frames 2400 --input "80:,4:RIGHT,8:,4:RIGHT,8:,4:RIGHT,8:,4:RIGHT,8:,4:RIGHT,8:,4:A,200:,110:B,30:,..." --shot 600:/src/.build/arena.bmp
```

`--dev arena` starts on the Arena instead of the title, by full rules as
every NetBattle is: BN5's installs made on the game's core first, as a
run's start makes them, so it needs BN5's ROM beside BN6's (without it
the way in stops, saying so). The run's MegaMan is carried from the data
folder's checkpoint with the first folder that holds chips. The player
is the cable's first GBA from the keyboard or pad; the second is a
script: on the Custom screen it takes the cursor to OK and presses it, in
the battle it stands still. Its keys come over the local link, held back as a
network would hold them. `arenadelay=N` sets the input delay in frames
(default 6, 1-60), `arenalag=N` the local link's one way in ticks
(default 3, 50 ms), `arenawindow=W` the rollback window (0: the delay
alone; without it, what a frame costs here affords: [Rollback](#rollback)),
`arenabusy` makes the second player step, shoot and OK the Custom screen
by the frame's number alone (a run that rolls back and one that does not
hand him the same keys), and `arenahp=N` makes the carried MegaMan's HP N
(the installer seen in the battle). The battle's end, or A+B+SELECT+START,
goes back to the title; the log says each step (`arena:` lines).

## What runs

| Part | File | What it does |
| --- | --- | --- |
| The pair | `src/online/arena_pair.c` | Two cores of the player's ROM with BN5's installs, full rules being every NetBattle's ([Full rules](#full-rules), below); flash saves in memory, the cable's scheduling as mGBA's own frontend has it (whole run loops, the second core on the first's cycles) and both cores halted at BN6's wait for a frame ([Rollback](#rollback)); one frame of the pair, a hash of both cores |
| The installs' record | `src/emu/xinstall_log.c` | Every byte BN5's installs write into the game's own core's ROM copy and every hook they set, in their order, as the core starts: what goes in each Arena core |
| The Souls across | `src/online/arena_souls.c` | A Soul one core's player committed carried into the other core's Souls' block as the pair's frame ends: BN6's link carries the form, not the Soul |
| The way in | `src/online/arena_boot.c` | One core boots as `src/emu/boot.c` does (NEW GAME, the game's warp through its own call, hooked on that core alone); each side's MegaMan is written into a copy of its state; then the pair, on the cable, goes through BN6's menus (PET, Comm, Link Cable, NetBattle, Single Battle, Practice) with the research's scripted keys to the Custom screen. No state is kept or committed; it runs in slices behind a "connecting" picture |
| A carried MegaMan | `src/online/arena_loadout.c`, `arena_carry.c` | The `OnlineLoadout` (`online_snapshot.h`) written into BN6's own RAM through `bn6.h`'s names: the first folder, the pack's own ids, the NaviCust's board, HP (full) and base HP, the buster, Regular and TagChips, the Crosses' and Beast Out's flags. Checked by what the pair's cores hold, never by the game's own ROMs, so both games check a MegaMan alike. Refused, with the reason, where the cores cannot carry it: a chip id past the chip registry, a code past `*`, a max HP of 0 or past 9999. Before v9 BN6's rules refused BN5's chips and the Souls too. `arena_carry.c` reads the data folder's checkpoint, its `run.state` loaded into a plain core that never runs |
| Keys | `src/online/arena_input.c` | Both players' keys by frame in a ring; each stream in order, a resent frame left, a gap or a frame past the ring refused; for a frame run on a guess the other's keys not yet come held as his last, and the first frame a key that came otherwise reached |
| The time sync | `src/online/arena_sync.c` | The leading game's tick held now and then, spread out, by the lag of the other's keys against its own round trip; the round trips kept in the Square for the delay asked for ([Smoothed delay](#smoothed-delay), below); the rollback window a frame's cost affords and the latency asked for less what it covers ([Rollback](#rollback)) |
| The window kept | `src/online/arena_window.c` | What a battle frame of the pair costs on this system, kept in the data folder (`arena.ini`, never in a `.cwsave`), for the delay the next challenge or Yes asks for |
| The snapshot | `src/online/arena_pair.c` | The pair's state for a rollback, both cores and what no core's state holds (`arena_pair_save`, `arena_pair_load`), and where two differ ([Rollback](#rollback), below) |
| The measurement | `src/dev/rollbench.c` | `--dev rollbench`: what a rollback costs on the machine it runs on, and the window the Arena picks there |
| The rollback tried | `src/online/arena_roll.c` | `--dev arena,arenaroll=W`: whole battles rolled back W frames every W against the local stand-in, the game's state checked each time |
| The link | `src/online/arena_link.h`, `arena_link_local.c` | The interface below; the local stand-in |
| The engine | `src/online/arena.c` | The way in, then the battle: each frame at its tick, on a guess within the rollback window, rolled back where a guess was wrong; the hashes and the end taken up on frames whose keys are all known |
| The scene | `src/online/arena_scene.c`, `arena_text.c` | `--dev arena`'s scene, its words |
| The picture | `src/online/arena_view.c` | My core's picture, the notes over it, the way in's bar and its slices at the frame's pace; both scenes' |
| The wire | `src/online/arena_wire.c` | ARENA and its four messages packed and read as the server's `protocol.py` packs them; a carried MegaMan as bytes |
| The network's link | `src/online/arena_link_net.c` | `ArenaLink` over the transport's fast outbox: keys, hashes, the ready, the peer's end |
| From the Square | `src/online/square_arena.c`, `arena_words.c`, `online_mail.c` | The challenge, its question and answer, the battle with the Square's core held, the word after; MegaMan's lines; the inbox handed out by kind |
| The stands | `src/online/arena_watch.c`, `watch_keys.c`, `square_watch.c`, `arena_report.c`, `stands_report.c`, `stands_wire.c`, `online_stands.c`, `stands_words.c`, `stands_text.c` | A battle watched: its pair built and run from both players' keys; the referee's offer and the watching screen; the report the cable's first sends and the channel's battles as the server tells them ([The stands](#the-stands), below) |

### Lockstep

A key sampled at tick *t* (a 60th of a second) is frame *t + D*'s, *D*
the link's delay; the first *D* frames are released keys on both sides,
on every client. Frame *N* of the pair is due at tick *N* and needs both
players' keys for *N* and the frame after it: a linked core crosses into
its next frame a little before its partner (the two run within a few
thousand cycles of each other, [Rollback](#rollback); the research's
harness, an instruction at a time in turn, let one run two frames
ahead, and a frame needed the three after it), and each core takes the
keys of its own frame clock. With a rollback window *W* the frame runs at
its tick while at most *W* frames have run on a guess of the other
player's keys ([Rollback](#rollback)); with none it runs once all its
keys are known, so a delay of 1 frame plus the link's one-way latency
never waits. A frame that may not run waits, once, as many ticks as it
falls short, and stays that far behind. A frame that waits moves the game's
clock (issue #128): the tick after it samples no key and runs that frame,
so a wait shows one frame twice and never skips one; the game that leads
holds a tick now and then until it does not ([Smoothed
delay](#smoothed-delay), below). A tick catches up two frames at most.
A core as far ahead as the keys go waits for its partner. My keys run at most
128 frames (half `ARENA_RING`) past the pair's next frame: past that a
tick samples none and waits, so the other game held still for seconds
(its app sent to the background, its window dragged) stalls the battle on
both and it goes on when that game does. Before this (issue #129, 19
October 2026), the waiting game kept sampling a key a tick, filled its
256-frame ring about four seconds after the other stopped, and stopped
the battle with "my keys could not be sent"; the other, on its return,
with "the other player's keys came out of order".

Every 60 frames of the pair, once both players' keys for that frame are
known, the pair's hash after it (FNV-1a over both cores' states but the
sound's lazily kept fields, their flash and the cable's numbers:
`arena_pair_hash`) is sent over the link and the peer's for the same
frame compared; a difference stops the
battle and says at which frame (`arena: stopped at frame N: the two
games went apart`). Each hash is also printed: two runs of the same keys
print the same.

### `ArenaLink`

```c
typedef struct ArenaLink ArenaLink;
struct ArenaLink {
	bool (*send)(ArenaLink *link, uint32_t frame, uint16_t keys);       /* my keys for frame (tick + delay), in order, once each */
	bool (*poll)(ArenaLink *link, uint32_t frame, uint16_t *keys);      /* the peer's keys for frame, once they have come */
	void (*send_hash)(ArenaLink *link, uint32_t frame, uint64_t hash);  /* my pair's hash after frame */
	bool (*poll_hash)(ArenaLink *link, uint32_t frame, uint64_t *hash); /* the peer's for that frame, once it has come */
	void (*tick)(ArenaLink *link);                                      /* a tick has begun: once a tick, before the sends */
	const char *(*broken)(ArenaLink *link);                             /* the link gone: its reason, else NULL */
	void (*close)(ArenaLink *link);
	unsigned side;    /* my player: 0 the cable's first GBA, 1 the second */
	unsigned delay;   /* D: frames from a key's sampling to its frame (1-60) */
};
```

The engine asks `poll` for the peer stream's next frame until it says
no; keys are ten bits (`ARENA_KEYS_MASK`). Both clients must agree on
`delay` and on who is side 0 before the battle (the network's half), and
both build the pair with the two loadouts in the same order:
`loadout[0]` side 0's, `loadout[1]` side 1's.

## Measured

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

## Smoothed delay

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
  rollback window covers: [Rollback](#rollback).)

A game that truly falls behind still waits as before, and its picture
says so after 20 ticks ("Waiting for the other PET"); the 128-frame cap
on its keys still holds.

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

## Rollback

Issue #128's second step (the owner's decisions of 19 and 20 October
2026): rollback on every system the Arena runs on, measured on the
weakest first, its window and delay tuned per system; the pair made
cheaper first, then measured again on the Nova, then turned on. It runs
in every NetBattle now. Each frame of the pair runs at its tick; the
other player's keys not yet come are guessed (his last held), the pair's
snapshot kept before each frame that runs on a guess, and a key that
comes otherwise than guessed puts the pair back before the first frame
that read it and runs the frames since again, unheard, to where they
were. On a link the window covers, the input delay is 2 frames (the
smoothed delay's 8 to 11 on the same links). It serves what the smoothed
delay served: a NetBattle that answers the player's own presses.

### A cheaper pair frame

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

### The battle (`arena.c`)

- **A frame at its tick.** Frame *N* runs at tick *N* while fewer than
  the window's *W* frames have run on a guess (`arena_input_ready`); the
  other player's keys not yet come for *N* and the frame after it are
  his last known, held (`arena_input_take`), the same guess for a frame
  every time it runs until it is made anew. The pair's snapshot is saved
  before each frame that runs on a guess, in a ring of *W*
  (`ArenaPairSnap`, 797 KB each).
- **A wrong guess** (`arena_input_put` marks the first key that came
  otherwise): the snapshot before the first frame that read it is loaded,
  the guesses made anew from what is known, and the frames since run again
  (`roll_back`); their sound is dropped (`arena_pair_mute`), the picture is
  the last frame's.
- **What only the keys' truth decides.** What a frame left (the pair's
  hash at every 60th, the battle under way, both cores gone back to the
  Comm menu, the results) is noted as it runs (and again where it runs
  again), and taken up only once both players' keys for it and the frame
  after are known (`settle`): the hash sent and compared, the battle's
  end. So two players and a spectator, who runs the pair straight from
  the relayed keys, agree frame for frame.
- **The hash** leaves out the sound's lazily kept PSG fields
  (`arena_pair_hash`; below), on every game since v8: a state loaded and
  run again is not byte-equal there to the same frames run straight.
- A frame that may not run (*W* on a guess already) waits as before, and
  the clock moves as the smoothed delay has it; the time sync still holds
  the leading game. `--dev arena` takes the same engine with its local
  link (`arenawindow=W`, `arenabusy`); `arenaroll=W`, the rollback tried,
  runs with no window of its own.
- At the battle's end the game says its rollbacks: `arena: window 7,
  delay 2: 93 rollbacks, 300 frames run again, 5 the most at once; a
  frame of the pair 1.61 ms (the worst 2.31), a tick at worst 9.63 ms, 0
  past a 60th`.

### The window and the delay, per system

- **The window** is the battle's own: the way in times its pair frames
  (BN6's menus, about two thirds of a battle frame's cost: 0.66 against
  0.94 ms here, 0.81 against 1.05 on the Nova), and `arena_window_for`
  takes a battle's as 1.6 of them: the frames a tick can run again after
  one load, and the new one, each saved, within three quarters of a 60th
  of a second, at a tenth over the frame's cost, 10 at most
  (`ARENA_WINDOW_MOST`: past it a guess shows too long). This machine
  gets 9 or 10, the Nova 7 (rollbench: "the Arena's window 7 from them, 9
  from the battle's"). A battle times its frames and keeps their mean in
  the data folder (`arena.ini`, `frame_us`; `--dev rollbench` keeps its
  own too), the system's, never carried in a `.cwsave`.
- **The delay asked for** (protocol v8): a challenge or a Yes says the
  game's latency as before (its round trips in the Square, their mean and
  twice their spread) less what its kept window covers (`arena_ask_ms`:
  *W* frames of it; 4 where nothing is kept yet, a first NetBattle). The
  server's `delay_for` gives the lockstep's frame, the tick a key waits
  to be taken, and the longer latency of the two in frames: 2 at least,
  30 at most. So the pair's delay is the larger of the two games' needs:
  a fast desktop near the server asks for 2, a slower system or a long
  link for more, and a frame's keys come late by at most the window of
  the game that asked for the most.
- **Overrides:** `--dev arenawindow=W` sets the window (0: the delay
  alone, a frame run once its keys have all come; it also asks with that
  window), `--dev arenadelay=N` asks for N frames (in `--dev arena`, its
  delay).
- Systems: the desktops, Android and iOS build the Arena and its
  rollback; the 3DS and the browser compile the Arena out (`CW_ARENA`),
  so neither battles nor rolls back there.

### Measured

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

### The measurement: `--dev rollbench`

`--dev rollbench` (`src/dev/rollbench.c`, DEVTOOLS.md) runs on the
player's ROM wherever the game runs and says what a rollback costs there:
the game's core alone from power-on (every build; the 3DS and the browser,
which compile the Arena out, stop there and say so), then the pair through
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
  cores' ([A cheaper pair frame](#a-cheaper-pair-frame)). (An earlier cut
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
[Run it](#run-it) with steps between the shots, `arena_roll.c`): the
pair's snapshot kept before each frame, and every W frames the last W
rolled back and run again from the first's, the battle going on from
there. Under BN6's rules and under full rules, W = 1, 6 and 8: every
battle over at the frame run straight, all of the game's hashes the same
(20 October: 2,313 frames and 38 hashes with the old stepping; 2,738 and
45 with run loops and the halt). Before the registers and the SIO's
words were kept, each of them came apart at the first frame.

### What a rollback puts back

Everything outside the two cores' states that the pair's next frames
read:

| State | Where | Put back | How |
| --- | --- | --- | --- |
| Both cores: CPU, RAM, IO, video, timers, DMA | mGBA's state | yes | `arena_pair_save` / `arena_pair_load`: each core's raw state (no flash, no PNG); exact, measured |
| Each GBA's registers as they read, the SIO's control words (`rcnt`, `siocnt`), mGBA's idle loop detection, the open bus, a DMA under way, an interrupt sprung | `struct GBA`, outside its state | yes | in the snapshot (`GbaSnap`), put back after the state: a state loads each register through a write (`SIOMLT_SEND`, the SIO's busy bit came back otherwise) |
| The sound's PSG timing and rendered samples | mGBA's state | as a load leaves them | not byte-exact (above): the pair's hash leaves them out on every game since v8 |
| The flash saves | each core's save data | no | a Practice NetBattle writes none (assumed: not yet checked byte for byte); the hash reads them |
| The cable's lockstep: `mLockstep`'s attached, transfer phase and cycles; `GBASIOLockstep`'s attached counts and both receive buffers | `ArenaPair.cable` | yes | in the snapshot. A core's state loaded writes its SIO registers through the cable (SIOCNT's start bit can begin a transfer), so the cable is put back after both cores |
| Each node's next event, event difference, SO line, mode, finished flag, and its event's place in its core's timing | `ArenaPair.node` | yes | in the snapshot; a core's state loaded clears its timing (`mTimingClear`), and the node's event is scheduled again where it stood |
| The pair's scheduling: each side's cycle credit, who waits, who sleeps, who runs, the transfers counted | `ArenaPair` | yes | in the snapshot and the hash |
| The pair's frame clock: each core's frame at the anchor, the pair's frames since | `ArenaPair.start`, `relative` | yes | in the snapshot; each core's own frame counter is in its state |
| The Souls carried (full rules) | `ArenaPair.carried` | yes | in the snapshot |
| The Dark Soul's chip lists (full rules) | both cores' RAM | yes | written before the battle's first frame, in the cores' states |
| What BN5's hooks keep between two calls, one a core: `xsea.c`'s and `xpanels.c`'s way back through their stubs, `soulchaos.c`'s charge stepped (full rules) | static arrays by `hook_slot` | yes | each named as its hooks install (`hook_keep_slots`), a core's entries saved and loaded by its slot with its state (20 bytes a core); every other hook (the idle halt among them) is a function of the registers and its core's memory and what its install read from the ROMs. Measured: full rules' rollbacks exact for the game |
| UNITE's Souls offered (`soulunite_held_core`), BN5's installed bytes and hooks, the idle hook | each core's slot, ROM copy and hook table | not needed | made as the pair opens, the same through the battle |
| The keys (`ArenaInput`) | `arena_input.c` | not rolled back | the keys come are the truth; the other's not yet come are guessed (his last held) as a frame needs them, and a key that came otherwise rewinds `next` to the first frame that read it, the guesses made anew |
| What a frame left (its hash, the battle's end, the results) | `arena.c` | rewritten | noted as each frame runs, again where it runs again, taken up only once its keys are all known |
| The hashes pending, the counters, the link, the time sync | `arena.c`, `arena_link_net.c`, `arena_sync.c` | no | not the pair's state |
| Each core's sound out (blip buffers) and picture | `arena_pair.c` | no | output: the frames run again are heard not at all (`arena_pair_mute`), the last one's picture shown |

The stands' report reads the battle as it settles, never a guess: each
frame run notes both MegaMen as the first core has them (`note`, again
where a rollback runs it again), and the report takes the last frame
whose keys are all known (`arena_reading`). A battle of `--dev
arena,arenabusy` at delay 1 ended on frame 5221 with the same report
(HP 75/100 and 0/100, winner 1) and the same hashes with no guess (lag
0, window 0) as with 778 rollbacks (lag 3, window 6; lag 5, window 8).

## From the Net Square

Facing another player's MegaMan, A reads his card and then offers
`Battle`, `Wave` and `More...`, the second box `Mute`, `Block` and `Back`
(three to a box, as BN6's chat box holds them); Battle only where both
games' NetPasses hold the Arena stamp, the full set of ROMs
([ONLINE.md](ONLINE.md#the-netpass)). Battle sends ARENA's challenge, and
MegaMan says "OK! Let's see if NAME takes us on!". The server answers the
challenger WAITING and the other player CHALLENGED; his MegaMan asks,
once no chat is open: "NAME's MegaMan wants a Practice NetBattle!", then
"Take it,Lan?" with the cursor on No. Yes accepts; No, B, or a challenge
from a player he has blocked declines (the challenger's MegaMan: "Aw...
No NetBattle this time."). Before the challenge or the Yes goes, the
folder MegaMan carries is asked where Lan has a choice (below). A
MegaMan with no folder that holds chips, or one the last check refuses
(a NaviCust past what a message holds), does not go: MegaMan says so and
no challenge or acceptance is sent. The server refuses a challenge or an
accept from a game without the full set, and a challenge aimed at one
(NOTICE 28). The 3DS and the browser build, which compile the Arena out
(`CW_ARENA`), never offer Battle and decline a challenge.

### Rematches and the referee

A battle with a result (issue #128) is offered again. Back in the
Square, MegaMan's word on it ("Alright! We won,Lan!!") goes on to
"Again with NAME?", the cursor on No, once no chat is open and while
NAME is still in the Square. Nothing is won or lost, so the question is
the whole cost: the default lets either player go (the Square's pull
between staying together and moving on), and both must say Yes.

No new message: a rematch is ARENA's challenge again, answered at once.
Lan's Yes sends it quietly; the other game takes a challenge from that
player at once where its player said Yes too,
turns it down at once where he said No (or B), and, his question still
open or the result still on the picture, holds it for the answer. Both
Yeses at once cross on the server, which turns the second challenge down
as busy; the first is taken (`again_in`, `square_arena.c`). The folder
carried is the last battle's, asked again only where it no longer fits.
An answer is kept 45 seconds from the battle's end (`AGAIN_MS`); a
challenge then is an ordinary one, asked as ever. A Yes nobody answers
ends as any challenge does, after the server's 30 seconds ("Aw... No
NetBattle this time."). A battle stopped (a desync, a leave) offers none.
An older build, which knows no rematch, sees an ordinary challenge.

The referee Mr.Prog by the ring keeps what this game saw: before any
bout he tells how a NetBattle is asked for; after one, who beat whom
("LAST BOUT: YOU BEAT ALPHA!", "LAST BOUT: ALPHA BEAT YOU!") and, from a
second bout with the same player on, the count ("THAT'S 2 TO 1. YOU
LEAD!", "1 TO 1! ALL EVEN,ALL EVEN!"). As MegaMan comes back he calls
the bout over his head ("YOU WIN!", "ALPHA WINS!"), as the greeter calls
a log-in. A new opponent starts the count again; it is this game's
alone, kept while the game runs.

### One rule: full rules

Every NetBattle is by full rules since protocol v9 (issue #131): both
games hold the NetPass's Arena stamp, which takes the full set of ROMs
([ONLINE.md](ONLINE.md#the-netpass)), so BN5's chips go, in the folder
and in the pack's extension (every id of the chip registry, BN6's blanks
among them), and so do MegaMan's Souls: UNITE on the Custom screen as in
a run. Both pair cores of each game hold BN5's installs ([Full
rules](#full-rules), below). Nothing on the wire names the rules: v9
took the rules byte out of ARENA, ARENA_STATE, STANDS and WATCH_STATE,
and the server holds the pair to the full set instead (NOTICE 28).

From protocol v4 to v8 a challenge named one of two formats, chosen from
both players' ROMs: **full rules** between two games with BN5's ROM, and
**BN6 rules** for any other pair, plain BN6 cores in which the Souls
stayed home and each BN5 chip of the folder was stood in for by the
Pack's nearest BN6 chip for that battle (`arena_standin.c`, the owner's
interim decision of 18 October 2026). The server refused full rules
otherwise (NOTICE 19). v9 has no mixed pairs, so BN6 rules, the
stand-ins and NOTICE 19 went with them.

**The folder.** A run's MegaMan holds Folder1, Folder2 and the Extra
Folder (BN6's names; the second's Regular and TagChips at NaviStats
+0x2F and +0x58, bn6f's NaviStats; the Extra Folder has none of its own
here). `arena_folders_fitting` says which can go: each that holds a chip
(`arena_folder_holds`). Where more than one can, MegaMan asks in BN6's
chat box, "Lan,which folder goes?" (`arena_folder.c`, flags
`0x14A1`-`0x14A4`); B sends nothing. Where one can, it goes unasked. A
run holds Folder1 alone (no run hands out Folder2 or the Extra Folder),
so the question shows only with the `folder` dev step (DEVTOOLS.md). The
carried MegaMan (`arena_format_carry`: the folder picked, its Regular and
TagChips, as the one the core's Folder1 holds) then passes
`arena_loadout_check`, the same last guard on both games.

START gives each game its side of the cable (the challenger's 0) and the
input delay. Each sends its MegaMan (ARENA_LOADOUT: the folder picked, the
NaviCust's placed programs and grid, max and base HP, the buster,
Regular and TagChips, Crosses, Souls and Beast Out; the pack stays home)
and builds the pair from both, its own as the other reads it, so that the
two pairs are the same byte for byte; the other's is checked with the
local refusals. The Square's core holds still meanwhile (the scene runs
the battle's frames in its place: `online_visit_held`), its connection
kept, MegaMan standing where he stood on the server's view.

After the way in each game says it is ready with the pair's hash at the
Custom screen (ARENA_HASH at frame 0xFFFFFFFF), and starts its ticks when
the other's has come, so both start within the link's one way; a
different hash there stops both before the first frame. My keys go a
frame a message as each tick samples them (two frames a message, tried
first, left the earlier side waiting half its ticks at a delay of 6:
`ARENA_NET_FLUSH`); where the transport's outbox is full they wait and go
together, eight a message at most, and a ninth breaks the link. The
pair's hash every 60 frames goes too, and a difference stops the battle
on both games with a plain word on the picture ("The NetBattle stopped:
the two games went apart by frame N") and MegaMan's after it ("Whoa! The
NetBattle broke off!").

The battle's end is the cable's first's to tell the server (ARENA over),
after its last keys; as is a stop both games see (the hashes apart),
lest the server answer the second with a notice that the battle is over.
A stop of one game alone (a link broken, the cable stuck, the reset
chord: back to the gate) leaves it. The server's ENDED to the other
game: its link spends the keys already come, the battle over on the same
frame there as here, and breaks after `ARENA_NET_GRACE` ticks
("the other player left the NetBattle"; MegaMan: "Huh? They're gone...").
The connection lost breaks it at once. The result shows on the picture
(A or B after half a second, or five seconds, goes back), then MegaMan
says it in the Square where he stood ("Alright! We won,Lan!!", "Ugh...
They got us." and "Next time,Lan!").

### Full rules

Each game runs both cores of the battle, so each installs BN5 into both
from its own BN5 ROM, the same way, and the two games' pairs stay the
same byte for byte. The installs are the game's own: as its core starts
(`src/emu/scene_emu.c`), every byte BN5's installs write into the core's
ROM copy and every hook they set is written down in their order
(`src/emu/xinstall_log.h`: `xinstall_core`'s tracks, the moved objects,
the Navis, the chips' code, the families, the sea and the obstacle
strike, then the chip registry's `xchips_install` and the Souls'
`soulproxy_install` and `soulunite_install`; measured 1,913,709 bytes and
132 hooks, 1,735,251 and 42 of them before the Souls'). In every NetBattle the boot core and both pair cores
take those bytes into their ROM copies before they start
(`arena_core_new`), and each pair core takes the hooks on its own table
(`hook_core`, after its state is loaded); the boot core meets none of
them, its ROM copy has the bytes without their BKPTs. Nothing is
installed again: the installers' own state (the moved code's places, the
chips' sit-outs) is the game core's, made once from the same ROMs.

Their RAM: an install that patches IWRAM code keeps it patched
(`emu_keep`) and patches the ROM image the game copies it from too
(`BN6_IWRAM_IMAGE`), so a core started on the recorded ROM has it from
its power-on. The record is the ROM's alone.

The hooks run on whichever core meets them, each reading and writing that
core (`hook_read*`, `hook_write*`), the two cores' frames run by one
thread an instruction at a time in turn. Each hook BN5's chips need is a
function of the CPU's registers and its core's memory, with no host state
but what its install read from the ROMs (the Souls' too): the deck's sit-outs
(`xchips_install.c`), BN5's green flash (`xchips_flash.c`), the moved
families' and Navis' names, pre-steps and delete counts, the Dark Soul's
region, the obstacle strike's test, the lava's and sea's panels, a
Soul's look, life, powers, passives, chips and mood. What a hook keeps
between two calls of its own (`xsea.c`'s and `xpanels.c`'s way back
through their stubs, a sea or lava panel's; Chaos Unison's charge
stepped, `soulchaos.c`), which both cores could meet at the same
instruction, is kept one a core (`hook_slot`), and so are the Souls UNITE
offers (`soulunite_held_core`: each pair core its own player's, from his
carried MegaMan).

Two games whose records differ (a game that took BN5 in after its core
started, another build) cannot fight: the record's hash is part of the
pair's (`arena_pair_hash`), so the ready at the Custom screen differs and
both stop before the first frame. Each game sends its record's hash just
before its ready (ARENA_HASH at frame 0xFFFFFFFE, `arena_link_net_record`;
the server relays it as any hash), so a ready apart whose records differ
says so on both games: "the two games installed BN5 apart (one took BN5
in after it started, or another build)" on the picture, then MegaMan's
"Huh? Our PETs don't match..." (before v9 "...on BN5...": since, no
character names a game) and "Lan,let's reboot the PET and
try again!"; a ready apart with the same records (or from a game that
sends none) keeps "the two games went apart before the battle". A game
whose other player left after his ready came apart from ours tells the
same cause, not "the other player left". A game whose core holds no
record (BN5's ROM read but the core's installs refused) is told so at
the way in ("BN5's installs are not in this game's core").

**The Souls across the cable.** UNITE stands in the pair's link battles
(`unite_stands`: a run's battles are never link ones, and the rule kept
them out before the pair could carry a Soul). Its commit writes the
turn's record (the proxy form, its chaos, its turns), which BN6's link
carries to the other GBA as it carries a Cross, and the Soul in the
Souls' block of the GBA whose player chose it, which BN6's link does
not carry. Both GBAs number the sides alike (`BN6_BATTLE_SIDE`: the
challenger's core 0, the other's 1, read on both pair cores), so the pair
carries the Soul: as each frame of the pair ends, a Soul one core's
player has committed (his side's `BN6_SOUL_NEXT`, its chaos, turns and
ColonelSoul's chip, the Souls used) goes into the other core's same side
(`arena_souls.c`), once a commit, before either GBA's turn takes it up;
both GBAs then unite the same side with the same Soul (read on both
cores: form 8, Soul 11, its used bit 0x800, from the same frame on). The
Soul's sprite is drawn by the side its MegaMan stands on (`soullook.c`:
the sprite selector's caller's r5 where it is a side's player, else
MegaMan's own side, as before), so the other GBA draws him in his Soul
too. The 52 differentials of the Souls and Chaos Unison
(`build.py xdiff soul_* chaos_*`) pass with the change.

**What only one GBA writes** (issue #130). The Souls' block holds a few
things a GBA writes for its own player alone, as its own Custom screen
opens or closes; a pair holds both GBAs, so each was looked at for
whether the other GBA reads it (both pair cores' blocks compared every
frame through whole battles, a check kept out of the build):

- **The Soul's end.** The Custom screen's stand hook marked a screen
  opened with the Soul on (`BN6_SOUL_SCREEN`, which the end waits for)
  for its own player only, so the other GBA never ended his Soul: seen
  with ColonelSoul (`--dev arena`), side 0's Soul 0 on its own GBA and still 7 on the
  other's after the turn its three turns ran out. The hook now marks it
  for every side with a Soul on, before it asks whether this GBA's own
  player holds any (`soulunite.c`'s stand): in a link battle both
  players' screens open together. Seen since: the Soul gone on both GBAs
  at the same turn (a frame apart, as their cores run) and on both
  screens.
- **ColonelSoul's button.** The chip his OK keeps in the button
  (`BN6_SOUL_NEXT_CHIP`, `soulcolonel.c`) is his own GBA's alone, and BN6's
  link knows no button; the turn's start makes his charged shot the chip
  shot where it holds one, ScreenDivide where not, on each GBA from its
  own block. Seen without the carry (`--dev arena`): his GBA's block held
  MiniBomb (54), the other's 0, and from the turn's start his chip 54 on
  his own GBA and 0 on the other's, so his own screen showed the bomb
  thrown while the other GBA ran ScreenDivide: the two GBAs' battles
  apart, though both clients' pairs, alike, hashed the same. The pair
  now carries it as it carries a Soul, wherever it differs ("arena: side
  0's ColonelSoul chip 1 carried to the other GBA", then 0 at a screen
  with no chip).
- **BN5's mood** (`BN6_SOUL_MOOD`, its opening mark, the stand's and
  the slot's bytes): each GBA opens its own player's as its own Custom
  screen opens and reads it for him alone (UNITE's stand, the worried
  rule's DarkChips at the hand's end, `soulmood.c` and `soulunite.c`);
  the other side's stays 0 there and is never read (BN5's writers write
  nothing at 0). What it decides crosses as BN6's link carries a pick,
  and as the pair carries a Soul. A DarkChip used darkens its user on
  both GBAs (`soulmood_chip_used`, run where the chip is used, on both).
  Seen: side 0's mood 0x9E on its own GBA, side 1's 0x3A (worried) on
  its own, the rest of the block the same on both through the battle.
- **The Dark Soul's chip lists** (`BN6_DARK_LISTS`, what a failed Chaos
  Unison's Dark Soul and BN5's DarkInvs play, `soulchaos.c`). Each GBA
  made a side's list, as the Dark Soul or DarkInvs came, from its own
  player's folder, so the other GBA made it from the wrong one. BN5 makes
  each player's from his own save and its link exchange carries it at
  the battle's setup (`BN5_DARK_LISTS`, `0x08009B92`); BN6's link carries
  none. Seen (`--dev arena`, NumberSoul by DarkPlus, B released 70 frames
  into its charge, past phase 2): side 0's list DarkPlus (0x122) on his
  own GBA, Cannon (1, the other player's) on the other; then the Dark
  Soul at (6,2) on one and (6,1) on the other and MegaMan's HP 294 on one
  and 254 on the other: the two GBAs' battles apart, both clients' pairs
  alike. The pair now makes each side's list on its own player's GBA as
  both cores reach the first Custom screen and writes it into both
  (`arena_souls_lists`: "arena: side 0's chips for a Dark Soul, 1 of its
  folder's, on both GBAs"), and in a link battle the hook fills none
  (`soulchaos_lists_fill`). The two GBAs the same since, through the Dark
  Soul's stay.

**The proofs** (18 October 2026, the player's Gregar and Team Colonel
ROMs). `--dev arena` with a checkpoint whose Folder1 is thirty MrkCan1 M
(`--scene emu --dev folder=411`, then `--dev arena` on the same data
folder): "full rules: BN5's installs in every core, 1913709 bytes and 132
hooks (record 1efe82b0894607e2)", the same record a run's start makes,
both cores at the Custom screen after 1,817 frames, side 0's
folder `199b` (MrkCan1 M), the Custom screen showing MrkCan1 with BN5's
picture and 70; picked and used, its marker locked on the other MegaMan
and the cannon took him from 100 to 30, a second MrkCan1 the next turn
deleted him ("MEGAMAN WIN!"): over at frame 1,502, result 1 and 2, 24
hashes. With a checkpoint holding every Soul and a folder of thirty
JustcOne J (`--dev souls=0x3F,folder=140`): A on the first JustcOne, OK,
UP to the emblem's place, now UNITE, lit, A, then OK: KnightMan's
summon, MegaMan in KnightSoul with his three turns; both pair cores read form 8 and Soul 11 on side 0, its used bit 0x800,
from the same frame on, and the second core's picture showed the summon
and side 0 in KnightSoul as well.

Two clients on a local server (the harness of [the two-client
proof](#the-two-client-proof), below, both games with BN5's ROM beside
BN6's, seed 120's Square, `arenahp=100`, the `chip` dev step's `chip 411
M 30` making Alpha's Folder1 thirty MrkCan1 M): Alpha read Beta's card
("His PET holds BN5 chips too!"), chose Battle and said "Full rules!
Let's see if Beta takes us on!"; Beta heard "Alpha's MegaMan wants a
Practice NetBattle!" and "Full rules: Souls and BN5 chips too!", and said
Yes. Both games printed the same record (1,913,709 bytes, 132 hooks,
`1efe82b0894607e2`), both pairs reached the Custom screen after 1,817
frames with Alpha's folder `199b` (MrkCan1 M) on side 0, both sent the
same ready hash (`f5111e6c47a0e5b1`) and the same hash at every 60th
frame (22 agreed). Alpha's MrkCan1, named on both screens, took Beta from
100 to 30 on both; charged shots ended it, over at frame 1,336 on both,
Alpha's result 1 and Beta's 2, both back in the Square. Alpha holding
every Soul (`souls=0x3F`) and thirty JustcOne J (`chip 140 J 30`), Beta
none: UNITE lit KnightSoul on Alpha's Custom screen, "arena: side 0's
Soul 11 (its turns 3) carried to the other GBA" on both games, and on
both screens KnightMan's summon, Alpha in KnightSoul with his three
turns and his wrecking ball's hits (Beta from 100 to 98; Beta's charged
shots took Alpha down), the same ready hash (`1a03915af762605a`)
and 36 hashes agreeing to the end, over at frame 2,163, Beta's result 1
and Alpha's 2. With both folders MrkCan1 (the build before the Souls':
ready `e000d63d5380d5e0`, 38 hashes agreed, over at frame 2,311 by
charged shots) Alpha used his while Beta stood a panel away and moved,
and its marker wandered without a lock, the same on both games. The marker is BN5's code moved (the differential `chip_mrkcan1`
passes, and with its foe a panel away as well, hit in both engines).
Against a target that moves, `chip_mrkcan1_moving` (19 October 2026:
BN5's Handi free, teleporting from (6,2) to (4,1) as the mark comes up)
plays the same in both engines: the mark, its lock on Handi's new panel
and the cannon's 70 at the same frame, the hit a counter in both; it is
held to its recorded FAIL at t75, Full Synchro's own object each
engine draws its own way (no other difference, `--survey`). So BN6's
moved code does what BN5's does with a target that moves; the
NetBattle's wandering marker, the same on both games, was not put
right, for nothing in it differs from BN5's yet seen. With Beta's copy of
Alpha's keys changed from frame 300 (`arenadesync=300`): both stopped,
apart by frame 359. With Alpha's core made without BN5's families
(`--dev nofamilies`: a record of 1,571,997 bytes and 115 hooks,
`3cc9e56dc3808e94`): the ready hashes differed and both stopped before
the first frame ("the two games went apart before the battle"; since
issue #130 "the two games installed BN5 apart", on both games, 19
October 2026).

**Every Soul, both players, Chaos Unison and ColonelSoul's button**
(19 October 2026, issue #130, the two-client harness above with
`souls=0x3F` and `arenahp=300` on both games, each Folder1 thirty of a
Soul's chip by the `chip` step: RockCube, Invisibl, BusterUp, Boomer,
BblWrap, JustcOne). The same record (`1efe82b0894607e2`) and every hash
agreeing on both games in each run:

| Run | Alpha (side 0) | Beta (side 1) | Seen on both screens | Hashes |
| --- | --- | --- | --- | --- |
| Each Soul alone | ColonelSoul, ShadowSoul, NumberSoul, TomahawkSoul, ToadSoul at the first screen | Cannon A | the summon (the Soul's Navi over MegaMan, on the other GBA from the other side), MegaMan in the Soul, its three turns; ScreenDivide's 50 a hit (ColonelSoul won at frame 2,668), the slash, the dice bomb, the whole field grass and the swing, the notes' 30 and their paralysis | 45, 57, 56, 57, 57 |
| Both uniting | ShadowSoul, NumberSoul, KnightSoul at the first screen | TomahawkSoul, ToadSoul, ColonelSoul at the second | both Souls at once, each GBA carrying its own player's ("side 1's Soul 10 (its turns 3) carried"), the second summon on the first player's screen | 74 each |
| ColonelSoul's button | ColonelSoul (three RockCubes in a folder of Cannon A), a Cannon put in the button at the second screen | Cannon A | the Cannon fired as his charged shot (Beta 250 to 210), ScreenDivide the turn after (no chip), the Soul ended at the fourth turn's start on both | 94 |
| Chaos Unison | ColonelSoul by a DrkSonic, NumberSoul by a DarkPlus (thirty of it) | Cannon A | the Chaos picture and the summon, the Soul in its dark palette, one turn; ColonelSoul's charged B its DrkSonic on Beta's screen | 67, 67 |

Each run's Souls' blocks were compared between the two pair cores every
frame (a check kept out of the build): the same but for each GBA's own
player's mood, stand and slot (above) and a frame's lag as the two cores
take a commit or a turn in turn; the Soul's end and the button's chip
apart until the two fixes above.

**What was left of them** (19 October 2026, issue #130, the same harness;
a full-rules `--dev arena` where named). Each run's two pair cores were
compared every frame, now wider (a check kept out of the build): the
battle's objects, both MegaMen's objects, AI data and collision data,
the panels, the Souls' block and the Dark Soul's lists, the same on both
but for each GBA's drawing of its own mirrored field (a MegaMan's +0x20,
+0x9A to +0x9F, +0xA3 and +0xA5), each GBA's own player's mood, stand and
slot, and a frame's lag; every hash agreed on both games.

| Run | Alpha (side 0) | Beta (side 1) | Seen on both screens | Hashes |
| --- | --- | --- | --- | --- |
| A failed Chaos Unison | NumberSoul by DarkPlus, B released 70 frames into its charge | Cannon A | the failure's flash, Alpha out of the Soul, the Dark Soul at the back of Beta's area firing at Alpha (300 to 243); before the lists' fix above, the two GBAs apart | 67 |
| Chaos Unison's attacks | ShadowSoul by DarkInvs, TomahawkSoul by DrkLance, KnightSoul by DarkDril, ToadSoul by DarkWide (thirty of each), B held 45 frames | Cannon A | DarkInvs: Alpha under BN5's chip AI, his LongSwrd, MegEnBom and HiCannon taking Beta from 300 to 14; DrkLance: the lances up Beta's back column, Beta at the front untouched, Alpha healing on his Soul's grass; DarkDril: Beta 300 to 200; DarkWide: Beta deleted from 300, over at frame 1,365 | 35, 67, 67, 23 |
| The passives | ShadowSoul, ToadSoul at the first screen | Cannon A | ShadowSoul's FloatShoe (his collision flags' 0x20, `BN6_OBJ_FLOATS`) and ToadSoul's flag (his AI data +0x48's 0x20) set as each unites, on both cores at the same frame; TomahawkSoul's grass above | 57, 57 |
| NumberSoul's hand of ten (`--dev arena`) | NumberSoul (BusterUp in fifteen of thirty), its second screen's two rows of five: the second row's first two picked, OK | the script's | CrakShot from the second row as "CrakShot 60+10", its 70 taking the second player from 100 to 30, then the BusterUp beside it; over at frame 3,883 | 64 |
| A Soul's chip charge | KnightSoul (thirty JustcOne J, two picked, the second given up for UNITE), A held 130 frames in its turn | Cannon A | "JustcOne440", the chip charged and doubled, Beta deleted; over at frame 1,507. ToadSoul's (BblWrap, a barrier: no number to see) the same on both GBAs | 25, 64 |
| BN5's DarkChips, worried | thirty Cannon A, the first four DarkDril D; his mood held worried on his own GBA once the opening set it (a step kept out of the build, as `--dev worried` holds it on the game's core) | Cannon A | the first screen none of them; the second dealt DarkDril at the hand's end, its fifth, the cursor on it and the screen dimmed as BN5's; picked, OK, used: "DarkDril 100" named and the drill on both screens, Alpha dark (his emotion window), the third screen dealing the next DarkDril at its end | 81, 65 |
| MrkCan1 against a MegaMan who steps | thirty MrkCan1 M, used each turn | stepping up and down every 32 frames | the mark locked on Beta and the cannon took him from 300 to 230 | 64 |

The Pack stays home in a NetBattle, so what a worried MegaMan is offered
is the carried folder's: its BN5 DarkChips are shelved as in a run's
battle and one comes to the hand's end while he worries or is dark
(`soulmood.c`'s worried rule, on his own GBA's Custom screen; the pick
crosses as any). His mood opens calm in a NetBattle, both MegaMen at
full HP (the opening's rule makes him worried at a quarter of his HP or
less), and hits lower it as BN5's do. A DarkChip used there takes its
own after-effect, as in a run's battle. BN5's dispatcher of them
(`0x0802C938`) gives the HP bug a step in its place where the battle
state's bit 0x40 is set (`0x0802C95E`, through `0x080090E8`), which
`soulmood.c` reads as BN5's link battle's; BN6's twin bit (`0x0800A8F8`
through `0x0802E09A`) is clear in the Arena's NetBattles, read on both
cores through the worried run, so nothing there asks for BN5's other
rule (what sets BN5's bit in its own NetBattles is not yet read:
docs/ROM_DATA.md, BN5's DarkChips' after-effects).

MrkCan1: BN5's own link battle, MegaMan against MegaMan, is out of the
differential's reach (it runs one guest core, no cable). Its nearest is
`chip_mrkcan1_darksoul`: the Dark Soul, MegaMan's body under BN5's chip
AI, steps at t39 and t76 while the mark sweeps from t32 to t84, and
neither engine's mark locks or hits, frame for frame (PASS over 600
frames). So BN6's moved code does what BN5's does against a target that
steps; in a NetBattle it has locked on a stepping MegaMan (above) and
wandered without a lock (the earlier run), the same on both games each
time.

### The two-client proof

Two host binaries on this machine (`SDL_VIDEODRIVER=dummy
SDL_AUDIODRIVER=dummy`), each with its own `--data-dir` and a
`network.ini` naming a local server (`python3 server/build.py`, then
`build/server/cyberworld-server.pyz --config` with a free port and its
`state_dir` under `.build/`), BN6 alone in `--rom-dir` (before v9; since
v9 BN5's ROM beside it, or no Battle is offered), `--scene home
--run-depth 7 --seed 3 --dev quiet,arenahp=40` (`arenahp=N` makes each
carried MegaMan's max HP N, so the battle is short). Beta starts first
(session 1, arriving at -24,364), Alpha a second later; after the gate's
steps Alpha is put beside Beta (`place -8 364 5`), reads his card (A
three times) and picks Battle (A); Beta, 635 frames on, presses A, LEFT
(to Yes: the question's options lie side by side) and A. Both then press
RIGHT five times and A six times over (the Custom screen's OK) and fire
charged shots (B held 110 frames). The harness stays in `.build/`.

18 October 2026, the player's Gregar ROM, delay 6 (the server's least):

| Run | Way in | Ready | Battle | Stalls (side 0 / 1) | Hashes | Result |
| --- | --- | --- | --- | --- | --- | --- |
| Two keys a message | 1,817 frames on both | the same hash `fd2fa7d630cf9080` on both | over at frame 1,271 | 636 / 0 | 21 agreed | 2 and 1 (Beta won) |
| A key a message | the same | the same | over at frame 1,270 | 2 / 0 | 21 | 2 and 1 |
| The same, three pairs of games at once | the same | the same | over at frame 1,070 | 0 / 131 | 17 | 2 and 1 |
| `arenadesync=300` on Beta (Alpha's keys from frame 300 changed there) | the same | the same | stopped on both: apart by frame 359 (299 agreed) | | 5 | the stop's note, then MegaMan's word |
| Alpha's reset chord mid-battle | the same | the same | Beta stopped at frame 309: the other player left | | | Alpha at the gate ("We're home,Lan!"), Beta in the Square ("Huh? They're gone...") |

Both games printed the same hashes at every 60th frame. The two
processes' wall clocks put their battles' start within 20 ms and their
ends within 40 ms of each other (15 s of battle). Back in the Square,
both MegaMen stood where they had stood, the loser's word and the
winner's in their chat boxes.

**A mixed pair, BN6 rules, before v9** (the same day, protocol v4).
Beta with BN5's ROM beside BN6's, Alpha with BN6's alone, each with
Folder1 copied into Folder2 (the `folder 2` dev step) and Beta's Folder1
given MrkCan1 M: both named BN6 rules, Beta's Folder1 stayed home and
Folder2 went, and the battle was over at frame 1,568 with 26 hashes
agreeing. v9 offers no NetBattle to a game without BN5's ROM.

**A rematch** (19 October 2026, issue #128, protocol v5 unchanged). Two
BN6 games, `arenahp=10`, the harness above: Alpha challenged, Beta took
it, and the battle was over at frame 848 (Alpha won, 14 hashes
agreeing). Back in the Square each referee called it ("YOU WIN!" on
Alpha's game, "ALPHA WINS!" on Beta's) and each MegaMan asked "Again
with NAME?", the cursor on No. Beta's LEFT and A said Yes and sent the
challenge; Alpha's question was still open, so his game held it
(`arena: Beta asks for a rematch (match 2)`) until his RIGHT and A said
Yes, and took it at once. The second battle's pair hash at the Custom
screen was the first's (`8dc89c5dad9af3e7`: the same rules and folders),
and it was over at frame 1,328 (Beta won, 22 hashes agreeing; all 36 the
same on both games). The referees then called it 1 to 1 on both. Declined:
the same first battle, then Beta's B on the question (No); Alpha's Yes
came after it, and Beta's game turned his challenge down at once (`arena:
Alpha's rematch turned down`), Alpha's MegaMan saying "Aw... No NetBattle
this time.".

## The stands

The owner's decision on issue #128 (19 October 2026): a NetBattle in the
Square is watched both ways. A game with the NetPass's Arena stamp that
can build the battle's pair **watches it for real**, the same two cores
run from the players' keys a second or two behind them; every game, one
that cannot watch among them (the 3DS, the browser, a game without the
Arena stamp, a machine too slow to catch up), gets **the report**: each
MegaMan's HP, the last chip each used and the winner, from the referee by
the ring and on the Board.

The design (the game-design skill): the Square's dialectic, **gather
against go**, and the session loop (arrive, see who is here, do one thing
together, read what happened, go home). Watching is the one thing to do
together that needs no partner of one's own: it shows other players'
folders and plays (learning by watching, social proof), a diegetic
spectacle in the Arena's wing, the referee its barker. The patterns:
spectating as presence (the stands fill: "2 NAVIS WATCHING!"), cheers
that reach the players afterwards rather than during (a crowd felt, never
a coach), and a report for everyone so no system is left at the door. The
stands feed nothing into a run: no reward, no power, and nothing a
spectator sends reaches the battle.

### Watching

The server writes down every started battle as it relays it
(`server/cyberworld_server/stands.py`): both MegaMen (ARENA_LOADOUT), the
installs' record and the ready (ARENA_HASH at 0xFFFFFFFE and 0xFFFFFFFF),
the cable's first's hash every 60 frames, and both players' keys from
frame 0 (a side's first keys come at its input delay's frame, the frames
before released keys on every game). A player of the channel who asks
(WATCH) is sent the battle's setup (WATCH_STATE: the two sessions and the
delay; v9 dropped the rules, every battle being by full rules), then the players' messages (WATCH_RELAY) and every key
from the first frame (WATCH_KEYS, 240 frames a message at most), at most
2 KB a tick and only while his output is under 4 KB, so a spectator who
came late catches up without the players' relay ever waiting on him. A
gap in a side's keys, or a battle past 20 minutes, stops its watching;
the battle itself goes on.

The spectator's game (`src/online/arena_watch.c`) builds the same pair a
player's does (`arena_boot`, both carried MegaMen as their bytes came,
BN5's installs from its own record), so only a game with the Arena stamp
is offered a battle to watch, and the server refuses a WATCH from one
without the full set (NOTICE 28). At the Custom screen its pair's hash must be the players'
ready, or it stops ("the two PETs here came out apart from theirs";
"this PET's BN5 is not theirs" where the installs' records differ). Then
it runs the pair as `arena.c` does, frame N once both sides' keys for N
and the frame after it are in (`watch_keys.c` keeps the whole
battle's keys, about 240 bytes a second), each core on its own frame
clock. More than 110 frames behind the frames whose keys it has (BEHIND_KEPT and
BEHIND_LEAST), it catches up, silent: as many frames a tick as 12 ms allow (eight
a tick headless, so a capture's frames are the same on every machine),
until it is 90 behind; from there it runs one a tick with the watched
side's sound, and within 20 it waits. So it stays a second and a half
behind and rarely stalls; the keys' end (the battle over) runs the rest
as fast. Catching up that
gains nothing in ten seconds gives up ("this PET is too slow to catch
up"), and the referee does not offer that battle again: the next one.
Every 60 frames its pair's hash is checked against the cable's first's
for the same frame; a difference stops the watching ("the games went
apart by frame N"). The keys spent once the server has said the battle
is over, without both cores out of it, end it as broken off.

In the Square (`src/online/square_watch.c`, its words in
`stands_words.c` and `stands_text.c`): the referee's archive is his
words, then the channel's battle as the report says it, then, where this
game can watch it, "WANNA WATCH?" with the cursor on Yes. To a game
without the Arena stamp he says "NO ARENA STAMP ON YOUR NETPASS!", calls
the battle all the same and asks nothing. Yes sets the
Square's flag `0x14C4`; as the chat closes the game sends WATCH and, on
WATCH_STATE, holds the Square's core as a battle of its own does
(`online_visit_held`): the screen is the pair's picture, the cable's
first's side (its heading and keys shown four seconds: "WATCHING ALPHA VS
BETA", "R: other side  L: cheer  B: leave"; "Catching up..." while far
behind). R shows the other GBA's picture, L sends the Square's Cheer
emote, B leaves at once; the end shows "ALPHA won the NetBattle." until A
or B, or five seconds, and MegaMan is back where he stood. A spectator is
busy for the server: a challenge to him is refused (NOTICE 16), and his
game sends nothing of the battle.

### The report and the cheers

The cable's first game reads both MegaMen off its pair's first core at
the last settled frame each tick (`arena_reading` in `arena.c`, the
report in `arena_report.c`; `stands_report.c` the reading, ROM-free): each
side's player object (`BN6_BATTLE_PLAYERS`, the same on both GBAs, which
number the sides alike), its HP and max HP (`BN6_T1_HP`, `BN6_T1_MAX_HP`),
and a chip counted as used where his action turns to a chip's
(`BN6_T1_ACTION` 0x12-0x16) and the chip he uses next (`BN6_T1_CHIP`, the
hand's first) gives way the same frame: seen in `--dev arena` (MiniBomb,
54, picked and used: the field 0x36 until the action's 0x12, 0xFFFF
from that frame; a charged shot's action is 0x16 with no chip, not
counted). It sends what changed every 30 frames at most (ARENA_STATUS)
and the winner as the battle ends, before ARENA's over; `--dev arena`
prints it at the end ("arena: the report: HP 100/100 and 0/100, chips 1
and 0 (last 54 and 0), winner 1"). It only reads.

The server tells each channel its battles (STANDS) as they begin and end
and at most once a second as they change: who fights whom, how many
watch and cheer, the report. The referee says it in BN6's
Mr.Prog voice ("A NETBATTLE'S ON! ALPHA VS BETA!", "ALPHA HAS 40 HP!
BETA HAS 20!", "ALPHA USED MINIBOMB!", "2 NAVIS WATCHING!", then "ALPHA
BEAT BETA! WHAT A BOUT!" and the cheers), and the Board posts it
(">NetBattle now! Alpha vs Beta", ">Alpha beat Beta! 3 cheers."). A
chip of BN5's is named only by a game with BN5's ROM; another leaves its
line out.

A phrase or emote said in the channel while a battle is on is a cheer:
the server counts it for the battle and holds its EVENT from the battle's
two players, and STANDS too until their battle is over. Back in the
Square, the referee tells them: "THE STANDS CHEERED FOR YOU! 3 CHEERS!".
A spectator cheers with L while he watches, or with the phrase chat in
the Square as anyone does.

### The three-game proof

20 October 2026, the player's Gregar ROM (and Team Colonel's where
named), protocol v7, a local server with seed 120's Square, three host
binaries headless (`SDL_VIDEODRIVER=dummy`), each with its own data
folder and `network.ini`, from `--scene home --run-depth 7 --seed 3`, the
harness of [the two-client proof](#the-two-client-proof) grown to three
(kept in `.build/`): Beta placed at 28,404 and Alpha beside him; Alpha
challenges, Beta says Yes, both pick a chip and OK and fire charged
shots, `arenahp=150`; Gamma, BN6's ROM alone, walks to the referee
(at -428,44: MegaMan at -428,28 facing him) and presses A through his
pages. These runs were before v9: the players' battles in the table were
by BN6 rules, which Gamma could watch with BN6's ROM alone. Under v9
every battle is by full rules and Gamma would get the report alone, as
in the full-rules run below.

| Run | Gamma came in | Gamma's pair | Caught up | The battle's end | Hashes |
| --- | --- | --- | --- | --- | --- |
| Watched from the way in | as the players' pairs reached their Custom screen | the players' ready (`0f5e9abea55dced4`) | from 708 frames behind to 90 at frame 773 | frame 2,506 on all three, Alpha's win (the cable's first's result 1) | 41 agreed between the players, 41 checked by Gamma |
| The same, an earlier build | the same | the same | from 785 to 90 at frame 863 | frame 2,915 on all three | 48 and 48 |
| A spectator late | 28 seconds into the battle | the same | from 1,686 frames behind to 90 at frame 2,028, live after | frame 2,506 on all three | 41 and 41 |
| Gamma's copy changed (`arenadesync=1200` on Gamma) | at the way in | the same | at frame 841 | Gamma stopped at frame 1,260 ("the games went apart by frame 1259"), the players fought on to 2,506 | 41 agreed between the players |

On Gamma's screen: the referee's "A NETBATTLE'S ON! ALPHA VS BETA!",
"WANNA WATCH?" with the cursor on Yes, "TO THE STANDS! ENJOY THE BOUT!",
then the battle's Custom screen with "WATCHING Alpha VS Beta" and
"Catching up...", the battle with its keys shown four seconds, R turning
the picture to Beta's GBA and back (the HP in the corner each player's
own), and "Alpha won the NetBattle." at the end; back in the Square on
the referee's panel. The players stalled 0 to 6 ticks a battle, as
without a spectator, but for one run while the machine built other
targets for other work (402 ticks on Beta's game, the spectator's own
pace unchanged). Gamma's three L's while watching came back as three
cheers: the Board's "Alpha beat Beta! 3 cheers." and, to Alpha at the
referee after his battle, "LAST BOUT: YOU BEAT Beta!", then "THE STANDS
CHEERED FOR YOU! 3 CHEERS!" (the server holds a cheer's EVENT from the
two players while they fight: `server/tests/test_stands.py`). Alpha's
report, where his MiniBomb came up: HP 10 of 150 and 0 of 150, MiniBomb
(54) used once, the winner Alpha.

A battle by full rules (Alpha and Beta with Team Colonel's ROM beside
BN6's, Gamma with BN6's alone): Gamma's referee called it, "A
NETBATTLE'S ON! ALPHA VS BETA!", "ALPHA HAS 150 HP! BETA HAS 150!", and
asked nothing; the server refused a WATCH from a game without BN5's ROM
for such a battle (NOTICE 19 then; under v9 NOTICE 28 refuses any WATCH
from a game without the full set, `server/tests/test_stands.py`). Watched
instead by Gamma with Team Colonel's ROM beside BN6's (the same three
games, Gamma asking 1,368 frames into the battle): his pair made with
the same installs' record as the players' (1,913,709 bytes and 132
hooks, `1efe82b0894607e2`) and their ready (`7eaaa466008086ab`), caught
up at frame 1,592, the battle over at frame 3,080 on all three, 51
hashes checked against the players' 51.

The browser build, where the Arena is compiled out (`CW_ARENA` 0):
`tools/online_browser.html`'s probe served beside a local v7 server
allowing its origin, in headless Chrome (agent-browser), its MegaMan put
before the referee and A pressed through his pages while two host builds
fought (to frame 2,918, 48 hashes agreeing): "SORRY! NO NETBATTLES FROM
THIS PET!", then "A NETBATTLE'S ON! ALPHA VS BETA!", "ALPHA HAS 150 HP!",
later "ALPHA HAS 120", "ALPHA USED MINIBOMB!" and "ALPHA HAS 10 HP!" as
the cable's first's reports came, and no question.

## Not yet

- The battle's sound: the scene plays the player's core only where the
  game's own core has made its sound ring (`arena_pair_sound`).
- The NaviCust's programs are written as the board; BN6 makes HP from
  the base HP and the board again in the menus (seen: the max HP alone
  was put back to 100), but the other programs' effects in a NetBattle
  are not yet checked one by one.
- After the battle the engine stops once both cores have left it; the
  game's own way back to the Comm menu is not shown.
- Over a real network: measured on the deployed server (19 October 2026,
  docs/ONLINE.md) before the smoothed delay: delay 7, two full-rules
  battles to their end with every hash agreeing, the leading game waiting
  about one frame in eight (issue #128). The smoothed delay is measured
  through a local relay ([Smoothed delay](#smoothed-delay)), not yet on
  the deployed server nor between two devices; stalls still show on the
  picture after 20 ticks ("Waiting for the other PET").
- Rollback ([Rollback](#rollback)): on in every battle, measured through
  a local relay and on the Nova headless; not yet a NetBattle between two
  devices or through the deployed server (which runs an older protocol
  until it is redeployed with v9), nor on Android, iOS, Windows or macOS. A first NetBattle on
  a system asks with a window of 4 before its frame's cost is kept.
  Mispredictions are guessed as the last keys held:
  a press seen late shows as the other MegaMan's step snapping back.
- The Souls under full rules ([Full rules](#full-rules), above): all of
  issue #130's seen on both GBAs. Not yet: a NetBattle in which a
  MegaMan grows worried by hits alone (the runs held his mood worried),
  ToadSoul's flag on BN5's sea and FloatShoe over a cracked panel (a
  NetBattle's field has neither; both flags were read set on both GBAs),
  and BN5's after-effect rule for a DarkChip where its battle's bit 0x40
  is set, if BN5's own NetBattles set it.
- BN5's own link battle as a reference: the differential runs one guest
  core and no cable, so MrkCan1 against a stepping MegaMan is compared
  against the Dark Soul (`chip_mrkcan1_darksoul`), not against BN5's
  NetBattle.
- The NetPass's rule in the Arena ([One rule](#one-rule-full-rules)):
  every proof above with a game without BN5's ROM in it predates v9;
  what v9's rule was seen doing between games: (to be filled).
- The stands ([The stands](#the-stands)): watching on Android and iOS
  (built, untried), and a catch-up's pace in a window and on a handheld
  (measured headless, eight frames a tick); a spectator too slow to
  catch up (the refusal and the referee leaving that battle out are in
  the code, not seen); the report's chips past one a hand.
- Battle (not Practice) and its chip forfeit, the pack
  (it stays home), and any platform but the desktops: the 3DS and the
  browser build compile the Arena out (`CW_ARENA`), Android and iOS build
  it but are untried.
