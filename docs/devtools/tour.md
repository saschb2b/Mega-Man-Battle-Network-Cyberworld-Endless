---
type: Tool
title: "Tour: the game shows every room"
description: "`python3 build.py tour [BIOMES]` runs the game headless and warps MegaMan through every room of one layer per area, saving `.build/tour/tour_bXX.png` to show objects, NPCs and collision problems the atlas cannot."
tags: [devtools]
generated: { by: claude-code/unrecorded, at: 2026-09-24T16:18:37Z }
---

# Tour: the game shows every room

```bash
python3 build.py tour [BIOMES]
```

Runs the game headlessly and warps MegaMan through each area's layer
(`src/dev/tour.c`): one layer per area at depth 2, every room in turn, the
frame saved once the map has settled. Random battles are off and MegaMan
cannot be deleted. Two areas take about 6 seconds.

Output: `.build/tour/tour_bXX.png`, the rooms of one area at 2x, four to a
row. This is what the game itself draws: objects, NPCs, decoration and
collision problems show up here that the atlas cannot show.
