# The link cable and the Arena

What the research on BN6's link cable measured (epic #120), what it means
for the production Arena, and how to run the tools it left. No release has
an Arena yet; the Net Square's presence and the checkpoint a visit borrows
are in [ONLINE.md](ONLINE.md).

## What the Arena builds on

- **Two linked cores on each client.** A client runs both games, its own
  and the opponent's, joined by mGBA's `GBASIOLockstep` cable in one
  process. The cable's words, cycle budgets and transfer phases never
  leave the client.
- **Only inputs cross the network.** Each player's controller state is
  sent once per frame, numbered by frame. A frame runs when both players'
  inputs for it are in. Both clients then run the same two games and stay
  in step without sending any game memory.
- **A buffer of frames.** One linked core can cross VBlank up to two
  frames ahead of its partner, so each client holds three frames of input
  ahead: about 50 ms of input latency on top of the network's.
- **Raw serial over the network does not work.** BN6 waits on every
  serial word; with the cable split across the network, a Practice fight
  at 50 ms made 95 battle frames in a minute.
- **Rollback needs a complete snapshot of the pair.** mGBA's own state
  save leaves out live state that the next frames read (below). Delay-based
  play needs no snapshot and comes first; rollback comes later.

Not reached yet: a run's loadout in the pair (folder, NaviCust, forms,
Souls, and refusing BN5's chips), a delayed fight with chips chosen,
attacks that draw many random numbers, rematches, jitter, reordering and
loss on a real network, prediction and rollback, handheld budgets, and
the server's challenge, accept, input and hash messages.

## What was measured

Each experiment ran BN6 Gregar USA (SHA-1 `89fe0bac…`) on mGBA 0.10.5's
Release library, both cores from the same pre-director `boot-3.state`,
their flash saves in memory. Times are from the Linux build image unless
another system is named; they vary with the host's load.

| Experiment | Result | What it means |
| --- | --- | --- |
| ROM-free cable, 15 Oct 2026 | 256 transfers, all 1,024 received words right, on Linux, Windows x64, macOS ARM64 and the Retroid Pocket Nova (11.0–13.6 ms there) | mGBA's lockstep cable links two cores on every system tried |
| State capture, 15 Oct 2026 | Raw state 397,312 bytes, saved in 0.010 ms and restored in 0.014 ms; a checkpoint with its 32,768 bytes of flash, 430,144 bytes, restored in 0.18 ms (Nova: 0.015 / 0.021 / 0.14 ms); all 294,912 bytes of RAM back exactly | Capturing and restoring a state is cheap; replaying frames is the cost still to measure on handhelds |
| Practice on one pair, 15 Oct 2026 | BN6's Single Battle, Practice, unpatched: 18,855 serial transfers, ten 10 HP charge shots, 100 to 0, results 1 and 2, both games back at the Practice menu, all 1,433 battle frames agreeing | BN6's own NetBattle runs on two linked cores without help |
| Serial words over a socket, 15 Oct 2026 | Each core in its own process: at no delay the fight completes (18,828 transfers each); at 50, 100 and 150 ms only 95, 48 and 35 battle frames in 60 s | Rules out sending the cable over the network |
| Inputs only, two cores per client, 18 Oct 2026 | At 0, 50, 100 and 150 ms one-way delay every fight completes: 2,822 frames, 5,644 inputs delivered, all 1,433 battle timers as the original; a flipped input bit diverged at frame 1,400 and stopped the pair in 107 ms, a killed peer stopped the other in 0.6 ms | The Arena's design |
| Snapshot of the pair, 18 Oct 2026 | Restored at six points (before a flash write, Custom, mid-transfer, a held charge, its release, after damage) and replayed 1, 4 and 8 frames, twice each: all 36 replays exact; 863,016 bytes a snapshot, 1.2 ms a restore; five planted faults each caught | Rollback is possible, with this snapshot |
| A carried chip, 18 Oct 2026 | Cannon A given by BN6's own `BN6_GIVE_CHIPS` and put in the folder with its folder editor: chosen, used and hitting for 40 (100 to 60) in the pair; `online_snapshot_read` reads the 6,664-byte loadout | A chip from a run fights natively in the pair; writing a loadout into a fresh pair is still to build |

What mGBA's state save leaves out, found by replaying from it: the
controller latch (`keysActive`) and the CPU's event countdown; the four
sound channels, which loading runs again; live IO latches such as
`SIOMLT_SEND`, which the cable reads for its next transfer; and the
cable's own state (phase, credits, received words, each node's next
event). `online_replay_snapshot.c` keeps all of them, rebuilds the
cable's pointers against the live pair and never stores a callback.
These are findings about mGBA 0.10.5, re-checked by running the pair
below; they are not ROM offsets.

BN6's Practice choice is checked in bn6f's text scripts
(`CompText86DA1B8`: 17 and 25 label Practice, 18 and 26 Battle, whose loss
forfeits a Pack chip) and by the fights above. The cable code was read in
mGBA's [lockstep driver](https://github.com/mgba-emu/mgba/blob/0.10.5/src/gba/sio/lockstep.c)
and [serializer](https://github.com/mgba-emu/mgba/blob/0.10.5/src/gba/serialize.c).
Tango (rollback for every Battle Network game) was looked at as a
product only; none of its code was read or used.

## The tools

| File | What it is |
| --- | --- |
| `tools/online_frame_pair.c` | The pair: two cores on one `GBASIOLockstep` cable, stepped a frame at a time from both players' inputs, with a hash of both states, both flashes and the cable |
| `tools/online_frame_wire.c` | Frame-numbered input packets and the queue that holds each side's inputs until both are in (`tests/test_online_frame_wire.c`, in `build.py test`) |
| `tools/online_link_script.c` | Practice from the boot state: the menu inputs, the fight's inputs and the check of its result |
| `tools/online_replay_snapshot.c` | The complete snapshot of the pair, taken and restored between frames or inside one (`online_replay_execute.c` steps the pair a scheduler round at a time) |
| `tools/online_pair_run.c` | Plays Practice on the pair, takes a snapshot, plays on, restores and replays, and compares every replayed frame |

```sh
python3 build.py online-pair --rom PATH/bn6g.gba --state PATH/boot-3.state [--at 1150] [--horizon 8]
```

It builds `build/host/online-pair` in the build image (`make online-pair`,
which CI builds with `-Werror`) and runs it with the ROM and state mounted
read-only; the per-frame CSV goes to `.build/online-pair/`. A game's
`.build/data/boot-3.state` is the state it needs, not a run's checkpoint,
whose map would need rebuilding. On 18 Oct 2026 it printed 2,822 frames,
18,855 serial transfers, a snapshot of 863,016 bytes at frame 1,150 whose
8 replayed frames all matched (64 at frame 2,527 too), and side 0's win.
