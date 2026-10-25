---
type: Engine Mechanism
title: "BN5's music in a run"
description: "Where a run plays BN5's songs through BN6's music player (`src/audio/xsong.c`): Before the Boss 0x1C at a guardian's prelude on BN5's areas, Area Liberated 0x18 and One-Turn Win 0x24 on dark layers."
tags: [multirom, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-23T19:58:38Z }
---

# BN5's music in a run

Where a run has a moment BN5 has, BN5's song for it plays there, as BN5
plays it, through BN6's own music player: its copy made once by
`src/audio/xsong.c` with the board's songs ([Liberation Missions](../game/liberation/overview.md)), after
every song copied before it, so each keeps its place in a saved state.
Each cue is the moment's own in BN5, read in its code or scripts
([BN5's maps, songs, text and objects](../rom/bn5-world.md), Where BN5 plays its songs):

| Song | Where BN5 plays it | Where a run plays it |
| --- | --- | --- |
| 0x1C, Before the Boss | before a fight: ProtoMan or Colonel showing himself, the scenario comps' last maps, a mission's opening | a guardian's prelude on BN5's areas, as he logs in (`guardian_prelude_song`, set by the layer with its battle themes); a dark layer's opening (PR #207) |
| 0x18, Area Liberated | a mission won: its board's routine 6, the Colonel's tally of target and actual phases | a dark layer won, as the words of its par begin; the board's own song again once the prize is given |
| 0x24, One-Turn Win | a liberation's battle won in its first turn, in place of the win's usual song (0x08007A82) | a liberation's battle won in its first turn, BN6's own win answered by a hook on its song (`BN6_WIN_SONG`, `liberation_cues.c`) |

**Why these** (the game-design skill's lens): music is a cue of where
the player is and what is at stake, never noise. A dark layer's
dialectic is clearing fast against clearing safely, the par its measure;
the one-turn jingle answers mastery at the moment it happens, beside the
1 TURN LIBERATE! banner that already says it (the sound never the only
sign), and the won song closes the board's session where BN5 closes its
mission, a beat apart from every battle's fanfare. The prelude keeps a
BN5 area BN5's through the guardian's whole scene: its theme, its
before-the-boss song, its boss theme, as its battles' music already is
(a super boss's staging keeps BN6's, [Guardians and super bosses](../game/bosses/overview.md)). Weighed and left
out: Nebula 0x02 (no scene of Nebula's own in a run), Hope 0x0E (no
friends arriving), Final Battle 0x17 (BN5's last boss is never fought),
Beach Day 0x06 and Challenge 0x23 (no such places), Sorrow 0x0D (BN6's
own defeat and game over already speak at a run's losses).

**What it costs:** 0x18's copy 1388 bytes of the songs' free space
(`FREE_SONGS`, 768 KB, some 231 KB used with every BN5 song and sound),
0x24's 796; 0x1C was the board's already. The core's ROM copy holds
them, so the New 3DS's heap is unchanged (`tools/heap3ds.py`).

**Verify:** `CYBERWORLD_EMU_DEBUG=1` prints each copy ("xsong: song 0x18
... in slot 0x2c", "0x24 ... in slot 0x2d") and the music player's song
each second ("music t... song"): on `--net-biome x0 --seed 2 --run-depth
3` with the autopilot, the guardian's prelude turns it to `089d5168`, the
slot 0x2B's copy of 0x1C, where on `--net-biome 0` it is BN6's own
`081a4484`; on a dark layer (`--run-depth 5 --dev dark,god,onehit --seed
2`), "liberation: a first turn's win plays BN5's song 0x24" before each
battle, a first-turn win's song `089d59f0` (stopping as it ends) and a
second turn's BN6's Victory `081a315c`, and "BN5's song 0x18" as the board
is won.
