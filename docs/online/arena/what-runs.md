---
type: Engine Mechanism
title: "What runs"
description: "The Arena's parts file by file (`arena_pair.c`, `arena_boot.c`, `arena_input.c`, `arena_sync.c`, `arena.c` and more), the lockstep's frame timing and 128-frame cap, the pair hash every 60 frames, and the `ArenaLink` interface."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-21T21:51:43Z }
---

# What runs

| Part | File | What it does |
| --- | --- | --- |
| The pair | `src/online/arena_pair.c` | Two cores of the player's ROM with BN5's installs, full rules being every NetBattle's ([Full rules](from-square/full-rules.md), below); flash saves in memory, the cable's scheduling as mGBA's own frontend has it (whole run loops, the second core on the first's cycles) and both cores halted at BN6's wait for a frame ([Rollback](rollback/overview.md)); one frame of the pair, a hash of both cores; the picture drawn for the side shown alone (`arena_pair_draw`); on the 3DS one ROM copy for both cores ([The New 3DS](rollback/new-3ds.md)) |
| The installs' record | `src/emu/xinstall_log.c` | Every byte BN5's installs write into the game's own core's ROM copy and every hook they set, in their order, as the core starts: what goes in each Arena core |
| The Souls across | `src/online/arena_souls.c` | A Soul one core's player committed carried into the other core's Souls' block as the pair's frame ends: BN6's link carries the form, not the Soul |
| The way in | `src/online/arena_boot.c` | One core boots as `src/emu/boot.c` does (NEW GAME, the game's warp through its own call, hooked on that core alone); each side's MegaMan is written into a copy of its state; then the pair, on the cable, goes through BN6's menus (PET, Comm, Link Cable, NetBattle, Single Battle, Practice) with the research's scripted keys to the Custom screen; as Practice's last confirm is pressed the bout's seed goes into BN6's random words on both GBAs, and the pair is kept there, in memory, for a rematch to go in from ([A rematch in a second](from-square/rematch-in-a-second.md)). No state is written or committed; it runs in slices behind a "connecting" picture |
| A carried MegaMan | `src/online/arena_loadout.c`, `arena_carry.c` | The `OnlineLoadout` (`online_snapshot.h`) written into BN6's own RAM through `bn6.h`'s names: the first folder, the pack's own ids, the NaviCust's board, HP (full) and base HP, the buster, Regular and TagChips, the Crosses' and Beast Out's flags. Checked by what the pair's cores hold, never by the game's own ROMs, so both games check a MegaMan alike. Refused, with the reason, where the cores cannot carry it: a chip id past the chip registry, a code past `*`, a max HP of 0 or past 9999. Before v9 BN6's rules refused BN5's chips and the Souls too. `arena_carry.c` reads the data folder's checkpoint, its `run.state` loaded into a plain core that never runs |
| Keys | `src/online/arena_input.c` | Both players' keys by frame in a ring; each stream in order, a resent frame left, a gap or a frame past the ring refused; for a frame run on a guess the other's keys not yet come held as his last, and the first frame a key that came otherwise reached |
| The time sync | `src/online/arena_sync.c` | The leading game's tick held now and then, spread out, by the lag of the other's keys against the other's lag of its own (protocol v11's notes; before the first, its own round trip), on a tick nobody sees; the round trips kept in the Square for the delay asked for ([Smoothed delay](smoothed-delay.md), below); the rollback window a frame's cost affords and the latency asked for less what it covers ([Rollback](rollback/overview.md)) |
| The window kept | `src/online/arena_window.c` | What a battle frame of the pair costs on this system, kept in the data folder (`arena.ini`, never in a `.cwsave`), for the delay the next challenge or Yes asks for |
| The snapshot | `src/online/arena_pair.c` | The pair's state for a rollback, both cores and what no core's state holds (`arena_pair_save`, `arena_pair_load`), and where two differ ([Rollback](rollback/overview.md), below) |
| The measurement | `src/dev/rollbench.c` | `--dev rollbench`: what a rollback costs on the machine it runs on, and the window the Arena picks there |
| The rollback tried | `src/online/arena_roll.c` | `--dev arena,arenaroll=W`: whole battles rolled back W frames every W against the local stand-in, the game's state checked each time |
| The link | `src/online/arena_link.h`, `arena_link_local.c` | The interface below; the local stand-in |
| The engine | `src/online/arena.c` | The way in, then the battle: each frame at its tick, on a guess within the rollback window, rolled back where a guess was wrong; the hashes and the end taken up on frames whose keys are all known |
| The scene | `src/online/arena_scene.c`, `arena_text.c` | `--dev arena`'s scene, its words |
| The picture | `src/online/arena_view.c` | My core's picture, the notes over it, the way in's bar and its slices at the frame's pace; both scenes' |
| The wire | `src/online/arena_wire.c` | ARENA and its four messages packed and read as the server's `protocol.py` packs them; a carried MegaMan as bytes |
| The network's link | `src/online/arena_link_net.c` | `ArenaLink` over the transport's fast outbox: keys, hashes, the ready, the peer's end |
| From the Square | `src/online/square_arena.c`, `arena_words.c`, `online_mail.c` | The challenge, its question and answer, the battle with the Square's core held, the word after; MegaMan's lines; the inbox handed out by kind |
| The stands | `src/online/arena_watch.c`, `watch_keys.c`, `square_watch.c`, `arena_report.c`, `stands_report.c`, `stands_wire.c`, `online_stands.c`, `stands_words.c`, `stands_text.c` | A battle watched: its pair built and run from both players' keys; the referee's offer and the watching screen; the report the cable's first sends and the channel's battles as the server tells them ([The stands](stands.md), below) |

## Lockstep

A key sampled at tick *t* (a 60th of a second) is frame *t + D*'s, *D*
the link's delay; the first *D* frames are released keys on both sides,
on every client. Frame *N* of the pair is due at tick *N* and needs both
players' keys for *N* and the frame after it: a linked core crosses into
its next frame a little before its partner (the two run within a few
thousand cycles of each other, [Rollback](rollback/overview.md); the research's
harness, an instruction at a time in turn, let one run two frames
ahead, and a frame needed the three after it), and each core takes the
keys of its own frame clock. With a rollback window *W* the frame runs at
its tick while at most *W* frames have run on a guess of the other
player's keys ([Rollback](rollback/overview.md)); with none it runs once all its
keys are known, so a delay of 1 frame plus the link's one-way latency
never waits. A frame that may not run waits, once, as many ticks as it
falls short, and stays that far behind. A frame that waits moves the game's
clock (issue #128): the tick after it samples no key and runs that frame,
so a wait shows one frame twice and never skips one; the game that leads
holds a tick now and then until it does not ([Smoothed
delay](smoothed-delay.md), below). A tick catches up two frames at most.
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

## `ArenaLink`

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
