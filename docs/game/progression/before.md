---
type: History
title: "How a run worked before"
description: "How a run's difficulty worked before the progression research: 19 layers a cycle from 15 areas, virus versions by depth, a random first area, guardian versions, the Recovery Mr. Prog's heal, growth and MegaMan's start."
tags: [game, progression]
generated: { by: claude-code/unrecorded, at: 2026-10-21T00:40:34Z }
---

# How a run worked before

The state this research started from. The points that decide difficulty
are listed in [Where it is decided](where-decided.md).

- **Acts.** 19 layers per cycle: acts 1-4 of 3 layers each in four areas
  drawn from 15 (`run.c`), act 5 the Graveyard, act 6 the Undernet, layer
  19 the Cybeast Nest. Layers 3, 6, 9, 12, 15, 18 and 19 have guardians, so
  18 and 19 follow each other. Later cycles repeat the same areas and
  guardians.
- **First area.** Any of Central, Seaside, Sky, Green, the Robot Control,
  Aquarium, Judge Tree, Mr. Weather and CopyBot comps, the two groups of
  home computers, and the Aquarium, ACDC, Green and Sky homepages.
- **Virus version by depth** (`virus_version`, `loot.c`): V1 on layers 1-4,
  V1 or V2 on 5-9, mostly V2 on 10-13, V3 on 14-19, one more per cycle.
  These steps do not fall on act boundaries. 3% of rolls at any depth,
  layer 1 included, turn one virus rare and the rest SP.
- **Virus families** are gated by where they first appear: families of the
  surface areas from layer 1, Undernet families from 7, Graveyard and Nest
  families from 13. Any family that Central, Seaside, Sky or Green lists
  counts as a surface family, so Sky's Catack, FgtrPlne and ScarCrow count
  as early. If no formation of an area passes, every formation is allowed.
- **Guardian version:** V1 on layers 3-9, EX on 12-18, SP in the Nest, the
  Secret Area and every later cycle. HP is the game's value for that version.
- **Healing:** a Recovery Mr. Prog on 30% of layers, and always in the room
  before an arena. He patches MegaMan once on his layer: half of his max
  HP, to full before the arena (issue #71, the owner's call on a player's
  proposal in discussion #5: a heal as often as asked refunded every
  detour's HP, and the run's dialectic, prepare or press on, lost its
  price). He asks before he patches, and at full HP keeps the patch,
  where he had spent it on an unhurt MegaMan unasked (session 75; event
  flag `0x1464`, which the director keeps set while HP is full). Asked
  again, he points to the Net Dealer's MiniEnrg, and L and the arrow stop
  leading to him. The Heals helper's Prog heals to full as
  often as asked. HP carries over between layers; a won guardian heals
  fully.
- **Growth:** 3 HPMemory (+60 max HP) per guardian, one HPMemory in each Net
  Dealer at `(10 + 2 × depth) × 100` zenny, a Cross from five of the Navis,
  Beast Out from the Cybeast at the endless net's own Nest, for the rest of
  the run (issue #109; from the first battle with the BeastOut helper, once
  that Nest has fallen: [The meta layer](../meta/overview.md); the acts' bands stay as they are with
  it). Guardians drop no chip.
- **MegaMan's start:** the game's own new game. 100 HP and the default
  folder of 30 chips: Cannon ×4, AirShot ×2, Vulcan1 ×3, MiniBomb ×4,
  Sword ×4, WideSwrd ×2, CrakShot ×2, AreaGrab, Atk+10 ×2, Recov10 ×4,
  BusterUp and WhiCapsl. That is about 50 damage per attack chip.
- **Optional fight:** a Server on `20 + depth` % of layers fights three
  layers deeper and one version higher. On layer 1 that is V2.
