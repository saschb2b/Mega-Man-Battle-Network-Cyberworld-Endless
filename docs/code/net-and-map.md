---
type: Code Map
title: "Layer generation and layers as game maps"
description: "What `src/net/` (layer generation: plan, signature rooms, detours, landmarks, set pieces) and `src/map/` (layers as BN6's maps: tiles learned, the map tables, Lan's HP) hold, file by file."
tags: [code]
generated: { by: claude-code/unrecorded, at: 2026-10-06T11:32:03Z }
---

# Layer generation and layers as game maps

| Path | Contents |
| --- | --- |
| `src/net/` | Layer generation (rooms, walkways, objects): `net_gen.c` the driver and where the services, data and people stand, `net_plan.c` a layer's layout and signature in its act, `net_signature.c` the signature rooms ([Identity](../world/level-design/identity.md)), `net_detours.c` the detours, `net_landmarks.c` the landmarks and props, `net_set_pieces.c` where the set pieces stand (`net_pieces.c` which a layer holds) |
| `src/map/` | Layers as game maps: tiles learned from the original maps (`tiles.c` their checks, `tiles_learn.c`, `tiles_pick.c`), walls and warp-pad triggers, the map tables taken over (`netmap.c`; the originals learned in `netmap_learn.c`, their scenery pasted in `netmap_paste.c`, the set pieces' cells in `netmap_extra.c`); Lan's HP, BN6's own map taken over as the run's warp zone (`lanhp.c`, `docs/world/home/`) |
