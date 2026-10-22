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
hand him the same keys), `arenahp=N` makes the carried MegaMan's HP N
(the installer seen in the battle), and `arenacarry=N` sends him in with
N HP of his max, as a Team NetBattle's winner goes on ([Team
NetBattles](#team-netbattles)). `arenaseed=N` gives the bout the seed N
and `arenaagain=N` fights N bouts more, each from the pair kept as the
last began, its seed one more ([A rematch in a
second](#a-rematch-in-a-second)). `arenaboth` makes the first player
a script too (the Custom screen's OK, then charged shots), so the battle
plays itself on a device; `quitend` ends the game after it (and after
`--dev rollbench`, which `--dev rollbench,arena` runs first), for a run
sent over 3dslink. The scene shows the Square's screens (the way in's
panel, the name plate and the link's badge, a stall's note, the result
card: [The way in, the battle and the result](#the-way-in-the-battle-and-the-result)),
the players named "You" and "Beta"; the card's A, or
A+B+SELECT+START, goes back to the title; the log says each step
(`arena:` lines). The badge's three states are seen with `arenabusy`
and `arenadelay=1,arenalag=5` (rolling back) and with
`arenadelay=1,arenalag=110,arenawindow=0` (a stall at the battle's start,
the note after a second); `arenalag=200` overflows the local link's
queue, a stop's card.

## What runs

| Part | File | What it does |
| --- | --- | --- |
| The pair | `src/online/arena_pair.c` | Two cores of the player's ROM with BN5's installs, full rules being every NetBattle's ([Full rules](#full-rules), below); flash saves in memory, the cable's scheduling as mGBA's own frontend has it (whole run loops, the second core on the first's cycles) and both cores halted at BN6's wait for a frame ([Rollback](#rollback)); one frame of the pair, a hash of both cores; the picture drawn for the side shown alone (`arena_pair_draw`); on the 3DS one ROM copy for both cores ([The New 3DS](#the-new-3ds)) |
| The installs' record | `src/emu/xinstall_log.c` | Every byte BN5's installs write into the game's own core's ROM copy and every hook they set, in their order, as the core starts: what goes in each Arena core |
| The Souls across | `src/online/arena_souls.c` | A Soul one core's player committed carried into the other core's Souls' block as the pair's frame ends: BN6's link carries the form, not the Soul |
| The way in | `src/online/arena_boot.c` | One core boots as `src/emu/boot.c` does (NEW GAME, the game's warp through its own call, hooked on that core alone); each side's MegaMan is written into a copy of its state; then the pair, on the cable, goes through BN6's menus (PET, Comm, Link Cable, NetBattle, Single Battle, Practice) with the research's scripted keys to the Custom screen; as Practice's last confirm is pressed the bout's seed goes into BN6's random words on both GBAs, and the pair is kept there, in memory, for a rematch to go in from ([A rematch in a second](#a-rematch-in-a-second)). No state is written or committed; it runs in slices behind a "connecting" picture |
| A carried MegaMan | `src/online/arena_loadout.c`, `arena_carry.c` | The `OnlineLoadout` (`online_snapshot.h`) written into BN6's own RAM through `bn6.h`'s names: the first folder, the pack's own ids, the NaviCust's board, HP (full) and base HP, the buster, Regular and TagChips, the Crosses' and Beast Out's flags. Checked by what the pair's cores hold, never by the game's own ROMs, so both games check a MegaMan alike. Refused, with the reason, where the cores cannot carry it: a chip id past the chip registry, a code past `*`, a max HP of 0 or past 9999. Before v9 BN6's rules refused BN5's chips and the Souls too. `arena_carry.c` reads the data folder's checkpoint, its `run.state` loaded into a plain core that never runs |
| Keys | `src/online/arena_input.c` | Both players' keys by frame in a ring; each stream in order, a resent frame left, a gap or a frame past the ring refused; for a frame run on a guess the other's keys not yet come held as his last, and the first frame a key that came otherwise reached |
| The time sync | `src/online/arena_sync.c` | The leading game's tick held now and then, spread out, by the lag of the other's keys against the other's lag of its own (protocol v11's notes; before the first, its own round trip), on a tick nobody sees; the round trips kept in the Square for the delay asked for ([Smoothed delay](#smoothed-delay), below); the rollback window a frame's cost affords and the latency asked for less what it covers ([Rollback](#rollback)) |
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
says so (the link's badge, then after a second a note naming whose PET
it waits for: [The way in, the battle and the result](#the-way-in-the-battle-and-the-result));
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
  (`roll_back`); their sound replaces the guess's where it is not yet
  heard ([Quicker on the wire](#quicker-on-the-wire-issue-167); dropped
  before), the picture is the last frame's.
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
  server's `delay_for` gives the lockstep's frame and the longer latency
  of the two in frames: 1 at least, 30 at most. Since issue #167 a game
  says its latency less its window but a frame, the tick a key waits to be
  taken, which the server added for every pair before (2 at least then;
  [Quicker on the wire](#quicker-on-the-wire-issue-167)). So the pair's delay is the larger of the two games' needs:
  a fast desktop near the server asks for 2, a slower system or a long
  link for more, and a frame's keys come late by at most the window of
  the game that asked for the most.
- **Overrides:** `--dev arenawindow=W` sets the window (0: the delay
  alone, a frame run once its keys have all come; it also asks with that
  window), `--dev arenadelay=N` asks for N frames (in `--dev arena`, its
  delay).
- Systems: the desktops, Android, iOS and the browser battle and roll
  back ([In the browser](#in-the-browser), below). The New 3DS builds the
  Arena (`--dev rollbench` and `--dev arena` run there) but offers no
  NetBattle and no watching (`CW_ARENA` 0): a pair frame takes half again
  a 60th of a second there ([The New 3DS](#the-new-3ds)).
- **The picture** is drawn for the side shown alone (`arena_pair_draw`,
  mGBA's frame skip for the other core: the game's state the same either
  way), and a rollback's frames run again draw none but the last; a
  spectator draws the side he looks at, and while catching up a tick's
  first frame alone. Here a pair frame took 0.94 ms with both pictures,
  0.79 with mine and 0.70 with none (rollbench, 20 October 2026).

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

### In the browser

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

### The New 3DS

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
  11 ms a frame in play, docs/EMULATION.md), and my GBA's picture 5 ms
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
(docs/ONLINE.md, The NetPass).

### The measurement: `--dev rollbench`

`--dev rollbench` (`src/dev/rollbench.c`, DEVTOOLS.md) runs on the
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
| Each core's sound out (blip buffers) and picture | `arena_pair.c`, `emu.c`'s ring | the sound's ring cut back | output: my GBA's sound into the ring a frame at a time, each frame's mark kept; a rollback cuts the ring back to the first frame run again, what is not yet heard replaced by the frames run again and what was heard skipped (`emu_audio_rewind`, issue #167; before, they were heard not at all); the last one's picture shown |

The stands' report reads the battle as it settles, never a guess: each
frame run notes both MegaMen as the first core has them (`note`, again
where a rollback runs it again), and the report takes the last frame
whose keys are all known (`arena_reading`). A battle of `--dev
arena,arenabusy` at delay 1 ended on frame 5221 with the same report
(HP 75/100 and 0/100, winner 1) and the same hashes with no guess (lag
0, window 0) as with 778 rollbacks (lag 3, window 6; lag 5, window 8).

### Quicker on the wire (issue #167)

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
([Smoothed delay](#smoothed-delay)), so the two lags summed to the two
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

## From the Net Square

Facing another player's MegaMan, A reads his card and then offers
`Battle`, `Team up` and `More...`, the second box `Wave`, `Mute` and
`More...`, the third `Block` and `Back` (three to a box at most, as BN6's
chat box holds them; Team up since protocol v12: [Team
NetBattles](#team-netbattles)); Battle and Team up only where both
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
(NOTICE 28). The New 3DS, which builds the Arena but cannot run its
pair in time (`CW_ARENA` 0), never offers Battle and declines a
challenge.

### The way in, the battle and the result

Issue #167's first two steps: the NetBattle told on PET screens from the
first word to the last, nothing added to BN6's own Custom screen wait
("Cstmzing."), its turn banner or its win banner. The design's lens is the
Square's pull between staying for one more bout and moving on, with
nothing at stake: staying is made cheap, leaving clean.

**Refusals.** A challenge the server turns down is said by MegaMan in the
chat box, never the corner's notice: the target busy (NOTICE 16: in a
NetBattle, watching one, at Trade Central, his place held) "Huh? NAME's
busy right now...", no longer here (17) "Huh? NAME isn't here
anymore...", the Arena closed on the server (18), a NetPass without the
Arena's stamp (28) (`square_arena_notice` keeps the notice out of the
corner, `refused`). A refusal that comes while the Square is held (a
rematch's challenge sent from the card) shows on the card, and never in
the corner afterwards.

**The way in** (`arena_card_way`, `square_card.c`): from white into a
PET Network panel in the project's PET frame (`second_frame.h`'s colours
and slots, drawn here, never taken from the ROM): NETBATTLE and
Practice in the band; a plate for each player with his name, his
MegaMan's HP, his Crosses and Souls as carried (the other's once his
MegaMan has come); under each plate whose wait it is: "Sending..." while
the other's MegaMan is on its way, my PET's bar as it links, "Linking..."
blinking on the other's side until his ready comes, "Ready!" on each that
is; and the set's score with this player ("Set 1 - 0") from the second
bout on. The panel opens with BN6's window sound and goes into the Custom
screen through white with its encounter sound (`arena_card_cue`).

**The battle** (`arena_view_over`, `arena_view_stall`): on the Custom
screen only, the other's name on a small plate under BN6's own label of
the other MegaMan (both are "MegaMan" there), the engine's small letters,
gone once the turn starts. Beside it, and at the top right corner once
the Custom screen closes, the link's badge: bars only, no number (the
owner's decision, 21 October 2026): three green bars steady, two gold
bars where a guess was undone or a moment waited in the last second
(`ARENA_LINK_SHAKY`), one red bar blinking while the battle waits on the
other's keys (`ARENA_LINK_STALL`, ten ticks in a row; `arena_link_state`).
A wait past a second (`STALL_NOTE`) names whose PET it waits for
("Waiting for NAME's PET...") over a bar that runs down to its end; past
ten seconds (`STALL_OFFER`) "B: leave the NetBattle" is offered (B
before that may be meant for the Buster); at twenty (`STALL_END`) the
battle stops by itself ("NAME's PET stopped answering."). A server that
drops a silent game's link first ends it sooner ("NAME left the
NetBattle.").

**The result** (`arena_card_result`): after BN6's own banner and fade,
a card rises from black: WIN, LOSE or OVER, "VS NAME", the HP each side
has left and the chips each used (counted as the stands' report counts
them, from the frames whose keys are all known: `arena_tally`), the time
fought (BN6's fight, the Custom screen left out) and the set's score. No
Busting Level, no reward: those are a virus battle's. A stop shows on
the card too, in the PET's words, the cause in the log alone: BROKE OFF
for a bout begun ("NAME left the NetBattle.", "Communication error! The
two PETs saw the battle apart.", "We left the NetBattle."), STOPPED for
one that never began ("NAME's PET never got ready.", "NAME's MegaMan never
came."); MegaMan's word follows in the Square. A broken-off bout is
counted neither won nor lost: the referee calls it ("BROKE OFF! NO
CALL!"), the Board says "broke off.", and the set's score stays. A stop's
card goes back by itself after ten seconds; A or B before.

### Rematches and the referee

A battle with a result (issue #128) is offered again on its card (issue
#167): "Again with NAME?", Yes and No side by side with the cursor on
No, while NAME is still in the Square. Nothing is won or lost, so the
question is the whole cost: the default lets either player go (the
Square's pull between staying together and moving on), and both must
say Yes. Each sees the other's answer as it comes: "NAME wants another!"
once his Yes is in, "Waiting for NAME..." under one's own, "No rematch
with NAME." where he said No or left. B, or A on No, goes back to the
Square; the card's answer is kept 45 seconds from the battle's end
(`AGAIN_MS`), then it goes back by itself.

No new message: a rematch is ARENA's challenge again, answered at once.
Lan's Yes sends it quietly; the other game, its card still up, holds a
challenge from that player and shows it, takes it at once where its
player says Yes (or said it first), and turns it down where he said No
or closed the card. Both Yeses at once cross on the server, which turns
the second challenge down as busy; the first is taken (`again_in`,
`square_arena.c`). Both Yes: straight from the card into the next bout,
gone into from the last bout's pair as it began (protocol v13: [A rematch
in a second](#a-rematch-in-a-second), below), the panel showing the set's
score. The folder carried is the last battle's, asked in the Square only
where it no longer fits. A Yes nobody answers ends as any challenge does,
after the server's 30 seconds. A battle stopped offers none.

The referee Mr.Prog by the ring keeps what this game saw: before any
bout he tells how a NetBattle is asked for; after one, who beat whom
("LAST BOUT: YOU BEAT ALPHA!", "LAST BOUT: ALPHA BEAT YOU!", a bout
broken off "LAST BOUT WITH ALPHA BROKE OFF!") and, from a second bout
with the same player on, the count ("THAT'S 2 TO 1. YOU LEAD!", "1 TO
1! ALL EVEN,ALL EVEN!"). As MegaMan comes back he calls the bout over
his head ("YOU WIN!", "ALPHA WINS!", "BROKE OFF! NO CALL!"), as the
greeter calls a log-in. A new opponent starts the count again; it is
this game's alone, kept while the game runs.

**Seen** (21 October 2026, two host builds through a local server, BN6
and BN5 in both, `--dev quiet,arenahp=10,arenaboth`): Alpha challenged,
the panel, the battle (over at frame 529, 8 hashes agreeing), both
cards; Beta's Yes on his, Alpha's card showing "Beta wants another!",
Alpha's Yes took the held challenge (`Beta's rematch taken (match 2)`),
the second way in (the same pair hash at the Custom screen) with "Set
1 - 0" on its panel, the second battle over at frame 1,323 (22 hashes
agreeing), the cards at 1 - 1; then Alpha's Yes and Beta's B: Alpha's
card "Waiting for Beta..." then "No rematch with Beta.", the referees
calling bout 2. A game held still mid-battle (`arenafreeze=400` on
Alpha, its link kept): Beta's note from a second, "B: leave" from ten,
the stop at twenty ("Alpha's PET stopped answering.", BROKE OFF), Alpha
"Beta left the NetBattle."; with B pressed (Alpha's process stopped):
Beta left at 648 ticks ("We left the NetBattle."); both referees "BROKE
OFF! NO CALL!" with the count 0 - 0.

### A rematch in a second

Issue #167's third step. The lens (the game-design skill's transitions
and its frequency budget): a first bout with a player is rare and may
carry its ceremony, the panel's way in of a few seconds; a rematch is the
Square's inner loop ("one more") and must be near-instant, or staying
costs more than leaving. Before, both Yeses replayed the whole way in:
the boot core from power-on and both GBAs scripted through BN6's menus,
1,817 frames, about five seconds on a desktop. Now a rematch goes into
its battle in about a quarter of a second of work, the hands and the
battle its own.

**Where BN6's random numbers are at a battle's start** (the
reverse-engineering skill: located, understood, implemented, proved). The
two words are `BN6_RNG_PRIMARY` and `BN6_RNG_SECONDARY` (docs/ROM_DATA.md,
BN5's random numbers: BN6's at `0x020013F0` and `0x02001120`). Read on
both GBAs of the pair a frame at a time through the way in's last two
steps (a probe in `entry_frame`, not kept): both words are the same on
the two GBAs all through the way in, each stepped as on a map, until
Practice's last confirm. 19 frames after that press is done the battle
draws the first word once and keeps it; 71 frames on it puts the kept
value back as its opening ends and both folders are shuffled from the
second word the frame after (the first hands appear in
`BN6_BATTLE_DECK`); from there the first word moves only as the battle
draws. So the bout's seed goes in at the start of that step
(`BOUT_STEP`, `arena_boot.c`): both words written on both GBAs from it,
each mixed and kept off the step's shortest cycles (`battle_rng_word`, as
BN5's guest battles do), the same on the two GBAs as the way in leaves
them. Nothing before it reads them for the battle.

**The bout's seed is its battle ID**, which both players and every
spectator know (START, WATCH_STATE): no new message carries it, and each
bout, the first among them, deals its own hands (seed 0, `--dev arena`'s
own, writes nothing, so its battles are as before). **The pair is kept
where the seed goes in**: `arena_boot_keep` saves the pair's snapshot
(both cores and what no core's state holds, the rollback's
`ArenaPairSnap`) on that step's first frame, before the seed; the
battle's end leaves the pair and the snapshot to `arena_keep`, and a
rematch's `arena_new_bout` goes in from them (`arena_boot_again`): the
snapshot loaded into the same two cores, the new bout's seed written, the
press and the battle's opening run, 244 frames of the pair, not 1,817,
to the same Custom screen a whole way in with that seed reaches.

**What the kept pair carries, and what it must not.** It is gone into
only for the same two MegaMen on the same sides: both carried MegaMen's
bytes as the wire packs them (`arena_loadout_pack`: the folder, the
NaviCust, the HP carried among them, a Team bout's included) are its key,
and anything else (another folder picked, a carried HP that changed,
the sides swapped) goes the whole way, the kept pair freed first. The
server keeps the sides of a pair of players' last battle for their next
(protocol v13: a rematch's challenger may be either), so both games go
in from their kept pairs. What the snapshot holds is a rollback's (What a
rollback puts back, above); the flash saves it leaves out, as a battle
writes none (a whole way in's ready and a kept pair's agree, below, which
they would not if it did). The stands keep nothing: a spectator builds
the pair the whole way, with the same seed, catching up from the
battle's first frame as ever, so his ready is the check that the kept
pair and the whole way agree.

**The proofs.** `--dev arena,arenaboth,arenalag=4,arenadelay=1`:

| Run | The first hands (each GBA's first five) | The battle | Its 44 hashes |
| --- | --- | --- | --- |
| no seed (as before) | `0001 34c0 0236 0201 2447` on both | over at frame 2,690 | the last `86d2f837337a1494`, as before this change |
| `arenaseed=7`, the whole way | `1636 2448 2448 0201 0605` on both | over at frame 2,690 (the script fights with the buster alone) | the last `7f529eb93f607b8e`: every one of the 44 apart from no seed's |
| `arenaseed=8`, the whole way | `0236 3459 34af 009a 0236` on both | over at frame 2,690 | the last `c2f08435ce33c6d4` |
| `arenaseed=7,arenaagain=1`: bout 1 the whole way, bout 2 (seed 8) from the kept pair | bout 2 `0236 3459 34af 009a 0236`, seed 8's | both over at frame 2,690 | bout 1 seed 7's 44, bout 2 seed 8's 44, byte for byte; bout 2 ready 4 ticks (192 ms here) after bout 1 ended |

`arenaseed=N` sets the local bout's seed and `arenaagain=N` fights N
bouts more, each from the kept pair with the seed one more
(docs/DEVTOOLS.md).

Between games (21 October 2026, protocol v13, the player's Gregar ROM and
Team Colonel's in each game's `--rom-dir`, a local server with seed 120's
Square, three host builds headless from `--scene home --run-depth 7
--seed 3`, driven by remote play from a harness that reads each game's
log, kept in `.build/`): Alpha and Beta with `--dev
quiet,arenahp=10,arenaboth`, Gamma with `quiet`. Alpha challenged Beta;
match 1 (seed 1) went the whole way in on both: the ready
`a653ab54a0e16600` on both, both GBAs dealt `34b8 24a3 0605 0201 34c0`,
over at frame 530 with 8 hashes agreeing, Alpha's win. Both said Yes on
the card (Set 1 - 0); match 2 (seed 2), its sides the last match's, went
in from the kept pair on both games ("the way in again from the last
bout's pair"): both ready 7 and 14 frames after START (headless, 64
frames of the pair a tick), the ready `6a61e6f60b956964` on both, both
GBAs dealt `0001 2447 0605 2448 0001`. Gamma asked the referee to watch
match 2 and built its pair the whole way with seed 2: his pair agreed
with the players' ready at the Custom screen, caught up from 593 frames
behind at frame 569, and the battle was over at frame 1,319 on all three,
Beta's win; the players' 21 hashes the same list on both games (the last
`70f0f35710d27130`), all 21 checked by Gamma. Both cards read Set 1 - 1,
Gamma's end "Beta won the NetBattle.". The mutation (`arenaseedoff=1` on
Beta, his seed one more than the match, the same harness): match 1's
ready `a653ab54a0e16600` on Alpha (seed 1) and `6a61e6f60b956964` on Beta
(seed 2, the very ready of match 2 above: a pair's ready is its two
MegaMen and its seed), and both stopped before the first frame ("the two
games went apart before the battle").

### The Square before a NetBattle

Issue #167's fourth step: what a player sees of the Arena while he walks
the Square, before anyone commits. The lens: the session loop (arrive,
see who is here, do one thing together), the Square's pull between
gathering and going, and telegraphs that tell the truth: who is free,
what a challenge is doing, how good the link is, with nothing in a
character's mouth about delay (docs/VOICE.md).

- **Busy marks** (P1). A small mark of the project's own drawing over a
  player's name: crossed blades in a NetBattle (his own, or a bout of his
  team match), an eye watching one, a chip at a trade; nothing over a
  player who is free or whose challenge is only asked. The server says it
  in SNAPSHOT (protocol v13), so a challenge to a busy player (NOTICE 16,
  "Huh? NAME's busy right now...") is mostly not made.
- **The bout in the world** (P20). The two fighters' blades pulse
  between red and gold while the bout runs, and the referee calls it over
  his head ("ALPHA VS BETA!"), again every ten seconds while it lasts
  (`bout_on`, `square_life.c`), as he calls a log-in's greeting and a
  bout's end.
- **A challenge open, seen and called off** (P3). The challenger's game
  shows a strip at the picture's top while his challenge is open: "Ringing
  NAME's PET..." with a mark going to and fro until the other reads the
  question, then "NAME is reading it..." over a bar running down his 30
  seconds, the link's bars at its end and "A on NAME: call it off". On
  that player's card Battle reads `Call off`: the challenge ends for both
  (ARENA's leave) and MegaMan says "OK. We'll leave NAME be for now.";
  Battle on anyone else meanwhile is MegaMan's "Lan,we already asked
  NAME!", not silence.
- **The challenged's PET rings, and his clock starts when he sees the
  question** (P4). As CHALLENGED comes his PET rings (BN6's effect `0x7E`,
  `ARENA_CUE_RING`; which effect BN6 rings with for a call is not yet
  checked) and a blinking "!" stands over the challenger's name on his
  screen alone. The question still waits for any chat box to close; as it
  opens his game tells the server (ARENA 5, `arena_seen`), which counts the
  30 seconds from then and tells the challenger, whose strip starts
  running down. A challenge never seen ends after 60 seconds.
- **The challenger's card in the question** (P5). Before "Take it,Lan?"
  MegaMan reads the challenger's card as A on his MegaMan reads it (his HP,
  act and layer, Crosses, Souls; never the folder), from the card the game
  already asked for (`square_card_facts`).
- **The link before committing** (P6). Bars, never a number (the owner's
  word): each game says its round trip in its PING (protocol v13), the
  server puts each player's bars in SNAPSHOT (three under 100 ms, two under
  200, else one), and a game shows the fewer of his and its own, the longer
  round trip deciding a battle's delay: at the top right while his card is
  open in the chat box, while his challenge's question is, and on the
  challenger's strip (`square_ask.c`, drawn in the battle badge's look,
  `arena_view_bars`).

Seen (21 October 2026, the harness of [A rematch in a
second](#a-rematch-in-a-second), headless captures): Alpha's A on Beta
(the card, its bars at the top right), Battle, and the strip: "Ringing
Beta's PET..." with its mark sweeping, then, as Beta's question opened
(the server's SEEN), "Beta is reading it..." over the bar running down;
on Beta's screen the "!" blinking over Alpha's name, "Alpha's MegaMan
wants a Practice NetBattle!", "HP 100 of 100. Act 3,layer 7!" and "Take
it,Lan?". Alpha's A on Beta again: `Call off` in Battle's place, "OK.
We'll leave Beta be for now.", the strip gone on Alpha's screen and the
"!" on Beta's. In the rematch's run, Gamma by the referee read "ALPHA VS
BETA!" over his head as match 1 began and, beside the two, the crossed
blades over both their names, pulsing.

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
here). `arena_folders_fitting` says which can go: each that holds a chip.
Where more than one can, MegaMan asks in BN6's
chat box, "Which folder goes,Lan?" (`arena_folder.c`, flags
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
on both games, the log saying at which frame ("the two games went apart
by frame N"), the result card in the PET's words ("Communication error!
The two PETs saw the battle apart.") and MegaMan's after it ("Whoa! The
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
that cannot watch among them (the New 3DS, a game without the Arena
stamp, a machine too slow to catch up), gets **the report**: each
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

The browser build before 20 October 2026, when the Arena was compiled
out there (`CW_ARENA` 0):
`tools/online_browser.html`'s probe served beside a local v7 server
allowing its origin, in headless Chrome (agent-browser), its MegaMan put
before the referee and A pressed through his pages while two host builds
fought (to frame 2,918, 48 hashes agreeing): "SORRY! NO NETBATTLES FROM
THIS PET!", then "A NETBATTLE'S ON! ALPHA VS BETA!", "ALPHA HAS 150 HP!",
later "ALPHA HAS 120", "ALPHA USED MINIBOMB!" and "ALPHA HAS 10 HP!" as
the cable's first's reports came, and no question.

## Team NetBattles

Epic #167's multi-battles, the owner's decision of 21 October 2026: a
**relay** first. Teams of two; each team's order picked blind; the leads
fight today's NetBattle; the winner stays on with the HP he has left and
the loser's partner comes in; a team is out when both its MegaMen are
deleted; whoever waits watches live from the stands. Two MegaMen a side
on one field is a separate engine track ([Two MegaMen a
side](#two-megamen-a-side)) and shares none of this code.

### The design

The game-design skill's lens, before the numbers and screens:

- **The dialectic: mine against ours.** Every MegaMan is a run's (its
  folder, its NaviCust, its HP), and a team asks what he is for the
  pair: who goes first (the lead spends HP the anchor never gets back,
  the anchor sees the other team's folders before he goes in), and
  whether the HP a win leaves is the team's gain or its next loss. Under
  it the Square's own pull, gather against go: a team pulls hard toward
  gather, so every step lets a player go (a No on the question, Split up
  on the partner's card, a leave that breaks the match off for nobody's
  loss).
- **The loop layer: the session loop** (arrive, see who is here, do one
  thing together, read what happened, go home), its "one thing together"
  grown from two players to four. Nothing comes home: no reward, no
  power, no record kept but the referee's words (the Square's rule since
  issue #120).
- **The patterns:** *a blind pick* (each captain names his lead without
  seeing the other's, KOF's order select: a small guessing game between
  lead and anchor); *attrition as the brake* (the winner's HP carried, no
  heal: a sweep costs the sweeper; KOF heals a little, ours starts with
  none and watches whether anchors sweep); *give the waiting player a
  verb* (he watches the bout live, with stakes: the folder he may meet
  next); *telegraph honestly* (the order shown to all four before the
  first bout, the result card's MegaMen standing and who is next, the
  way in's HP of his max); *subtract* (two a side, the relay before one
  field, BN6's own two-Navi engine untouched); *name failure in the
  fiction's voice* (a stop broke off, never a loss). Canon: BN5's
  Liberation Missions, a team that fights one Navi at a time.

The numbers: the team card stays up 6 seconds (`TEAM_CARD_TICKS`) while
the match goes on (the next bout's way in follows it; A closes it
sooner), 10 at the match's end as a stop's card does; a captain's order
waits 30 seconds (`ORDER_WAIT`), after which he leads himself; a bout
waits 30 seconds for both its fighters (the Arena's `PENDING`), after
which the match breaks off; a proposal to team up waits 30 seconds
(`ASK_WAIT`). No heal on a carried HP.

### Forming a team

On another player's card, where both passes hold the Arena stamp, the
chat box offers `Battle`, `Team up` and `More...`, then `Wave`, `Mute`,
`More...`, then `Block` and `Back` (BN6's box holds three; Team up beside
Battle). Team up asks him (TEAM propose): MegaMan says "Let's see if NAME
teams up with us!"; the other's MegaMan asks, once no chat is open,
"NAME wants to team up with us!" and "Team up,Lan?" with the cursor on
No. Yes makes them a team ("Alright! We're a team with NAME!" on both);
No, B, or 30 seconds without an answer turn it down ("Aw... NAME won't
team up this time."). On the partner's card the choice reads `Split up`,
and either one's Split up, his leaving the channel or the server, or a
dropped link split the team ("OK... Our team with NAME split up."). The
one who asked to team up is the team's **captain**. A player on a team
asked to team up again is answered by MegaMan ("Lan,we're with NAME
already!"), and an ask the server refuses is MegaMan's word too, never
the corner's notice: NAME on a team already (NOTICE 29, "Huh? NAME's on a
team already..."), busy or gone (16, 17) as a challenge's are.

A Battle between players of two teams is a **team challenge**: the
server checks all four free (in no battle, trade, watch or match;
NOTICE 16 otherwise); the challenged's MegaMan asks "NAME's team wants a
Team NetBattle!" and "Take it,Lan?". A Battle from a team's player to a
lone one, or the other way, is refused (NOTICE 29, `notice_team`),
MegaMan saying why ("Huh? NAME has no partner...", "Huh? NAME's on a
team..."); one on his own partner's card MegaMan answers at once, as the
server would refuse it ("Huh? NAME's on our team,Lan!"). A lone
player's Battle with a lone player is today's NetBattle, unchanged.

### The match

The challenge's Yes begins the match (TEAM_STATE match to all four; the
challenge's own battle is dropped without an ENDED). Each captain's
MegaMan asks "Who goes first,Lan?" with `We do` and the partner's name
(B: we do); the server keeps the first pick in and shows nothing until
both are, then sends all four the order (TEAM_STATE ordered), and each
MegaMan says it ("We're up first,Lan! Against NAME!", or "NAME goes
first! Go get 'em,NAME!").

Each **bout** is an ARENA battle (`arena.py`'s Match, its id the bout's)
the server calls (TEAM_STATE bout to all four): its two fighters' games
go into it once nothing is on their screen (MegaMan's word read, the
last card closed, a watch over) by ARENA accept, and the server STARTs
both once both have. From there it is today's NetBattle: the way in's
panel (its band says Team), the Custom screen, the battle, the hashes,
the report. As it starts (TEAM_STATE fight), the two others' games ask
the server to watch it, unasked, as the referee's Yes would
(`square_watch_team`): the waiting partner, and the MegaMan already
deleted.

The bout's winner is the cable's first's report (ARENA_STATUS's winner,
`stands.py`): the loser's team's next comes in; a bout with no winner
puts both out. The staying MegaMan carries **the HP he has left**: his
own game's tally at the battle's end (`square_arena_last_hp`), logged
beside the server's report ("we stay on with 7 HP (the server's report:
7)"), and carried in his MegaMan's bytes (ARENA_LOADOUT v2, `hp` beside
max HP). BN6 makes HP whole again from the NaviStats in its menus on the
way in, so the carried HP is written where the battle has made its
objects: as the pair reaches the first Custom screen (`carried_hp` in
`arena_boot.c`), into that side's battle object's HP (`BN6_BATTLE_PLAYERS`,
`BN6_T1_HP`) on both GBAs of the pair, read back from both, before the
pair is anchored and hashed. Both games and a spectator build the same
pair, so the ready's hash covers the carried HP: a game that carried
another number stops before the first frame. Seen with `--dev
arena,arenacarry=40` (the carried MegaMan going in with 40 of 100): the
HUD's 40 from the Custom screen through three turns, on both GBAs.

The **result card** of a team bout shows, in the rematch's place, the
MegaMen standing ("Standing 2 - 1", ours first) and who is next ("Next:
Alpha vs Delta"); no rematch is asked. It closes itself after 6
seconds while the match goes on, and the next bout's way in follows (or
the stands, for a game whose MegaMan waits). At the match's end the card
says the team's result (TEAM WIN!, TEAM LOSE, TEAM DRAW where both
teams' last MegaMen went down in one bout, TEAM BROKE OFF), and MegaMan
says it back in the Square ("Alright!! Our team won,Lan!!", "Aw... Our
team lost.", "Phew... Both teams went down at once!", "Whoa! The Team
NetBattle broke off!"). A leave or a dropped link mid-match, a bout
broken off or never begun, ends the match **broken off** for both teams:
no result for anyone, as a lone bout broken off counts neither won nor
lost. The teams stay teams afterwards, for another.

The referee calls a team's bout ("TEAM NETBATTLE! BOUT 2!", then "ALPHA
VS DELTA!") and the Board posts it (">Team NetBattle, bout 2! Alpha vs
Delta"), from STANDS's bout byte.

### On the wire (protocol v12)

TEAM (33, client → server: an action, a session, a value) and
TEAM_STATE (34, server → client: the stage, the partner, the match, the
bout under way, the four players in order, each team's MegaMan up now
first, the MegaMen standing each side, the bout's number, the result,
the staying MegaMan's HP as the report left it); STANDS's entry grows a
byte, the team bout's number; ARENA_LOADOUT's MegaMan, its own version
2, carries his HP; NOTICE 29 refuses a challenge between a team and a
lone Navi. `server/cyberworld_server/teams.py` keeps the teams and the
matches; `server/tests/test_teams.py` the relay from the ask to the last
bout, a blind order, an order not picked, a draw, a drop, and a team
split between the challenge and its Yes (the challenge ends, no match).
[shared/online/README.md](../shared/online/README.md) has the bytes.

### The four-game proof

21 October 2026, the player's Gregar ROM and Team Colonel's in each
game's `--rom-dir`, protocol v12, a local server with seed 120's Square,
four host builds headless from `--scene home --run-depth 7 --seed 3
--dev quiet,arenaboth,arenahp=40` (`arenaboth`: the battles play
themselves, the bouts' fighters' keys a script), driven by remote play
(`--remote`) from a harness that reads each game's log (kept in
`.build/team/`): Beta, Alpha, Delta, Gamma join; Alpha asks Beta, Gamma
asks Delta, each answered Yes; Alpha challenges Gamma, who takes it;
Alpha picks Beta to lead, Gamma himself.

| Bout | Fighters (side 0 first) | Carried | The ready's hash, on both players and both spectators | Over | Hashes | Winner, HP left |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | Beta vs Gamma | none | `85776fd9af3f0dc2` | frame 1,250 | 20 agreed, 20 checked by Alpha and Delta | Beta, 6 of 40 |
| 2 | Beta vs Delta | Beta 6 of 40, "written on both GBAs" on all four games | `ae970b1e7c3951ca` | frame 655 | 10 agreed, 10 checked by Alpha and Gamma | Delta, 29 of 40 |
| 3 | Delta vs Alpha | Delta 29 of 40 (side 1), on all four games | `eb579513b4c5f1d2` | frame 1,010 | 16 agreed, 16 checked by Beta and Gamma | Alpha, 17 of 40 |

Each staying MegaMan's own tally matched the server's report ("we stay
on with 6 HP (the server's report: 6)", 29 and 29), and the HUD showed
the carried HP from the Custom screen on (6 for Beta in bout 2). The
loser of a bout watched the next one, unasked, as the waiting partner
did (Beta watched bout 3). The match ended TEAM WIN on Alpha's and Beta's
cards ("Standing 1 - 0") and TEAM LOSE on Gamma's and Delta's; between
bouts, Beta's card after bout 1 read "WIN VS Gamma", "Standing 2 - 1",
"Next: Beta vs Delta"; back in the Square, "Alright!! Our team won,Lan!!".

**A drop mid-match** (the same harness, `--drop`): Delta's game killed
three seconds into bout 2 (Beta, 7 of 40 carried, against Delta). Beta's
battle stopped at frame 192 ("the other player left the NetBattle"), his
card BROKE OFF with "Delta left the NetBattle.", "Standing 2 - 1" and
TEAM BROKE OFF; Alpha's and Gamma's watching stopped at frame 185 ("the
NetBattle broke off"); all three were told the match broken off (result
3), Gamma's team split with Delta gone, and MegaMan said "Whoa! The Team
NetBattle broke off!" back in the Square.

**The referee** (the same harness, `--referee`): Epsilon, a fifth
game, at the referee during bout 1: "TEAM NETBATTLE! BOUT 1!", "BETA VS
GAMMA!", "BETA HAS 40 HP! GAMMA HAS 30!". The match ran again after the
team's course moved under the Arena's own frame (`square_arena_update`,
`square_arena_held_frame`): three bouts, every hash agreeing, TEAM WIN for
Alpha and Beta.

**Split up** (the same harness, `--split`): Alpha's Battle on Beta's
card, his partner, answered at once ("Huh? Beta's on our team,Lan!"),
then `Split up` on the same card ("OK... Our team with Beta split up.";
both games told).

**The words** (`--words` and `--question`, a fifth game, Epsilon, alone):
Epsilon's Team up on Alpha ("Let's see if Alpha teams up with us!", then
"Huh? Alpha's on a team already...") and Battle on him ("Huh? Alpha's on
a team...", "We'd need a partner too,Lan."); Alpha's Battle on Epsilon
("Huh? Epsilon has no partner..."); Gamma's game without the bot asked
"Alpha's team wants a Team NetBattle!" and "Take it,Lan?" with the cursor
on No.

**The way in with a carried HP** (`--dev arena,arenacarry=40`, the
local scene): the plate reads "HP 40/100", the Custom screen's HP 40.

## Two MegaMen a side

Four MegaMen on one field in BN6's own engine, two a side (epic #167,
issue #181): a side-by-side team match from the Net Square ([Over the
server](#over-the-server-protocol-v15)), and its research and proofs
behind `--dev arena2v2=1` (`src/online/arena_team*.c`) with `--dev
arena` (its local link) or `--dev rollbench`. A hook at the battle's
spawn (`BN6_SPAWN_ENTITIES`) trades a NetBattle's list for one of four
MegaMen; a hook at the battle loop's copy of the sides' keys
(`BN6_KEY_COPY`, side 0's call) counts the loop's iterations in the
core's RAM (`RAM_TEAM_TICK`) and writes each partner's keys into his AI
data as BN6 writes a player's. `arena2v2=N` (N > 1) flips one key of
side 0's partner on the second GBA alone for six iterations from
iteration N: a mutation that must part the GBAs.

The rule everything here keeps: both GBAs of the pair run the same
battle, compared iteration by iteration (the four MegaMen's panels, HP,
actions and AI data's keys, the random number, the battle's timer: the
log's "arena2v2: iterations ... differing"), and whatever a partner
does reaches both at the same iteration. A lead's pad does so by BN6's
own link; since [The partners' words in BN6's
packet](#the-partners-words-in-bn6s-packet) a partner's keys and choices
ride the same packets, so they do so by the same construction. Under
full rules BN5's chip AI gave both of side 0's MegaMen its one block of
AI data (`RAM_PLAYER_AI`), so the flag keeps it the first's
(`darkinvs_two_a_side`), and each other MegaMan takes a room of his own
as the chip AI first takes him (issue #182: [What a partner meets under
full rules](#what-a-partner-meets-under-full-rules)). The research notes,
with the harness's whole battle and its fields compared byte by byte,
and the link's timing traced, are kept out of the repository
(`.build/research/battles/engine-2v2.md`, `linkdelay.md`).

### Four streams

The partners are players of the Arena's like the two GBAs' (21 October
2026):

- **The input** (`arena_input.c`) carries `ARENA_PLAYERS` streams: 0 and
  1 the GBAs' pads as before, 2 and 3 side 0's and side 1's partners
  (`arena_input_players`). A frame settles once every stream in play is
  known for it and the one after; a partner's key not yet come is
  guessed as his last held and rolled back where it came otherwise, as
  the other player's are. Two streams leave the partners' columns
  released, so a NetBattle without the flag takes the same keys as
  before.
- **The local link** (`arena_link_local.c`) queues each remote stream on
  its own, after the same latency: the peer's (player 1) and, with
  `arena_link_local_partners`, both partners', by the frame as the
  peer's. `--dev arena,arena2v2=1` gives them `arena_team_script`, by the
  frame's number alone (side 0's partner steps up to row 1 and fires
  charged shots along it; side 1's fires along row 1 and steps down and
  back between), so a run that rolls back and one that does not hand
  them the same keys. The link's `poll_player` takes them; the
  network's link has none (below).
- **The pair** (`arena_pair_step_all`) samples a side's partner's keys
  on that side's GBA's frames, as a GBA's pad is read: before a core's
  first run loop in a frame of its own clock, his keys for that frame
  (a core as far ahead as the keys go samples once the next frame of
  the pair gives it its own). The frame last sampled is each core's in
  the pair and in its snapshot.
- **The packets** (`arena_team_sample`, `arena_team_link.c`): the
  sample is handed to its side's GBA alone, as its pad is, and goes into
  the link packet that GBA sends for the frame, beside its lead's pad;
  both GBAs take it from the exchange that brings the lead's pad, at the
  same iteration ([The partners' words in BN6's
  packet](#the-partners-words-in-bn6s-packet), which replaced the first
  design's rings: the sample queued in both cores' RAM at the sampling
  GBA's iteration plus 4, which two GBAs further apart than that took at
  different iterations). The spawn empties them with the battle; they
  are core RAM, so a rollback puts them back with the state and the
  pair's hash covers them.

Proved with BN5's ROM beside BN6's (full rules), delay 1, fresh data
folders, `--dev arena,arenaboth,arena2v2=1,teamhp=500,quitend` (the
partners' scripts, records, hands and own Custom screens as they stand
since, each partner's keys in his side's GBA's packets), on this build
(22 October 2026, the integration of issue #181):

| Run | End | Hashes | The two GBAs |
| --- | --- | --- | --- |
| `arenalag=4` (window 8: 285 rollbacks, 1,140 frames run again) | over at frame 3427, side 0 won | 57 agreed | 2,696 iterations, 0 apart; the partners' keys 0 iterations taken apart |
| `arenalag=0,arenawindow=8` (0 rollbacks) | over at 3427 | all 57 the same as the run above, frame for frame | 2,696, 0 apart |
| `arenalag=4,arenawindow=0` (the delay alone) | over at 3427 | 57 | 2,689, 0 apart |
| `arenalag=4,arenaroll=8` (8 frames rolled back every 8) | over at 3427 | all 57 the same as the delay alone's | 2,689, 0 apart; 428 rollbacks of 8 frames, the game's state the same after all 428 |
| `arena2v2=300,arenalag=4` (the mutation) | over at 3427 | 57 (the local link echoes them) | 6 of 2,696 apart, from iteration 300: the flip's six |
| `--dev rollbench,arena2v2=1` | 600 frames | | the battle held at its first Custom screen for partners who have no client there (the leads wait for them); rollbacks of 8: 11 of 11 the game's state the same |

(The delay alone stalls, and `arenaboth`'s first player is a script by
the tick, so its battle is another than the rolling one's; the rollback
tried runs with no window and comes back to its hashes.) A NetBattle
without the flag is the battle it was:
`--dev arena,arenaboth,arenalag=4,arenadelay=1` over at frame 2690 with
the last hash `86d2f837337a1494`, on this build as before the change.

Each MegaMan's HP is his own object's (`BN6_T1_HP`): in the battle
above both partners stand on the field's purple panels of columns 1 and
6 (poison, as the picture and the drain read: 1 HP each 7 iterations
from iteration 62, the leads beside them on plain ones untouched), side 0's partner's
charged shot takes side 1's partner from 40 to 30 at iteration 477 and
side 1's takes side 0's from 50 to 40 at 407, side 0's player's charged
shots take side 1's player from 100 to 0 (iterations 184 to 2344), and
side 0's player, whom nothing hits, stays at 100 throughout, his
partner deleted at iteration 685; both GBAs alike at every iteration.
On the first GBA's screen side 1's MegaMen carry their own HP under
them (89 and 51 at the battle's frame 600); a GBA's own partner has no
label.

### The partners' words in BN6's packet

**The fault.** The first design queued a partner's keys in both GBAs'
RAM at the iteration the sampling GBA's battle loop was at plus 4, and
set his picks there from that GBA's frame. That holds while the two
GBAs' loops run within 4 iterations of each other in time, and they need
not: two runs met it. Over the network with a partner who never picks
(his client sending nothing, the leads waiting to the limit), the two
GBAs parted from iteration 73 though every game and the spectator still
agreed (the hashes cover both GBAs alike, so all four games ran the same
two diverging GBAs). With all four MegaMen in Beast Out under the delay
alone (`teambeast=3,arenalag=4,arenawindow=0`), 71 of 2,375 iterations
apart from iteration 2305, side 1's partner's B held on the second GBA
and not on the first, 46 iterations taken late. A bigger margin would
only have moved the line.

**The cause, in BN6's link.** A lead's pad never meets this, because of
how BN6's link battle keeps its two GBAs together (the link study,
`.build/research/battles/linkdelay.md`). Each frame of a battle, each
GBA reads the exchange the cable delivered into its two receive buffers
(`BN6_LINK_IN`, side 0's packet then side 1's, every GBA alike), takes
both sides' pads from them into the key array, and writes its own pad
into its packet (`BN6_LINK_OUT`); the link's frame stages that packet,
the library queues it, the cable exchanges it with the other GBA's, and
four frames later both GBAs read that exchange. The library never drops
a packet, so a GBA that falls behind (a frame of work past its VBlank,
a hold) keeps a packet standing in its queue: the two GBAs then run
apart in time, by as many frames as stand, but each still reads one
exchange a frame of its battle, the same exchanges in the same order.
Frame n of one GBA's battle and frame n of the other's read the same
exchange: the two run the same battle at any distance in time. The
queue's depth is all the drift is; four MegaMen in Beast Out (more work
a frame) and a held packet (the wait) are what deepen it.

**The fix.** A partner's word now rides in his side's GBA's own packet.
BN6 writes eight halfwords a packet and leaves two bytes that nothing
of its own writes or reads: +5 and +7, the other halves of the word
stream's index (+4) and the ready flags (+6), each written and read as
a byte (every writer of `BN6_LINK_OUT` and reader of `BN6_LINK_IN` in
bn6f, matched to Gregar's bytes: docs/ROM_DATA.md, A link battle's
packet). The library carries all eight halfwords. So:

- The pair hands a side's GBA its partner's value for each of its
  frames before the frame runs, as it hands it its pad (`team_link_put`,
  into that GBA's RAM alone, `RAM_TEAM_LINK`): keys kept as the last held
  for its next packet, a message of his Custom screen (a pick, his
  Cross, a Program Advance) queued in an outbox of eight, one a packet.
- At the battle's store of its pad into its packet (`BN6_LINK_PAD_STORE`,
  `0x0801FF8C` in sub_801FF18, and `0x08020062` in sub_801FFD6, the
  other routine of the battle's link table), a hook first reads each
  side's partner's word out of that side's received packet (where its
  pad's mark is set, as BN6 asks before it takes the pad), then writes
  this GBA's own partner's word into +5 and +7 of its packet.
- Each GBA works the partner's turn out from those words on its own
  (`arena_team_custom_link`): his deck dealt as its Custom screen first
  opens (seeded by its random number, the same on both at that frame),
  his turn opened and closed with BN6's phase, his picks in as their
  last message comes, his hand set at the battle loop's next iteration
  after his side's screen has closed with them in. The wait's limit is
  said by his side's GBA in his place: at `TEAM_WAIT_LIMIT` frames held
  it queues "no picks" in its own outbox, and both GBAs take it from its
  packet as they would his own. No byte of his turn is written into one
  core by the other any more.
- At `BN6_KEY_COPY` each partner's keys are those the last packet of his
  side brought (`team_link_keys`), none on a Custom screen or in a
  message's frame, as before.

**Why it cannot drift.** Both GBAs take a partner's word from the same
exchange as his lead's pad, in the same frame of their battles: the
invariant a NetBattle of two already rests on. A distance between the
two GBAs in time that parted their partners would part their leads
too, and BN6's own battle with them. The words keep BN6's four-frame
pipeline, the timing a lead's keys have (the owner's call: the link's
timing kept), and the wire is unchanged: a partner's stream crosses the
server as before, and only inside each game's pair does it ride the
cable. A NetBattle of two never writes the two bytes (no hook without
two a side). The runs on the cable traced it: a partner's pick written
at +5/+7 of one GBA's packet came out of both GBAs' receive buffers four
frames later with the same sequence (`AF` on both), and the outbox
held at most a message or two.

Proved, 22 October 2026, fresh data folders, both ROMs (the runs before
the fix as their branches reported them):

| Run | Before | Now |
| --- | --- | --- |
| `--dev arena,arenaboth,arena2v2=1,teamhp=500,arenadelay=1,quitend,teambeast=3,arenalag=4,arenawindow=0` (all four in Beast Out, the delay alone) | 71 of 2,375 iterations apart from iteration 2305 | over at frame 7075, 117 hashes, 2,376 iterations 0 apart, the partners' keys 0 taken apart |
| ... `arenaroll=8` | as above | 884 rollbacks of 8 frames, the game's state the same after all 884, its 117 hashes the delay alone's |
| Four games and a spectator through a local server, Beta's client sending nothing (`teamslow=-1`, the leads waiting to the limit) | the GBAs apart from iteration 73 on every game, the hashes agreeing | over at frame 5810 on all four, 96 hashes agreed and the spectator's 96 checked, the GBAs 0 apart over 1,325 iterations on every game; side 0's GBA said his picks none at 3,600 frames held |
| `--dev arena,arenaboth,arena2v2=1,teamhp=150,teamslow=-1,arenadelay=1,quitend,arenalag=4` (the same, one machine) | over at 13010, 3,155 iterations 0 apart | over at 16730, 278 hashes the same as `arenalag=0,arenawindow=8`'s, 3,230 iterations 0 apart |

Every battle of the flag ends otherwise than before (a partner's keys
now as late as BN6's pipeline makes a lead's, frame for frame rather
than iteration for iteration, and his picks in his hand at the next
iteration rather than four on), so the tables below are this build's.
Run again on the integration's last build (with [A partner's
UNITE](#a-partners-unite) and the Souls' modules by object), the local
runs give the same hashes, frame for frame: all four in Beast Out with
the delay alone and with `arenaroll=8`, the Cross, the Program Advance,
the slow and the gone partners, the NaviCust from the playtest's
checkpoint, the DarkInvs rooms from the rules branch's, the plain run.

### The partners' own NaviStats

BN6 keeps one battle NaviStats a side (`BN6_BATTLE_NAVI`, `0x64` bytes:
HP, the buster's levels, abilities, mood, form), and every reader and
writer goes through one routine with a side (`BN6_NAVI_STATS_ADDR`). Five
routines take that side from the object in r5 (`BN6_NAVI_BY_OBJECT`:
330 calls in bn6f, a MegaMan's HP as he is set up, his forms, his mood);
the rest pass a side (the HP box, the Custom screen, the battle's end)
and stay the side's lead's. `src/online/arena_team_navi.c` answers the
five by object (21 October 2026):

- **Each partner's record**, a copy of his side's made as the battle
  spawns its field, his own max HP in it where `teamhp=N` gives one (as
  his own loadout will).
- **Their room is BN6's own.** The RAM BN6 leaves alone (docs/
  EMULATION.md, BN6's RAM) has `0x70` bytes left past the partners'
  words (`RAM_TEAM_LINK`), room for one record of `0x64`, not two. So each record takes an
  object slot of BN6's fourth kind (the effects: 32 of `BN6_T4_SIZE`,
  `0xC8` bytes), slots 30 and 31, taken in its pool (`BN6_T4_ACTIVE`,
  slot i bit 31 - i) as the field spawns, the pool just set up. BN6 runs
  and frees its objects through their run order alone (bn6f
  `object_freeMemory`: an object frees itself), and reads the pool's
  bits only to find a free slot as it spawns (the lowest first) and to
  clear them all as the next battle sets up
  (`InitializeStructsOfObjectType`): a slot taken and never linked is
  never run, never freed, never zeroed until then. The battle keeps 30
  effects. A slot's `0xC8` bytes hold his record and his hand (`0x50`,
  below).
- **By object**: an answering hook at `BN6_NAVI_STATS_ADDR` returns a
  partner's record where the ask comes from one of the five (its return
  address), for his side, and r5 is his object, or one he spawned, or
  one they spawned in turn: every spawn (`BN6_SPAWN_COMMON`, r5 the
  spawner) marks the object it makes (`BN6_SPAWN_MADE`) a partner's or
  no one's, for each of the three kinds' 32 slots. The marks are kept a
  core between calls (`hook_keep_slots`), so a rollback puts them back;
  the records are core RAM, in the pair's hash.

Proved with `--dev arena,arenaboth,arena2v2=1,teamhp=150,arenadelay=1`
(the partners' records and hands as they stand since, their own Custom
screens and the leads' wait among them), this build:

| Run | End | Hashes | The two GBAs |
| --- | --- | --- | --- |
| `arenalag=4` (169 rollbacks) | over at frame 1850 | 30, all the same as `arenalag=0,arenawindow=8`'s (0 rollbacks) | 1,299 iterations, 0 apart |
| `arenalag=4,arenaroll=8` | over at 1846 | all 30 the same as the delay alone's (`arenawindow=0`) | 1,292, 0 apart; 230 rollbacks of 8 frames, the game's state the same after all 230 |
| `teamhp=-150` (the records' HP on the second GBA alone) | over at 1850 | | apart from iteration 1 (1,299 of 1,299) |

With the records their sides' copies (`arena2v2=1` alone, before the
partners took chips) the battle's 194 changes of HP were those without
the hooks, iteration for iteration. Both partners are set up with 150 HP
from their records while their leads keep 100: the field's poison takes
both from iteration 62; side 1's partner's charged shot takes side 0's
from 100 to 90 at iteration 407 and his Cannon from 84 to 43 at 454,
while side 0's player, whom nothing hits, stays at 100 and side 1's
falls by his own hits alone (100 to 90 at 184, 132 left on his partner
then); side 0's partner is deleted at iteration 685 and side 1's at
1035. Each GBA's screen shows its enemies' own HP under them: on the
first, side 1's player 100 to 67 and his partner 144, 123, 101 at the
battle's frames 300 to 600; on the second (`arenaother`), side 0's
player 100 throughout and his partner 144 to 37 by frame 700. The
records answered some 40,000 to 80,000 asks each a battle, all of them
the partners' own objects': of the 64 to 86 objects marked theirs
(their buster's shots, their Cannons and the effects) none asked by
object, so an attack's inheriting his record is built and its marks
counted, but not yet seen to matter. The HP box on a partner's own GBA
(BN6 draws its lead's) and a partner's own loadout (his NaviCust's
programs, his Cross) wait on his own GBA's screen and the wire.

### The partners' own hands

BN6 keeps one hand a side (`BN6_BATTLE_HAND`, `0x50` bytes: the chip up
next, six chips, their powers), every reader through one routine with a
side (`BN6_HAND_ADDR`, `0x08010018`: 20 calls in bn6f, and
`getCurChipInBattleHand_8010004`'s 7 through it, most with the side of
the object in r5). The same answer by object as the NaviStats': a hook
there returns a partner's own hand, in his room after his record, where
r5 is his or his marks' and the side asked for is his. His hand starts
as his side's as the battle sets it up (empty); at the hands' commit it
was dealt from his lead's each time the Custom screen dealt his lead's
anew, and since his own Custom screen (below) it takes his own picks
alone. Under the flag a lead takes the chip under the cursor before the
script's OK, so that a hand holds one (Cannon).

Measured with the runs above (the partners tapping A a few times a
cycle until they are free to use a chip): side 0's partner uses his
Cannon at iteration 177 and side 1's at 429, each one's own hand's chip
up next going from 0 to 1 while his lead's stays at 0 (each lead still
holds his Cannon, "Cannon 40" still at the foot of his GBA's screen);
side 1's partner's Cannon takes side 0's partner from 84 to 43 at
iteration 454 (40 and the poison's 1). On both screens the chip's icon
is drawn over each partner from his own hand and leaves him as he uses
it, his lead's staying; the second GBA's (`arenaother`) shows side 1's
partner firing it from the left and side 0's partner's own HP under him
falling 85, 52, 40. The hands answered some 5,000 to 9,000 asks each a
battle, all the partners' own objects'; the GBAs agree at every
iteration; with 221 rollbacks and with none all 44 hashes the same,
`arenaroll=8` 335 of 335 the same. Before the partners tapped A (one
press each, which met them busy), the hands as their leads' copies left
the battle's 272 changes of HP as with the hook switched off (a debug
run). (These numbers are the hands' commit's, with his lead's chips in
his hand; his own folder and Custom screen follow.)

### A partner's own Custom screen

A partner picks his chips on BN6's own Custom screen, from his own deck
(the owner's direction, 21 October 2026: BN6's screen, never one of
ours), and only his picks cross (22 October 2026; `arena_team_custom.c`,
`arena_team_screen.c`, `arena_team_client.c`):

- **His deck**, the pair's, every client's and spectator's alike: his
  folder (a carried MegaMan's where one is given,
  `arena_team_custom_folder`; under the dev flag his side's lead's, as his
  side's GBA holds it) dealt as BN6 deals a side's, the Regular chip
  first, the TagChips after it, the rest shuffled (our shuffle, seeded by
  the battle's random number as its first Custom screen opens, in the
  way in: each GBA deals it on its own at that frame, the same on both,
  `D07D8E18` in the runs below), a chip that sits out of BN6's battles
  left out. It lives in each core's RAM (`RAM_TEAM_CUSTOM`, docs/
  EMULATION.md), so a rollback puts it back and the pair's hash covers it;
  the folder itself, which no core holds, is folded into the pair's hash.
- **His screen, on his own client alone**: a partner has no GBA in the
  pair. A lone core taken from his side's GBA at a Custom screen will not
  do: within frames BN6 sees no other GBA on its cable and Lan's box says
  "Communication error" (seen). So his client makes a plain copy of the
  pair (`arena_pair_open_plain`: both GBAs on their own cable, BN5's
  bytes in their ROMs, no hook on them, never hashed, never on a
  spectator's client; where he holds Souls, his side's GBA there with
  BN5's installs' hooks for UNITE: [A partner's UNITE](#a-partners-unite)),
  once, as his side's GBA first opens its Custom screen. Each turn the pair is copied into it (`arena_pair_save`,
  `arena_pair_load`), his deck is written into BN6's battle deck on his
  side's GBA there and the Custom screen's phase step put back to 0
  (`BN6_BATTLE_STEP`): the next frame BN6 sets its Custom screen up anew
  from his deck (bn6f sub_8026840: his hand dealt, its icons loaded) and
  runs it on his keys, by its own rules (the same code or the same chip,
  five at most, Atk+ chips); the other GBA there holds still
  at its own. His client shows that copy's picture while his side's
  GBA's screen is open, the battle behind it the battle's own as the turn
  began.
- **Only his picks cross**: at his OK (`BN6_CUSTOM_MODE` going to
  `BN6_MODE_OK`) his client reads the hand's slots he picked, in order
  (`BN6_CUSTOM_PICKS`), and sends them in his stream as pick messages,
  one a frame (`TEAM_PICK`: bit 15 set, the count, the index, the slot;
  past the GBA's ten buttons, so a partner's stream keeps its sixteen
  bits, `ARENA_PARTNER_MASK`). Since the leads wait for him (below), his
  side's GBA's screen closes before his OK only at the wait's limit,
  when his client sends the picks he has so far.
- **Into his hand**: a pick message rides his side's GBA's packet as his
  keys do, and each GBA takes it from that packet at the same frame of
  its battle ([The partners' words in BN6's
  packet](#the-partners-words-in-bn6s-packet)). Once the screen has
  closed and his picks are in (or as they come in after it), each GBA
  sets them for the battle loop's next iteration, the chips taken out
  of his deck, and writes them into his own hand there, as BN6's OK
  writes a side's (bn6f sub_8029110: the chips' ids, powers and folder
  entries, the screen they came from, an Atk+10, Atk+30 or Navi+20
  merged into the chip before it as bn6f sub_8029224 merges them). (The
  first design set them four iterations on from his side's GBA's frame,
  as a margin for the other GBA's distance; the packets need none.)
  While his side's GBA's screen is open his battle keys go in released.
  His picks are never dropped: the leads wait for them (below).

His client's cost (the host build, this machine; the numbers below): a
frame of his screen about a pair's frame, the turn's copy 0.14 ms to
some 20 ms, making it once some 155 ms (the battle's worst tick, 7 to 8
ms without it): a client that makes it at the way in pays that outside
the battle. A deleted partner has no turn.

### The leads wait for their partners

The owner's call (22 October 2026): the leads' Custom screen waits for
a slow partner, his picks never dropped; a sign tells each partner his
lead is ready, and each lead that his partner still picks; and an
absent partner cannot hold the others forever.

The game-design lens first. The dialectic is mine against ours again:
each player's turn is his own, but the battle goes on for all four
only together, so the wait must read as a teammate taking his time,
never as a fault. The loop layer is the battle's turn (pick, wait, fight)
inside the session's "one thing together". The patterns: *telegraph
honestly* (the wait is shown where it is, and by whom), *presence over
pressure* (a quiet sign that someone is there and busy; no countdown, no
nagging, no word of explanation: the players know BN6's Custom screen),
*diegetic first* (BN6's own way of showing a GBA still picking before
any sign of ours), and *respect the player's time* (a generous limit
past which the idle one cannot hold three others: 60 seconds, the figure
the epic's research found players' communities use for a turn).

**How BN6 waits, and where the hold goes.** In a link battle each GBA,
as its lead's OK has slid the window out, sends its packet (bn6f
sub_800B3A2, `BN6_LINK_SEND`: the mark `0x56789123` at `0x0203CBE0`, the
hand its OK built, its battle NaviStats, its Cross's record) and waits in
mode `0x14` (`BN6_CUSTOM_WAIT`, sub_8026DC4) until both sides' packets
have come with that mark (`0x0203F4A0` and `0x0203F5A0`, which every GBA
receives, checked each frame at `BN6_CUSTOM_WAIT_LOOP`); then the screen
closes on both. While it waits, the other GBA shows "Cstmzing..."
blinking over the side still picking (seen in the pair's captures). So the hold is the packet: a
side's GBA keeps its packet back while its partner picks
(`arena_team_wait.c`: its send skipped at `BN6_LINK_SEND`'s entry and
owed, then sent from `BN6_CUSTOM_WAIT_LOOP` once his picks are in). Both
GBAs then wait by BN6's own cable, so the moment the screen closes is
the cable's, one moment every client and spectator agrees on; the hold
itself is read in that GBA's RAM (his turn's state, which each GBA works
out from his words in the packets), never a clock.
The lead's OK is answered as BN6 answers it: the window slides out and
the battlefield shows, as when he waits for the other lead.

**The signs.** On the other side's GBA, BN6's own "Cstmzing..." blinks
over the side held for its partner, as over a lead still picking: no
drawing of ours. On a lead's own GBA, whose partner stands on his own
half where BN6 shows nothing, a quiet plate of ours under the name
plate: his partner's seat (`P3`, `P4`) and three dots, one lit in turn,
while his partner picks (`arena_team_view.c`, `sign`). On the partner's
own screen, the same plate with his lead's seat and a tick once his
lead has pressed OK and waits for him. No words, no count.

**The limit.** A side's GBA held for its partner counts its frames
(`RAM_TEAM_CUSTOM`, its own core's); his own client sends the picks he
has two seconds before the limit (`TEAM_WAIT_SAY`, 120 frames), and at
the limit that GBA says "no picks" in his place, in its own packet, so
both GBAs take it at the same frame as they would his own, and the
screen closes (`TEAM_WAIT_LIMIT`: 3,600 of that GBA's frames, 60
seconds, counted only while his lead waits for him). Nothing is shown
before it. The number is the owner's to confirm.

Proved (`--dev arena,arenaboth,arena2v2=1,arenadelay=1,quitend`, both
ROMs, fresh data folders, 22 October 2026, this build):

| Run | End | The two GBAs, the hashes | The wait |
| --- | --- | --- | --- |
| `teamhp=500,teamslow=600,arenalag=4` (side 0's partner's hands off his screen 10 seconds each turn; 461 rollbacks) | over at frame 5197 (3427 without him slow) | 2,755 iterations, 0 apart; 86 hashes | his picks taken each turn, never dropped |
| ... `arenawindow=0` and `arenaroll=8` | over at 5205 | 2,755, 0 apart; 650 rollbacks of 8 frames, the game's state the same after all 650; the 86 hashes of the two the same | as above |
| `teamhp=150,teamslow=99999,arenalag=4` (a partner who never picks) | over at 7970 | 1,830, 0 apart | his client sends the picks he has at 3,480 and 3,485 frames held, both turns; the screen goes on |
| `teamhp=150,teamslow=-1,arenalag=4` (a client gone quiet, sending nothing) | over at 16730 | 3,230, 0 apart; all 278 hashes the same as `arenalag=0,arenawindow=8`'s | his side's GBA says his picks none at 3,600 frames held, both turns |
| `teamhp=500,arenalag=4` (no one slow) | over at 3427 | 2,696, 0 apart | a partner's OK comes a few frames after his lead's |

The mutation of these runs is the picks' (`teampick=-1`, the table
below: apart from iteration 190): the hold itself cannot part the GBAs,
since it crosses by BN6's cable; held wrongly it would change the
battle on both alike. A NetBattle without the flag is the battle it was
(over at frame 2690, last hash `86d2f837337a1494`).

Captured (`teamslow=600`, kept out of the repository): side 0's lead's
screen with his window slid out after his OK, the battlefield and his
partner's sign (`P3` and its dots) under the name plate; side 1's lead's
screen with BN6's own "Cstmzing..." blinking over side 0 for the ten
seconds; side 0's partner's own screen, BN6's Custom screen with his
deck still untouched, and his lead's sign (`P1` and a tick).

### The partner's seat: his HP box and the four told apart

`teamseat=N` shows seat N's screen (`arena_team_view.c`): 0 and 1 the
GBAs' players, 2 and 3 side 0's and side 1's partners, who see their
side's GBA. On a partner's seat his own HP box covers BN6's, which shows
his lead's, in the fight at the picture's top left and right of the
Custom screen's window while it is open, read from his own object. In
the fight each MegaMan's seat is marked by his feet (`P1` to `P4`, the
seat plus one, his own side's in cyan, the other's in gold), and the
ally BN6 leaves unlabelled (a lead's partner on the lead's GBA, the lead
on his partner's) has his HP under him as BN6 gives the enemies theirs.

### His loadout in his record

A partner's record (his battle NaviStats, above) is his side's as the
battle spawns its field, then his own loadout's numbers where one is
given (`arena_team_navi_loadout`): his max HP and base HP, and his
buster's Attack, Speed and Charge, written where BN6 keeps them in a
NaviStats (`BN6_NS_ATTACK`, `BN6_NS_SPEED`, `BN6_NS_CHARGE`, as the menu's
`BN6_NAVI_ATTACK` ones; a battle reads its copy's), so that the asks
answered by object for him and his shots read his. Under the dev flag
side 0's partner carries the run's MegaMan with the next folder that
holds chips after his lead's, where the data folder has a checkpoint;
`teambuster=N` gives both partners Attack N. Seen with
`teamhp=500,teambuster=5`: each partner's charged shot takes 50 from the
other partner (450 to 400 at iteration 407, 440 to 390 at 477) where it
took 10 without it, the leads' shots still 10; 2,454 iterations 0 apart,
52 hashes the same with 266 rollbacks and with none.

His NaviCust's programs past HP and the buster, his Cross and his
Program Advances: the three sections below (issue #181).

### His NaviCust, compiled by BN6

BN6 makes a NaviStats' abilities from the NaviCust's board in its menus:
its reload of MegaMan's stats (`BN6_NAVICUST_RELOAD`, bn6f
reloadCurNaviStatBoosts_813c3ac, Gregar's `0x0813E18C`) puts his NaviStats
back to his own, runs each program's routine over them
(applyNavicustPrograms_813C684, `0x0813E464`) and the bugs after. A
lead's are made so on his own GBA on the way in, from the board the Arena
writes into it. A partner has no GBA, so his own client runs the same
routine (`src/online/arena_team_navicust.c`, 22 October 2026): his
checkpoint's state loaded into a core of the plain ROM, his loadout
written in as the Arena writes a lead's (`arena_loadout_install`), the
routine called once in the game's update (the game's own call, as
`gamecall.c`'s and the Arena's boot's), and the NaviStats it leaves read
out, his HP whole. No rule of ours: what BN6 computes is what he is. That
record, the same on every client, is what crosses (with his loadout, once
the wire carries partners); under the dev flag his client is this one.

- **His record** takes it whole as the field spawns: BN6 copies a link
  battle's NaviStats whole from the menu's into its start packet and from
  there into the battle's (bn6f sub_800B144), but for two bytes the start
  sets for every MegaMan of the battle alike (his start form,
  `BN6_NS_START_FORM`, and the BeastOutCounter, `BN6_NS_BEAST_TURNS`),
  kept from his side's record. Measured: each GBA's own menu NaviStats and
  its side's battle record the same, all 0x64 bytes, at the spawn. His
  abilities (SuperArmor, the shoes, UnderShirt, the buster's levels, the
  Custom level) are then his own wherever BN6 asks by object.
- **His hand's size** is his own: BN6's Custom screen deals as many chips
  as its side's battle NaviStats' Custom level says (`0x0802A440`), so his
  screen copy gets his record as his side's before BN6 sets the screen up
  (`arena_team_screen.c`), and deals his number.

Under the dev flag side 0's partner's NaviCust is compiled wherever the
data folder has a checkpoint (he carries the run's MegaMan), and
`teamcust=1` gives side 1's partner the checkpoint's MegaMan's NaviCust
too; `teamcust=-1` leaves the records their sides' copies on the second
GBA alone, the mutation. Proved with a playtest's checkpoint whose
MegaMan runs Custom1, UnderShirt and HP+100 (base 360), both ROMs,
`--dev arena,arenaboth,arena2v2=1,teamcust=1,arenadelay=1,quitend`, this
build (the checkpoint copied into each run's fresh data folder):

| Run | End | Hashes | The two GBAs |
| --- | --- | --- | --- |
| `arenalag=4` (290 rollbacks) | over at frame 3530 | 58, all the same as `arenalag=0,arenawindow=8`'s (0 rollbacks) | 2,798 iterations, 0 apart |
| `arenalag=4,arenaroll=8` | over at 3526 | all 58 the same as the delay alone's (`arenawindow=0`) | 2,787, 0 apart; 440 rollbacks of 8 frames, the game's state the same after all 440 |
| `teamcust=-1,arenalag=4` (the mutation) | | | apart from iteration 1 |

The log: "side 1's partner's NaviCust compiled by BN6 (0813E18C) on his
own core: HP 460 (base 360), Custom 6, buster 1 1 1, ... UnderShirt 1".
Side 1's partner is set up with 460 HP beside his lead's 100 (the poison
of the field takes him from 460 at iteration 62); his HP box on his seat
reads 460 and 451 (`teamseat=3`); his own Custom screen deals six chips,
five on the top row and the sixth under them, where his lead's deals
five. The compile costs a core made, a state loaded and a frame or two
run, once a run of the game.

Over the network every game and spectator of a side-by-side match
compiles each partner's NaviCust so, from the MegaMan he sent
(`arena_seats_partners`, `team_client_compile`), over its own
checkpoint's state, so that the four games and the stands set him up
alike (the four-game runs of [Over the
server](#over-the-server-protocol-v15): "side 1's partner's NaviCust
compiled by BN6 ... HP 40" on all five, the hashes agreeing). The
compile reads his loadout's board and numbers; that the checkpoint under
it leaves nothing of its own in his record is so far what the hashes
say, games of different runs not yet tried.

### His Cross

BN6's crossing controller (`BN6_CROSS_SIDES`, bn6f dword_20367F0) keeps
a record a side, copied from the turn's packets as the Custom screen
closes, and crosses those records' objects alone: each side's player
(What a partner meets under full rules, below, read in bn6f; now seen).
A partner's Cross (`src/online/arena_team_cross.c`, 22 October 2026):

- **His choice** is his own CROSSSELECT's: his screen copy holds his own
  Crosses' flags (his lead's off) and the Crosses he has chosen this
  battle (`BN6_CROSSES_USED`, kept by his client from his last OK), so
  BN6 lists his. At his OK the form his CROSSSELECT committed
  (`BN6_CROSS_RECORD` +0) goes in his stream before his picks
  (`TEAM_CROSS`: the pick message's count 7, the form), and both GBAs
  keep it as his pending form for the turn (`RAM_TEAM_CUSTOM`), before his
  side's GBA can send its packet: the leads wait for him, so every GBA
  has it before either can begin the turn's crossing.
- **BN6's crossing for his object**, by three hooks: the turn's form check
  (`BN6_TURN_FORMS_SET`) sends the controller to its crossing where a
  partner on the field has a Cross, as where a record has one; the
  crossing's step (`BN6_CROSS_ACTIVATE`) asks his object to cross at its
  first frame as BN6 asks a record's (`BN6_AI_CROSS_ASK` into his AI
  data) and holds BN6's wait while he still crosses (`BN6_AI_CROSSING`);
  and the crossing's phases, which read the pending form by the side
  (`BN6_CROSS_PENDING_GET`), are answered his own where the crossing
  object is his. His form is then written into his own record (the
  NaviStats answered by object), so his element, his buster and the
  weakness that breaks it are his; the leads' Crosses go as before.
- **What we met**: the step is reached by `mov lr,pc; bx r1`, which leaves
  lr's Thumb bit clear; a hook's return there ran the game on in ARM and
  stopped it on the crossing's black field. The wait returns by a jump to
  lr | 1, as the routine's own `pop {pc}` does.

Proved with fresh data folders, `--dev
arena,arenaboth,arena2v2=1,teamcross=1,teamhp=500,arenadelay=1,quitend`
(each partner's client takes HeatCross on his CROSSSELECT at his first
turn), this build:

| Run | End | Hashes | The two GBAs |
| --- | --- | --- | --- |
| `arenalag=4` (379 rollbacks) | over at frame 4800 | 80, all the same as `arenalag=0,arenawindow=8`'s (0 rollbacks) | 3,499 iterations, 0 apart |
| `arenalag=4,arenaroll=8` | over at 4810 | all 80 the same as the delay alone's (`arenawindow=0`) | 3,499, 0 apart; 601 rollbacks of 8 frames, the game's state the same after all 601 |
| `teamcross=-1,arenalag=4` (no Cross on the second GBA, the mutation) | | | apart from iteration 1: the crossing comes before the fight's first iteration, and the second GBA's battle begins its frames earlier |

His form is folded into the GBAs' comparison (the records' form). Seen:
his CROSSSELECT listing HeatCross alone, his emblem taken; on the first
GBA (`teamseat=2`) after the first turn's crossing both partners in
HeatCross and both leads as they were, "TURN 1 START!" over them; on the
second (`teamseat=3`) BN6's black crossing field with both partners
crossed. The log: "the partners' Crosses: the turn's crossing begun for a
partner alone 4 times, side 0's partner asked to cross 2 times and side
1's 2, BN6's wait held for a partner still crossing 204 frames, their own
pending forms answered 210 and 210 times, every core's" (each GBA's, the
rollbacks' frames run again among them). His pending form is taken as the
crossing's last mode begins (`BN6_CROSS_RESTORE`), as BN6 takes the
records' once their objects have crossed: before that, kept to his next
turn, the turn's check as a turn's change begins (`0x0800840C`) met it
again and began the crossing anew for him (asked 6 times where 2 is right,
the first proof's runs).

**His Beast Out, and its turns as a lead's.** Gregar's emblem under OK
commits a Beast Out at OK into the same record as CROSSSELECT
(`BN6_BEAST_COMMIT`, `0x08029344`: form 0xB, 0xD on from a Cross, 0x17
BeastOver), so it crosses in his stream and through BN6's crossing as his
Cross does: `teambeast=1` sets Beast Out's flag on his screen copy (his
record's BeastOutCounter, 3, as his side's), his client takes the emblem
at his first turn (RIGHT five times, DOWN, A, then UP and A on OK), and
both partners turn into Gregar's beast at the first turn's crossing while
their leads stay as they are, on both GBAs. A Beast Out's turns BN6
counts for each side's player alone: his AI data's turn count as each
Custom screen opens (`BN6_FORM_TURN_DOWN`, called with the side-player
getter's object; two of its call sites are the Souls' hooks,
`BN6_TURN_COUNT`), his BeastOutCounter as the crossing controller is done
(`BN6_BEAST_TURN`), its end in the turn's form check for the records'
objects (`BN6_FORM_END_CHECK`, then `BN6_FORM_LEAVE_ASK`) and the wait
while he leaves it in the controller's mode 8 (`BN6_CROSS_RESTORE`). For a
partner each is BN6's own code run for his object: the two counts' ends
hooked (`BN6_FORM_TURN_DOWN_END`, `BN6_BEAST_TURN_END`) and their bodies
entered again with his object where the one just done was his side's
player; the end's check and the leave asked from the turn check's end
(`BN6_TURN_FORMS_SET`) for each partner without a form this turn, BN6's
routines called and come back to the hook (the run kept a core, never past
the frame); mode 8 waiting while he leaves his form. (The controller BN6
sets up as each screen opens for the two players, `BN6_TURN_SIDES`, asks a
routine that answers 0 in Gregar: nothing there for him.) All four
MegaMen run the same code each turn, a partner's own forms aside, so the
partners' pair hashes differ from the first proofs' while every battle
ends at the same frame (`teamcust`, `teampa` and the flag alone; `teamcross`
by its pending form above).

The control: `teambeast=3` has the leads take Beast Out by the same keys
(its flag on both GBAs as the field spawns), `teambeast=2` the partners at
every turn. At each turn's form check (the log's "the turn's check at
iteration ..." lines) each lead and his partner are in the same form with
the same BeastOutCounter: form 0xB and 3 at the first, 0xB and 2 at
iteration 1084, 0xB and 1 at 1897 in the delay alone's run (the first
GBA's; with the window of 8, 0xB and 2 at 1088 before the battle ends).
Proved, fresh data folders, `--dev
arena,arenaboth,arena2v2=1,teamhp=500,arenadelay=1,quitend`:

| Run | End | Hashes | The two GBAs |
| --- | --- | --- | --- |
| `teambeast=1,arenalag=4` (260 rollbacks) | over at frame 3169, side 1 won | 52, all the same as `arenalag=0,arenawindow=8`'s (0 rollbacks) | 2,167 iterations, 0 apart |
| `teambeast=1,arenalag=4,arenaroll=8` | over at 3169 | all 52 the same as the delay alone's (`arenawindow=0`) | 2,160, 0 apart; 396 rollbacks of 8 frames, the game's state the same after all 396 |
| `teambeast=-1,arenalag=4` (no Beast Out on the second GBA, the mutation) | | | apart from iteration 1 |
| `teambeast=3,arenalag=4` (the leads too; 522 rollbacks) | over at 6266, side 0 won | 104, all the same as `arenalag=0,arenawindow=8`'s | 2,896, 0 apart |
| `teambeast=3,arenalag=4,arenawindow=0` (the delay alone) | over at 7075 | 117 | 2,376, 0 apart |
| `teambeast=3,arenalag=4,arenaroll=8` | over at 7075 | all 117 the same as the delay alone's | 2,376, 0 apart; 884 rollbacks of 8 frames, the game's state the same after all 884 |

(This build's; the build before the packets parted the last two at
iteration 2305, 71 of 2,375 iterations apart: in that schedule, with all
four in Beast Out, the two GBAs' battle loops came more than 4
iterations apart in time and the partners' keys, queued 4 on, were taken
late on one GBA, side 1's partner's B held on the second GBA and none on
the first. With the partners' turn hooks switched off it parted at the
same iteration, so it was not a partner's form: [The partners' words in
BN6's packet](#the-partners-words-in-bn6s-packet) has the cause and its
end.)

### A Program Advance and his hand's size

BN6's OK (`0x08029110`) builds the hand from the picks
(`BN6_CUSTOM_BUILT`, `0x020366C0`), checks them for a Program Advance
(`0x08029520`, Gregar's 30 records, ids `BN6_PA_FIRST` 0x140 to
`BN6_PA_LAST` 0x15D), joins its picks into it (`0x0802B6F2`) and sets its
next mode to the PA's show (`BN6_CUSTOM_NEXT` = `BN6_MODE_PA`) before the
plus chips merge. On his screen copy it does so as on any GBA. His
client then (`arena_team_screen.c`, `arena_team_client.c`):

- reads his picks once the screen waits for the cable (mode 0x14), the
  hand BN6 built whole, where a Program Advance was made (without one, at
  his OK as before);
- finds which picks it joined: the place and count (3 or 4) whose join,
  through the pair's own build of a hand (`team_hand_build`: the PA's
  join, then the plus chips' merge, as BN6's OK), gives the hand BN6 built;
- sends them before his picks (`TEAM_PA`: the pick message's count 6, its
  first pick, the count and the PA). The pair joins them into his hand as
  BN6 does, with the PA's power from its record and the hand's end after
  it.

Proved with `teampa=1` (side 0's partner's folder opens with Cannon A,
B and C as its Regular chip and TagChips, so his first hand deals them
first and his client picks them in order), fresh data folders, `--dev
arena,arenaboth,arena2v2=1,teampa=1,teamhp=500,arenadelay=1,quitend`,
this build:

| Run | End | Hashes | The two GBAs |
| --- | --- | --- | --- |
| `arenalag=4` (161 rollbacks) | over at frame 2035 | 33, all the same as `arenalag=0,arenawindow=8`'s | 1,284 iterations, 0 apart |
| `arenalag=4,arenaroll=8` | over at 2038 | all 33 the same as the delay alone's | 1,284, 0 apart; 254 rollbacks of 8 frames, the game's state the same after all 254 |
| `teampa=-1,arenalag=4` (the PA left out of his hand on the second GBA, the mutation) | | | apart from iteration 1, his hand taken |

On his screen: Cannon A, B, C picked, OK, BN6's "Sending chip data...".
BN6's built hand: chips 0x140, 0xFFFF, 0x0001, powers 300, 0, 40 (the PA
first, the hand's end after it). The log: "a partner's Program Advance
140 from his picks 0 to 2, as BN6's OK built it on his screen", and on
each GBA "side 0's partner's hand a Program Advance, 140 (power 300)
first of 1 chips, from 3 picks". He uses it at iteration 443: his
Z-Canon1 (BN6's dark cannon over the field) takes side 1's partner from
422 to 122 at iteration 467. (The build before the packets: at 446,
421 to 121 at 470, and side 1's player from 78 to 0 at 471.) His hand's size by his own Custom level is the
NaviCust's (above): six chips with Custom1.

A partner's whole loadout leaves the rest as it was: the runs without the
new flags give the battles of the sections above (`teamhp=500,arenalag=4`
over at 3427, 57 hashes, 2,696 iterations 0 apart; `teampick=-1` over at
3836, apart from iteration 1, where its short hand is taken: the
partners' hands' chips are in the comparison since the Program Advance),
and a NetBattle without the flag is
the battle it was (over at frame 2690, the last hash
`86d2f837337a1494`).

### What a partner meets under full rules

Every NetBattle runs BN5's installs on both GBAs (the Souls with their
mood, ColonelSoul's button and the Chaos Unison, DarkInvs's machine,
BN5's chips and Navis, the green flash), recorded by `xinstall_log.c`
and hooked on the pair's cores. What of it was keyed by a side's player
rather than by object was read in the code (22 October 2026) and is now
answered by object, a side's partner (`BN6_BATTLE_PLAYERS`' second word
of a side: `soul_partner`) a Navi of his own; none of it changes a
NetBattle without the flag, nor the installs' record (no hook or byte
added: `--dev arena,arenaboth,arenalag=4,arenadelay=1` over at frame
2690, its last hash `86d2f837337a1494`, as before). The proofs, 22
October 2026, the player's Gregar and Team Colonel ROMs, `arenadelay=1,
arenalag=4`, `CYBERWORLD_EMU_DEBUG=1` (each answer said by core: the
pair's cores 1 and 2 are the two GBAs):

- **BN5's chip AI: a room of its own for each MegaMan** (issue #182).
  BN5's chip AI keeps its state past BN6's 0x100 bytes of AI data, so a
  MegaMan under it needs BN5's 0x140; side 0's player's AI data moves
  into `RAM_PLAYER_AI` at his set-up. In a plain 1v1 DarkInvs on side 1
  moved side 1's AI data into that same block: seen on both GBAs with a
  checkpoint of thirty DarkInvs carried by both sides (`arenapeer=2`,
  DEVTOOLS.md) "side 0 020115e0 side 1 020115e0", then side 0 grey under
  side 1's DarkInvs, both MegaMen standing still and the battle going
  nowhere (BN5's machine ran for side 0 alone, on the keys both copies
  wrote). Now each other MegaMan of a NetBattle (side 1's player, the
  partners) moves into a room of BN5's size of his own at the end of
  BN6's AI data's pool as the chip AI first takes him (`darkinvs_room.h`,
  docs/EMULATION.md, BN6's RAM; docs/ROM_DATA.md, BN6's AI data's pool),
  and the machine, its time and its idle frames run for any MegaMan whose
  AI data is of BN5's size. Seen: side 1 under the chip AI (HiCannon,
  MegEnBom, LongSwrd) and side 0 himself, "seat 2's AI data 02034180
  moved into his room 02034740" on both GBAs, over at frame 1179 (side 1
  won); both sides at once (`arenapeer=3`), each driven apart; with
  `arenaroll=8` 147 rollbacks of 8, the state the same after each, its 19
  hashes those of `arenawindow=0`. Two a side with all four under it
  (`arena2v2=1,arenapeer=3,teamhp=500`): side 1's player's room and the
  partners' ("seat 2's AI data 02034280 moved into his room 02034740",
  seat 3's into `0x020344C0`, on both GBAs) taken, both GBAs alike at
  all 2,372 iterations, over at frame 2675; `arenaroll=8` 319 rollbacks,
  the state the same after each, its 42 hashes those of
  `arenawindow=0`; `chipaibreak=2` (no room on the second GBA) apart from
  iteration 340. On this build (the partners' words in BN6's packets,
  the same checkpoint): side 1's DarkInvs in a 1v1 over at 1179 as
  before; two a side over at 2261, 1,954 iterations 0 apart (193
  rollbacks); `arenawindow=0` and `arenaroll=8` over at 2399, 2,092
  iterations 0 apart, 299 rollbacks of 8, the same 39 hashes;
  `chipaibreak=2` apart from iteration 340 again. A partner was left out of the chip AI before
  (`held_short`): he is not now.
- **The Chaos Unison's hooks act for the side's player alone**
  (`soulchaos.c`: the charge, its phases, the release, the request taken,
  the counter's test, the glow; the Navis' buster arm's chaos,
  `darkinvs.c`). A partner charging B while his lead was in a Chaos
  Unison stepped his lead's unison and took its release: his charged B
  became his lead's chaos attack or its failure. Seen with side 0's lead
  in NumberSoul by DarkPlus (thirty DarkPlus, `souls=0x04`, UNITE by the
  keys of `tests/xdiff/chaos_number_45.json`, B held 45 frames six
  times), his partner holding B 110 frames twice a turn: "side 0's
  partner's charged B his own, his side's player in a Chaos Unison" on
  both GBAs, his lead's chaos attack his lead's; over at frame 3659, both
  GBAs alike at all 3,123 iterations; `arenaroll=8` 456 rollbacks, the
  state the same after each, its 60 hashes those of `arenawindow=0`;
  `partnerbreak=2` (the partner his lead on the second GBA: the rule
  before) apart from iteration 222, the second GBA's lead releasing in
  phase 2 where the first's failed. On this build, the same checkpoint
  and keys: the partner's charged B his own on both GBAs, over at 3419,
  2,883 iterations 0 apart (287 rollbacks); `arenawindow=0` and
  `arenaroll=8` over at 3415, 426 rollbacks of 8, the same 56 hashes;
  `partnerbreak=2` apart from iteration 219.
- **A Soul's powers and look are its MegaMan's** (`soulchips.c`: the chip
  charge and its double, TomahawkSoul's grass, KnightSoul's guard,
  ShadowSoul's sword and shadow; `soullook.c`: the palette of an object
  drawn in his side's form, the arm flash and its height, the crossing's
  sound; `soullife.c`: the turns counted, the end and the weakness break,
  each refused for a partner). Read, and seen in the run above: the
  partner's charged shots plain, his look his own. (Since [A partner's
  UNITE](#a-partners-unite) each reads the MegaMan's own Souls state:
  his side's for a lead, his own block for a partner, who has a Soul of
  his own to read.)
- **BN5's mood and its DarkChip rule by object** (`soulmood.c`). A
  partner's mood and dark are kept in a block of his own after the
  sides' (`BN6_SOULS_PARTNERS`, docs/EMULATION.md), his NaviStats his own
  (`soul_navi_stats`, his room's in `arena_team_navi.c`, told by
  `arena_team_rules.c`): a BN5 DarkChip he uses darkens him and lays its
  after-effect on his NaviStats, a hit on him is his mood's, BN5's dark
  gate reads his. Before, all of it was his lead's (his DarkChip zeroed
  his lead's mood, BN6's own mood byte with it: no Cross, no Beast Out, a
  weak buster for the lead; every hit on him lowered his lead's mood, the
  poison panels under the partners from the battle's first second). So a
  2v2 NetBattle hashes otherwise from frame 59 than before: the run of
  the proofs below (`arena2v2=1,teamhp=500,arenalag=4`, a fresh data
  folder) still ends over at frame 3426, side 0 won, 57 hashes, 2,700
  iterations 0 apart, and with that one line of the hit as it was it
  hashes as the build before did, all 57 (`219d892589f3cb16` at frame
  3419). Seen: "side 0's partner dark for the battle:
  his mood at 0203ffbe, his NaviStats at 02037fd0" (the fourth kind's
  slot 30), side 1's partner's at `0x0203FFDE` and `0x02038098`; with
  `partnerbreak=2` the second GBA's says "side 0's player ... 0203ce00"
  at the same use. A partner's mood is never opened (his Custom screen is
  a copy with no hook), so BN5's writers leave it 0 and the worried rule
  never offers him a DarkChip; a hit's gain still goes to the attacker's
  side's player (BN5 has one a side).
- **BN5's Navis' and chips' targets: nothing in a NetBattle asks.** The
  audit took five chips of BN5's (ZSaber, Katana, MoonBld, CustSwrd,
  DarkCirc) for asks of BN6's side-player getter (`BN6_SIDE_PLAYER_GET`,
  which names a side's player alone). Read again: their recipes copy a
  block of BN5's (`0x08012530`-`0x08012704`) holding a routine that reads
  a side's player's chaos flag (BN5's `0x080125EE`) which none of their
  code calls (BN5's one caller lies outside every unit); a hook counting
  the getter's asks from moved code saw none in 2v2 NetBattles with
  thirty Katana, MoonBld, CustSwrd or ShadoMan (kept out of the build).
  BN5's Navis that ask it (ShadowMan's, NumberMan's and TomahawkMan's
  AIs) aim at MegaMan as enemies in a run's battles and never stand in a
  NetBattle. Were one ever to, his ask from moved code would be answered
  with the nearer of the side's two (columns and rows apart, the side's
  player where as near), the shape tried and taken out again since
  nothing reaches it and the getter runs some hundred times a frame.
- **Left out still:** a united Soul carried to the other GBA is the
  lead's alone (a partner's crosses in his stream: [A partner's
  UNITE](#a-partners-unite), which since gives him UNITE, his Soul's
  life and its powers by object). The
  Dark Soul's chip lists, which DarkInvs's machine plays too, are a
  side's, made from the lead's folder (`arena_souls.c`): a partner under
  the chip AI plays his lead's list. The green flash (`xchips_flash.c`)
  is counted once a side but tints both MegaMen of the side. BN6's own
  DarkChips' rule (`darkbn6.c`) is a run's, not on the pair.
- **A Cross left him as he was** (read in bn6f; his own since His Cross,
  above, which builds what this asks for): BN6's
  crossing controller (`0x020367F0`, bn6f dword_20367F0) keeps an entry
  a side (+8 and +0x18: +0 a Cross chosen, +4 a Beast Out, 0xFF none,
  +8 the side's player's object). Its first mode (sub_801486C) flags a
  Beast Out's object (its AI data +0x44 bit 0x4000000, sub_802DCDE) and
  sends a Cross to its second (sub_80148CC: sub_80148EC,
  activateCrossHappensHere_8014944, sub_801498E), which crosses the
  entries' objects; its third (sub_8014A00) waits for both. A crossing
  object reads its form by its alliance (sub_801595E from sub_8014A38),
  but only the entries' objects cross: a lead's Cross leaves his partner
  as he is. His own Cross would need entries of his own in that
  controller (or its modes run again for his object), his own pending
  record answered by object at `0x0801595E`, and the controller's wait
  taught to wait for him.

### A partner's UNITE

A partner unites with his own Souls on his own Custom screen, as his lead
does (22 October 2026, issue #181; `arena_team_soul.c`, `soullife.c`,
the Souls' modules by object):

1. **UNITE on his copy.** UNITE and ColonelSoul's button are hooks of
   BN5's installs on the Custom screen (`soulunite.c`, `soulcolonel.c`).
   His side's GBA of his copy now takes the recorded installs' hooks
   (`xlog_hook_core`) and his own Souls held (`soulunite_held_core`,
   the loadout's `souls`), without the pair's two-a-side hooks
   (`arena_pair_open_screen`); a copy whose partner holds no Soul stays
   plain, no hook on it. That is a hook core more on a partner's game,
   two where the dev flag stands in for both partners: `HOOK_CORES` is 5
   (BN6's, the pair's two, a copy's one or two). Each turn his own Souls
   block (`BN6_SOULS_PARTNERS`) is copied into his side's block there,
   as his record is into his side's NaviStats, so UNITE reads his mood,
   the Souls he has used and a Soul he is in.
2. **Crossing as his Cross does.** His OK on UNITE commits the proxy form
   (8) in his copy's record and the Soul in its side's block, as BN5's
   OK does a lead's (`BN6_SOUL_NEXT`, `_NEXT_CHAOS`, `_NEXT_CHIP`); his
   client says his Cross (the proxy form) and then the Soul
   (`TEAM_SOUL`: the Cross count's index 1, the Soul and its chaos; and
   `TEAM_SOUL_CHIP`, index 2 or 3, ColonelSoul's chip). Each GBA takes
   them from his side's packet into his own block, with the Soul's used
   bit and his turns (BN5's three and his own record's
   `BN6_NAVI_TURN_EXTRA`, one for a Chaos Unison), and answers his
   crossing's pending record with its chaos and turns
   (`arena_team_cross.c`'s `pending`). A Soul said for a turn that never
   crossed is dropped as his next turn opens.
3. **His Soul's life by object.** `soullife.c` takes a Soul up as a
   MegaMan enters the proxy form and lets it go as he leaves it by his
   own Souls state (`soul_field_of`): a side's player's in his side's
   block as before, a partner's in his own (his own mood calmed, BN5's
   chip AI left, a Colonel's side bit his lead's alone). BN6 counts turns
   and asks a form's end for the sides' players alone, so the partners'
   turn hooks call the Soul's countdown (`soul_count_of`, from the
   countdown's body run for him) and its end (`soul_ended_of`, BN6's
   request to leave the form in place of Beast Out's end check, as for a
   lead); a Custom screen opening marks the partners' blocks too
   (`BN6_SOUL_SCREEN`).
4. **His Soul's own powers.** Every read of a Soul where its MegaMan is
   at hand reads his own Souls state (`soul_id_of`, `soul_field_of`): the
   Chaos Unison's charge, phases, release, request, counter and glow
   (`soulchaos.c`), the chip powers (`soulchips.c`), the abilities and
   loadout (`soulpower.c`), the passives and the chip bonus
   (`soulpassive.c`, `soulproxy.c`), the look, palette, crossing's sound,
   face and arm flash (`soullook.c`), BN5's moved code's reads of its AI
   data's Soul fields and the form getter (`soulcode.c`), BN5's DarkChip
   ending a Soul (`soulmood.c`) and ColonelSoul's chip at the turn
   (`soulcolonel.c`). For a side's player each reads what it read
   before: the Souls' differential (`build.py xdiff`, the 72 specs of the
   Souls, their chips, BN5's mood and DarkChips, the Dark Soul and
   ColonelSoul) passes as before, and a NetBattle of two hashes as
   before.

`teamunite=1` (DEVTOOLS.md) gives each stood-in partner's copy the
carried MegaMan's Souls and has his client take the chip under the
cursor and UNITE at his first turn; on a partner's own game in a
side-by-side match it gives the MegaMan he carries NumberSoul and Atk+10
`*` as his folder's first chip, a UNITE in his first hand. Proved, 22
October 2026, both ROMs, `--dev
arena,arenaboth,arena2v2=1,souls=0x04,teamunite=1,teamhp=500,arenadelay=1,quitend`
from the rules branch's checkpoint of thirty DarkPlus (each partner's
first chip DarkPlus: NumberSoul's Chaos Unison), `CYBERWORLD_EMU_DEBUG`:

| Run | End | Hashes | The two GBAs |
| --- | --- | --- | --- |
| `arenalag=4` (388 rollbacks) | over at frame 4846 | 80, all the same as `arenalag=0,arenawindow=8`'s | 3,600 iterations, 0 apart |
| `arenalag=4,arenaroll=8` | over at 4857 | all 80 the same as the delay alone's (`arenawindow=0`) | 3,600, 0 apart; 607 rollbacks of 8 frames, the game's state the same after all 607 |
| `teamunite=-1,arenalag=4` (his Soul left out on the second GBA, the mutation) | | | apart from iteration 1 |
| `teamunite=2,arenalag=4` (each partner's folder opening with Atk+10 `*`: NumberSoul, three turns; 671 rollbacks) | over at frame 7766 | 129, all the same as `arenalag=0,arenawindow=8`'s | 3,226 iterations, 0 apart |
| `teamunite=2,arenalag=4,arenaroll=8` | over at 7777 | all 129 the same as the delay alone's | 3,226, 0 apart; 972 rollbacks of 8 frames, the game's state the same after all 972 |
| `teamunite=-2,arenalag=4` (the mutation) | | | apart from iteration 1 |
| `teamunite=3,arenalag=4` (RockCube `*` first, ColonelSoul held: ColonelSoul, three turns; 552 rollbacks) | over at frame 6204, side 1 won | 103, all the same as `arenalag=0,arenawindow=8`'s | 2,117 iterations, 0 apart |
| `teamunite=3,arenalag=4,arenaroll=8` | over at 6354 | all 105 the same as the delay alone's | 2,263, 0 apart; 794 rollbacks of 8 frames, the game's state the same after all 794 |

On both GBAs: "side 0's partner united with Soul 9 (chaos 1, turns 1),
his own", his chaos B "released, out of phase 2: the failure", then "side
0's partner's Soul 9 ended, his own". With `teamunite=2` side 1's
partner's NumberSoul of three turns: "united with Soul 9 (chaos 0,
turns 3), his own" on both GBAs, its turns counted at each Custom screen
from the countdown's body run for him, and its end asked through the
turn check as a lead's ("a Soul ended 2 times", both GBAs), "Soul 9
ended, his own"; side 0's partner, his lead's folder of DarkPlus behind
the Atk+10, took his UNITE screen but never pressed OK at his next two
turns (his stood-in script's keys, not the Soul's), his client sending
his picks at the wait's limit. With `teamunite=3` side 1's partner's
ColonelSoul: "united with Soul 7 (chaos 0, turns 3), his own" on both
GBAs, his turn's charged shot set by ColonelSoul's turn hook for him (no
chip on his button: ScreenDivide). From his seat (`teamseat=2`, the
pictures kept out of the repository): his own screen of DarkPlus, his
OK, NumberSoul's image over him as he crosses, then his MegaMan in the
Chaos Unison's palette beside his lead, his chaos B's glow charging.
Over the network (the four games of [Over the
server](#over-the-server-protocol-v15), `teamunite=1` on Beta's game,
Delta's HeatCross beside it, Epsilon watching): Beta's UNITE of
NumberSoul on his own screen, his Soul said on every game ("said 2
times", both GBAs), his MegaMan in NumberSoul's look on the field; over
at frame 1846 on all four, 30 hashes agreed and the spectator's 30
checked, 960 iterations 0 apart on all five.

Not exercised yet: ColonelSoul's button on his copy (a chip taken onto
it at a later turn, its chip shot: built, the stood-in client's script
never presses it), and his Chaos Unison's failure's Dark Soul, which plays his side's lists, made from his lead's folder
(`arena_souls.c`): his own folder's would need lists past the sides'
two in `RAM_DARK_LISTS`.

### Proved: the partners' own folders and screens

The player's Gregar ROM and Team Colonel's, a fresh data folder each,
`--dev arena,arenaboth,arena2v2=1,teamhp=500,arenadelay=1,quitend`, both
partners' clients stood in for on the machine (their screens' keys a
script by the frame: the cursor right and A every 20 frames), side 0's
lead tapping R every 600 frames in the fight so that the Custom screen
opens again once its gauge is full (three turns; with `teamhp=150` the
partners are deleted before the second), 22 October 2026, with the leads
waiting for their partners (below):

| Run | End | Hashes | The two GBAs | His picks |
| --- | --- | --- | --- | --- |
| `arenalag=4` (285 rollbacks, 1,140 frames run again) | over at frame 3427, side 0 won | 57, all the same as `arenalag=0,arenawindow=8`'s (0 rollbacks) | 2,696 iterations, 0 apart | three turns each, set as his side's screen closed, taken by both GBAs at the same iteration |
| `arenalag=4,arenaroll=8` | over at 3427 | all 57 the same as the delay alone's (`arenawindow=0`) | 2,689, 0 apart; 428 rollbacks of 8 frames, the game's state the same after all 428 | as above |
| `teampick=-1,arenalag=4` (a chip short in his hand on the second GBA alone, the mutation) | over at 3836 | 63 (the local link echoes them) | apart from iteration 1 (2,696 of 2,696) | |

Each partner's hand's chip up next moves from his own picks alone:
side 0's at iterations 166 and 190, 1361 and 1393, 2145 and 2625, side
1's at 418, 818, 1373, 1773 and 2397, both back to 0 at 1028 and 1983 as
their picks of the next turn go in. (Before the wait, the same runs:
2,725 iterations, 57 hashes, the chips 11 iterations later; with
`teamhp=150`, one turn each, 1,314 iterations 0 apart, 30 hashes the
same with 173 rollbacks and with none, `arenaroll=8` 230 of 230.) A
NetBattle without the flag is the battle it was:
`--dev arena,arenaboth,arenalag=4,arenadelay=1` over at frame 2690, the
last hash `86d2f837337a1494`.

His client's screen: a frame 1.3 to 1.5 ms (the worst 2.1); the pair's
copy into it 0.14 ms at the first turn and some 20 ms at the later ones
(the worst 31: a frame or two of his own screen held as a turn begins).
Timed apart: the pair's save 0.05 to 0.08 ms every turn, its load into
the copy 0.08 ms at the first and 17 to 19 ms at the later ones, inside
mGBA's state load (not yet looked into). Made once at the first turn,
some 155 ms (the battle's worst tick then, 3 past a 60th in a battle).
His screen offered no Cross and no Beast Out then (their flags cleared on
the copy's GBA); since His Cross it offers his own Crosses and Beast Out
(`teambeast`).

Captured (`teamseat=2` and `3`, the pictures kept out of the
repository): BN6's own Custom screen on each partner's client with his
own deck (side 0's: BusterUp *, Sword S, Cannon B, Atk+10 *, MiniBomb L,
where his lead's GBA offers a Cannon A first; side 1's: WhiCapsl *,
Sword S and three chips of code D), his picks in BN6's column as he
takes them, his OK and BN6's "Sending chip data..." after it; his second
turn opening on the chips he left (Cannon B and MiniBomb L first, then
Cannon A, Sword S and an L chip), as BN6 keeps the chips not taken at
the deck's front; in the fight his HP box (150), the four marks, his
lead's HP under his lead, and his own chips' icons over him.

### Over the server (protocol v15)

Two MegaMen a side between four games over the server (issue #181, 22
October 2026): a team challenging a team in the Net Square names its
format, the relay ([Team NetBattles](#team-netbattles)) or side by side,
and side by side all four play one battle.

**The design.** The game-design lens first. The dialectic is Team
NetBattles' own, mine against ours, pushed to its end: in the relay a
MegaMan is the team's one at a time, side by side he is the team's at
every moment (his shots, his chips, his place on the field next to his
partner's). The loop layer is the session loop's "one thing together",
now with nobody waiting in the stands. The patterns: *subtract* (one
question with two answers, asked of the challenger as his Battle goes;
no settings screen, no order asked side by side: each team's captain
leads, his game the GBA of his side), *telegraph honestly* (the
challenged team reads the format in its question before its Yes),
*diegetic first* (MegaMan asks in BN6's chat box, the way in and the
card are the PET's panels) and *give every player a verb* (side by side
nobody watches; each of the four plays the whole battle). The words are
MegaMan's: "How do we fight 'em,Lan?" with `One by one` and `Side by
side` (B: no challenge), and on the other team's side "NAME's team wants
to fight side by side!", "Take it,Lan?" (docs/VOICE.md: one beat a box,
nothing explained; the formats are named by what they look like, as a
kid would say them). No number about the link is shown anywhere.

**The wire** (shared/online/README.md, protocol v15):

- ARENA's last byte is a team challenge's format (`team_relay` 0,
  `team_side_by_side` 1); ARENA_STATE tells it to the challenged and the
  challenger, TEAM_STATE to all four (a relay's order is asked only for
  the relay).
- Side by side the team match is one battle of four **seats**: seat 0
  the challenger's team's captain (the cable's first GBA), 1 the other
  captain (the second GBA), 2 and 3 their partners. All four send ARENA
  accept when their screens are clear; START goes to all four once all
  four have, each with its seat (ARENA_STATE's side byte, 0 to 3) and the
  four seats' sessions, the delay the longest of the four latencies'.
- What a seat sends (its MegaMan, its keys, its hashes, its ready and
  installs' record, its lag notes) goes to the three others as
  **ARENA_SEAT** (35): the seat, then the message as sent. A partner's
  keys keep their sixteen bits (his Custom screen's picks, bit 15).
- Seat 0 reports the battle (ARENA_STATUS) and sends its end, as the
  cable's first ever did; the winner's team wins, no winner is a draw, and
  a leave, a decline or a dropped link of any of the four ends it broken
  off for both teams (TEAM_STATE over, result 3). All four are busy in
  the Square's marks while it runs.
- The stands keep four seats' MegaMen and keys: WATCH_STATE names the
  partners' seats, WATCH_RELAY and WATCH_KEYS carry the seat, and the
  cable's first's hashes are relayed as ever. STANDS's bout byte says
  side by side (`stands_side_by_side`, 255): the referee calls "TEAM
  NETBATTLE! SIDE BY SIDE!", "ALPHA'S TEAM VS GAMMA'S!" (the leads'
  teams), then the leads' HP as ever; a spectator's end says "Alpha's
  team won the NetBattle.".

**Every game runs the same pair.** Each of the four games (and a
spectator's) builds the pair from the four MegaMen as they came: seats 0
and 1's into the two GBAs as a NetBattle's, seats 2 and 3's into the
partners' own records (`arena_seats.c`: each partner's folder dealt into
his deck, his max HP, base HP and buster in his NaviStats), the dev
flag's two-a-side hooks switched on for the pair (`arena_seats_on`), so
that the pair's hash covers them. The network's link holds four seats
(`arena_link_net_seats`): each other seat's stream, hashes, ready and
record apart; the engine puts my keys in my seat's stream and takes the
three others' (`arena_link_seat`, `poll_player`), each guessed and rolled
back as before. A ready, a record or a hash is said once every other
seat's has come: mine where all three agree with mine, else the first
apart, so that a game apart from any of the others stops before the
first frame or at the next hash. The time sync keeps to the other GBA's
stream (a partner's to the other side's lead's) and its lag notes; a
partner's own keeps to his round trip.

**The partner's own client.** A partner's game shows his side's GBA and
makes his own Custom screen (BN6's, on a plain copy of the pair:
[A partner's own Custom screen](#a-partners-own-custom-screen)) as the
way in ends, before the battle's first frame, so its cost (some 155 ms)
falls outside the battle. Each tick his keys go through his own client
(`team_client_stream`): on his screen while his side's GBA's is open,
his picks crossing as pick messages at his OK (or at the wait's limit),
his battle keys otherwise. The leads wait for him as in the dev runs,
by BN6's own cable; the limit stays 60 seconds (`TEAM_WAIT_LIMIT`, the
owner's to confirm).

**The way in and the card.** The way in's panel shows four plates, my
team's two on the left (its captain first, mine lit), theirs on the
right, each MegaMan's HP as he goes in and his PET's wait (coming,
linking, ready) under it; the band says `Side by side`. The battle shows
my seat's screen (a partner's own Custom screen in his turn, the four
told apart by their seats' marks); a stall names whose PET of the three
it waits for. The result card says the team's result (TEAM WIN!, TEAM
LOSE, TEAM DRAW, TEAM BROKE OFF), against the other team's two names,
each MegaMan's HP left (my team's on the left) and the time; no
standing, no next, no rematch.

**The four-game proof** (22 October 2026, protocol v15, the build of
the four seats before the partners' words rode BN6's packets; this
build's runs follow the table; the player's
Gregar ROM and Team Colonel's in each game's `--rom-dir`, a local server
with seed 120's Square, four host builds headless from `--scene home
--run-depth 7 --seed 3 --dev quiet,arenaboth,arenahp=40`, each MegaMan
40 HP; `arenaboth`: the leads' keys the bot's, the partners' their
stood-in script's, on BN6's Custom screen of each partner's own game and
in the fight; driven by remote play from a harness that reads each
game's log, kept in `.build/2v2/`). Beta, Alpha, Delta, Gamma join;
Gamma asks Delta to team up, Alpha asks Beta, each answered Yes; Alpha's
Battle on Gamma asks its format (DOWN to `Side by side`, A); Gamma's game
takes it; all four go in, seat 0 Alpha, 1 Gamma, 2 Beta, 3 Delta. Every
game's ready said the same pair's hash, `a5bfd374f4a0a3dc`, on all four
and on Epsilon, a fifth game watching from the stands through the
referee.

| Run | End (every game) | Hashes | The two GBAs | The partners |
| --- | --- | --- | --- | --- |
| Side by side, no lag (delay 1) | over at frame 1249, Alpha's team won | 20 agreed on each of the four, 20 checked by the spectator | | each picked on BN6's own Custom screen of his own game |
| Beta's hands off his screen 10 seconds a turn (`teamslow=600` on his game; the last build, twice: the numbers the second's) | over at frame 1850, Alpha's team won (the first: 1849, Gamma's) | 30 agreed on each, 30 checked by the spectator | 970 iterations compared, 0 apart; the partners' keys 0 late, 0 taken apart | side 0's GBA held its packet for Beta 589 frames at most in a turn, side 1's for Delta 10; their picks taken into their own hands on both GBAs |
| Through a local relay, 30 ms and up to 20 ms of jitter each way | delay 5; over at frame 1254 | 20 agreed on each, 20 checked by the spectator; 105, 100, 120 and 51 rollbacks | 952 compared, 0 apart; 0 late | as above |
| Delta's game killed three seconds into the battle | Alpha's and Gamma's battles stopped at frame 189, Beta's at 187: "the other player left the NetBattle" | | | all three told the match broken off (result 3) |
| Delta's game held still from its tick 300 (`arenafreeze=300`, its link kept) | the three others' notes "Waiting for Delta's PET...", B offered at 600 ticks, given up at 1200; the card BROKE OFF, "Delta's PET stopped answering.", TEAM BROKE OFF | | | the match broken off for all four |
| Beta's game never picking (`teamslow=-1`: his client sends nothing) | side 0's GBA held 3600 frames, the wait's limit, his picks none; over at frame 5571 | 92 agreed on each of the four | **apart**: 856 of 1,132 iterations from 73; the partners' keys 107 iterations late, 44 taken apart | (below) |

The last run was the fault [The partners' words in BN6's
packet](#the-partners-words-in-bn6s-packet) mends: every game and the
spectator agreed on every hash, so the pair was the same everywhere, but
inside it the two GBAs' battle loops ran up to 5 to 8 iterations apart
(read from the rings of then: GBA 1 at iteration 75 found side 0's slot
still holding 67; GBA 0 at 72 found side 1's already holding 80), past
the rings' 4 iterations, in streaks of 20 to 50 iterations about every
60: from GBA 0's iteration 66 of the fight, both leads charging, GBA 0's
count went up one every two frames while GBA 1's went up about one a
frame (a probe of both GBAs' counts after each pair frame).

**A partner's whole MegaMan over the network.** Each game sets the
partners up from the MegaMen they sent (`arena_seats_partners`): his
folder, max HP and buster as before, and now his NaviCust compiled by
BN6's own reload on a core of that game ([His NaviCust, compiled by
BN6](#his-navicust-compiled-by-bn6)), his record and his screen's hand
size from it, on every game and spectator alike. His own Custom screen
offers his own Crosses and Beast Out as his MegaMan has them
(`team_client_seat`, from the loadout he sent), and his CROSSSELECT's
choice, his Beast Out and a Program Advance his OK makes cross in his
stream as on one machine. `teampa=1` on a partner's own game makes the
MegaMan he carries in a Program Advance's first (Cannon A, B and C his
folder's first chips, Regular chip and TagChips: `arena_seats_pa_folder`),
his own folder as it crosses; the deck override of the stood-in partners
is theirs alone, so that no game's dev flag deals another's deck.

**This build** (22 October 2026, after [The partners' words in BN6's
packet](#the-partners-words-in-bn6s-packet), the same harness and
games, `--dev quiet,arenaboth,arenahp=40` and each run's flags on the
games named; every game's log and the spectator's compare the pair's two
GBAs iteration by iteration):

| Run | End (every game) | Hashes | The two GBAs, on each game and the spectator | The partners |
| --- | --- | --- | --- | --- |
| Beta's game never picking (`teamslow=-1`), Epsilon watching | over at frame 5810 | 96 agreed on each of the four, the last `d9df79c7251b8482`; 96 checked by the spectator | 1,325 compared, 0 apart on all four; the partners' keys 0 taken apart | side 0's GBA said Beta's picks none at 3,600 frames held, on every game |
| Beta's HeatCross (`teamcross=1` on his game), Delta's Program Advance (`teampa=1` on his), Epsilon watching | over at frame 1020, Gamma's team won | 16 agreed on each (the pair's ready hash `3d4dbab92c15e96d` on all), 17 checked by the spectator | 390 compared, 0 apart on all five | Beta's Cross sent once and taken by BN6's crossing for him on every GBA ("side 0's partner asked to cross 2 times", both GBAs); Delta's Z-Canon1 (`140`, power 300) from Cannon A, B and C, "side 1's partner's hand a Program Advance" on both GBAs of every game; both partners' NaviCusts compiled on all five (HP 40) |
| Beta's hands off his screen 10 seconds a turn (`teamslow=600`), Epsilon watching | over at frame 1849 | 30 agreed on each, 30 checked by the spectator | 964 compared, 0 apart on all five | side 0's GBA held its packet for Beta 593 frames at most in a turn, side 1's for Delta 14 |
| Through a local relay, 40 ms and up to 30 ms of jitter each way; Beta's Beast Out (`teambeast=1` on his game), Delta's Cross 2 (`teamcross=2` on his), Epsilon watching | delay 7; over at frame 1855 | 30 agreed on each, 30 checked by the spectator; 144, 179, 138 and 153 rollbacks | 971 compared, 0 apart on all five | Beta in Beast Out's form (0xB) and Delta in his Cross's (2) from the first turn on both GBAs of every game, BN6's crossing run for each ("asked to cross 2 times", both GBAs); Beast Out's turns counted for them as for the leads |
| Beta's UNITE (`teamunite=1` on his game: NumberSoul held, Atk+10 `*` first), Delta's HeatCross, Epsilon watching | over at frame 1846 | 30 agreed on each, 30 checked by the spectator | 960 compared, 0 apart on all five | Beta's NumberSoul from his own screen, said on every game and taken up on both GBAs, his MegaMan in NumberSoul's look ([A partner's UNITE](#a-partners-unite)) |
| Delta's game killed three seconds into the battle | Alpha's, Beta's and Gamma's battles stopped at frame 192: "the other player left the NetBattle" | | 5 compared, 0 apart | all three told the match broken off (result 3) |
| Delta's game held still from its tick 300 (`arenafreeze=300`, its link kept) | the three others waited on Delta's keys and gave up at 1,200 ticks: "the other's keys stopped coming", the match broken off (result 3) | the four agreed to frame 239 | | |

The same runs again on the integration's last build (the games pinned to
it, 22 October 2026): the Cross and the Program Advance frame for frame
as above (over at 1020, the last hash `cbc8963da56d0ffb` on all four);
the partner who never picks over at 5809, 96 hashes agreed and the
spectator's 96 checked, 1,332 iterations 0 apart on all five; Beast Out
and Cross 2 under 40 ms lag and 30 ms jitter over at 1856, 30 hashes,
973 iterations 0 apart on all five; the slow partner over at 1850, 965
iterations 0 apart on all five; Beta's UNITE beside Delta's Beast Out
over at 967, 16 hashes agreed and checked, 390 iterations 0 apart on all
five; the drop stopped at frame 190 on the three, the frozen game given
up at 1,200 ticks, both broken off (result 3).

The partner's own client, seen from Beta's seat (seat 2, the pictures
kept out of the repository): the way in's four plates (Alpha and Beta on
the left, his own lit, Gamma and Delta on the right, `Side by side` in
the band), BN6's Custom screen of his own deck while his lead's window
has slid out, his lead's sign (`P1` and a tick), his OK and BN6's
"Sending chip data...", the fight with his own HP box (40, 35, 26, 17 as
he was hit) and the four marks, "MEGAMAN DELETED", and the card: LOSE,
VS Gamma & Delta, Alpha 0 and Beta 0, Gamma 6 and Delta 0, 0:17, TEAM
LOSE; back in the Square, "Aw... Our team lost.". From Alpha's (seat 0):
his window slid out after his OK and his partner's sign (`P3` and its
dots) for the ten seconds, then the same battle from the left GBA. From
Delta's (seat 3): his own Custom screen (Sword first), "Sending chip
data..." while the other side waited for Beta, and the card's WIN, VS
Alpha & Beta, TEAM WIN!. From Epsilon's: "WATCHING Alpha VS Gamma" and
the battle as the cable's first GBA has it. Gamma's game without the bot
asked "Alpha's team wants to fight side by side!" and "Take it,Lan?",
the cursor on No; B turned it down. A NetBattle of two is the battle it
was: `--dev arena,arenaboth,arenalag=4,arenadelay=1 --frames 4200` over
at frame 2690, last hash `86d2f837337a1494`; and `--dev
arena,arenaboth,arena2v2=1,teamhp=500,arenadelay=1,quitend,arenalag=4`
over at 3426, 57 hashes, 2,703 iterations 0 apart, as before (over at
3427 on this build, 57 hashes, 2,696 iterations 0 apart: [Four
streams](#four-streams)).

## Not yet

- Two MegaMen a side ([Two MegaMen a side](#two-megamen-a-side)): over
  the server ([Over the server](#over-the-server-protocol-v15)), seen
  through a local server on one machine, not yet over a real network,
  through the deployed server or between devices; the stands' heading
  names the two leads alone; a partner's NaviCust compiled on games of
  different runs (each over its own checkpoint: the hashes say they agree
  where the runs are the same); a partner's ColonelSoul's button and a
  Soul's three turns exercised ([A partner's UNITE](#a-partners-unite)),
  the Dark Soul's chip lists his own folder's, the green flash's tint a
  MegaMan's own, a hit's mood gain the attacker's own; a chip of
  computed power in his hand (his hand takes a computed chip's power as
  0), WhiCapsl's and Uninstll's marks; the bytes past a Program Advance's
  end in his hand (BN6's built hand leaves the joined picks' last chip
  there, ours the hand's end: nothing reads past it, not checked by
  experiment); the screen copy's cost at the later turns (some 20 ms
  where the first is 0.14), and its later loads seen at up to 97 ms after
  a long wait; the leads' wait's limit for an absent partner (60
  seconds, `TEAM_WAIT_LIMIT`, the owner's to confirm); an attack of his
  seen asking his record by object (built, none asked yet).
- Team NetBattles ([Team NetBattles](#team-netbattles)): teams of three
  (6 of a channel's 8); a heal on a carried HP, which the relay starts
  without (watch whether anchors sweep); partners' fixed phrases to each
  other during a bout; the referee's own count of team matches (he
  calls each bout, and each player's last bout as ever); the panel and
  the card on a handheld and in a browser, and a team match through the
  deployed server; whether four players gather in a channel (the
  online statistics can count it).

- The battle's sound: the scene plays the player's core only where the
  game's own core has made its sound ring (`arena_pair_sound`).
- The NaviCust's programs are written as the board; BN6 makes HP from
  the base HP and the board again in the menus (seen: the max HP alone
  was put back to 100), but the other programs' effects in a NetBattle
  are not yet checked one by one.
- After the battle the engine stops once both cores have left it; the
  game's own way back to the Comm menu is not shown (the result card
  rises from BN6's fade instead).
- Issue #167's third and fourth steps ([A rematch in a
  second](#a-rematch-in-a-second), [The Square before a
  NetBattle](#the-square-before-a-netbattle)): seen through a local
  server on one machine; not yet over a real network or on a handheld.
  Which sound BN6's PET rings with for a call is not yet checked (the
  ring is BN6's effect `0x7E` for now); a question opened under another
  screen (the NetPass's stamp moment) is told seen as it opens. A
  spectator goes the whole way in for a rematch (his pair kept from the
  bout before would do, untried); a Team bout keeps no pair for the next
  (its MegaMen differ). The challenged player's game still waits for a
  chat box to close before the question (the research's "opens over
  reading" not done).
  The panel and the card on a handheld's screen and in the browser are
  drawn by the same code but seen headless only; the screens' sounds are
  BN6's own effects mixed over the held game's sound (`audio.c`), not yet
  heard on a device.
- Over a real network: measured on the deployed server (19 October 2026,
  docs/ONLINE.md) before the smoothed delay: delay 7, two full-rules
  battles to their end with every hash agreeing, the leading game waiting
  about one frame in eight (issue #128). The smoothed delay is measured
  through a local relay ([Smoothed delay](#smoothed-delay)), not yet on
  the deployed server nor between two devices; a stall shows as the
  link's badge and, past a second, a note naming whose PET it waits for.
- Issue #167's quick wins ([Quicker on the
  wire](#quicker-on-the-wire-issue-167)): a capture of whether WinHTTP
  and iOS's WebSocket hold small sends back; the rollback's sound heard on
  a device; BN6's link battle's own three frames of input delay over a
  single battle's (measured; shortening them is the owner's call); the
  time sync's notes on lopsided links between devices.
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
  (it stays home), and Android and iOS, which build the Arena but are
  untried. The browser battles (seen against another browser and a native
  game through a local server); the New 3DS builds the Arena but cannot
  run its pair in time ([The New 3DS](#the-new-3ds)): two CPU cores for
  the two GBAs is the way left, and needs a cable driver of our own.
- The browser: a NetBattle seen against another browser and against a
  native game through a local server, in headless Chrome; not yet in a
  browser with a screen, on a phone's browser, against the deployed
  server, or with a tab sent to the background mid-battle (its frames
  stop there, and the other game waits as for a phone sent to the
  background).
