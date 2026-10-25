---
type: Code Map
title: "Where it is decided"
description: "Where the economy is decided in code: plans and bands in `src/core/economy.c`, Mystery Data and chip rolls in `src/core/loot.c`, data placement, battle rewards, dealers, traders, dark-layer prizes, requests' pay, the report, run counts."
tags: [game, economy]
generated: { by: claude-code/unrecorded, at: 2026-10-25T00:18:16Z }
---

# Where it is decided

| What | Where |
| --- | --- |
| The plans, their curves, the bands, the report's assumptions | `src/core/economy.c` |
| A Mystery Data's content | `src/core/loot.c` (`loot_mystery_roll`); its key items `src/layer/layer_objs.c` (`fill_mystery`) |
| Green and blue data on a layer | `src/net/net_gen.c` (`place_data`) |
| Chip rarity, codes leaned to the folder | `src/core/loot.c` (`roll_chip`, `loot_fit_code`, `loot_folder_code`) |
| A guardian's chip-entry zenny | `src/core/loot.c` (`loot_guardian_zenny`), `src/director/encounter.c` |
| A dark layer's par prize: its chips, the zenny over par | `src/layer/lib_prizes.c`, `src/director/liberation_board.c` (`lib_par_zenny`) |
| Battle rewards (BN6's rows, the lean) | `src/director/encounter.c` (`lean_row`) |
| Dealer stock, prices | `src/layer/shop.c`, `src/core/loot.c` (`chip_price`) |
| Traders: their pools as BN6 has them, its code pick | `src/layer/trader.c` |
| A dark layer's chips ([Liberation Missions](../liberation/overview.md)): its par's prizes, its item panels' and its BonusPanels' (one of four, the item panels' band, as BN5's are its mission's standard chips) | `src/layer/lib_prizes.c` |
| Requests' pay | `src/core/jobs.c` |
| The report | `src/dev/economy_report.c` |
| The run's counts, sent at its end | `src/director/runlog.c` (kept, saved as `run.tally`, a battle's reward and a Guardian Data's chip counted), `src/director/director_jobs.c` (the Mystery Data opened, as a layer is left), `src/analytics/analytics.c` (`analytics_count_*`, `analytics_run_end`) |
