---
type: Research
title: "A phase, a panel liberated, Order Points, items"
description: "BN5's Liberation phase: each Navi moves then liberates, commands or passes; a liberation battle is won within three turns, a one-turn win frees the panels around; Order Points 3 to 8; items and traps."
tags: [game, liberation, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-25T00:47:43Z }
---

# A phase, a panel liberated, Order Points, items

## A phase (text, guides)

The player's phase, then the Darkloids' phase. In the player's phase each
Navi acts once: he moves over clean panels, then liberates a dark panel
beside him, uses his command for one Order Point, or rests ("Pass": he
skips his turn and regains some HP, **text**; a fifth of his max HP,
**play**, where the guides say half: Stage 3, as built). The more dark
panels around a Navi, the harder his battle (**text**). R switches Navis
(**play**: to the next yet to act), L moves the camera over the board,
START offers to retreat.

## Liberating a panel (ROM, text)

A liberation battle is a battle record of type 1 (`BN5_BATTLE_LIBERATION`):
all viruses deleted within three turns (**ROM**, read in its battle loop:
[Stage 2, as built](../building/stage-2-battle.md): BattleState +7 counts the Custom screens opened,
TURN n START! and FINAL TURN! as each turn's fight starts, TIME UP! and
LIBERATE FAILED! where the third turn's Custom screen is asked for).
Deleting them within the first turn liberates every panel around the Navi
too (**text**; the ROM's win shows 1 TURN LIBERATE! where +7 is 1). Which columns the field gives depends on which of the
panels around him are dark, so viruses can stand on both sides of him
(**text**: L and R turn him around; the guides give the rule). His HP
carries from battle to battle. Every Navi but MegaMan has one special chip
for the Custom screen, once a battle (**text**: Colonel's C-Cannon).
From a liberation battle's eleventh turn every hit does double
(`BN5_LIB_LATE_DOUBLE`, **ROM**), which only a battle that outlasts three
turns meets: assumed to be the Darkloid's.

## Order Points (ROM, text)

One pool for the team: 3 at the start, 8 at most (`BN5_LIB_OP_ADD` caps
it), 1 for every command (**text**), found in item panels (2 or 3, **ROM**)
and from the BonusPanel's roulette (5, **ROM**, **play**).

## Items, traps and BonusPanels (ROM)

An item panel's item (`BN5_LIB_PTR_ITEMS`, its kinds' table
`BN5_LIB_ITEM_KINDS`): a heart (HP, its amount times ten: 30-900, rising
with the missions), a BarrierPanel's key, Order Points (2-3), zenny (its
amount times 100), BugFrags, a chip. Traps (`BN5_LIB_PTR_TRAPS`) from
mission 5: damage (50 in missions 5 and 6, 100 later) or paralysis. A
BonusPanel's roulette (`BN5_LIB_BONUS_ROLL`) starts on one of five slots
from the random number and is stopped by A; its five slots are each
mission's records in this order (**ROM**, **play**): the team untouchable
for a phase, the acting Navi's HP full, five Order Points, one of the
mission's four chips, the nearest guardian deleted (none: the Darkloid a
fifth of his max HP down). The guides' list was right; its order and what
each does exactly are traced below (BonusPanels as BN5's).
