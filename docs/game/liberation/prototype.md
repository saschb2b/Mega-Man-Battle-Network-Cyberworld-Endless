---
type: Measurement
title: "The prototype's measures"
description: "Measures of the ROM-free Liberation prototype (`tools/liberation.py`, `tools/liberation_bn5.py`) beside BN5's nine boards: boards kept and rejected per act, shape and dark bands, par, each command's worth, and the quality bar's numbers."
tags: [game, liberation, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-23T18:51:29Z }
---

# The prototype's measures

`python3 tools/liberation.py --seeds 30 --runs 12` (acts side by side on
all cores, about an hour for the late acts with the command rule's
plays); `python3 tools/liberation_bn5.py` with BN5's ROM for its nine and
the par's yardstick. BN5's own boards, measured the same way, stand
beside them:

| Act | BN5 | Kept of tried | Rejected most for | Panels | Fields | Fill | Dark | Dense | Par (BN5's) | Two under / at / over par | Close calls | Ways to the first holes |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 2 | 1 | 30 of 70 | commands 24 | 111 (100) | 70% (72%) | 28% (22%) | 42% (33%) | 3.6 (3.6) | 9.9 (7) | 8 / 43 / 49% | 41% | 1.3 |
| 3 | 2-3 | 30 of 127 | commands 57, barrier 26 | 194 (175-220) | 78% (74-82%) | 27% (23-28%) | 57% (43-48%) | 3.9 (3.5-3.9) | 14.1 (9) | 14 / 34 / 52% | 43% | 2.2 |
| 4 | 4-5 | 30 of 150 | commands 74, barrier 30 | 222 (220-237) | 94% (97%) | 40% (43-46%) | 66% (51-61%) | 4.9 (5.5-6.0) | 13.3 (9-10) | 14 / 38 / 48% | 46% | 2.8 |
| 5 | 6-7 | 24 of 360 | commands 105, barrier 103, situation 94 | 232 (217-248) | 94% (94-97%) | 40% (43-48%) | 68% (59-60%) | 5.0 (4.8-5.4) | 13.0 (10-12) | 19 / 35 / 45% | 43% | 3.4 |
| 6 | 8-9 | 30 of 297 | commands 106, barrier 88 | 224 (222-223) | 94% (88-99%) | 40% (46-48%) | 66% (50-64%) | 4.9 (4.1-5.1) | 12.6 (14-16) | 11 / 44 / 45% | 45% | 2.8 |

(Fields: panels in a 2x2 block of floor. Fill: the floor's share of its
box. Dense: dark panels around a dark panel. Close calls: the planner's
decisions whose second-best choice was within 10% of its best, a measure
of real choices. Ways: distinct orders of the first two DarkHoles over
the plays. A situation: a command's situation could not be placed.)

Each command's worth on the boards kept, the phases its Navi saves over
himself without it, the same plays paired: TomahawkSwing +1.1 to +1.7,
ScrenDiv +0.5 to +1.7, LifeMelody +0.2 to +0.9, NumberCheck +0.3 to
+1.0 (and 1.1 to 2.6 items taken a play with no battle), SneakAttack +0.3
to +0.5 (100 to 240 HP struck off a play), KnightMan's StoneBody +0.2 to
+0.8, MegaMan's LongSwrd +0.15 to +0.9. What it says:

- **The boards read as BN5's.** Their shape and their dark keep to BN5's
  bands (the pictures in the pull request), and route choice opens up
  from act 3: two to three and a half ways to take the first DarkHoles.
- **Every command earns its Order Point where it is kept**, and the rule
  costs boards: it keeps between one in two and one in fifteen of those
  tried, most often for a utility command the planner had no use for. On
  every board tried (before the rule) TomahawkSwing alone pays everywhere
  (+0.6 to +1.2); ScrenDiv pays early; SneakAttack, NumberCheck and
  LifeMelody pay where their situation is, little elsewhere. The
  situations are what make them, as BN5's hand-made missions do.
- **The par sits above BN5's in acts 2-5 and below in act 6.** It is the
  planner's median times BN5's own ratio at that stage, so it follows
  the generated boards, which are a little larger early and lack BN5's
  late maze of barriers (59 on mission 9 against about 5 here).
- **Left to tune:** the late acts' rejections (a barrier wall that cuts
  a field of two-wide ways, a cache's clean panel), the late maze of
  barriers, and the cost of the command rule in the engine, where its
  plays run as the layer is made (a precomputed check, or the rule's
  plays on a worker, if it is too slow on the 3DS).

## The quality bar's measures, the prototype

The same staircase, barriers, traps and measures in the prototype, its
teams BN5's (`python3 tools/liberation.py --seeds 20 --runs 8`, an act a
core; the route's player's plays a quarter of the par's):

| Act | Kept of tried | Rejected most for | Dead decisions on the way | Sensible moves a decision | Par (middle half) | The route | Traps |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 2 | 20 of 160 | dead 81, commands 22, situations 36 | 46% | 1.6 | 8.7 (2.3 wide) | none | none |
| 3 | 2 of 240 | dead 106, barrier 94, route 19 | 51% | 1.6 | 11.5 (3.3) | +2.5 | none |
| 4 | 13 of 240 | barrier 147, route 35, commands 22 | 50% | 1.8 | 12.1 (2.7) | +2.4 | none |
| 5 | 2 of 240 | barrier 162, situations 26, holes 16 | 49% | 2.0 | 14.0 (2.1) | +3.7 | 4 |
| 6 | 5 of 240 | barrier 137, route 31, commands 21 | 50% | 1.8 | 13.0 (2.4) | +2.5 | 4 |

A team decides about as often a decision as MegaMan alone (one in two
dead), and its phases hold more decisions, one a Navi. The prototype's
floors, BN5's trees of platforms, seldom cut off a sealed region by a wall
of its own (three to thirty panels with nothing in them): from act 3 most
of its tries fail there, where the engine's layers, looped, keep 38 to 40
of 40. Its numbers stand for the rules' direction; the engine's for the
boards a run meets.
