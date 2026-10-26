---
type: Design
title: "Special routes"
description: "Special routes: a sealed region's route found on a dark board is kept for the run (`run.route_found`, `run_route_turns`) and opens a third way at the next fork, or the Nest's second guardian."
tags: [game, liberation, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-25T00:18:26Z }
---

# Special routes

A sealed route is the region behind the sealed barrier (The quality bar,
6): its key off the way, its region a detour of 2 to 10 phases, the route
at its far end an item panel the player liberates. Found, it is kept for
the run and turns the act's next fork ([The meta layer](../meta/overview.md), 7) toward a third way;
the owner decided so on 25 October 2026. In the separate mode it would
lead to another last Darkloid.

BN5 has nothing like it: its item kinds are six (a heart, a key, Order
Points, zenny, BugFrags, a chip) and its nine boards hold no other item or
panel ([Sources](../../research/sources.md), verified in the ROM). The route is the player's
request the design began with ("special routes to other bosses"), told in
BN5's own way of telling a find: the panel liberated, MegaMan's line over
it ("A way out of here nobody's used..."), Lan's answer.

**The design** (the game-design skill):

- **The dialectic** is the board's, *prepare or press on*, priced in
  phases: the route costs its player 2 to 10 phases over the par, about a
  tier of the par's prize, and pays in the run's shape, not its power. At
  the fork it is restated as *the known or the found*: the two ways MegaMan
  reads like any other, or the one he and Lan found themselves.
- **The loop layers:** the session (one board) feeds the run (the next
  fork's shape); nothing reaches the meta. A route not found, or found and
  left at its fork, costs nothing but the phases spent.
- **The patterns:** a branching map whose extra branch only a detour
  opens (Slay the Spire's keys, taken off the path at a price, open its
  fourth act), its cost paid up front and its reward variety
  (meta-as-variety-not-power: the way's guardian fits the act's band like
  the others').

**What it opens:**

| The board's act | The fork it turns | The third way |
| --- | --- | --- |
| 3 (the endless net's) | act 4's | another area of the act's tier the run has not taken, nor the other way's, its guardian fit for the act and none of the run's six acts' nor the other way's |
| 4 (the endless net's) | the Nest's, past the deep's acts 5 and 6, which have no fork | the Nest again, a second way: its second guardian (`run_nest_second`) instead of the Cybeast, at the Nest's strongest version and the Cybeast's HP at least (`run_nest_hp`: 2500 on the first Net, 4000 after), a different last boss and never an easier one; the net rebuilds past him as past the Cybeast, and its den is the Cybeast's again |
| 3 (the short net's) | the Nest's | the Nest's second guardian first (on threat 10 the second below him is drawn again, never him) |
| 2, where a board has one | act 3's | in the short net, its dark way to the Undernet where the Secret Area has not opened it; else another area's |

Act 2's boards hold no sealed region (the staircase's rules), so the last
row waits for a board that does; it is built so that no route is ever
wasted: a fork with no room for a third way (the short net's act 3 with
its dark way open) passes the route on to the Nest's.

**How it is told:** at home the fork's words name it where it comes out
("And the link on the right! The route we found in the dark!"; at the
Nest "It comes out at the link up top. Way down,too!"), checked or stepped
on its portal MegaMan knows it ("Lan,look! Our route from the dark!")
before what he reads through it, as for any way (its data, a guardian's
signal, named only where he has battled him), and the act's card says
"By the route found in the dark" in a liberated panel's green, in the
motto's place, as a dark layer's says its own in Nebula's purple; at the
endless Nest by the route MegaMan feels the signal is not the beast's.

**As built** (25 October 2026): `run.route_found`, the act whose board's
route was found (1-based, 0 none), set as MegaMan leaves the dark layer
with the route found (`run_route_keep`) and kept until the fork it turns
is passed (`home_route_passed`), and `run.route_way`, the act that came by
the route, for its words and card; both in `Run`'s padding, saved as
"CWEA" (`RUN_MAGIC`; a "CWE9" save reads them as 0). `run_route_turns`
names the fork, `run_ways` offers the way (`RunWay.route`), the third
area's from the seed as the other way's is (`route_third`), so a CONTINUE
at home offers the same. Its portal is the dark way's link on the right;
the Nest's second way the link up top.

**Checked:** `build.py pacing`'s Special routes over 500 runs, 0 past its
band or clashing (act 4's third way's guardians all in 1000-1300, none
another way's or act's, the same twice; the Nest's second guardians SP at
the endless Nest, fought at the Cybeast's 2500, EX in the short net's
band). Their own SP, 1500 to 2000, would have made the route an easier
last boss than the Cybeast's 2500: a gift, which the report showed and
`run_nest_hp` closes; the autopilot from seed
1's act 3 dark layer (`CYBERWORLD_AUTOPILOT=1 CYBERWORLD_AUTOPILOT_ROUTE=1
--scene emu --run-depth 8 --seed 1 --dev dark,god,onehit`): the sealed
key, the route, the Darkloid, the act's guardian, home with three ways,
the route's taken to CopyBot's Comp; `--dev route` at home before act 4
and before the Nest (seed 7): the fork's words, the portal's, its
question, the way taken, the card; a CONTINUE at home (the same three
ways) and on the route's act (its card).
