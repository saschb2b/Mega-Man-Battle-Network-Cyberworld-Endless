---
type: Feature
title: "3. Going back, and the Net's clock"
description: "Going back (issue #95): two older portals in Lan's HP lead to one `LAYER_BACK` layer of an area won, and each trip moves the Net's clock (`run.clock`), a tenth more HP for each later guardian."
tags: [world, home]
generated: { by: claude-code/unrecorded, at: 2026-10-12T21:51:29Z }
---

# 3. Going back, and the Net's clock

An older portal goes back to an area the run has won: a layer of it at
its act's band, its battles, dealers and data of its tier. Each trip back
moves the Net's clock a notch: the layers ahead are made a notch harder
(the numbers come with the story, through the game-design skill and
`build.py pacing`), and L and the second screen say how far it has run.
The clock is the price that keeps going back a choice: grinding an old
area until the next act is trivial would be the dominant strategy if it
were free.

Reasoned with the game-design skill for issue #95. The dialectic is the
run's own, prepare or press on: an older portal is the preparing, the
next act's portals the pressing on, and the clock the price that keeps
the choice honest. It feeds the progression loop, the act: a trip's
chips, zenny and BugFrags go into the next guardian's fight, and the
price lands on that same fight, so the trade is read where it is paid.
The patterns: the waypoint hub (Diablo's waypoints) and its pitfall, the
farm; the time-difficulty clock (Risk of Rain), counted in trips rather
than in seconds, as nothing else in a run is timed; the bonus with a
drawback; and skipping as a choice of its own (a trip is worth taking
when the folder lacks something, not as a habit).

As built (issue #95):

- **The portals.** The two link squares the ways leave free (portals 3
  and 4: the link on the left, the link down front) go back to the last
  two acts the run has won, newest first (`pacing_older_acts`: across a
  new cycle into the last one's acts, never the Nest). Each opens once a
  visit: taken, it stays dark until a way is taken. Two trips a visit at
  most; a short net can take five (one after act 1, two after acts 2 and
  3).
- **A trip.** One layer of that area, of a kind of its own
  (`LAYER_BACK`), at the depth of its act's middle layer: its band, its
  battles, its Mystery Data and its stock of that tier, the Net Dealer and
  a Recovery Mr.Prog as an act's middle layer has them, the NaviCust
  vendor's chance, traders and Servers as rolled, its set pieces; no
  guardian, duel, gate, dark warp, ScrtData, HPMemory or RegUp (what the
  run's own layers hold stays there, `run.home_depth` the layer it comes
  home to). Its layout is new each trip (the clock is in its seed), dressed
  as its act was. Its card says "A trip back" and the clock, MegaMan "We're
  back in Seaside Area,Lan!", L "The exit pad takes us home." Its exit pad
  leads to Lan's HP: the run's own layer is made again, MegaMan says "Home
  again,Lan!", and the run is saved there.
- **What it pays.** What the area's layer holds: its battles' chips and
  zenny (MegaMan is stronger than when he left it, so they go quickly), its
  Net Dealer's chips (his pick answers the area's viruses: its guardian is
  deleted), its Mystery Data. The run's power, HPMemory and the guardians'
  data, stays where the run goes on: a trip prepares, it does not level.
- **The price.** Each trip moves the Net's clock a notch (`run.clock`),
  and every guardian after it has a tenth more HP a notch
  (`RUN_CLOCK_PERCENT`, `pacing_clock_hp`) as they spawn (the hook that
  caps the duel's, `emu_battle_hp`), BN5's on their HP held to the act's
  band. The pacing report's last section shows the effect: act 1's
  median guardian (500 HP) stands at act 2's band floor after two notches
  (600), act 2's (700) at act 3's after two (840). Two trips cost about an
  act of a guardian's HP. Random battles are left as they are: a version
  up brings BN6's better drops, which would pay the price back, and the
  guardian is the fight a trip prepares for.
- **Where it is said.** Beside an older portal MegaMan names the area (he
  has been there) and the price, all of it the first time ("But the Net's
  clock keeps running...|Each trip back,the guardians ahead get
  tougher!"), then a line; L in Lan's HP says in words how far it has run;
  on a guardian's layer L adds "The Net kept copying while we went
  back..."; the second screen's home panel shows the Net's clock ("2
  notches: guardians' HP +20%"), and Dad's report in the PET's Comm the
  trips and the percent.
- **What must not happen.** Grinding an old area until the next act is
  trivial (each older portal once a visit; the run's HP and the guardians'
  data stay ahead), a death spiral (a trip is safe, and its price a tenth
  of a guardian's HP), a price kept hidden (it is said before the first
  trip, on the card and on the second screen).
- **The fiction.** The Endless Net copies every battle MegaMan fights, its
  guardians too ([The player's guide](../../GUIDE.md#a-run), A run): while he goes back, whatever is
  at the bottom keeps copying.

For tests, `--dev clock=N` starts a run with the clock at N notches, and
`CYBERWORLD_AUTOPILOT_BACK=N` has the autopilot take N trips back from
Lan's HP before it takes a way ([Dev tools](../../devtools/overview.md)).
