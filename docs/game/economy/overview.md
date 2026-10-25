---
type: Overview
title: "Economy"
description: "What a run hands out (chips, their codes, zenny, Mystery Data) measured act by act, and issue #154's opening plan in `src/core/economy.c` that makes act 1 scarcer, opens up deeper and is guarded against drift."
tags: [game, economy]
generated: { by: claude-code/unrecorded, at: 2026-10-20T22:52:50Z }
---

# Economy

What a run hands out (chips, their codes, zenny, Mystery Data), measured
act by act; how other games and BN6 itself pace theirs; and the opening
plan that makes the first act scarcer and opens the run up deeper, with a
guard against drift. [Issue #154](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/154),
the owner on 20 October 2026: "to make trading meaningful we should do a
balance sweep. I feel players could get too many chips early. And too many
good chips. And too convenient letters for chips early. And too much zenny
early. And too many mystery data early maybe? [...] And we need a way to
make sure it doesn't drift too hard in the future."

**As built.** The proposal below went to the owner on 20 October 2026
and he said yes on the 21st, every decision as proposed
([Decided](decided.md)): the opening plan is the game's
(`src/core/economy.c`), the traders pick codes as BN6 does, and the
run-end statistics count the economy. "Before" and "flat" below are the
rolls as they were; "after" and "opening" what a run hands out now.

## Summary

- **The first act was generous, and its finds came in the folder's
  codes.** A run's first three layers gave about **12 chips** (8 from
  Mystery Data, 3 from battles, the guardian's), a quarter of them good,
  and **3,500 zenny**, twice what the act's answer chips and HPMemory
  cost. **63% of the chips found came in the starting folder's codes or
  \*** (the engine leaned finds to the folder's codes), so by the first
  guardian a run held five chips of one code with the \*s: a whole
  hand.
- **Deeper, the economy was already rich, and stays so.** By act 6 a
  run had earned 44,000 zenny against 23,000 for the acts' answers and
  HPMemory, held 24 chips of one code, and seven in ten of its finds were
  good. The owner's complaint is the early run, so the plan leaves the
  sixth act as it is: no power creep, and nothing a later act gains.
- **The opening plan (in play):** five levers, each a curve from a floor
  at the first act to the flat plan's at the net's last, on a logarithm
  (fastest opening early): fewer green Mystery Data (half on act 1), a
  green data's chip kept 60% of the time, fewer good chips in the plain
  finds and fewer extra rich data (a quarter of their weight), half the
  zenny in a find, and **no find leaned to the folder's codes in act 1**.
  Act 1 gives **9.5 chips, 2,630 zenny, 11.6 Mystery Data, three chips of
  one code by the guardian**, and opens to the flat plan's by act 6.
  BN6's own battle drops and busting levels are untouched, so skill pays
  a larger share of the early chips (34% against 28%).
- **The traders** give a prize's code as BN6's own Chip Trader does,
  three times in four one the pack holds none of, rather than the
  folder's: a run's codes are its own, and worth a trade.
- **The guard:** `python3 build.py economy` measures both plans and holds
  the plan in play to bands a fifth either side of its own numbers (0
  outside on both nets); the curves have ROM-free tests; the run log
  keeps each battle's busting level, reward and zenny on hand, and each
  run's economy's counts; the anonymous statistics' run-end event carries
  those counts; and a rule in AGENTS.md's checks table.

## Contents

- [Measuring it](measuring.md)
- [Before: what a run handed out (the flat plan)](flat-plan.md)
- [What BN6 does](bn6.md)
- [Research: how other games pace it](other-games.md)
- [The opening plan](opening-plan.md)
- [The guard](guard.md)
- [Decided](decided.md)
- [A dark layer's prize](dark-layer-prize.md)
- [A held Soul's kind](held-soul-kind.md)
- [Where it is decided](where-decided.md)
