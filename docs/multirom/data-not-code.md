---
type: Engine Mechanism
title: "Data, not code"
description: "BN4, BN5 and BN6 share formats, so another game's maps, songs, sprites and chip tables play in BN6 as data, its code only once moved; each pool a second ROM fills and its cost."
tags: [multirom, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-12T21:51:29Z }
---

# Data, not code

Every battle of a run is fought in BN6's own engine; the engine directs
it and writes data into BN6's in-memory ROM ([Running the game's own logic](../engine/emulation/overview.md)). Battle
Network 4, 5 and 6 share one engine family: their maps, tile sets,
palettes, coordinate data, sprites, text scripts, chip tables and MP2K
songs are the same formats or close to them. A second game's data can be
drawn and played by BN6; its code cannot run inside BN6 as it stands (it
calls its own game's addresses), but a virus family's, a Navi's, a
chip's or a Soul's can once it is moved: copied into BN6's free space,
its calls aimed at BN6's routines (Transplanted families, Navis and
Souls, [The chips' code](chips-code.md)), each proved against its own game's
battle frame by frame.

| Pool | How another game fills it | Cost |
| --- | --- | --- |
| Net areas | Its maps learned as BN6's are (`tiles_learn.c`), its tile set and palette copied into BN6's free space | moderate |
| Towns | The same, from its real-world maps | moderate |
| Music, sounds | Its MP2K songs and their voice groups into BN6's song table | low |
| NPCs, mugshots | Its sprites as bystanders and dealers | low to moderate |
| Chips | Where BN6 has the behaviour (most standard chips); its own moved into BN6's engine one by one ([The chips' code](chips-code.md)), registered in BN6's chip tables (The chip registry); its twelve DarkChips one chip each, BN6's five with BN6's code, its own seven with BN5's rule ([DarkChips](../game/meta/darkchips.md)) | limited; a unit per chip moved |
| Viruses | The families both games have, BN6's; its own moved whole into BN6's engine, their code with their calls aimed at BN6's (Transplanted families: every family its areas' records set) | a recipe per family moved |
| Guardians | Its Navis moved into BN6's engine as its families are (Transplanted Navis: Team Colonel's six and the Dark Soul) | a recipe per Navi moved |
| Souls | Moved with their look, abilities and passives onto a form BN6 keeps and never offers (Transplanted Souls) | high |

Every battle of a run is BN6's own engine's since issue #82's last
round ([One net](one-net.md)). Before it, what needed BN5's code was fought in
BN5's: a second core running BN5's ROM, BN6's waiting beside it (Guest
battles, below, kept as the history it is). That core stays, for one
use: the differential's reference, BN5's own battle frame by frame
against BN6's with the code moved (`build.py xdiff`, [The differential: a moved family against its own game](../devtools/differential.md),
The differential).
