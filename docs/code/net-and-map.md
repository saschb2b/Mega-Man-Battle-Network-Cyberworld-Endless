---
type: Code Map
title: "Layer generation and layers as game maps"
description: "What `src/net/` (layer generation: plan, signature rooms, detours, landmarks, set pieces) and `src/map/` (layers as BN6's maps: tiles learned, the map tables, Lan's HP) hold, file by file."
tags: [code]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-13T00:55:00Z }
---

# Layer generation and layers as game maps

| Path | Contents |
| --- | --- |
| `src/net/` | Layer generation (rooms, walkways, objects): `net_gen.c` the driver and where the services, data and people stand, `net_services.c` which services and gates a layer rolls, on numbers of their own so a fork reads them before it is built (`layer_holds`, [a fork's read](../world/level-design/fork-read.md)), `net_plan.c` a layer's layout and signature in its act, `net_signature.c` the signature rooms ([Identity](../world/level-design/identity.md)), `net_detours.c` the detours, `net_landmarks.c` the landmarks and props, `net_set_pieces.c` where the set pieces stand (`net_pieces.c` which a layer holds); the run's Glitch warp in a dark warp's place and a Glitch layer's copy at its farthest detour, its data the deeper the farther off the way (`net_glitch.c`, rolled in `net_services.c`) |
| `src/net/layer_make.h`, `src/net/layer_make.d/` | The layers' make, which a save carries so a run continues afresh where a build makes its layer otherwise: 100 and a file a bump (`NNN-WORDS.txt`, what changed), counted by the build ([Merging in parallel](../project/merging.md#counters)) |
| `src/map/` | Layers as game maps: tiles learned from the original maps (`tiles.c` their checks, `tiles_learn.c`, `tiles_pick.c`; `seams.c` the tiles and the strips of pixels the maps set side by side, `tilemap.c` a whole map's pick; `panels.c` the maps' panels laid whole before it, where an area asks (`TILES_WHOLE`); `breaks.c` the breaks in a layer's picture, for the tile test alone), walls and warp-pad triggers, the map tables taken over (`netmap.c`; the originals learned in `netmap_learn.c`, their scenery pasted in `netmap_paste.c`, the set pieces' cells in `netmap_extra.c`); Lan's HP, BN6's own map taken over as the run's warp zone (`lanhp.c`, `docs/world/home/`); another look of the layer built, its floor picked in another area's tiles and that area's tile set and colours as VRAM takes them, nothing written (`netmap_look`, `area_src_vram`: a Glitch layer's flips, [the Glitch layer](../game/glitch-layer.md)) |
