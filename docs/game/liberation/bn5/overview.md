---
type: Research
title: "BN5's missions, dissected"
description: "BN5's nine Liberation Missions dissected from its ROM, play in romlab and guides: where a mission lives (sub-mode 0x18, `BN5_LIB_STATE`), each mission's map, Darkloid, par, team, panels and rewards, and how they rise."
tags: [game, liberation, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-23T13:56:43Z }
---

# BN5's missions, dissected

How each fact is known: **ROM** (a record or routine read, named in
`src/emu/bn5_liberation.h` and [BN5's Liberation Missions](../../../rom/liberation.md)'s BN5's Liberation
Missions), **play** (seen in romlab on the plain ROM), **text** (BN5's own
tutorial, read in its ROM, not kept), **guides** (the players' guides,
assumed until checked: Casebier's GameFAQs guides to both versions, played
on the Japanese release in 2005; MMKB's Liberation Mission page; Wikibooks'
Liberation Guide and its walkthroughs).

## Where they live

A mission runs on the map game state as a sub-mode of its own: EnterMap
(`BN5_ENTER_MAP_BOARD`) sets sub-mode `0x18` instead of the map's 4 where a
mission is under way and its map is the map entered (`BN5_LIB_ON_MAP`). The
liberation state (`BN5_LIB_STATE`) holds the mission number, the Order
Points, the phase and par, and pointers to the mission's record and to its
panels in RAM. **Play:** with the mission number set and BN5's own warp
into the mission's map from a state after its intro, the board starts: its
camera tour, its tutorial, MegaMan and Colonel on the start, the Order
Points gauge at 3, PHASE 1 / TRGT PHASE 7 (pictures in the pull request).
Its 100 panels in RAM are the ROM's list for mission 1, panel for panel,
with each one's look worked out.

## The nine missions (ROM)

Every mission stands on a map of group 0x86, the same net maps BN5's areas
use ([Other games' ROMs](../../../multirom/overview.md)), its panels 32 world units each. The Darkloids and
areas by the guides, the rest by the records:

| # | Map | Darkloid | Par | Team (Team Colonel) | Panels | Dark | Holes | Guardians | Items | Traps | Bonus | Barriers | Rewards: best / par / over |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | 86:00 ACDC 3 | BlizzardMan | 7 | MegaMan, Colonel | 100 | 33% | 2 | 2 | 7 | 0 | 2 | 0 | chip at 5 or fewer / chip at 6-7 / 1000 z |
| 2 | 86:01 Oran 3 | ShadeMan | 9 | + KnightMan | 175 | 43% | 3 | 4 | 8 | 0 | 3 | 0 | 7 / 8-9 / 2000 z |
| 3 | 86:02 SciLab | CloudMan | 9 | + ShadowMan | 220 | 48% | 4 | 6 | 8 | 0 | 2 | 7 | 7 / 8-9 / 3000 z |
| 4 | 86:03 End 2 | Dark MegaMan | 10 | Colonel, KnightMan, ShadowMan, TomahawkMan | 220 | 61% | 5 | 7 | 9 | 0 | 3 | 4 | 8 / 9-10 / 4000 z |
| 5 | 86:04 End 5 | CosmoMan | 9 | MegaMan, Colonel, KnightMan, TomahawkMan, NumberMan | 237 | 51% | 5 | 6 | 21 | 14 | 3 | 13 | 7 / 8-9 / 5000 z |
| 6 | 86:05 Undernet | Dark Colonel | 10 | MegaMan, ShadowMan, TomahawkMan, NumberMan, ToadMan | 248 | 60% | 5 | 7 | 18 | 8 | 3 | 6 | 8 / 9-10 / 5000 z |
| 7 | 86:06 Nebula 1 | ShadeMan | 12 | as 5 | 217 | 59% | 5 | 6 | 18 | 8 | 4 | 14 | 10 / 11-12 / 5000 z |
| 8 | 86:07 Nebula 3 | CloudMan | 14 | as 6 | 223 | 64% | 5 | 7 | 29 | 12 | 3 | 12 | 12 / 13-14 / 5000 z |
| 9 | 86:08 Nebula 5 | CosmoMan (and BlizzardMan) | 16 | MegaMan, Colonel, KnightMan, ShadowMan, ToadMan | 222 | 50% | 5 | 7 | 12 | 0 | 2 | 59 | 14 / 15-16 / 5000 z |

What rises with the missions: the board (100, then 175-250 panels), the
DarkHoles (2 to 5), the team (2 to 5), the guardians and their kinds, the
items and, from mission 5, traps; barriers from mission 3, a maze of them in
9. The par rises from 7 to 16 but not with the board: missions 5 and 6 are
larger than 2 and 3 at the same par, with more Navis. The rewards are one
table a mission (`BN5_LIB_PTR_REWARDS`): a chip for two phases under par or
better, another for within par, zenny after (**ROM**; the guides agree).
Every mission starts with 3 Order Points (the record's +0xB). The record's
+9 (13-22) is read into the state's +0xA; what it does is not traced.

## Contents

- [The par's prize](par-prize.md)
- [The board](board.md)
- [A phase, a panel liberated, Order Points, items](phase.md)
- [The Navis](navis.md)
- [The Darkloids' phase in BN5](darkloids-phase.md)
- [How the board looks and moves](look-and-motion.md)
- [BonusPanels as BN5's](bonus-panels.md)
- [Not found, and assumed](not-found.md)
