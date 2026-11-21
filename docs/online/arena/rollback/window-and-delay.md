---
type: Engine Mechanism
title: "The window and the delay, per system"
description: "How the rollback window and input delay are set per system: `arena_window_for` from the pair frame's cost kept in `arena.ini`, the latency asked for less the window (`arena_ask_ms`), the server's `delay_for`, overrides."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-21T13:53:33Z }
---

# The window and the delay, per system

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
  own too), the system's, never carried in a `.cwsave`. Before a
  system's first NetBattle none is kept: the window is then guessed from
  the run's own core, its frames timed since the game started (in the
  Square, about 0.45 to 0.50 ms here), a pair frame taken as 2.8 of them
  (the scenario's battles measured 2.4 to 2.7, rollbench's 2.2 without
  the pictures), 8 at most; 4 only where no second of frames was timed
  (issue #180: here, the core at 0.45 to 0.50 ms, a first NetBattle asks
  with 7 rather than 4, three frames of delay fewer on a link the window
  covers; with two scenarios running at once the core measured 0.74 to
  0.78 and the guess was 4, a busy machine guessing small).
- **The delay asked for** (protocol v8): a challenge or a Yes says the
  game's latency as before (its round trips in the Square, their mean and
  twice their spread) less what its kept window covers (`arena_ask_ms`:
  *W* frames of it; 4 where nothing is kept yet, a first NetBattle). The
  server's `delay_for` gives the lockstep's frame and the longer latency
  of the two in frames: 1 at least, 30 at most. Since issue #167 a game
  says its latency less its window but a frame, the tick a key waits to be
  taken, which the server added for every pair before (2 at least then;
  [Quicker on the wire](quicker-on-the-wire.md)). Since issue #180 the
  server reads each game's need from the path its keys take
  (`path_delay`): a key goes up half its sender's round trip and down half
  its receiver's, and the time sync (v11) holds both games' lags at that
  trip, so a game's need is its latency asked less half its own round
  trip plus half the other's (the round trips its PINGs say). On a
  lopsided pair this is less than the longer latency: asks of 10 and 150
  ms over round trips of 60 and 200 gave delay 10, now 6. Where a game
  asked 0 (its window covering more than its trip, by how much not said),
  a round trip is unknown, or the battle has four seats, the longer
  latency decides as before. So the pair's delay is the larger of the two games' needs:
  a fast desktop near the server asks for 2, a slower system or a long
  link for more, and a frame's keys come late by at most the window of
  the game that asked for the most.
- **Overrides:** `--dev arenawindow=W` sets the window (0: the delay
  alone, a frame run once its keys have all come; it also asks with that
  window), `--dev arenadelay=N` asks for N frames (in `--dev arena`, its
  delay).
- Systems: the desktops, Android, iOS and the browser battle and roll
  back ([In the browser](browser.md), below). The New 3DS builds the
  Arena (`--dev rollbench` and `--dev arena` run there) but offers no
  NetBattle and no watching (`CW_ARENA` 0): a pair frame takes half again
  a 60th of a second there ([The New 3DS](new-3ds.md)).
- **The picture** is drawn for the side shown alone (`arena_pair_draw`,
  mGBA's frame skip for the other core: the game's state the same either
  way), and a rollback's frames run again draw none but the last; a
  spectator draws the side he looks at, and while catching up a tick's
  first frame alone. Here a pair frame took 0.94 ms with both pictures,
  0.79 with mine and 0.70 with none (rollbench, 20 October 2026).
