---
type: Feature
title: "His loadout in his record"
description: "A partner's loadout in his battle NaviStats (`arena_team_navi_loadout`): his max and base HP and his buster's Attack, Speed and Charge written where BN6 keeps them, seen with `teamhp` and `teambuster`."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-22T13:29:28Z }
---

# His loadout in his record

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
