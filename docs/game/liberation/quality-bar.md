---
type: Design
title: "The quality bar"
description: "The Liberation quality bar's eight rules (`liberation_board.c`, `liberation_plan.c`): at most 55% dead phases, a bare board still a puzzle, one new rule per act, honest telegraphs, teammate niches, costed routes, graduated rewards, measured fun."
tags: [game, liberation, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-25T01:28:59Z }
---

# The quality bar

The owner's question (23 October 2026): what makes a liberation worthy of
the run, not just more of it. Reasoned with the game-design skill: the
experience stays the decision's ("I see the whole dark area and plan my way
through it; every phase I spend is the better reward slipping away"), the
dialectic the run's own, and the patterns a cumulative-modifier staircase
(one lesson a rung, introduce, develop, twist, test), an honest telegraph, a
branching map whose branch is variety, and a hand-made unit set laid out by
the generator. Each rule below is held by the generator
(`src/director/liberation_board.c`), its planner
(`src/director/liberation_plan.c`) and the prototype (`tools/liberation.py`):
a board that fails one is rejected and the next seed tried, and the tests
(`test_liberation_board`) and the prototype print the measures over
hundreds of boards.

1. **One dialectic, every phase.** The run's *press on or prepare*, at the
   board's scale: liberate toward a DarkHole, or take Order Points, a heart,
   a rest, a guardian out of the way, a key, a detour for loot or the
   route; the par's prize against HP and Order Points. **Measured:** the
   planner, a fair player, counts each phase's sensible moves, those of a
   different purpose worth four fifths of the best or more. A purpose is
   what the move is for (the DarkHole it heads to, the Darkloid, an item, a
   key, the route, a guardian, a rest), its command (Liberation or
   LongSwrd), and for a panel whether it is safe or bold (four or more dark
   round it: the one-turn win's gamble, BN5's own skill). A phase with one
   is dead. A strike at a DarkHole or the Darkloid beside him is the board's
   beat and counted apart. **Rule:** no more than 55% of the phases on the
   way dead.
2. **The bare board is a puzzle.** The same plays with every reward
   stripped (Order Points, hearts, zenny, chips, BonusPanels) still decide:
   no more than 60% dead. The dark's shape carries it: fields, not
   corridors (a one-wide way's panel dark half as often, the loneliest dark
   thinned first), the guardians' reach, the DarkHoles' order.
3. **One new rule per act** (a staircase, not a slider). Each act asks one
   question more, the earlier ones kept:

   | Act | The new rule | Its question |
   | --- | --- | --- |
   | 2 | The basics (BN5's mission 1): dark panels, DarkHoles and their BigBrutes, the Darkloid, Order Points and hearts, BonusPanels | Which way through the dark, and when is an Order Point worth a LongSwrd? |
   | 3 | Barriers and keys, met safe: the sealed region (rule 6) | Is the route worth the phases? |
   | 4 | Guardians' reach: a roaming TinHawk, and a BarrierPanel across the way with a way round | Round his reach, through him, or fetch the key? |
   | 5 | The board fights back: trap panels on the way, each with a way round; Bladia darkening clean panels | HP or phases? |
   | 6 | The maze: a second barrier across the way, its key past the first (where the floor gives one) | In what order? |

   A run meets dark layers in acts 2 to 4 (`run_dark_area`); acts 5 and 6
   are `--dev dark`'s and the separate mode's to come. The size grows with
   the questions: a lone MegaMan's boards hold 14 dark panels beyond the
   DarkHoles' rings at act 2 and 26 at act 6, two DarkHoles to three, and
   the par from 12 to 21 (BN5's 7 to 16 for teams of two to five).
4. **Honest telegraphs.** Everything that can hurt or cost shows before the
   move. A trap is a panel of its own look (BN5 hides its traps in item
   panels; ours show BN5's paled dark), the par is said in the briefing, and
   the guardians' reach and next step are a function of the board
   (`lib_threat`: the HP the units would take off a Navi standing on a
   panel in the next Darkloids' phase, their steps as the phase takes
   them), which the planner reads as a player reads the board. No unit
   reaches the start. Drawing that reach (BN5 has no such overlay; the L
   view is the place) is lib-feel's to do.
5. **Every teammate has a niche.** Each command's shape has a board moment
   where it is the best move: the command rule (The generator) for the
   teammates in the prototype, NumberCheck's new niche the traps it
   disarms. In the engine, MegaMan alone, LongSwrd is taken half a time a
   play or more and is the best move in at most 40% of the phases: never
   useless, never the only answer.
6. **Routes that matter.** From act 3 every board holds a sealed region:
   five to sixteen panels cut off by a BarrierPanel of their own, nothing
   of the board's in them, in sight from the walk; its key in a dark panel
   two or more off the way; the route at its far end. Never required (the
   tests open every other barrier and reach every target without it).
   Its cost is measured: a player going for it takes 2 to 10 phases more
   than the par's player, else the board is rejected (cheaper is a gift,
   dearer a wall). Its reward is variety, not power: the act's next route
   ([Special routes](special-routes.md)).
7. **Rewards feed the run, failure is graduated.** The par's tiers are
   BN5's (the owner's decision 6): a Server's prize two under par, one
   tier lower at par, zenny over it (The par's prize, as built); item
   panels add a chip of the layer's band (one at act 2, two later) and the
   act's zenny (BN5's amounts, 300 at act 2 to 700 at act 6), off the way:
   a detour's worth. Over par is a lesser prize, never a lost run. The dark
   way stays a costed choice at the fork: the second layer's Net Dealer and
   heal given up.
8. **Fun measured.** The planner plays every board 12 times for the par
   and the measures, 6 times stripped, 4 times for the route; the tests and
   the prototype print, act by act, dead phases, sensible moves a phase,
   each command's share of the best moves and LongSwrd's uses, the par's
   middle half, the DarkHoles' orders, the route's cost and what was
   rejected for what ([The quality bar's measures](prototype.md#the-quality-bars-measures-the-prototype)).
