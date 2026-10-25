---
type: Feature
title: "The par's prize, as built"
description: "The par prize as built, BN5's tiers exactly, with its game-design reasoning: a Server's chip two under par, one a tier lower at par, zenny over par from 1000 to 2000 by act (`lib_prizes.c`)."
tags: [game, liberation, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-25T01:28:59Z }
---

# The par's prize, as built (25 October 2026)

BN5's tiers exactly, the owner's decision 6: the board won two phases
under par or better pays a chip, within par a chip, over par zenny, the
same three rows and bounds as every BN5 mission's (The par's prize, above;
`LB.prize` from the phase and par as the Darkloid falls,
`src/director/liberation.c`). What each pays, reasoned with the
game-design skill (`src/layer/lib_prizes.c`, `lib_par_zenny` in
`src/director/liberation_board.c`):

- **The chips:** BN5's two are a better chip and a lesser one of the same
  kind (the Darkloid's SP and plain chips, later a Mega chip and his DS
  one). The run's are a chip a Server's win pays two under par (two
  layers deeper, the rarer tiers weighted) and one a tier lower at par,
  as stage 1 had them: our Darkloid is a BN6 Navi whose own battle pays
  BN6's rows for him beside the prize, so his chip again would be a
  second copy, not a better one.
- **The zenny:** BN5 pays 1000 a mission, 5000 from the fifth. The run
  pays BN5's first mission's 1000 at the second act, where a board first
  stands, and a quarter of it more each act, to 2000 at the sixth; dearer
  on later cycles as a chip's price is, and the zenny curve's share as
  every zenny the engine sets (653, 959, 1,288, 1,636, 2,000 under the
  opening plan). BN5's own climb would pay 3,435 to 5,000 from the fourth
  act, more than the chip at par is worth at a dealer's (2,345 to 2,430,
  `build.py economy`'s dark table), and racing the par would lose to
  dawdling: over par stays the lesser prize, never a lost run, as BN5's.
- **How it is told:** the mission won's tally, in BN5's own words (The
  board's words and banners as BN5's, below: SUCCESS! and MegaMan's
  "Target" and "Actual"), then the chip or the zenny given in BN6's own
  box at once, as BN5's "You got a battle chip for:" and "You got:"
  follow its tally (`ta_lib_prize`, `ta_lib_prize_zenny`,
  `src/layer/service_words.c`). Without BN5's ROM, Lan's words say the
  phases and the tier (`lib_won_words`). (This pull request first had
  MegaMan say what the Darkloid left before the give; merged with the
  board's words, BN5's tally says it, and the give follows bare.) BN5's
  Officials stay out: no one on our board stands for them.
- **Checked:** `build.py test` (the zenny by act and cycle, under either
  plan), `build.py economy` (0 outside the bands; the dark table flags an
  act whose zenny over par reaches the chip at par's price), and seed 3's
  act 2 board played by the autopilot at each tier with `--dev
  libpar=N` (its 8 phases two under a par of 10, at a par of 8, over one
  of 6), beside BN5's mission 1 at 5, 7 and 8 phases in romlab (the
  pictures in the pull request).
