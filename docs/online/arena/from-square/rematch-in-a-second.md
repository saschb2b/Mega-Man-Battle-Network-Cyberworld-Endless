---
type: Feature
title: "A rematch in a second"
description: "A rematch in about a second (issue #167, protocol v13): BN6's random words seeded at a battle's start from the bout's battle ID, and the pair kept for the next bout (`arena_boot_keep`), with proofs."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-21T21:51:43Z }
---

# A rematch in a second

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
two words are `BN6_RNG_PRIMARY` and `BN6_RNG_SECONDARY` ([BN5's battles, records and rewards](../../../rom/bn5-battles.md),
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
([Dev tools](../../../devtools/overview.md)).

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
