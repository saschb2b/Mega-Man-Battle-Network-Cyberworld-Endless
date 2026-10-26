---
type: Feature
title: "The quality bar, as built"
description: "The quality bar as built for a lone MegaMan: barriers and keys, the sealed region's route, traps and finds (`lib_guard_rule`, `lib_harm`), and `test_liberation_board`'s measures per act: par 12 to 21, dead phases, LongSwrd's niche."
tags: [game, liberation, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-25T01:17:25Z }
---

# The quality bar, as built (23 October 2026)

The staircase, the barriers and their keys, the sealed region and its
route's hook, the traps, the chips' and zenny's finds and the planner's par
in the engine (The quality bar, The generator), MegaMan alone. In play: a
key opens its barrier's panels (their fence's walls switched off as a
liberated panel's are), a trap's harm is taken as it is liberated (never his
last HP, BN6's board harm), a chip or zenny is given by a script of BN6's
own (`ta_lib_find_chip`, `ta_lib_find_zenny`, installed with the layer as
the par's prizes are), and the route found is told and kept for the layer
(`lib_route_found`, the state in words' `route found`). The units' rules
(steps, reach, harm) are the board's (`lib_guard_rule`, `lib_harm`), so the
engine, the planner and the telegraph read the same ones. `LAYER_MAKE` 98.

**The measures** (`tests/test_core.c`'s `test_liberation_board`, 40
layers an act, each board made twice; the planner's 12 plays, 6 stripped
and 4 for the route):

| Act | Kept | Panels | Dark | Par (range) | Middle half | Dead on the way | Sensible moves a phase | Best: Liberation / LongSwrd / Pass / fight | LongSwrd a play | DarkHole orders | The route | Dead, stripped | Rejected most for |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 2 | 40 of 40 | 197 | 20% | 12 (9-19) | 9-14 | 43% | 1.5 | 49 / 27 / 7 / 15% | 3.4 | 1.4 | none | 43% | dead 65, LongSwrd 20 |
| 3 | 38 of 40 | 198 | 25% | 14 (9-21) | 12-18 | 45% | 1.5 | 48 / 27 / 9 / 14% | 3.8 | 1.6 | +4 | 42% | dead 66, barrier 31 |
| 4 | 39 of 40 | 233 | 25% | 17 (12-31) | 14-20 | 46% | 1.6 | 44 / 30 / 14 / 9% | 5.0 | 2.7 | +4 | 51% | dead 116, stripped 49 |
| 5 | 25 of 40 | 195 | 32% | 20 (12-34) | 17-23 | 49% | 1.5 | 44 / 29 / 13 / 12% | 5.8 | 2.9 | +5 | 53% | dead 231, barrier 53 |
| 6 | 21 of 40 | 188 | 36% | 21 (15-33) | 19-25 | 48% | 1.5 | 41 / 29 / 17 / 11% | 6.1 | 1.9 | +5 | 50% | dead 217, unwon 91 |

(Dead on the way: phases whose sensible moves were one, strikes at a
DarkHole or the Darkloid apart. Middle half: the plays' phases from the
quarter to the three quarters. The route: the route's player's phases
over the par's. A layer whose sixteen tries fail is made plain: acts 5 and
6, which a run does not meet yet, keep half their boards.)

What they say:

- **A lone MegaMan decides about one phase in two.** Half his phases offer
  two sensible moves or more (another DarkHole, an item, a rest, LongSwrd
  or not, a bold panel or a safe one); the other half are a corridor's
  next panel or a rest he cannot put off. The rule keeps the boards at 55%
  or under; the dead half is the lever stage 3 pulls, a team's Navis each
  a decision a phase (the prototype's teams: 46% to 54%).
- **The staircase reads in the numbers.** Each act's boards are a little
  larger in what they ask: the par grows 12, 14, 17, 20, 21; the DarkHoles'
  orders open from 1.4 to 2.9 ways as the holes grow from two to three;
  resting matters more (the best move 7% of the phases at act 2, 14-17%
  from act 4) as the reach and the tolls come in.
- **The route is a price, not a gift.** It costs its player four or five
  phases over the par, about a tier of the par's prize; a board where it
  costs fewer than two or more than ten is rejected.
- **LongSwrd has its niche.** It is the best move in 27-30% of the
  phases, taken three to six times a board: neither the only answer nor
  idle.
- **The autopilot** (`--dev dark,god,onehit`, its one-hit battles all won
  in one turn, so better than the planner's player) cleared act 2's board
  in 11 phases against its par 13 and act 3's in 9 against 18 from depth 5,
  and from depth 8 act 3's again and act 4's in 16 against 19, both of its
  keys found and their barriers opened, on to act 5's dark layer.
  Started at depth 11 directly, MegaMan was deleted in a liberation's
  battle on dev's build as on this one: not this change's.

**Left for later:** the board's
make on the 3DS timed (about 55 ms a board on the desktop, the planner's
plays the most of it: a dark layer's make on the New 3DS may take a second
or two); acts 5 and 6 tuned for a lone MegaMan (their BN5 harm wants a
team); the teammates' commands in the engine (stage 3), the command rule
with them.
