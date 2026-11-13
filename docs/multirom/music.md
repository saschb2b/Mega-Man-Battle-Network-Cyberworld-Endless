---
type: Engine Mechanism
title: "BN5's music in a run"
description: "Where a run plays BN5's songs through BN6's music player (`src/audio/xsong.c`): Before the Boss 0x1C at a guardian's prelude on BN5's areas, its Victory 0x19 and Fanfare 0x1F at a battle won there, Area Liberated 0x18 and One-Turn Win 0x24 on dark layers."
tags: [multirom, bn5]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-13T22:09:57Z }
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
| 0x24, One-Turn Win | a liberation's battle won in its first turn, in place of the win's usual song (0x08007A82) | a liberation's battle won in its first turn, BN6's own win answered by a hook on its song (`BN6_WIN_SONG`, `win_song.c`, armed by `liberation_cues.c`) |
| 0x19, Victory, and 0x1F, Fanfare | a battle won (`BN5_BATTLE_WIN_SONG`, 0x08007530, and a liberation's, 0x08007A82): 0x19 for its battle kinds 4 and 5 or where the options' bit 2 is set, else 0x1F. BN6's win picks between its own 0x19 and 0x1F the same way (kind 8 too), so the numbers name the same moments in both games | a battle won on a layer drawn in BN5's areas or a dark layer's mission maps: BN6's win's pick kept, its number answered by BN5's song of that number (`win_song.c`, the same hook; their copies at the fixed slots 0x41 and 0x42, beside BN5's other sounds, `XSOUNDS`). BN4's areas keep BN6's (their win not traced) |

**Why these** (the game-design skill's lens): music is a cue of where
the player is and what is at stake, never noise. A dark layer's
dialectic is clearing fast against clearing safely, the par its measure;
the win's own songs keep a BN5 area BN5's to the end of each fight, as
its battle theme already does (variety a veteran hears: the core loop's
lack 5), and the one-turn jingle answers mastery at the moment it happens, beside the
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
(`FREE_SONGS`, 640 KB since 13 November 2026, 472,944 bytes of it taken with every theme and sound of BN5's and BN4's, [Free space](../engine/emulation/free-space.md), The measured pass; some 231 KB with BN5's alone when this was written),
0x24's 796; 0x1C was the board's already. The core's ROM copy holds
them, so the New 3DS's heap is unchanged (`tools/heap3ds.py`). 0x19 and 0x1F
take 2564 bytes more (1816 and 748), copied when the first layer of BN5's areas is
built.

**Verify:** `CYBERWORLD_EMU_DEBUG=1` prints each copy ("xsong: song 0x18
... in slot 0x2c", "0x24 ... in slot 0x2d") and the music player's song
each second ("music t... song"): on `--net-biome x0 --seed 2 --run-depth
3` with the autopilot, the guardian's prelude turns it to `089d5168`, the
slot 0x2B's copy of 0x1C, where on `--net-biome 0` it is BN6's own
`081a4484`; on a dark layer (`--run-depth 5 --dev dark,god,onehit --seed
2`), "liberation: a first turn's win plays BN5's song 0x24" before each
battle, a first-turn win's song `089d59f0` (stopping as it ends) and a
second turn's BN5's Victory, and "BN5's song 0x18" as the board is won.
The win's songs: "win song: a won battle plays ... 0x19 as BN6's 0x41,
its 0x1F as 0x42" as a layer of BN5's areas is built, and on
`--net-biome x0 --seed 2` with the autopilot (both ROMs beside BN6's) the
first battle's win turns the music player's song to `089f1564`, the slot
0x41's copy of 0x19 (its header the copy's last 0x20 bytes: "song 0x19
... 333188 bytes of songs so far" from `FREE_SONGS`), where BN6's own is
`081a315c`. `--render-song x0/0x19:6:PATH` and `x0/0x1F:6:PATH` render
them (0x19 plays on as the results' loop, 0x1F ends in two seconds).
