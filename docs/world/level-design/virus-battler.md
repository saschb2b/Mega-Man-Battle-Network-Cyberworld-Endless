---
type: Research
title: "The Virus Battler"
description: "BN6's Virus Battler researched as a set piece, not built: the BtlrCard flag, battle mode 6's VIRUS LIST and timed fight, the five Battlers' series and prizes, and what a run could take by BN6's code."
tags: [world, level-design]
generated: { by: claude-code/unrecorded, at: 2026-10-25T14:07:16Z }
---

# The Virus Battler (researched, not built)

BN6's own minigame for the rare viruses, researched as a set piece a run
could hold now and then (a Battler Mr.Prog on a layer, as the traders
stand); the addresses are [ROM data](../../rom/overview.md)'s three Virus Battler rows,
the names `src/emu/bn6_battler.h`, the claims [Sources](../../research/sources.md)'s, and
`tools/virus_battler.py` reads the series from the ROM and plays one.

**What BN6 does.** The BtlrCard is a flag (0x1D8): with it set, a rare
virus (version 4 or 5) deleted for the first time ends its results with
"You got new Virus Data", a flag per family, 27 families, Mettaur and
Gunner always held. A Battler's battle is a battle mode of its own (6):
in place of the Custom screen the VIRUS LIST shows the held families with
their V1's HP, attack and MB; one or two are picked (50 MB at most), each
placed on MegaMan's panels by a cursor, and after "Fight with this set?"
both sides fight on their own, MegaMan off the field, his HP untouched,
no Game Over. A gauge at the top is the time limit, about 21 seconds:
every enemy deleted wins, the team deleted loses, the gauge full is TIME
UP and a draw. Each of the five Battlers runs 3, 5, 5, 10 or 10 rounds in
a row, each round a fixed pair of V1 viruses; a loss ends the series, a
first win pays a chip or a NaviCust program, a later one zenny or
BugFrags.

**What a run can take from it.** All of it, by BN6's own code: the
engine's forced battle (`emu_battle_force`'s record) given a mode 6
record started BN6's VIRUS LIST, placing and fight on a layer, and the
run went on after it, MegaMan's HP as it was (25 October 2026, a local
change not kept). The run's own rares already reach the card: with flag
0x1D8 set in the run's game, a PiranhaRare the run fielded ended in BN6's
chat and the list showed Piranha. What the engine would add: the record
(BN6's own 33, or one of its own with an act's pair, in the free space),
the flags of the families held set at the run's start, the series and the
prize in the director (the result at `BN6_BATTLE_RESULT`: 1 won, 2 lost,
7 a draw), and the Mr.Prog's words. One thing to mind: during the fight
the battle's player object is the first virus placed, not MegaMan, so
nothing may read his HP from it.

How often it comes, what it stakes and pays, and what the card keeps
across runs are design decisions not yet taken.
