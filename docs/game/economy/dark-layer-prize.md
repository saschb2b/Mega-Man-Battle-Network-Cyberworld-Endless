---
type: Design
title: "A dark layer's prize"
description: "A dark layer's par prize as BN5's Liberation Missions pay it: a Server's prize two under par, one a tier lower within par, zenny over par by the zenny curve (`src/layer/lib_prizes.c`, `lib_par_zenny`)."
tags: [game, economy]
generated: { by: claude-code/unrecorded, at: 2026-10-24T23:57:46Z }
---

# A dark layer's prize (25 October 2026)

A dark layer ([Liberation Missions](../liberation/overview.md)) pays its par's prize as BN5's
Liberation Missions do, the owner's decision of 25 October 2026: a chip
two under par or better, a chip within par, zenny over par. The chips are
a Server's prize and one a tier lower (`src/layer/lib_prizes.c`); the
zenny is BN5's first mission's 1000 at act 2 and 250 more an act, by the
zenny curve (`lib_par_zenny`). `build.py economy` prints them act by act,
each board's (a run meets one at most an act, by its choice at a fork, in
place of a layer's finds: not in the acts' sums or bands):

| Act | Two under: good, price | At par: good, price | Over par: zenny (flat, opening) |
| --- | --- | --- | --- |
| 2 | 69%, 2,335 | 47%, 1,677 | 1,000, 653 |
| 3 | 76%, 2,697 | 57%, 1,875 | 1,250, 959 |
| 4 | 80%, 2,863 | 67%, 2,345 | 1,500, 1,288 |
| 5 | 81%, 2,807 | 73%, 2,430 | 1,750, 1,636 |
| 6 | 74%, 2,593 | 73%, 2,428 | 2,000, 2,000 |

The price is a dealer's for that chip (`chip_price`), the yardstick
between a chip and zenny. BN5's own zenny (1000 a mission to 5000) would
pay 3,435 to 5,000 from act 4, more than the chip at par is worth, and
dawdling would beat racing the par; the report counts an act outside
where the zenny over par reaches the chip at par's price (0 now). The
opening plan's chips are as the flat plan's here: a Server's prize rolls
its rarer tiers by their own weight, which the good chips' lever leaves
alone.
