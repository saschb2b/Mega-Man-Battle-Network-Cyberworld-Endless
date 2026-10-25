---
type: Research
title: "The par's prize"
description: "How a BN5 Liberation Mission pays at its end (`BN5_LIB_REWARD_PICK`): the Darkloid's SP chip two under par or better, his plain chip within par, 1000 zenny a mission (5000 at most) over par."
tags: [game, liberation, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-24T23:57:46Z }
---

# The par's prize (ROM, play)

The Darkloid deleted, the mission's end (the state's routine 0x18) shows
SUCCESS!, then its step 2 picks the rewards' row whose phases hold the
phase taken (`BN5_LIB_REWARD_PICK`) and opens the end's message for its
tier (`BN5_LIB_END_TEXT`, [BN5's Liberation Missions](../../../rom/liberation.md)'s BN5's Liberation Missions'
rewards). The team's leader speaks (Colonel in Team Colonel): the target
and the actual phases, then the Officials' reward, then the chip or the
zenny given by the message's own command. Played in romlab on mission 1
(par 7) at 5, 7 and 8 phases: BlizMan SP B ("Good job,everyone!"), BlizMan
B ("Good Job."), 1000 Zennys ("Too bad you couldn't make it."; the zenny
`0x0200299C` 0 before, 1000 after).

| Mission | Two under par or better | Within par | Over par |
| --- | --- | --- | --- |
| 1 | BlizMan SP B | BlizMan B | 1000 z |
| 2 | ShadeMn SP S | ShadeMan S | 2000 z |
| 3 | CloudMn SP C | CloudMan C | 3000 z |
| 4 | FullCust * | AntiNavi M | 4000 z |
| 5 | CosmoMn SP C | CosmoMan C | 5000 z |
| 6 | Z Saver Z | Colonel C | 5000 z |
| 7 | BlakWing W | ShadeMn DS S | 5000 z |
| 8 | Muramasa M | CloudMn DS C | 5000 z |
| 9 | Anubis A | CosmoMn DS C | 5000 z |

The two chips are a better one and a lesser one: the Darkloid's SP chip
and his plain one, from mission 7 a Mega chip and his DS chip. The zenny
is 1000 a mission, 5000 at most. No row holds a phase: 100 zenny (the
lookup's default; every mission's last row runs to 99).
