---
type: Feature
title: "The Virus Battler"
description: "BN6's Virus Battler in a run: his Mr.Prog on a side path, the BtlrCard on the profile, battle mode 6's VIRUS LIST and timed fight, the five Battlers' series, the stake and the pot, by BN6's own code."
tags: [world, level-design]
generated: { by: claude-code/unrecorded, at: 2026-10-25T17:18:46Z }
---

# The Virus Battler

BN6's own minigame for the rare viruses, a set piece a run holds now and
then: a Battler Mr.Prog at the end of a side path, his series fought by
BN6's own code (the owner's decisions of 25 October 2026). The addresses
are [ROM data](../../rom/overview.md)'s Virus Battler rows, the names
`src/emu/bn6_battler.h`, the claims [Sources](../../research/sources.md)'s, and
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
BugFrags. Each Battler is a Mr.Prog of his own colour (red, blue, orange,
yellow, gold), never the green one.

**Where he stands** (`src/net/net_pieces.c`, `layer_battler`; placed by
`src/net/net_gen.c`'s `place_battler`). On an act's first layer, from the
second act to the last before the Nest: each of those acts holds one by a
coin of the run's seed, and where no coin came up, one of them anyway, so
a run meets one at least: over 300 seeds, 51% of the endless net's five
such acts (2.6 Battlers a run) and 63% of the short net's two (1.25),
which the unit tests hold between 45% and 65% of both together. He takes the end of a side path after
the blue data have taken theirs (a detour of three panels or more, else a
shorter one, else a room), as BN6's Battlers stand off the way. The
Battler is BN6's by how far the act is through its net: the endless
net's second act has the first, its sixth the fifth (Mettaur and BlastMan
last of all); the short net's second act the first, its third the fifth;
a later cycle's acts the fifth. Never on a side layer, a trip back, a
guardian's layer or a dark one. He wears his own Mr.Prog, set by BN6's own
NPC command, which gives his sprite its colour (`npc_variation`).

**The card** (`src/director/btlr_card.c`). The profile keeps it like the
Library: the card held and a bit per family (`Profile.btlr_card`,
`btlr_data`). The first Battler a profile meets gives it as BN6's
HeelNavi sells it, its key item and the flags of Mettaur's and Gunner's
data, with his rules said once. Every run's game gets the card's flags on
each layer (and its key item, in KeyItem), so BN6's own results register
the run's Rares (act 3 on, 3% of battles, `src/core/loot.c`, unchanged),
and the director reads them back into the profile after every battle.
The viruses on it do nothing outside the Battler.

**The visit** (`src/director/director_battler.c`, his words
`src/layer/battler_words.c`). He names his first pair, as BN6's record
for the round holds it, and his stake: that Battler's repeat prize in
BN6, 500, 700, 1500, 2000 or 3000 zenny, taken by BN6's own zenny check
on Yes (the question starts on No). Each round is BN6's own record
forced (`emu_battle_force_record`): its BATTLE n START!, the VIRUS LIST,
the placing, the fight. The pot starts at the stake, and each win adds
it again; after a win he names the next pair and asks TAKE THE POT (and B)
or FIGHT ON, up to his series' end. A loss or a draw leaves the pot his.
The whole series won pays the pot, and in a run's first series won
against him his BN6 first prize (BN6's own flag for it, `0x1EC` + his
number, clear in a fresh run's game): WindRack \*, HP+500, BugFix \*,
FldrPak2 or SuprVulc V. One series a visit: then he says it is over. While
a series is under way the PET's Save waits (a CONTINUE would find the
stake paid and no pot), and nothing of MegaMan's battles is counted for
its rounds (the run log, a request's battle, the viruses deleted), as
their player object is the first virus placed.

**The numbers** (the game-design skill; [The Virus Battler's stake](../../game/economy/virus-battler-stake.md), The Virus
Battler). Push your luck on a telegraph: the pair is shown before each
choice, so the risk is input, not output. With the stake as the unit, a
win doubles it and each one after adds one more; fighting on pays where
the chance of the next win beats (wins + 1) / (wins + 2). A small card
loses more than it wins; a card grown over runs (variety, never power)
turns it into a long series' pot. One series a visit keeps a strong card
from farming him.

Not built: BN6's BugFrags as a repeat prize (the zenny alone is staked).
