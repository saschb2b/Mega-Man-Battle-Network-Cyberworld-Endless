---
type: Feature
title: "The battle's field, as BN5's"
description: "A liberation battle's field as BN5's (`src/director/liberation_field.c`): the dark round the panel weighed into a level (`BN5_LIB_LEVEL`) that picks the column pattern, up to Surrounded, which BN6's engine keeps with no code moved."
tags: [game, liberation, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-25T00:15:50Z }
---

# The battle's field, as BN5's (25 October 2026)

**BN5's rule, read in its code and played** (`src/emu/bn5_liberation.h`,
[BN5's Liberation Missions](../../../rom/liberation.md), A liberation battle's field). As a Navi liberates a
panel, BN5 weighs the dark round it by where he stands (`BN5_LIB_LEVEL`):
the panel beyond it 20, its two sides 10 each, the two diagonals beside
him 2 each, the far diagonals and his own panel nothing; dark, an item's
or trap's panel and a DarkHole count, the Darkloid's ring not. The sum's
level (0 none, 1 to 9, 2 to 19, 3 to 30, 4 above) picks the battle from
the mission's lists (`BN5_LIB_BATTLE_OF`: a DarkHole's and each unit's
five, a dark panel's eight a level by weight), and every battle of a level
has one field, its record's column pattern (+6 of BN5's 860 liberation
records, `BN5_LIB_BATTLES`):

| Level | Pattern | Field |
| --- | --- | --- |
| 0 | 0x30 | MegaMan's four columns, his foes in 5-6 |
| 1 | 0x38 | three and three |
| 2 | 0x3C | MegaMan's two, his foes in 3-6 |
| 3, 4 | 0x33 | Surrounded: MegaMan's two in the middle (he at column 3, row 2), his foes in 1-2 and 5-6 |

The Darkloid's battle is never Surrounded (his lists 0x30, 0x38, 0x3C,
0x3C, 0x3C). In a Surrounded battle L or R turns MegaMan round, and the
first one a save meets opens on the operator's line from BN5's battle
archive, "Surrounded! Press L or R Button to turn around!" (the opening's
phase 0x28, then flag 3:0x0E). The Custom screen opens by itself as the
gauge fills in every liberation battle (above), so a Surrounded battle
never needs L or R for it.

**In BN6's engine, no code moved.** BN6 reads the same byte of a
BattleSettings (bn6f GetBattlePanelColumnPattern, `BN6_SIDES_OF`) and kept
BN5's Surrounded whole, though none of its battles asks it: each column's
owner from the pattern's bits (`BN6_SIDES_SET`), an object's facing taken
from its own where the pattern is none of the plain three
(`BN6_SIDES_FACING`), and MegaMan given his turn (`BN6_SIDES_TURNER`,
`BN6_AI_TURNS`) so that L or R turns him instead of asking for the Custom
screen (`BN6_SIDES_KEYS`). The `foes` step's `sides@33` shows it: the
viruses on the left face MegaMan and strike to the right, L turns him and
his buster hits them. So the run's battle for a panel keeps its foes and is
set out on BN5's field for the level (`src/director/liberation_field.c`):
its record's +6 (the Encounter's `sides`), MegaMan at column 3 where
Surrounded, every other foe mirrored to the left-hand pair (BN5's records
share them so), the foes in columns 5-6 where his side is four wide, the
field's objects left out where they would stand on someone. The foes'
strength still follows stage 1's bands of the unweighted dark.

**What is not BN5's yet.** The "Surrounded!" line: BN6 dropped the opening
phase that says it (no reader of the pattern at its battle's start), and
its chat would need driving over the battle's opening; for now the
stakes' box says "SURROUNDED: L/R TURNS" before the battle instead. A
unit's HP: BN5's battle reads its board for that alone, a guardian's or
the Darkloid's HP from the board as its enemy spawns (`BN5_LIB_UNIT_HP_IN`)
and the HP left back to the unit at the end (`BN5_LIB_UNIT_HP_OUT`), so a
time up leaves him hurt; the units' HP on the board are their own piece of
work.

**Checked:** in romlab, BN5's own Surrounded battle (mission 1, its record
24 handed out by the board's getter patched in the loaded copy) and its
gauge opening the Custom screen; ours on seed 1's act 2 board
(`place 76 -124 5`, Liberation: `dark level 3, sides 33`), the Custom
screen opening by itself in turns 1 and 2 and TIME UP in the third, the
recordings side by side in the pull request.
