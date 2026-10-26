---
type: Design
title: "The opening plan"
description: "The opening plan: five levers (green Mystery Data, their chips, good chips, zenny, the folder-code lean) rising on a logarithm from act 1's floor to the flat plan's, with its reasoning, before-and-after numbers and costs."
tags: [game, economy]
generated: { by: claude-code/claude-opus-5-5, at: 2026-10-26T20:30:00Z }
---

# The opening plan

As proposed on 20 October 2026 and built on the 21st.

## Through the game-design skill

- **The dialectic: the folder you have against the folder you want.** It
  is BN's own, restated at every scale: in a battle, the hand you drew
  against the code you need; on a layer, the far detour's data against
  pressing on; in an act, the dealer's answer against his HPMemory; at
  Trade Central, a spare you hold against the code someone else holds.
  The flat early economy collapsed it: the finds came in the folder's
  codes, and the zenny bought both.
- **The loop layers.** Moment to moment, the busting level picks the
  reward (BN6's); the layer loop, the walk for Mystery Data; the act loop,
  the act card, the dealer and the guardian; the meta loop, the Library
  and its vault; and the social loop, Trade Central and its board. The
  plan thins the layer loop's output early, so the act loop's choices
  bite and the social loop has something to answer.
- **The patterns.** *Opening curve* (the sawtooth's rising floor, Dead
  Cells' tiers by biome): scarce first, open deeper. *Honest telegraph*
  (the skill's): a blue data at a detour's end stays the layer's prize,
  its odds whole; the plain finds thin. *The chase is the game* (Path of
  Exile): the right code reachable, rarely handed over. *Heterogeneity
  for trade* (version exclusives): with no lean early, each run's codes
  are its viruses' and its areas', its own. *Opportunity cost felt*: act
  1's zenny buys the answer or the HPMemory at the dealer, not both.
  *A floor against droughts* (pity): the start gift, the far detour's
  rich data and the dealer's answer stay certain.
- **What it does not touch** (the owner's standing decisions): BN6's
  battle drops and busting table (skill pays, as BN6's; its share of act
  1's chips rises from 28% to 34%); the guardians' HPMemory and the
  heals (growth and difficulty pacing, [Progression](../progression/overview.md)); the buster
  (NaviCust-only); DarkChips; the meta (variety, not power); prices.

## The curves

Each lever's share of the flat plan's at act `a` of a net whose last act before
its Nest is `A` (the endless net's sixth, the short net's third), `x =
a / A` (0 for the first act, 1 for the last, the Nest and later cycles):

```
f(x) = F + (1 - F) * ln(1 + b * x) / ln(1 + b)
```

`F` the first act's share, `b = 2` the bend (how soon it opens; near 0
a straight line). `f` is 1 for the flat plan everywhere, and never above 1
for the opening one (tested): no act hands out more than it did.

| Lever | Where | Flat | F | Act 1 | 2 | 3 | 4 | 5 | 6 | Short net 1 / 2 / 3 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Green Mystery Data | `place_data` (net_gen.c): their count | 2-4 a layer, +1 deeper | 0.50 | 0.50 | 0.65 | 0.77 | 0.86 | 0.93 | 1 | 0.50 / 0.82 / 1 |
| A green data's chip | `loot_mystery_roll`: kept by the share, else the data's zenny; a blue one keeps its odds | 50% of green data | 0.60 | 0.60 | 0.72 | 0.81 | 0.89 | 0.95 | 1 | 0.60 / 0.85 / 1 |
| Good chips | `roll_chip`: tiers 2-4's weight in the plain finds (green and blue data, a dealer's shelf, a request's chip); `place_data`: the extra rich data's 30% | as rolled | 0.25 | 0.25 | 0.48 | 0.65 | 0.79 | 0.90 | 1 | 0.25 / 0.72 / 1 |
| Zenny | `loot_mystery_roll`: a data's zenny; `loot_guardian_zenny`: a guardian's chip entry | 100-500 green, 800+ blue, 600+ guardian | 0.50 | 0.50 | 0.65 | 0.77 | 0.86 | 0.93 | 1 | 0.50 / 0.82 / 1 |
| Lean to the folder's codes | `loot_mystery_roll`'s second roll; the battle rows' second entries | half the data, every row | 0 | 0 | 0.31 | 0.54 | 0.72 | 0.87 | 1 | 0 / 0.63 / 1 |

Kept whole: the far detour's rich data, purple data, a Server's prize, a
vault's three, the Guardian Data, the gift, the dealer's answer (its two
copies and its price cap, in the folder's codes: convenience paid for).

## Before and after

300 runs each, the endless net (the report's first table):

| Act | Chips | Good | Mystery Data | Zenny | Codes held | Fit | Earned / wanted so far |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | 12.2 → **9.5** | 26% → 23% (3.2 → **2.2** chips) | 14.8 → **11.6** | 3,550 → **2,630** | 5 → **3** | 63% → **35%** | 3,480 → 2,600 / 1,600 |
| 2 | 11.4 → 9.4 | 31% → 29% | 15.1 → 12.6 | 4,350 → 3,610 | 8 → 5 | 51% → 33% | 7,850 → 6,320 / 3,800 |
| 3 | 14.7 → 12.5 | 38% → 34% | 20.3 → 17.9 | 6,880 → 5,880 | 11 → 8 | 56% → 40% | 14,920 → 11,990 / 7,400 |
| 4 | 13.9 → 12.9 | 46% → 44% | 19.5 → 18.2 | 7,900 → 7,100 | 15 → 11 | 56% → 48% | 22,400 → 19,100 / 11,400 |
| 5 | 17.8 → 17.1 | 57% → 57% | 24.7 → 24.7 | 10,890 → 10,740 | 20 → 15 | 56% → 52% | 33,210 → 30,050 / 16,800 |
| 6 | 15.8 → 15.8 | 70% → 70% | 22.6 → 22.6 | 10,760 → 10,700 | 24 → 19 | 55% → 55% | 44,190 → 40,500 / 22,800 |

At the act's dealer (layer 2), a run had 1,460 zenny against his 1,600 and
has 930: the answer's two copies or the HPMemory. Three chips of one code
come by layer 3 rather than 2, five by layer 6 rather than 3; a layer
keeps its far detour's rich data (1.06 rich a layer on act 1, from
1.26). The short net (a new run's) follows its own curve, open by its
third act: act 1 as above (9.6 chips, 2,630 zenny, codes 3), act 2 10.0
chips and 4,000 zenny, act 3 the flat plan's.

## What it costs

- **A starved first act.** Act 1 keeps about three chips a layer (one
  from a battle), the gift's choice, a dealer whose answer costs 400 a
  copy, and 930 zenny at his counter. The answer was built to beat the
  first guardian beside the starting folder ([The structure](../progression/structure.md#growth),
  Growth); the plan leaves it, the gift and the heals as they are.
  Its risk is the run that finds little and then has to choose at the
  dealer: the act card names the guardian before, so the choice is
  informed. Watch the first guardian's win rate in the statistics before
  and after.
- **Fewer and plainer Mystery Data** make a layer's corners emptier. The
  plan takes half the green data out, and four in ten of the chips
  the rest held become zenny, half of the flat plan's (50-250 on act 1); a
  playtester's battle data of zenny "read as no find at all" (loot.c), so
  the count falls more than the content. Every blue data stays, its odds whole but for
  the good chips' weight in the plain blue ones.
- **Codes left to chance early** make the first folders patchier: three
  of one code by the guardian rather than five. That is the point for
  trading, and BN6's own trader still answers it (three chips for one,
  in a code the pack lacks, were the project's traders BN6's).
- **Fewer spares to trade early.** Trade Central swaps a spare (a copy
  past the run's three folders) at home after an act ([Online: the Net Square](../../online/overview.md)):
  fewer chips mean fewer spares after act 1, each worth more. Most of a
  first act's finds are spares either way (a 30-chip folder takes few of
  them), and the codes they come in now differ from run to run, which is
  what makes one worth asking for.
- **Switching it on** bumped `LAYER_MAKE` to 96 (a layer made by the
  flat plan differs: a run saved before continues its layer afresh), and
  the battle rows' lean, which runs in a hook that draws no random number
  (`encounter.c`, `lean_row`), takes the curve by a hash of the layer, the
  enemy and the entry (`econ_keep_by`). Saves need no change (`Run` is
  untouched; the counts are a file of their own, `run.tally`).

## The code leans, not the chip

Changed 26 October 2026 ([the core loop](../core-loop.md), Phase 2).
The lean's share stays as the curves above set it (half the data, by the
act); what it does changed. A Mystery Data's chip used to be rolled
again, eight times at most, for one that comes in the folder's codes, a
gem's three twelve times. Now the chip stays as it rolled and only its
code leans, to the first of the folder's codes it comes in (a gem's, else
its \*): the way BN6 hands out codes, one chip from a virus in the code
the busting level earns.

Through the game-design skill: the dialectic is variety against
concentration. Smart loot (Diablo 3) leaned which chip came, and with the
starting folder's S and A it kept the chips that come in those codes and
threw away the rest, Megas first (most come in one code). Borderlands'
bet, the roll as the content, is the other pole. BN's own answer sits
between: the chip is the net's roll, the code is the folder's. The
folder still converges by code, and a folder edit still steers it (the
codes are read as each layer is made); the net varies by chip again.
Act 1's dealer says it once, beside his own: "Deeper in,Mystery Data
too!" (none lean on act 1).

300 runs, the endless net, the opening plan; kinds as [Measuring it](measuring.md)
defines them:

| Act | Kinds | Fit | Codes held | Good | Megas |
| --- | --- | --- | --- | --- | --- |
| 1 | 158 → 158 | 34% → 35% | 3 → 3 | 22.6% → 22.5% | 0.92 → 0.92 |
| 2 | 156 → 157 | 32% → 29% | 5 → 5 | 28.9% → 28.2% | 1.00 → 1.00 |
| 3 | 188 → 199 | 41% → 33% | 8 → 8 | 34.1% → 34.2% | 1.20 → 1.27 |
| 4 | 199 → 221 | 48% → 37% | 10 → 10 | 43.6% → 41.5% | 1.40 → 1.44 |
| 5 | 190 → 224 | 52% → 37% | 15 → 14 | 57.3% → 55.3% | 1.78 → 2.02 |
| 6 | 172 → 219 | 54% → 35% | 19 → 17 | 70.5% → 69.3% | 1.88 → 2.05 |
| Nest | 155 → 200 | 61% → 38% | 20 → 18 | 68.6% → 66.9% | 0.45 → 0.54 |

The codes held moved on purpose, so their bands' centres in `economy.c`
were set again from this measure (the opening plan's 3, 5, 8, 10, 14, 17,
18; the short net's 3, 6, 9, 10; the flat plan's too); nothing else left
its band. The good chips' share fell two points in acts 4 and 5: the
reroll had kept rare standard chips, which come in more codes, over
Megas.

## Not proposed, and why

- *Capping the busting pair by act* (the research's first finding): it
  would cut the early chips most, but it takes BN6's skill reward away
  where it is felt first.
- *A pity counter in the run* (Slay the Spire's offset): the far detour's
  rich data already gives one good chip a layer where the walk earns it,
  without saving a counter (a `Run` change, `RUN_MAGIC`).
- *Rising prices or new sinks:* deeper zenny is plenty and the dealers'
  shelves and the Order Service (twice a dealer's price) absorb it; a
  rising sink (Slay the Spire's removal, Balatro's reroll) is the next
  lever if the statistics show hoards.
- *The traders as BN6's* (a code the pack lacks three times in four):
  the strongest single step for trading, a change to the traders' rules,
  not their numbers; proposed to the owner apart, and built (below). Their
  prizes stay chips new to the Library first, as the layers' traders gave
  them.
