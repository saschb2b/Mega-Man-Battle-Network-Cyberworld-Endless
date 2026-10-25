---
type: Tool
title: "Nav study: the way across BN6's own maps"
description: "`--atlas nav` walks each area's original BN6 net maps and counts the one-wide walkways crossed between their farthest panels, setting `narrow_cap`, and `--atlas arrivals` lists where BN6's maps place MegaMan as he arrives."
tags: [devtools]
generated: { by: claude-code/unrecorded, at: 2026-10-06T13:05:29Z }
---

# Nav study: the way across BN6's own maps

`build/linux/cyberworld --headless --rom-dir DIR --data-dir DIR --atlas nav`
reads each area's original net maps (the maps whose random battles it
fights, `NetAreaDef.battles`), lays the floor their walls give out a panel
at a time, walks between each map's two farthest panels, and prints the
one-wide walkways that walk crosses, of any platforms and between big
ones, and its length: per map, then the area's average
(`src/dev/navstudy.c`, the count `grid_way_narrows` in `net_way.c`, which
the unit tests run on the layers too: `CW_WAY_STATS=1 build/host/test_core`
prints them per area). The originals' counts set `narrow_cap`
([The generator](../world/level-design/generator.md#navigation), Navigation).

`--atlas arrivals` reads where BN6's own maps put MegaMan as he arrives
(the same file, `navstudy_arrivals`): every jack-in destination and every
warp of another map (the real world's groups and the internet's, but a
teleport within one map and a jack-out) onto each area's original maps,
each place and facing once, the side of the map's floor on the screen it
lies at (its edge within a quarter of the floor's box, else the middle) and
whether the way it faces MegaMan points into the map; per map, per area,
and over the net areas' own maps (groups 0x90-0x96) once each. They set the
layers' arrivals ([Arrivals](../world/level-design/arrivals.md)).
