---
type: Feature
title: "A Program Advance and his hand's size"
description: "A partner's Program Advance: his client finds the picks BN6's OK joined on his screen copy and sends `TEAM_PA` before them, and the pair joins them into his hand, proved with `teampa=1`."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-22T14:54:41Z }
---

# A Program Advance and his hand's size

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
