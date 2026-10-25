---
type: Research
title: "The Navis"
description: "The Navis of BN5's Liberation Missions: each one's board HP by mission (`BN5_LIB_NAVI_HP`) and the commands BN5 lists, with each command's shape from the guides (LongSwrd, ScrenDiv, SneakAttack, NumberCheck, LifeMelody)."
tags: [game, liberation, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-23T13:56:43Z }
---

# The Navis (ROM, text, guides)

Each Navi's HP rises mission by mission (`BN5_LIB_NAVI_HP`, **ROM**):
Colonel 200 to 800, KnightMan 400 to 900, ShadowMan 250 to 700, TomahawkMan
300 to 700, NumberMan 300 to 650, ToadMan 300 to 600; MegaMan has his own.
The commands are BN5's own list (**text**): Liberation, LongSwrd, WideSwrd,
ScrenDiv, TwinLiberation (twice), LifeMelody, BombLiberation, SneakAttack,
PanelSearch, NumberCheck, Napalm, TomahawkSwing, MagnetBarrier, Save, Pass.
Their shapes are the guides' (the routines that apply them are not traced):

| Navi | Command | What it does |
| --- | --- | --- |
| MegaMan | LongSwrd | Two panels in a line, one battle |
| Colonel (ProtoMan) | ScrenDiv (WideSwrd) | Three across, one battle; can save |
| TomahawkMan | TomahawkSwing | Two deep and three wide, one battle |
| NapalmMan | Napalm | Four ahead and both sides of the third |
| NumberMan | NumberCheck | Two by three ahead, no battle: takes the items, disarms traps, liberates nothing |
| SearchMan | PanelSearch | A line of dark panels: items shown, then liberated after one battle |
| ShadowMan | SneakAttack | Passes over dark panels; strikes a guardian or the Darkloid one or two ahead (never below 1 HP) |
| GyroMan | BombLiberation | Flies over dark panels; liberates the one beneath him without a battle |
| KnightMan | (none) | Takes no damage on the board; shields a Navi beside him |
| MagnetMan | MagnetBarrier | The team takes no damage on the board until the next phase |
| ToadMan | LifeMelody | A teammate beside him liberates five ahead this phase |
| Meddy | TwinLiberation | Battles at both ends of a line; a second Navi finishes it from the far end |

Team ProtoMan's six (ProtoMan, GyroMan, NapalmMan, SearchMan, MagnetMan,
Meddy) are the other version's; this ROM keeps their commands' names.
