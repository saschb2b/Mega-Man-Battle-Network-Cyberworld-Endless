---
type: Code Map
title: "Where it is decided"
description: "Where progression is decided in code: acts, bands, versions and heals in `src/core/pacing.c`, area order and guardian pools in `src/core/run.c`, super bosses, formations, BN5's records, visitors, guardian versions, chip rarity and rewards."
tags: [game, progression]
generated: { by: claude-code/unrecorded, at: 2026-10-20T22:14:37Z }
---

# Where it is decided

| What | Where |
| --- | --- |
| Acts, bands, versions, area tiers, guardian bands, heals | `src/core/pacing.c` |
| Area order, guardian pools | `src/core/run.c` (`run_new`) |
| The super bosses' places, forms and records | `src/core/super_boss.c` ([Guardians and super bosses](../bosses/overview.md)) |
| Act and guardian layers | `src/net/net_gen.c` (`biome_for_depth`, `is_boss_depth`) |
| Formation pick, versions inside the band | `src/core/loot.c` (`make_encounter`) |
| BN5's records inside the band, their viruses' versions | `src/core/xrecords.c` (`xrecords_pool`), `src/core/xbattles.c` (`xterritory_encounter`) |
| The layer's nets: a territory, the deep, the older net's chips | `src/core/loot.c` (`loot_layer_nets`, `loot_random_battle`), `src/core/visitors.c` (`nets_battle_area`) |
| Visitors: the gate, the roles, the band | `src/core/visitors.c`, `src/core/visitor_families.c` |
| Guardian version | `src/core/loot.c` (`make_boss`); BN5's guardians' `src/layer/xguardian.c` (`xguardian_version`, `xguardian_encounter`) and their places `src/core/run.c` (`run_xguardian_at`) |
| Chip rarity and prices | `src/core/loot.c` (`roll_chip`, prices); the economy's plans and their curves `src/core/economy.c` ([Economy](../economy/overview.md)) |
| Services and Mystery Data per layer | `src/net/net_gen.c`, `src/layer/layer_objs.c` |
| `src/director/director_layer.c` (roll_encounter), `src/director/director.c` (act_on_choices) | `src/director/director.c` |
| Guardian rewards, the start gift | `src/layer/scripts.c`, `src/layer/guardian_objs.c`, `src/director/powers.c` |
