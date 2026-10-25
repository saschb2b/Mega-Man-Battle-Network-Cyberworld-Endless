---
type: Design
title: "Measuring it"
description: "How `build.py economy` and `src/dev/economy_report.c` measure a run's handouts without running the game: layers and Mystery Data simulated by the engine's code, battle rewards from BN6's ROM rows, the assumptions, definitions and autopilot check."
tags: [game, economy]
generated: { by: claude-code/unrecorded, at: 2026-10-20T22:52:50Z }
---

# Measuring it

```sh
python3 build.py economy [RUNS]     # .build/economy.txt, 300 runs by default
```

`src/dev/economy_report.c` (`--economy PATH[:RUNS]`, [Dev tools](../../devtools/overview.md))
plays the endless net's first cycle (and the short net's ten layers) of
many runs' seeds without running the game. Two kinds of number, said
apart:

- **The engine's, simulated with its own code.** Every layer as
  `layer_generate` makes it, so the Mystery Data, dealers, Servers,
  traders and gates are the game's own; each Mystery Data's content by
  `loot_mystery_roll` (moved out of `layer_objs.c` for this, the same
  rolls in the same order); each Net Dealer's stock by
  `shop_dealer_stock`, the NaviCust vendor's by `shop_program_stock`; the
  requests' pay by `jobs_offers`; a guardian's Guardian Data (his Navi
  chip, five HPMemory).
- **The game's, read from the ROM.** Each random battle the engine rolls
  (`loot_random_battle`, the opening battles' rules as the director's)
  pays as BN6 picks a reward: one enemy at random, its reward row
  (`BN6_DROP_ROWS`, 20 entries), the busting level's row of BN6's busting
  table (16 pairs' offsets), a coin, the row's second half at low HP; the
  second entry of a pair leaned to the folder's codes as `encounter.c`'s
  hook leans it; a guardian's Navi chip entries his act's zenny
  (`loot_guardian_zenny`). [BN6's battles](../../rom/battle.md), Battle rewards.

Assumptions, each a number in the code and said in the report's header:

| Assumption | Value | From |
| --- | --- | --- |
| Random battles a layer | 1 to 3, even | the playtest sessions' run logs (`.build/play/*/data/runlog.txt`, 94 sessions): 1.7 to 2.4 battles a layer, layers 1-12 |
| Busting levels | S 20%, 10, 9, 8 and 7 15% each, 6 10%, 5 and 4 5% each | assumed: a fair player; the autopilot's are lower (below) |
| Won at low HP (3/8 or less) | 10% | assumed |
| A battle's own Mystery Data still there at its end | 50% | assumed |
| Every Mystery Data opened, every battle won | | the most a run can take |
| The folder | BN6's starting one, codes S, A and B (`loot_folder_codes`) | the standard setup |
| The areas | BN6's alone, threat 0, no helpers | where BN5's dress an area, half the chips are BN5's of the same tiers |

Definitions:

- **Good chip:** tier 2 or more of `chip_pool.h` (BN6's rarity of 3 stars
  or more for a standard chip, every Mega and Giga chip). **Strong:** a
  straight hit of 100 or more.
- **Codes held:** the most chips gained in one letter, with the \*s,
  median run, at an act's end: how big a hand of one code the finds alone
  make (BN6's Custom screen takes up to five chips of one code; \* joins
  any). **Fit:** a chip found in one of the folder's three codes or \*.
- **Wanted:** what the act's middle-layer dealer asks for his first chip
  (the act's answer, two copies) and his HPMemory, summed act by act: the
  purchases a run plans around ([The structure](../progression/structure.md#growth), Growth).

**Checked against the game.** The run log keeps each won battle's busting
level and reward since this issue ([Run log](../../devtools/run-log.md)). Twelve
autopilot runs (seeds 1-12, fresh data folders, 40,000 frames; with `--dev
god` 90,000; with `--dev god,onehit` 20,000) won 110 random battles:

| | Autopilot (game) | Report at the same levels |
| --- | --- | --- |
| Busting levels | 1-10, median 7; 11 (S) with one-hit kills | the skill table: level 6 15% chips 143 zenny, level 8 62% chips 85 zenny |
| Act 1 battles paying a chip | 38-42% | 15-62% between levels 6 and 8 |
| Zenny a battle, act 1 | 89-131 | 85-143 |
| Guardian's reward, act 1 | 600 (his Navi chip's entry), 1000 or 2000 (his row's zenny) | the same words, 1,264 on average at the report's mix |

The autopilot opens no Mystery Data and buys nothing, so it checks the
battles' half; the Mystery Data are `layer_generate`'s and
`loot_mystery_roll`'s own, as the layers in play are (`LAYER_MAKE_HASH`
unchanged then). The opening plan's levers ran in play before it was
switched on: twelve one-hit autopilot runs under it reached acts 4 and
5, and their guardians' chip-entry zenny was the zenny curve's share of
the flat 600 to 1,800 to the zenny, act by act (300, 588, 921, 1,288,
1,683), BN6's own rows' zenny (1,000, 2,000, 3,000, 6,000) as before.
