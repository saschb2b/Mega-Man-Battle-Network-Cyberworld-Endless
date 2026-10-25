---
type: Feature
title: "Rush's gaps"
description: "Rush's gaps (issue #14): BN6's bone panels over a void gap to a pad of its own holding one blue Mystery Data, crossed with RushFood the Net Dealer sells (`ng_plan_gap`, `ng_carve_gap`, `rush.c`)."
tags: [world, level-design]
generated: { by: claude-code/unrecorded, at: 2026-10-19T16:59:51Z }
---

# Rush's gaps (built, issue #14)

BN6 lays 13 bone panels over 9 gaps (Central 2, Seaside 1, Green 1 and 2,
Sky 1 to 3, Undernet 1, ACDC's homepage), 1 to 3 panels each: six are
shortcuts, three lead to one Mystery Data. Press A at the edge with RushFood
held, as many as the gap's panels, and Rush comes, eats one, and lies in
the gap for good. The game runs all of it ([The net's maps and their pieces](../../rom/net-maps.md), Rush's gaps);
a layer gives it what its own maps do.

- **The place** (`ng_plan_gap`, `ng_carve_gap` in `net_set_pieces.c`): a walkway's tip
  (else a platform's edge) off the way, at ground height, aimed across the
  gap at a pad of its own, 1-3 panels on (the area's longest: Sky's 3,
  Central's and the Undernet's 2, the rest 1), the gap's panels and the
  pad void with void a panel round them and no prop, inside the camera's
  window; the farthest off the way first. The stand and the panel behind
  it are kept clear of what is placed after; the pad is carved last, as a
  room of its own, and holds one blue Mystery Data, of the best quality
  past one panel (two or three bones' worth).
- **The map** (`netmap.c`, `coords.c`): for the walls the gap is a
  walkway, walled along its sides; across its mouths the floor's edges
  stand again with the flag byte 0xFF (the second gap's 0xFE), which Rush
  lying there switches off. Both mouths: a mouth's row is centred on the
  panels' edge, and the one on a gap's far edge (from its stand towards
  -X or -Y, the near one) had counted as the next panel's and stood
  unwalled, so MegaMan walked onto the island without Rush and the
  island's edge held him there (issue #111). Its trigger strip is the lane's floor 24 units
  in from the near mouth, where the engine's A probe lands (BN6's strip is
  the mouth's row alone, its probe 8 ahead). Its tiles stay void.
- **The objects** (`rush.c`): per panel Rush (handler 0x25, shown once
  bridged) and the bones (OverworldMapObjects 0xD5 + g, shown until then).
- **The key:** RushFood, sold by the Net Dealer where the act holds gaps
  from his layer on (one for each gap, and as many more as the longest
  needs held), at 300 zenny in act 1 and 100 more an act (BN6's seller asks
  3000). The dealer names the gap and its width; how Rush comes is BN6's
  own, unsaid (MegaMan's lesson at its stand went in the voice sweep of
  19 October 2026, docs/voice/sweep/net.md).

Over the tests' 300 layers, 40 hold a gap, 55 panels in all (the act's
budget shared with more kinds of piece since: 62 and 91 before the
obstacles); the way never needs one, and each island is reached only
across it, holding its one Mystery Data.
