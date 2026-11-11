---
type: Research
title: "Encounter groups, measured"
description: "The encounter groups' measured numbers of 11 November 2026: every virus family in the engine tagged (versions, element, weakness, role, pressure, tricks, reward row), each ROM set's pool by act, how many ways each template fills, and what sampled battles cost a standing MegaMan and the autopilot."
tags: [game, progression, multirom]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-11T17:30:00Z }
sources:
  - id: tools
    resource: "tools/encounter_tags.py report --write, from tools/encounter_data/ (tags.tsv, families.tsv, danger.tsv)"
    title: "The tagging tool's report on dev at b04cdad6, BN6 Gregar, BN5 Team Colonel and BN4 Red Sun (USA) read"
---

# Encounter groups, measured

The numbers [encounter groups](encounter-groups.md) rest on, written by
`tools/encounter_tags.py report --write` ([Encounter tags and
danger](../../devtools/encounter-tags.md)) from the three files of
`tools/encounter_data/`. Everything under the mark is the tool's; run it
again after a family is moved or a band changes. The families were read
on `dev` at b04cdad6; the fill counts are since 11 November 2026 without
the limit of one pooled BN4 family a battle, which their fixed sound
slots lifted, and the engine's own pool gives the same counts
(`build.py pacing`, [Encounter groups](encounter-groups.md), Step 1,
built).

How to read it:

- **A family** is its net and BN6's AI index: `6:25` is BN6's family 25,
  `5:36` BN5's moved in as AI 36, `4:64` BN4's as AI 64. A family both
  games have by name is BN6's.
- **Versions**: BN6's V1 to V3 (its SP and rare ones are left to the Nest
  and the later cycles), the older nets' six. HP and hit are the first
  and the last of them.
- **Weak to** is the first wheel's (Aqua beats Fire, Elec Aqua, Wood Elec,
  Fire Wood); **second wheel** the weakness BN6's traits name (Sword,
  Wind, Cursor, Breaker). A slash marks a family whose element changes
  with its version. BN5's and BN4's families carry no second-wheel
  weakness: their games have none to move (assumed from their records'
  empty bytes; not looked for in their code).
- **Role** is the visitors' (`visitor_role`); **lean** where most of its
  game's records set it, with the placements in the enemy's first, middle
  and last column.
- **Stage**: 0 the surface, 1 the Undernet, 2 the Graveyard, as its own
  game first sets it; `-` a family none of the records read places.
- **Pressure**: the HP a MegaMan standing at his middle panel loses in 20
  seconds to one of it at V1. **Autopilot**: how long the test autopilot's
  starting folder takes to delete that one and the HP it loses, `open`
  where 50 seconds were not enough.
- **Tricks**: `panels:` it changes MegaMan's panels to the type named,
  `field:` its own side's, `area` it takes panels, `summons` more enemies
  came than the battle set, `tough` the autopilot did not delete it.
- **Chip entries**: of the ten entries of its reward row a healthy MegaMan
  draws from, how many are chips, at its first and its top version; in
  brackets the most kinds of chip one row holds.
- **The pool by act**: the families a battle of the act may hold, by lean,
  by element at the version the act takes, and by trick.
- **The templates**: the ways each fills, a group of families once; in
  round brackets those of two nets or more; in square brackets, for acts
  1 to 4, the fewest and the most an area of the act's pool holds one of
  its own families in. **none** marks a template the ROM set cannot fill.
- **Families that can carry an act**: those that fill three templates or
  more, of the act's pool.
- **Danger**: each act's own battles (as the act takes them: the highest
  version inside its band) and two fillings of each template with every
  ROM read, fought idle and by the autopilot. Predicted is the sum of the
  families' own pressures, each by its hit at the version fought over its
  V1's; the ratio says what standing together adds.

<!-- measured: tools/encounter_tags.py report --write -->

## The families

65 families in the engine with every ROM read: 29 of BN6, 25 of BN5, 11 of BN4.
Read from the ROMs by `tools/encounter_tags.py dump`; pressure and tricks fought by `tools/encounter_danger.py families`.

| Family | Versions | HP | Hit | Element | Weak to | Second wheel | Role (the visitors') | Lean (front/middle/back placements) | Stage | Pressure | Autopilot at V1 (time, HP) | Tricks | Chip entries of 10, V1 to top (kinds a row at most) | Note |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 6:01 | 3 | 40-120 | 10-40 | none | - | - | free | free 92/93/78 | 0 | 70 | 2 s, -0 | - | 4-3 (1) |  |
| 6:02 | 3 | 70-150 | 20-60 | aqua | elec | - | free | free 47/42/42 | 0 | 20 | 14 s, -0 | - | 3-3 (1) |  |
| 6:03 | 3 | 130-210 | 50-50 | none | - | - | free | free 8/9/6 | 2 | 240 | 41 s, -20 | - | 3-3 (1) |  |
| 6:04 | 3 | 90-160 | 30-90 | none/fire/aqua | -/aqua/elec | - | free | free 20/37/9 | 0 | 180 | 19 s, -0 | area | 3-2 (1) |  |
| 6:05 | 3 | 100-190 | 30-90 | elec | wood | - | free | free 7/11/4 | 1 | 100 | 2 s, -0 | - | 4-2 (1) |  |
| 6:06 | 3 | 80-200 | 10-40 | none | - | - | free | free 24/28/26 | 0 | 120 | 2 s, -0 | - | 3-3 (1) |  |
| 6:07 | 3 | 130-220 | 50-150 | none | - | - | free | back 6/14/15 | 0 | 210 | 14 s, -35 | area | 2-2 (1) |  |
| 6:08 | 3 | 60-180 | 50-150 | fire | aqua | - | free | free 24/36/29 | 0 | 160 | 14 s, -20 | - | 5-3 (1) |  |
| 6:09 | 3 | 130-160 | 100-150 | none | - | - | back | back 0/2/15 | 3 | 0 | 18 s, -0 | - | 3-3 (1) |  |
| 6:10 | 3 | 80-130 | 90-150 | none | - | - | back | back 0/0/17 | 0 | 0 | 43 s, -0 | - | 5-4 (1) |  |
| 6:11 | 3 | 80-180 | 10-40 | fire | aqua | - | free | free 31/34/30 | 0 | 60 | 29 s, -20 | - | 4-3 (1) |  |
| 6:12 | 3 | 150-240 | 60-100 | fire | aqua | - | free | free 3/10/9 | 1 | 200 | 14 s, -100 | - | 3-4 (1) |  |
| 6:13 | 3 | 100-100 | 10-80 | none | - | - | free | back 1/5/5 | 2 | 0 | open, -0 | tough | 0-2 (1) |  |
| 6:14 | 3 | 80-200 | 80-200 | aqua | elec | - | free | back 16/37/51 | 0 | 50 | 2 s, -0 | - | 2-2 (1) |  |
| 6:15 | 3 | 60-210 | 10-40 | aqua | elec | - | free | back 7/21/24 | 0 | 320 | 15 s, -100 | - | 3-2 (1) |  |
| 6:16 | 3 | 200-260 | 100-170 | fire/elec/aqua | aqua/wood/elec | - | free | free 6/13/5 | 2 | 300 | open, -390 | tough | 2-2 (1) |  |
| 6:17 | 3 | 100-180 | 80-180 | elec | wood | - | free | free 5/15/14 | 0 | 90 | 2 s, -0 | - | 3-2 (1) |  |
| 6:18 | 3 | 120-200 | 60-120 | elec | wood | - | free | free 7/23/9 | 1 | 120 | open, -0 | tough | 3-3 (1) |  |
| 6:19 | 3 | 100-200 | 10-30 | none | - | - | free | back 3/7/7 | 1 | 0 | 43 s, -0 | - | 3-2 (1) |  |
| 6:20 | 3 | 140-240 | 20-60 | wood | fire | - | free | back 12/33/34 | 0 | 180 | 28 s, -50 | - | 5-3 (1) |  |
| 6:21 | 3 | 100-210 | 20-80 | wood | fire | - | free | back 11/12/18 | 2 | 0 | 17 s, -0 | area | 5-2 (1) |  |
| 6:22 | 3 | 130-210 | 5-9 | wood | fire | - | free | back 11/12/17 | 0 | 0 | 29 s, -215 | - | 4-3 (1) |  |
| 6:23 | 3 | 60-220 | 10-40 | none | - | - | free | back 29/49/61 | 0 | 80 | 2 s, -0 | - | 3-2 (1) |  |
| 6:24 | 3 | 140-250 | 20-60 | none | - | - | free | free 27/29/31 | 0 | 100 | 18 s, -20 | - | 2-2 (1) |  |
| 6:25 | 3 | 180-270 | 50-150 | none | - | cursor | free | free 4/11/5 | 1 | 440 | open, -550 | tough | 2-2 (1) |  |
| 6:26 | 3 | 130-250 | 60-140 | none | - | wind | free | free 8/9/3 | 1 | 180 | 41 s, -60 | - | 2-2 (1) |  |
| 6:27 | 3 | 100-160 | 20-120 | none | - | - | front | front 51/29/5 | 0 | 200 | 42 s, -30 | - | 3-3 (1) |  |
| 6:28 | 3 | 120-200 | 30-90 | none | - | - | free | free 12/10/10 | 2 | 150 | 42 s, -140 | panels:hole panels:cracked | 2-2 (1) |  |
| 6:29 | 3 | 100-180 | 40-80 | none | - | - | back | back 3/1/18 | 2 | 80 | open, -140 | tough | 3-3 (1) | special panels only |
| 5:32 | 6 | 60-280 | 10-200 | none | - | - | free | free 20/14/21 | 0 | 60 | 3 s, -0 | - | 5-3 (1) |  |
| 5:33 | 6 | 80-220 | 50-200 | none | - | - | free | back 0/6/7 | 0 | 150 | 44 s, -190 | - | 3-2 (1) |  |
| 5:34 | 6 | 60-290 | 20-190 | none | - | - | free | back 8/8/11 | 0 | 40 | 2 s, -0 | panels:cracked | 4-2 (1) |  |
| 5:35 | 6 | 80-290 | 15-200 | none | - | - | free | back 5/10/20 | 0 | 150 | 16 s, -60 | - | 2-2 (1) |  |
| 5:36 | 6 | 160-340 | 60-260 | none | - | - | free | front 7/4/4 | 2 | 540 | open, -0 | tough | 3-3 (1) |  |
| 5:37 | 6 | 100-300 | 20-200 | fire | aqua | - | back | back 3/1/18 | 0 | 150 | 15 s, -0 | - | 2-2 (1) |  |
| 5:38 | 6 | 100-300 | 40-200 | elec | wood | - | back | back 0/0/3 | 1 | 200 | 19 s, -110 | - | 4-3 (1) |  |
| 5:39 | 6 | 100-330 | 30-200 | none | - | - | free | free 0/0/0 | - | 90 | open, -190 | tough | 4-3 (1) | not in the records read |
| 5:40 | 6 | 100-350 | 20-200 | aqua | elec | - | free | back 1/1/3 | 1 | 60 | 30 s, -30 | - | 4-3 (1) |  |
| 5:41 | 6 | 80-320 | 30-200 | none | - | - | free | free 0/0/0 | - | 60 | 15 s, -30 | - | 2-2 (1) | not in the records read |
| 5:42 | 6 | 130-360 | 30-200 | none | - | - | free | free 0/0/0 | - | 240 | 32 s, -30 | area | 4-2 (1) | not in the records read |
| 5:43 | 6 | 90-300 | 30-200 | none | - | - | free | free 7/5/6 | 0 | 150 | 15 s, -60 | - | 4-2 (1) |  |
| 5:44 | 6 | 80-240 | 20-200 | none | - | - | free | free 9/16/13 | 0 | 100 | 2 s, -20 | - | 3-3 (1) |  |
| 5:45 | 6 | 120-340 | 40-200 | none | - | - | free | free 12/12/11 | 1 | 160 | open, -30 | tough | 3-2 (1) |  |
| 5:46 | 6 | 120-340 | 30-220 | fire | aqua | - | free | free 3/4/3 | 1 | 210 | open, -170 | tough | 2-2 (1) |  |
| 5:47 | 6 | 90-230 | 20-250 | aqua | elec | - | free | back 8/5/11 | 0 | 60 | 42 s, -50 | - | 4-3 (1) |  |
| 5:48 | 6 | 120-320 | 30-200 | aqua | elec | - | free | back 1/2/3 | 0 | 90 | open, -80 | tough | 3-2 (1) | special panels only |
| 5:49 | 6 | 130-320 | 30-200 | elec | wood | - | free | back 1/2/2 | 2 | 90 | 19 s, -0 | - | 4-2 (1) |  |
| 5:50 | 6 | 130-350 | 20-120 | elec | wood | - | free | free 2/6/3 | 1 | 90 | 14 s, -30 | - | 3-3 (1) |  |
| 5:51 | 6 | 70-270 | 20-200 | wood | fire | - | free | free 11/21/8 | 0 | 60 | 3 s, -0 | - | 5-3 (1) |  |
| 5:52 | 6 | 130-330 | 20-200 | wood | fire | - | front | front 4/0/3 | 1 | 60 | 17 s, -0 | summons | 3-2 (1) |  |
| 5:53 | 6 | 150-330 | 30-200 | wood | fire | - | free | free 0/0/0 | - | 30 | open, -110 | panels:grass tough | 2-2 (1) | not in the records read |
| 5:54 | 6 | 120-360 | 30-200 | none | - | - | free | free 0/0/0 | - | 120 | open, -80 | tough | 8-8 (1) | not in the records read |
| 5:55 | 6 | 100-300 | 30-150 | none | - | - | free | free 0/0/0 | - | 300 | open, -110 | tough | 8-8 (1) | not in the records read |
| 5:56 | 6 | 200-400 | 30-200 | none | - | - | free | free 0/0/0 | - | 150 | open, -190 | panels:cracked tough | 8-8 (1) | not in the records read |
| 4:64 | 6 | 50-300 | 15-200 | elec | wood | - | free | free 28/32/17 | 0 | 30 | 17 s, -15 | - | 5-3 (1) |  |
| 4:65 | 6 | 60-330 | 20-250 | fire | aqua | - | free | free 28/38/42 | 0 | 90 | 16 s, -30 | - | 5-2 (1) |  |
| 4:66 | 6 | 60-280 | 20-250 | aqua | elec | - | free | front 45/24/29 | 0 | 90 | 42 s, -50 | - | 5-3 (1) |  |
| 4:67 | 6 | 100-260 | 20-200 | none | - | - | free | back 9/23/37 | 0 | 140 | 3 s, -0 | - | 3-2 (1) |  |
| 4:68 | 6 | 100-300 | 30-200 | none | - | - | free | free 4/9/3 | 1 | 210 | 2 s, -0 | - | 0-0 (0) |  |
| 4:69 | 6 | 110-300 | 30-200 | none | - | - | free | back 13/4/17 | 0 | 210 | open, -80 | tough | 0-0 (0) |  |
| 4:70 | 6 | 40-250 | 10-200 | none | - | - | free | free 0/0/0 | - | 80 | 2 s, -0 | - | 5-3 (1) | summoned only |
| 4:71 | 6 | 80-280 | 15-200 | none | - | - | free | free 0/0/0 | - | 45 | 14 s, -15 | - | 0-0 (0) | summoned only |
| 4:72 | 6 | 80-320 | 20-250 | fire | aqua | - | free | back 11/28/29 | 0 | 80 | open, -70 | tough | 2-2 (1) |  |
| 4:73 | 6 | 90-300 | 20-200 | wood | fire | - | free | back 8/6/14 | 0 | 120 | 42 s, -70 | - | 3-2 (1) |  |
| 4:74 | 6 | 90-300 | 20-200 | elec | wood | - | free | front 20/17/10 | 0 | 120 | 5 s, -0 | - | 3-2 (1) |  |

## Each ROM set's pool by act

**BN6**

| Act | Band | Cap | Families | Front | Back | Free | None | Fire | Aqua | Elec | Wood | Panels | Field or area | Summons | Tough |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | 90-160 | 30 | 11 | 1 | 4 | 6 | 6 | 1 | 2 | 0 | 2 | 0 | 1 | 0 | 0 |
| 2 | 150-280 | 80 | 15 | 1 | 6 | 8 | 7 | 2 | 3 | 1 | 2 | 0 | 2 | 0 | 0 |
| 3 | 200-360 | 120 | 22 | 1 | 8 | 13 | 10 | 4 | 3 | 3 | 2 | 0 | 2 | 0 | 2 |
| 4 | 280-420 | 160 | 22 | 1 | 8 | 13 | 10 | 4 | 3 | 3 | 2 | 0 | 2 | 0 | 2 |
| 5 | 350-500 | 200 | 27 | 1 | 10 | 16 | 13 | 3 | 5 | 3 | 3 | 1 | 3 | 0 | 4 |
| 6 | 400-580 | 240 | 27 | 1 | 10 | 16 | 13 | 3 | 5 | 3 | 3 | 1 | 3 | 0 | 4 |
| Nest | 450-650 | 280 | 27 | 1 | 10 | 16 | 13 | 3 | 5 | 3 | 3 | 1 | 3 | 0 | 4 |

**BN6+BN5**

| Act | Band | Cap | Families | Front | Back | Free | None | Fire | Aqua | Elec | Wood | Panels | Field or area | Summons | Tough |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | 90-160 | 30 | 25 | 1 | 8 | 16 | 16 | 2 | 3 | 0 | 4 | 2 | 2 | 0 | 4 |
| 2 | 150-280 | 80 | 31 | 1 | 11 | 19 | 19 | 3 | 4 | 1 | 4 | 3 | 3 | 0 | 5 |
| 3 | 200-360 | 120 | 44 | 2 | 15 | 27 | 23 | 6 | 5 | 5 | 5 | 3 | 3 | 1 | 9 |
| 4 | 280-420 | 160 | 44 | 2 | 15 | 27 | 23 | 6 | 5 | 5 | 5 | 3 | 3 | 1 | 9 |
| 5 | 350-500 | 200 | 51 | 3 | 18 | 30 | 27 | 5 | 7 | 6 | 6 | 4 | 4 | 1 | 12 |
| 6 | 400-580 | 240 | 51 | 3 | 18 | 30 | 27 | 5 | 7 | 6 | 6 | 4 | 4 | 1 | 12 |
| Nest | 450-650 | 280 | 51 | 3 | 18 | 30 | 27 | 5 | 7 | 6 | 6 | 4 | 4 | 1 | 12 |

**BN6+BN4**

| Act | Band | Cap | Families | Front | Back | Free | None | Fire | Aqua | Elec | Wood | Panels | Field or area | Summons | Tough |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | 90-160 | 30 | 19 | 3 | 8 | 8 | 8 | 3 | 3 | 2 | 3 | 0 | 1 | 0 | 2 |
| 2 | 150-280 | 80 | 23 | 3 | 10 | 10 | 9 | 4 | 4 | 3 | 3 | 0 | 2 | 0 | 2 |
| 3 | 200-360 | 120 | 31 | 3 | 12 | 16 | 13 | 6 | 4 | 5 | 3 | 0 | 2 | 0 | 4 |
| 4 | 280-420 | 160 | 31 | 3 | 12 | 16 | 13 | 6 | 4 | 5 | 3 | 0 | 2 | 0 | 4 |
| 5 | 350-500 | 200 | 36 | 3 | 14 | 19 | 16 | 5 | 6 | 5 | 4 | 1 | 3 | 0 | 6 |
| 6 | 400-580 | 240 | 36 | 3 | 14 | 19 | 16 | 5 | 6 | 5 | 4 | 1 | 3 | 0 | 6 |
| Nest | 450-650 | 280 | 36 | 3 | 14 | 19 | 16 | 5 | 6 | 5 | 4 | 1 | 3 | 0 | 6 |

**BN6+BN5+BN4**

| Act | Band | Cap | Families | Front | Back | Free | None | Fire | Aqua | Elec | Wood | Panels | Field or area | Summons | Tough |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | 90-160 | 30 | 33 | 3 | 12 | 18 | 18 | 4 | 4 | 2 | 5 | 2 | 2 | 0 | 6 |
| 2 | 150-280 | 80 | 39 | 3 | 15 | 21 | 21 | 5 | 5 | 3 | 5 | 3 | 3 | 0 | 7 |
| 3 | 200-360 | 120 | 53 | 4 | 19 | 30 | 26 | 8 | 6 | 7 | 6 | 3 | 3 | 1 | 11 |
| 4 | 280-420 | 160 | 53 | 4 | 19 | 30 | 26 | 8 | 6 | 7 | 6 | 3 | 3 | 1 | 11 |
| 5 | 350-500 | 200 | 60 | 5 | 22 | 33 | 30 | 7 | 8 | 8 | 7 | 4 | 4 | 1 | 14 |
| 6 | 400-580 | 240 | 60 | 5 | 22 | 33 | 30 | 7 | 8 | 8 | 7 | 4 | 4 | 1 | 14 |
| Nest | 450-650 | 280 | 60 | 5 | 22 | 33 | 30 | 7 | 8 | 8 | 7 | 4 | 4 | 1 | 14 |

## The templates, and how many ways each fills

**BN6**

| Template | Act 1 | Act 2 | Act 3 | Act 4 | Act 5 | Act 6 | Nest |
| --- | --- | --- | --- | --- | --- | --- | --- |
| wall and artillery | 2 [2-2] | 6 [1-6] | 24 [3-22] | 19 [3-18] | 36 | 26 | 16 |
| panel war | **none** | **none** | **none** | **none** | 14 | 8 | 4 |
| summoner and guard | **none** | **none** | **none** | **none** | **none** | **none** | **none** |
| clashing answers | 2 [2-2] | 11 [0-10] | 35 [0-35] | 20 [2-20] | 52 | 30 | 8 |
| one element | 1 [0-1] | 2 [0-2] | 13 [0-12] | 7 [1-6] | 25 | 14 | 4 |
| pincer | **none** | 24 [2-24] | 215 [37-211] | 280 [74-276] | 748 | 750 | 727 |
| swarm | 1 [1-1] | 8 [0-6] | 14 [0-9] | 19 [2-14] | 23 | 20 | 23 |
| held ground | 2 [1-2] | 11 [2-10] | 15 [2-15] | 11 [7-11] | 23 | 15 | 2 |
| glass cannons | **none** | 1 [0-1] | 7 [0-6] | **none** | 9 | **none** | **none** |
| bulwark | **none** | 3 [0-3] | 19 [3-17] | 8 [0-8] | 5 | **none** | **none** |
| nets meet | **none** | **none** | **none** | **none** | **none** | **none** | **none** |

**BN6+BN5**

| Template | Act 1 | Act 2 | Act 3 | Act 4 | Act 5 | Act 6 | Nest |
| --- | --- | --- | --- | --- | --- | --- | --- |
| wall and artillery | 9 (5) [5-6] | 60 (31) [6-26] | 165 (85) [11-84] | 160 (85) [11-80] | 236 (128) | 221 (123) | 199 (111) |
| panel war | 12 (5) [4-4] | 43 (19) [0-15] | 78 (38) [0-29] | 78 (38) [5-29] | 116 (62) | 110 (62) | 103 (59) |
| summoner and guard | **none** | **none** | 10 (3) [0-2] | 10 (3) [0-2] | 13 (5) | 13 (5) | 12 (4) |
| clashing answers | 4 (2) [3-3] | 28 (14) [0-18] | 110 (55) [0-80] | 95 (55) [7-65] | 156 (79) | 130 (75) | 106 (73) |
| one element | 3 (2) [0-3] | 11 (8) [0-8] | 45 (26) [0-34] | 39 (26) [3-28] | 74 (41) | 61 (39) | 48 (36) |
| pincer | **none** | 414 (314) [6-293] | 3255 (2453) [281-2181] | 4107 (3113) [554-2830] | 7326 (5626) | 7346 (5644) | 7322 (5643) |
| swarm | 1 [1-1] | 17 [0-6] | 31 [0-9] | 39 [2-14] | 46 | 43 | 47 |
| held ground | 3 (1) [1-3] | 32 (16) [3-20] | 44 (22) [3-35] | 40 (22) [15-31] | 65 (34) | 55 (32) | 39 (29) |
| glass cannons | **none** | 5 (4) [0-4] | 23 (16) [0-20] | 26 (23) [0-23] | 51 (35) | 30 (19) | 1 |
| bulwark | **none** | 10 (6) [0-8] | 48 (25) [3-36] | 28 (18) [0-26] | 15 (7) | 7 (4) | 1 |
| nets meet | 23 (23) [18-21] | 84 (84) [0-64] | 181 (181) [0-139] | 179 (179) [25-137] | 224 (224) | 221 (221) | 205 (205) |

**BN6+BN4**

| Template | Act 1 | Act 2 | Act 3 | Act 4 | Act 5 | Act 6 | Nest |
| --- | --- | --- | --- | --- | --- | --- | --- |
| wall and artillery | 11 (6) [5-8] | 48 (28) [5-25] | 82 (44) [7-54] | 77 (44) [7-50] | 109 (59) | 94 (54) | 78 (48) |
| panel war | **none** | **none** | **none** | **none** | 17 (3) | 11 (3) | 7 (3) |
| summoner and guard | **none** | **none** | **none** | **none** | **none** | **none** | **none** |
| clashing answers | 17 (9) [3-10] | 45 (26) [0-28] | 77 (34) [0-65] | 62 (34) [4-50] | 108 (48) | 85 (47) | 62 (46) |
| one element | 7 (4) [2-4] | 15 (11) [0-9] | 34 (19) [0-27] | 28 (19) [3-21] | 53 (26) | 40 (24) | 30 (24) |
| pincer | 2 (2) [2-2] | 338 (286) [34-263] | 1123 (866) [159-961] | 1268 (946) [255-1106] | 2289 (1499) | 2291 (1499) | 2268 (1499) |
| swarm | 2 [1-1] | 14 [0-6] | 23 [0-9] | 28 [2-14] | 32 | 29 | 32 |
| held ground | 2 [1-2] | 19 (8) [2-14] | 23 (8) [2-23] | 19 (8) [11-19] | 35 (12) | 27 (12) | 13 (11) |
| glass cannons | **none** | 1 [0-1] | 16 (9) [0-14] | 19 (19) [0-19] | 28 (18) | 15 (13) | **none** |
| bulwark | **none** | 5 (2) [0-5] | 26 (7) [3-24] | 8 [0-8] | 5 | **none** | **none** |
| nets meet | 10 (10) [8-8] | 15 (15) [0-11] | 39 (39) [0-30] | 39 (39) [6-30] | 48 (48) | 48 (48) | 46 (46) |

**BN6+BN5+BN4**

| Template | Act 1 | Act 2 | Act 3 | Act 4 | Act 5 | Act 6 | Nest |
| --- | --- | --- | --- | --- | --- | --- | --- |
| wall and artillery | 25 (18) [8-12] | 139 (96) [10-46] | 283 (189) [15-116] | 278 (189) [15-112] | 377 (255) | 357 (245) | 329 (227) |
| panel war | 14 (7) [4-4] | 49 (25) [0-15] | 87 (47) [0-29] | 87 (47) [5-29] | 128 (74) | 122 (74) | 115 (71) |
| summoner and guard | **none** | **none** | 14 (7) [0-2] | 14 (7) [0-2] | 17 (9) | 17 (9) | 16 (8) |
| clashing answers | 26 (18) [4-11] | 76 (54) [0-35] | 180 (117) [0-110] | 165 (117) [9-95] | 242 (157) | 215 (152) | 190 (149) |
| one element | 12 (9) [2-5] | 29 (24) [0-15] | 79 (58) [0-49] | 73 (58) [5-43] | 117 (82) | 102 (78) | 89 (75) |
| pincer | 3 (3) [3-3] | 1460 (1332) [53-800] | 6826 (5982) [534-3982] | 8052 (7016) [933-4866] | 12723 (10981) | 12745 (11001) | 12721 (11000) |
| swarm | 2 [1-1] | 23 [0-6] | 40 [0-9] | 48 [2-14] | 55 | 52 | 56 |
| held ground | 3 (1) [1-3] | 44 (28) [3-24] | 56 (34) [3-43] | 52 (34) [19-39] | 81 (50) | 71 (48) | 54 (44) |
| glass cannons | **none** | 5 (4) [0-4] | 33 (26) [0-28] | 52 (49) [0-42] | 77 (60) | 54 (41) | 3 (2) |
| bulwark | **none** | 12 (8) [0-10] | 55 (32) [3-43] | 28 (18) [0-26] | 16 (8) | 8 (5) | 1 |
| nets meet | 47 (47) [26-29] | 121 (121) [0-75] | 262 (262) [0-169] | 260 (260) [31-167] | 314 (314) | 311 (311) | 293 (293) |

## Families that can carry an act

| ROM set | Act 1 | Act 2 | Act 3 | Act 4 | Act 5 | Act 6 | Nest |
| --- | --- | --- | --- | --- | --- | --- | --- |
| BN6 | 1 of 11 | 11 of 15 | 18 of 22 | 19 of 22 | 24 of 27 | 23 of 27 | 15 of 27 |
| BN6+BN5 | 9 of 25 | 30 of 31 | 44 of 44 | 44 of 44 | 51 of 51 | 51 of 51 | 51 of 51 |
| BN6+BN4 | 10 of 19 | 21 of 23 | 31 of 31 | 30 of 31 | 35 of 36 | 33 of 36 | 32 of 36 |
| BN6+BN5+BN4 | 18 of 33 | 38 of 39 | 53 of 53 | 53 of 53 | 60 of 60 | 60 of 60 | 60 of 60 |

## Danger

| Act | Battles | Sampled | Pressure: least / median / most | Median, of the act's max HP | Median measured / predicted | Autopilot won in 50 s | Autopilot HP lost, median | ... most | Lost the act's max HP | Past the own battles' most |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | own | 5 | 60 / 80 / 120 | 80% | 0.50 | 4 of 5 | 0 | 90 | 0 | - |
| 2 | own | 5 | 50 / 120 / 180 | 60% | 0.50 | 1 of 5 | 90 | 160 | 0 | - |
| 2 | bulwark | 2 | 100 / 105 / 110 | 52% | 0.75 | 0 of 2 | 105 | 140 | 0 | 0 |
| 2 | clashing answers | 2 | 140 / 225 / 310 | 112% | 0.92 | 0 of 2 | 115 | 130 | 0 | 1 |
| 2 | glass cannons | 2 | 50 / 150 / 250 | 75% | 0.63 | 1 of 2 | 240 | 240 | 2 | 1 |
| 2 | held ground | 2 | 230 / 295 / 360 | 148% | 0.67 | 0 of 2 | 355 | 370 | 2 | 2 |
| 2 | nets meet | 2 | 70 / 105 / 140 | 52% | 0.37 | 1 of 2 | 115 | 230 | 1 | 0 |
| 2 | one element | 2 | 160 / 205 / 250 | 102% | 0.64 | 1 of 2 | 175 | 210 | 1 | 1 |
| 2 | panel war | 2 | 180 / 185 / 190 | 92% | 0.97 | 1 of 2 | 45 | 60 | 0 | 1 |
| 2 | pincer | 2 | 170 / 180 / 190 | 90% | 0.47 | 0 of 2 | 140 | 140 | 0 | 1 |
| 2 | swarm | 2 | 120 / 180 / 240 | 90% | 0.56 | 0 of 2 | 220 | 360 | 1 | 1 |
| 2 | wall and artillery | 2 | 120 / 150 / 180 | 75% | 0.66 | 0 of 2 | 258 | 455 | 1 | 0 |
| 3 | own | 5 | 80 / 120 / 230 | 40% | 0.36 | 2 of 5 | 90 | 140 | 0 | - |
| 3 | bulwark | 2 | 400 / 400 / 400 | 133% | 0.83 | 0 of 2 | 450 | 450 | 2 | 2 |
| 3 | clashing answers | 2 | 60 / 240 / 420 | 80% | 0.67 | 0 of 2 | 450 | 540 | 2 | 1 |
| 3 | glass cannons | 2 | 0 / 175 / 350 | 58% | 0.24 | 0 of 2 | 120 | 150 | 0 | 1 |
| 3 | held ground | 2 | 680 / 890 / 1100 | 297% | 0.74 | 0 of 2 | 715 | 840 | 2 | 2 |
| 3 | nets meet | 2 | 450 / 535 / 620 | 178% | 1.00 | 0 of 2 | 250 | 410 | 1 | 2 |
| 3 | one element | 2 | 540 / 660 / 780 | 220% | 0.75 | 0 of 2 | 690 | 790 | 2 | 2 |
| 3 | panel war | 2 | 290 / 365 / 440 | 122% | 0.94 | 0 of 2 | 255 | 330 | 1 | 2 |
| 3 | pincer | 2 | 150 / 210 / 270 | 70% | 0.42 | 1 of 2 | 175 | 200 | 0 | 1 |
| 3 | summoner and guard | 2 | 480 / 550 / 620 | 183% | 0.89 | 0 of 2 | 645 | 650 | 2 | 2 |
| 3 | swarm | 2 | 80 / 100 / 120 | 33% | 0.43 | 1 of 2 | 65 | 80 | 0 | 0 |
| 3 | wall and artillery | 2 | 380 / 395 / 410 | 132% | 0.54 | 0 of 2 | 475 | 540 | 2 | 2 |
| 4 | own | 5 | 140 / 230 / 720 | 58% | 0.61 | 0 of 5 | 250 | 340 | 0 | - |
| 4 | bulwark | 2 | 540 / 550 / 560 | 138% | 1.17 | 0 of 2 | 655 | 970 | 1 | 0 |
| 4 | clashing answers | 2 | 360 / 480 / 600 | 120% | 1.02 | 0 of 2 | 630 | 830 | 2 | 0 |
| 4 | glass cannons | 2 | 0 / 135 / 270 | 34% | 0.17 | 1 of 2 | 65 | 110 | 0 | 0 |
| 4 | held ground | 2 | 520 / 710 / 900 | 178% | 0.77 | 0 of 2 | 805 | 940 | 2 | 1 |
| 4 | nets meet | 2 | 520 / 650 / 780 | 162% | 0.99 | 0 of 2 | 530 | 860 | 1 | 1 |
| 4 | one element | 2 | 270 / 405 / 540 | 101% | 0.75 | 0 of 2 | 760 | 810 | 2 | 0 |
| 4 | panel war | 2 | 240 / 450 / 660 | 112% | 1.55 | 0 of 2 | 220 | 280 | 0 | 0 |
| 4 | pincer | 2 | 480 / 515 / 550 | 129% | 0.56 | 0 of 2 | 540 | 680 | 2 | 0 |
| 4 | summoner and guard | 2 | 540 / 650 / 760 | 162% | 0.97 | 0 of 2 | 655 | 880 | 2 | 1 |
| 4 | swarm | 2 | 155 / 252 / 350 | 63% | 0.41 | 0 of 2 | 238 | 240 | 0 | 0 |
| 4 | wall and artillery | 2 | 630 / 765 / 900 | 191% | 0.61 | 0 of 2 | 400 | 510 | 1 | 1 |
| 5 | own | 5 | 210 / 840 / 1010 | 168% | 0.55 | 1 of 5 | 550 | 1080 | 3 | - |
| 5 | bulwark | 2 | 720 / 740 / 760 | 148% | 0.75 | 0 of 2 | 1100 | 1230 | 2 | 0 |
| 5 | clashing answers | 2 | 1120 / 1260 / 1400 | 252% | 1.08 | 0 of 2 | 1875 | 2000 | 2 | 2 |
| 5 | glass cannons | 2 | 600 / 600 / 600 | 120% | 0.65 | 0 of 2 | 740 | 1040 | 1 | 0 |
| 5 | held ground | 2 | 480 / 740 / 1000 | 148% | 0.91 | 0 of 2 | 1015 | 1240 | 2 | 0 |
| 5 | nets meet | 2 | 240 / 480 / 720 | 96% | 0.48 | 0 of 2 | 755 | 1310 | 1 | 0 |
| 5 | one element | 2 | 720 / 790 / 860 | 158% | 1.11 | 0 of 2 | 1070 | 1150 | 2 | 0 |
| 5 | panel war | 2 | 430 / 535 / 640 | 107% | 0.59 | 0 of 2 | 1100 | 1130 | 2 | 0 |
| 5 | pincer | 2 | 360 / 510 / 660 | 102% | 0.69 | 0 of 2 | 490 | 600 | 1 | 0 |
| 5 | summoner and guard | 2 | 730 / 865 / 1000 | 173% | 0.86 | 0 of 2 | 1090 | 1250 | 2 | 0 |
| 5 | swarm | 2 | 0 / 200 / 400 | 40% | 0.33 | 0 of 2 | 115 | 230 | 0 | 0 |
| 5 | wall and artillery | 2 | 800 / 1275 / 1750 | 255% | 1.27 | 0 of 2 | 1220 | 1300 | 2 | 1 |
| 6 | own | 5 | 0 / 320 / 780 | 53% | 0.62 | 0 of 5 | 500 | 680 | 1 | - |

### The pairs' pass

1830 pairs, every two families of the pool once, each at V1 where its lean puts it, MegaMan standing: the HP he loses in 20 seconds against the sum of the two families' own pressures. The median pair presses 0.71 of its sum (-60 HP); 27 press past 1.25 of it, and the most any presses past it is 120 HP. A pair 40 HP or more past its sum is tagged (27 pairs): a group that holds it is predicted that much harder, by the larger of the two families' hits at the version fought over their V1's.

| Pair | Together | Their own, added | Past the sum |
| --- | --- | --- | --- |
| 6:17 + 6:21 | 210 | 90 | +120 |
| 6:05 + 6:21 | 220 | 100 | +120 |
| 6:02 + 6:21 | 140 | 20 | +120 |
| 6:18 + 6:25 | 660 | 560 | +100 |
| 6:25 + 5:49 | 620 | 530 | +90 |
| 6:19 + 6:25 | 530 | 440 | +90 |
| 6:19 + 5:43 | 240 | 150 | +90 |
| 6:21 + 6:28 | 230 | 150 | +80 |
| 6:12 + 6:21 | 280 | 200 | +80 |
| 6:15 + 5:49 | 480 | 410 | +70 |
| 6:22 + 5:35 | 210 | 150 | +60 |
| 6:21 + 5:53 | 90 | 30 | +60 |
| 6:20 + 6:21 | 240 | 180 | +60 |
| 6:19 + 6:21 | 60 | 0 | +60 |
| 6:19 + 6:20 | 240 | 180 | +60 |
| 6:15 + 6:18 | 500 | 440 | +60 |
| 6:15 + 5:38 | 580 | 520 | +60 |
| 6:13 + 5:35 | 210 | 150 | +60 |
| 6:09 + 5:35 | 210 | 150 | +60 |
| 6:05 + 5:36 | 700 | 640 | +60 |
| 6:01 + 6:21 | 130 | 70 | +60 |
| 6:18 + 5:54 | 280 | 240 | +40 |
| 6:16 + 6:21 | 340 | 300 | +40 |
| 6:16 + 6:18 | 460 | 420 | +40 |
| 6:06 + 6:21 | 160 | 120 | +40 |
| 6:02 + 6:12 | 260 | 220 | +40 |
| 6:02 + 5:34 | 100 | 60 | +40 |

### A cap on pressure

| Act | Own battles of the act's areas | Predicted pressure: least | Median | Three-quarter mark (the cap) | Most | MegaMan's max HP there |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 74 | 40 | 160 | 220 | 390 | 100 |
| 2 | 220 | 60 | 240 | 360 | 960 | 200 |
| 3 | 243 | 80 | 420 | 620 | 1920 | 300 |
| 4 | 84 | 120 | 480 | 720 | 1420 | 400 |
| 5 | 37 | 120 | 700 | 880 | 1720 | 500 |
| 6 | 47 | 0 | 712 | 1040 | 3680 | 600 |
| Nest | 18 | 535 | 840 | 945 | 1560 | 700 |

**BN6, under the cap**

| Template | Act 1 | Act 2 | Act 3 | Act 4 | Act 5 | Act 6 | Nest |
| --- | --- | --- | --- | --- | --- | --- | --- |
| wall and artillery | **none** | 3 | 21 | 16 | 24 | 19 | 8 |
| panel war | **none** | **none** | **none** | **none** | 11 | 7 | 2 |
| summoner and guard | **none** | **none** | **none** | **none** | **none** | **none** | **none** |
| clashing answers | 1 | 10 | 32 | 18 | 43 | 26 | 7 |
| one element | **none** | 2 | 11 | 7 | 20 | 11 | 3 |
| pincer | **none** | 8 | 180 | 244 | 716 | 714 | 611 |
| swarm | 1 | 5 | 12 | 16 | 19 | 15 | 15 |
| held ground | **none** | 7 | 13 | 9 | 18 | 12 | 1 |
| glass cannons | **none** | 1 | 6 | **none** | 7 | **none** | **none** |
| bulwark | **none** | 3 | 16 | 8 | 5 | **none** | **none** |
| nets meet | **none** | **none** | **none** | **none** | **none** | **none** | **none** |

**BN6+BN5+BN4, under the cap**

| Template | Act 1 | Act 2 | Act 3 | Act 4 | Act 5 | Act 6 | Nest |
| --- | --- | --- | --- | --- | --- | --- | --- |
| wall and artillery | 11 | 114 | 272 | 222 | 252 | 230 | 121 |
| panel war | 13 | 49 | 87 | 87 | 120 | 114 | 88 |
| summoner and guard | **none** | **none** | 14 | 14 | 17 | 17 | 11 |
| clashing answers | 19 | 69 | 171 | 147 | 197 | 170 | 100 |
| one element | 8 | 26 | 76 | 62 | 89 | 77 | 43 |
| pincer | 1 | 816 | 6407 | 7678 | 12215 | 12195 | 10174 |
| swarm | 2 | 17 | 32 | 40 | 38 | 28 | 28 |
| held ground | 1 | 30 | 53 | 35 | 51 | 39 | 14 |
| glass cannons | **none** | 5 | 22 | 27 | 31 | 15 | **none** |
| bulwark | **none** | 12 | 50 | 27 | 11 | 4 | **none** |
| nets meet | 40 | 109 | 256 | 223 | 248 | 231 | 140 |

### Every battle sampled

| Act | Battles | Families (version index from 1) | HP | Hit | Pressure | Predicted | Ratio | Autopilot (per seed) |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | own | 6:02 v1 + 6:02 v1 | 140 | 20 | 100 | 40 | 2.50 | won 15s -0 |
| 1 | own | 6:06 v1 + 6:06 v1 | 160 | 10 | 120 | 240 | 0.50 | open 49s -90 |
| 1 | own | 6:23 v1 + 6:23 v1 + 6:01 v1 | 160 | 10 | 70 | 230 | 0.30 | won 20s -0 |
| 1 | own | 6:23 v1 + 6:11 v1 | 140 | 10 | 60 | 140 | 0.43 | won 14s -0 |
| 1 | own | 6:02 v1 + 6:02 v1 | 140 | 20 | 80 | 40 | 2.00 | won 14s -0 |
| 2 | own | 6:04 v1 + 6:04 v1 | 180 | 30 | 180 | 360 | 0.50 | won 39s -0 |
| 2 | own | 6:06 v1 + 6:06 v1 | 160 | 10 | 120 | 240 | 0.50 | open 49s -20 |
| 2 | own | 6:22 v1 + 6:20 v1 | 270 | 20 | 180 | 180 | 1.00 | open 50s -125 |
| 2 | own | 6:01 v1 + 6:01 v1 + 6:22 v1 | 210 | 10 | 50 | 140 | 0.36 | open 50s -160 |
| 2 | own | 6:06 v1 + 6:06 v1 | 160 | 10 | 120 | 240 | 0.50 | open 49s -90 |
| 2 | bulwark | 5:56 v1 + 6:14 v1 | 280 | 80 | 100 | 200 | 0.50 | open 49s -140 |
| 2 | bulwark | 4:64 v2 + 6:14 v1 | 200 | 80 | 110 | 110 | 1.00 | open 50s -70 |
| 2 | clashing answers | 6:20 v1 + 5:37 v2 | 280 | 50 | 310 | 555 | 0.56 | open 49s -100 |
| 2 | clashing answers | 6:14 v1 + 4:64 v2 | 200 | 80 | 140 | 110 | 1.27 | open 49s -130 |
| 2 | glass cannons | 6:14 v1 + 5:41 v2 | 210 | 80 | 50 | 170 | 0.29 | open 49s -240 |
| 2 | glass cannons | 6:14 v1 + 5:33 v2 | 180 | 80 | 250 | 260 | 0.96 | won 42s -240 |
| 2 | held ground | 6:04 v1 + 4:73 v2 | 220 | 40 | 230 | 420 | 0.55 | open 50s -340 |
| 2 | held ground | 6:07 v1 + 4:73 v2 | 260 | 50 | 360 | 450 | 0.80 | open 49s -370 |
| 2 | nets meet | 6:01 v1 + 5:56 v2 | 270 | 60 | 140 | 370 | 0.38 | open 49s -230 |
| 2 | nets meet | 6:01 v1 + 5:51 v2 | 150 | 40 | 70 | 190 | 0.37 | won 42s -0 |
| 2 | one element | 6:08 v1 + 4:72 v2 | 180 | 50 | 250 | 360 | 0.69 | open 50s -210 |
| 2 | one element | 6:14 v1 + 4:66 v2 | 180 | 80 | 160 | 275 | 0.58 | won 42s -140 |
| 2 | panel war | 5:34 v2 + 6:04 v1 | 190 | 40 | 190 | 260 | 0.73 | won 43s -60 |
| 2 | panel war | 5:53 v2 + 6:17 v1 | 280 | 80 | 180 | 150 | 1.20 | open 50s -30 |
| 2 | pincer | 4:66 v1 + 5:47 v1 + 5:43 v2 | 280 | 60 | 190 | 450 | 0.42 | open 49s -140 |
| 2 | pincer | 5:39 v1 + 5:47 v1 + 6:04 v1 | 280 | 30 | 170 | 330 | 0.52 | open 49s -140 |
| 2 | swarm | 4:73 v1 + 4:73 v1 + 4:73 v1 | 270 | 20 | 240 | 360 | 0.67 | open 49s -360 |
| 2 | swarm | 4:66 v1 + 4:66 v1 + 4:66 v1 | 180 | 20 | 120 | 270 | 0.44 | open 50s -80 |
| 2 | wall and artillery | 5:53 v2 + 6:23 v1 | 240 | 60 | 120 | 140 | 0.86 | open 50s -60 |
| 2 | wall and artillery | 5:39 v2 + 6:07 v1 | 270 | 60 | 180 | 390 | 0.46 | open 49s -455 |
| 3 | own | 6:06 v2 + 6:02 v2 + 6:02 v2 | 350 | 40 | 230 | 320 | 0.72 | open 49s -140 |
| 3 | own | 6:11 v2 + 6:11 v2 | 240 | 20 | 80 | 240 | 0.33 | won 49s -60 |
| 3 | own | 6:17 v1 + 6:17 v1 + 6:24 v1 | 340 | 80 | 100 | 280 | 0.36 | open 50s -120 |
| 3 | own | 6:01 v2 + 6:01 v2 + 6:22 v2 | 330 | 20 | 210 | 280 | 0.75 | open 50s -90 |
| 3 | own | 6:01 v2 + 6:01 v2 + 6:01 v2 | 240 | 20 | 120 | 420 | 0.29 | won 42s -50 |
| 3 | bulwark | 5:50 v3 + 6:17 v2 | 350 | 120 | 400 | 405 | 0.99 | open 49s -450 |
| 3 | bulwark | 5:50 v3 + 6:08 v2 | 330 | 100 | 400 | 590 | 0.68 | open 49s -450 |
| 3 | clashing answers | 6:11 v1 + 5:53 v4 | 330 | 120 | 60 | 180 | 0.33 | open 48s -360 |
| 3 | clashing answers | 6:12 v2 + 5:52 v2 | 360 | 80 | 420 | 417 | 1.01 | open 50s -540 |
| 3 | glass cannons | 6:08 v2 + 4:72 v3 | 280 | 100 | 350 | 720 | 0.49 | open 50s -150 |
| 3 | glass cannons | 6:10 v2 + 5:32 v4 | 280 | 120 | 0 | 600 | 0.00 | open 49s -90 |
| 3 | held ground | 6:07 v2 + 5:35 v4 | 350 | 100 | 1100 | 1420 | 0.77 | open 49s -840 |
| 3 | held ground | 6:04 v2 + 4:73 v4 | 350 | 100 | 680 | 960 | 0.71 | open 49s -590 |
| 3 | nets meet | 5:46 v2 + 4:64 v4 | 360 | 100 | 620 | 620 | 1.00 | open 49s -410 |
| 3 | nets meet | 5:53 v3 + 4:65 v3 | 360 | 90 | 450 | 450 | 1.00 | open 50s -90 |
| 3 | one element | 5:51 v3 + 4:73 v4 | 360 | 100 | 780 | 810 | 0.96 | open 50s -790 |
| 3 | one element | 5:38 v2 + 4:74 v4 | 330 | 120 | 540 | 1020 | 0.53 | open 49s -590 |
| 3 | panel war | 5:53 v3 + 6:12 v1 | 360 | 90 | 290 | 290 | 1.00 | open 49s -180 |
| 3 | panel war | 5:53 v2 + 6:25 v1 | 360 | 60 | 440 | 500 | 0.88 | open 49s -330 |
| 3 | pincer | 4:66 v4 + 6:14 v1 + 6:01 v2 | 340 | 120 | 270 | 730 | 0.37 | won 31s -150 |
| 3 | pincer | 5:56 v1 + 6:23 v1 + 6:17 v1 | 360 | 80 | 150 | 320 | 0.47 | open 49s -200 |
| 3 | summoner and guard | 5:52 v3 + 4:72 v3 | 350 | 100 | 620 | 640 | 0.97 | open 49s -650 |
| 3 | summoner and guard | 5:52 v2 + 6:25 v1 | 340 | 50 | 480 | 590 | 0.81 | open 49s -640 |
| 3 | swarm | 5:54 v1 + 5:54 v1 + 5:54 v1 | 360 | 30 | 120 | 360 | 0.33 | open 50s -80 |
| 3 | swarm | 6:14 v1 + 6:14 v1 + 6:14 v1 | 240 | 80 | 80 | 150 | 0.53 | won 36s -50 |
| 3 | wall and artillery | 6:27 v2 + 5:34 v4 | 320 | 110 | 410 | 720 | 0.57 | open 49s -410 |
| 3 | wall and artillery | 6:27 v2 + 5:47 v3 | 270 | 80 | 380 | 740 | 0.51 | open 50s -540 |
| 4 | own | 6:18 v2 + 6:07 v2 | 320 | 100 | 720 | 600 | 1.20 | open 48s -270 |
| 4 | own | 6:07 v2 + 6:01 v2 + 6:01 v2 | 320 | 100 | 430 | 700 | 0.61 | open 49s -340 |
| 4 | own | 6:24 v1 + 6:24 v1 + 6:24 v1 | 420 | 20 | 140 | 300 | 0.47 | open 49s -150 |
| 4 | own | 6:11 v2 + 6:14 v2 + 6:14 v2 | 360 | 140 | 230 | 295 | 0.78 | open 50s -250 |
| 4 | own | 6:04 v1 + 6:07 v1 + 6:07 v1 | 350 | 50 | 180 | 600 | 0.30 | open 50s -125 |
| 4 | bulwark | 5:50 v4 + 6:14 v2 | 370 | 140 | 540 | 448 | 1.21 | open 50s -340 |
| 4 | bulwark | 5:50 v4 + 6:17 v2 | 390 | 120 | 560 | 495 | 1.13 | open 50s -970 |
| 4 | clashing answers | 6:11 v2 + 4:66 v4 | 300 | 120 | 360 | 660 | 0.55 | open 49s -430 |
| 4 | clashing answers | 6:02 v2 + 5:50 v4 | 370 | 80 | 600 | 400 | 1.50 | open 49s -830 |
| 4 | glass cannons | 6:14 v2 + 4:74 v4 | 320 | 140 | 270 | 808 | 0.33 | won 31s -20 |
| 4 | glass cannons | 6:10 v2 + 4:67 v4 | 290 | 120 | 0 | 840 | 0.00 | open 50s -110 |
| 4 | held ground | 6:07 v2 + 4:73 v4 | 370 | 100 | 900 | 1020 | 0.88 | open 49s -940 |
| 4 | held ground | 6:07 v2 + 5:33 v4 | 310 | 120 | 520 | 780 | 0.67 | open 49s -670 |
| 4 | nets meet | 5:55 v4 + 4:65 v4 | 400 | 120 | 780 | 1440 | 0.54 | open 49s -860 |
| 4 | nets meet | 6:26 v2 + 5:53 v4 | 420 | 120 | 520 | 360 | 1.44 | open 49s -200 |
| 4 | one element | 6:14 v2 + 4:66 v4 | 300 | 140 | 270 | 628 | 0.43 | open 50s -710 |
| 4 | one element | 6:14 v2 + 5:47 v4 | 290 | 140 | 540 | 508 | 1.06 | open 50s -810 |
| 4 | panel war | 5:53 v3 + 4:64 v4 | 410 | 100 | 660 | 290 | 2.28 | open 48s -280 |
| 4 | panel war | 5:53 v3 + 6:24 v2 | 390 | 90 | 240 | 290 | 0.83 | open 50s -160 |
| 4 | pincer | 4:66 v1 + 5:38 v3 + 5:51 v4 | 420 | 100 | 480 | 790 | 0.61 | open 49s -680 |
| 4 | pincer | 4:69 v1 + 5:37 v1 + 5:46 v3 | 410 | 100 | 550 | 1060 | 0.52 | open 49s -400 |
| 4 | summoner and guard | 5:52 v2 + 6:25 v2 | 400 | 100 | 540 | 1030 | 0.52 | open 50s -880 |
| 4 | summoner and guard | 5:52 v4 + 6:18 v2 | 380 | 120 | 760 | 540 | 1.41 | open 49s -430 |
| 4 | swarm | 4:72 v2 + 4:72 v2 + 4:72 v2 | 360 | 50 | 350 | 600 | 0.58 | open 49s -240 |
| 4 | swarm | 6:07 v1 + 6:07 v1 + 6:07 v1 | 390 | 50 | 155 | 630 | 0.25 | open 49s -235 |
| 4 | wall and artillery | 4:69 v4 + 5:33 v4 | 370 | 120 | 630 | 1200 | 0.53 | open 50s -510 |
| 4 | wall and artillery | 5:46 v3 + 4:73 v4 | 410 | 100 | 900 | 1300 | 0.69 | open 50s -290 |
| 5 | own | 6:27 v3 + 6:08 v3 + 6:10 v3 | 470 | 150 | 840 | 1680 | 0.50 | open 49s -890 |
| 5 | own | 6:06 v3 + 6:18 v3 | 400 | 120 | 950 | 720 | 1.32 | open 50s -1080 |
| 5 | own | 6:26 v2 + 6:23 v2 + 6:23 v2 | 450 | 80 | 310 | 560 | 0.55 | open 50s -340 |
| 5 | own | 6:23 v2 + 6:23 v2 + 6:23 v2 | 420 | 20 | 210 | 480 | 0.44 | won 42s -50 |
| 5 | own | 6:01 v3 + 6:18 v3 + 6:27 v3 | 480 | 120 | 1010 | 1720 | 0.59 | open 50s -550 |
| 5 | bulwark | 5:50 v5 + 6:08 v3 | 480 | 150 | 760 | 930 | 0.82 | open 49s -1230 |
| 5 | bulwark | 5:50 v5 + 4:72 v4 | 500 | 150 | 720 | 1050 | 0.69 | open 49s -970 |
| 5 | clashing answers | 6:22 v2 + 5:38 v6 | 470 | 200 | 1400 | 1000 | 1.40 | open 39s -2000 |
| 5 | clashing answers | 6:12 v2 + 4:73 v6 | 500 | 200 | 1120 | 1467 | 0.76 | open 49s -1750 |
| 5 | glass cannons | 6:10 v3 + 4:66 v5 | 360 | 180 | 600 | 810 | 0.74 | open 49s -440 |
| 5 | glass cannons | 6:08 v3 + 5:33 v6 | 400 | 200 | 600 | 1080 | 0.56 | open 49s -1040 |
| 5 | held ground | 6:07 v3 + 5:47 v5 | 420 | 200 | 1000 | 1230 | 0.81 | open 50s -1240 |
| 5 | held ground | 6:21 v3 + 5:49 v5 | 490 | 160 | 480 | 480 | 1.00 | open 50s -790 |
| 5 | nets meet | 6:17 v3 + 5:51 v6 | 450 | 200 | 240 | 802 | 0.30 | open 49s -200 |
| 5 | nets meet | 6:28 v2 + 5:56 v5 | 500 | 160 | 720 | 1100 | 0.65 | open 49s -1310 |
| 5 | one element | 5:50 v4 + 4:64 v5 | 500 | 150 | 860 | 660 | 1.30 | open 50s -1150 |
| 5 | one element | 6:18 v2 + 5:49 v6 | 480 | 200 | 720 | 780 | 0.92 | open 50s -990 |
| 5 | panel war | 5:34 v6 + 4:65 v4 | 490 | 190 | 430 | 920 | 0.47 | open 49s -1130 |
| 5 | panel war | 6:28 v2 + 5:39 v6 | 490 | 200 | 640 | 900 | 0.71 | open 49s -1070 |
| 5 | pincer | 6:25 v1 + 5:33 v3 + 4:65 v4 | 500 | 120 | 660 | 1250 | 0.53 | open 50s -600 |
| 5 | pincer | 5:53 v2 + 5:40 v3 + 6:11 v2 | 500 | 80 | 360 | 420 | 0.86 | open 48s -380 |
| 5 | summoner and guard | 5:52 v4 + 6:25 v3 | 490 | 150 | 730 | 1680 | 0.43 | open 50s -930 |
| 5 | summoner and guard | 5:52 v6 + 6:18 v2 | 490 | 200 | 1000 | 780 | 1.28 | open 49s -1250 |
| 5 | swarm | 5:44 v3 + 5:44 v3 + 5:44 v3 | 420 | 80 | 400 | 1200 | 0.33 | open 49s -230 |
| 5 | swarm | 6:21 v2 + 6:21 v2 + 6:21 v2 | 480 | 40 | 0 | 0 | - | open 50s -0 |
| 5 | wall and artillery | 5:53 v2 + 4:69 v6 | 480 | 200 | 1750 | 1460 | 1.20 | open 49s -1300 |
| 5 | wall and artillery | 5:52 v6 + 6:22 v2 | 500 | 200 | 800 | 600 | 1.33 | open 49s -1140 |
| 6 | own | 6:26 v2 + 6:05 v2 + 6:23 v2 | 460 | 80 | 600 | 600 | 1.00 | open 50s -500 |
| 6 | own | 6:07 v2 + 6:07 v2 + 6:19 v2 | 470 | 100 | 0 | 840 | 0.00 | open 48s -490 |
| 6 | own | 6:08 v3 + 6:18 v3 + 6:18 v3 | 580 | 150 | 780 | 960 | 0.81 | open 49s -590 |
| 6 | own | 6:20 v2 + 6:24 v2 + 6:20 v2 | 540 | 40 | 320 | 920 | 0.35 | open 50s -450 |
| 6 | own | 6:22 v2 + 6:28 v2 + 6:14 v2 | 450 | 140 | 240 | 388 | 0.62 | open 49s -680 |
