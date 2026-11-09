---
type: Code Map
title: "The real world"
description: "What `src/world/` holds: the real world and Central Town, BN6's indoor maps taken over as home's places (Lan's house, AsterLand, the Academy), the town's folk and their words."
tags: [code]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-09T14:45:00Z }
---

# The real world

| Path | Contents |
| --- | --- |
| `src/world/` | The real world: Central Town, home, learned from its own tiles and planned (`docs/world/overworld.md`), and BN6's own indoor maps taken over as home's places (`indoors.c`, side by side in `home_places.c`): Lan's house and room, where a run begins and his PC jacks in (`lan_house.c`, `docs/world/home/`), AsterLand (`aster_land.c`) and the Cyber Academy (`academy.c`), their people and what they say in `place_lines.c`, the askers' requests in `job_words.c`, posted on BN6's own Request BBS at AsterLand by `request_bbs.c`; the town's folk and what is said there in `town_lines.c`, at each visit home who stands where (`town_folk.c`) and what they say of the run (`town_words.c`); its other ports in `town_ports.c`, for the jobs' errands; another game's town ([A BN5 town on the way](../world/bn5-town.md)): Oran Isle as the fifth of the towns' styles (`town_styles.c`, their table and plans, shared with `town.c` through `town_style.h`), copied whole from BN5's ROM (`townsrc_learn`'s game), its tile set and colours copied into their own range by `xtown.c`, its three people's talk (the guardian's word, the stall, the job) in `oran.c` and their lines in `oran_words.c` |
