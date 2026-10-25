---
type: Engine Mechanism
title: "The battle (arena.c)"
description: "How `arena.c` runs a battle with rollback: each frame at its tick on the other's last keys held, a ring of snapshots (`ArenaPairSnap`), a wrong guess rolled back and rerun, hashes settled on known keys."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-21T13:53:33Z }
---

# The battle (`arena.c`)

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
  heard ([Quicker on the wire](quicker-on-the-wire.md); dropped
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
