---
type: Code Map
title: "The director: the run on the game"
description: "What `src/director/` holds, file by file: the frame's course and its shared state, layers, checkpoints, encounters, bosses, home and Lan's HP, the requests, a dark layer's liberation, and the words files."
tags: [code]
generated: { by: claude-code/unrecorded, at: 2026-10-23T17:01:56Z }
---

# The director: the run on the game

| Path | Contents |
| --- | --- |
| `src/director/` | The run on the game: the town, layers, warps, encounters, bosses, checkpoints, powers; the game's events from its hooks (`events.h`: battles in `encounter.c`, maps, choices and key items in `events.c`), taken up after each frame. `director.c` is the frame's course and the state its parts share (`director_state.h`); its parts are `director_*.c` (the run's end, lost or won, `director_end.c`; layers made and entered, the run's chips, checkpoints, what the drawing reads, the map, the way on, the pad, the NaviCust's board, the duel, DarkChips, BN5's records fought, dev steps, the state in words, home after an act, Lan's HP and its portals: `director_home.c`, the home's frame `director_town.c`, the requests taken, kept and counted `director_jobs.c`; a dark layer's liberation, [Liberation Missions](../game/liberation/overview.md): its board from the layer's floor `liberation_board.c`, its course `liberation.c`, its drawing `liberation_draw.c`, BN5's own look on it `liberation_look.c`, BN5's gauge, L view, window, banners and items drawn from its ROM `liberation_pieces.c`, the dark way's portal at home in BN5's swirl `liberation_way.c`, its fence, BN6's walls switched as panels are freed, `liberation_fence.c`, the autopilot's play `liberation_auto.c`), and what they say is in `*_words.c` (L's briefing, the story's beats, Chaud, the NaviCust, DarkChips, where the nets meet, lessons, coming home) |
