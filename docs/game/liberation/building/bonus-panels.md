---
type: Feature
title: "BonusPanels as BN5's, as built"
description: "BonusPanels as built from BN5's ROM: freed by Liberation without a battle or a spent action (`liberation_bonus.c`), BN5's roulette and five slots (`liberation_roulette.c`), chips in the item panels' band, placement at stubs' ends."
tags: [game, liberation, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-25T00:18:16Z }
---

# BonusPanels as BN5's, as built (25 October 2026)

A BonusPanel is BN5's (traced: BonusPanels as BN5's, above), drawn from
BN5's ROM as the game runs, nothing of it kept:

- **The rule** (`src/director/liberation_bonus.c`): fenced like a dark
  panel, so MegaMan no longer steps on it; facing it, A opens the menu
  with Liberation (LongSwrd not offered), which frees it without a battle,
  and his action is not spent: the board goes back to his move. A CONTINUE
  saved during it resumes at his move with the panel freed.
- **The picture** (`src/director/liberation_roulette.c`): "A BonusPanel!",
  the panel freed as BN5's dissolve, the three flashes and their sound,
  the band, the slot's icon sliding in, "PRESS A BUTTON", the slot every
  4 frames from a random first one with BN5's tick, A to stop it with
  BN5's sound, its name held 88 frames, out and shut: BN5's sprites
  (list 9's icons and words) and frames.
- **The slots, BN5's order:** no harm on the board this phase
  (`LB.shield`, BN5's sound); HP full; five Order Points; a chip; Major
  Hit (the camera on the nearest guardian, deleted, its DarkHole sending
  it back as ever; with none left, the Darkloid: `LB.darkloid_cut` counts
  the fifths of his max HP BN5 takes, for his battle's HP once the board
  keeps it). HP, Order Points and the chip pop over the freed panel in
  BN5's icons with their words, as an item panel's do.
- **The chip:** BN5's are a mission's standard chips, picked among four
  alike, as its item panels' are; a board's BonusPanels give one of four
  chips rolled in its item panels' band (`lib_prizes.c`, the layer's
  seed's rolls of their own), so a chip from a BonusPanel is worth what an
  item panel's is ([Where it is decided](../../economy/where-decided.md)). The economy
  report does not play dark layers yet (Stage 1's left for later).
- **The placement** (`place_bonus`): a board without a sealed region got
  none (its test read every panel as the sealed region's), and the floor
  of a layer's map has few stubs; BonusPanels now go to stubs' ends, else
  to a clean corner (two neighbours), never by a DarkHole: every board has
  its act's two or three.
- **The planner** frees a BonusPanel for nothing as before and stops the
  roulette as a player does, where it helps most (HP when below half, a
  guardian within three, else Order Points, else no harm), one in five
  missed; boards made in acts 5 and 6 stay above half (43 of 80).
  `LAYER_MAKE` 101. Without BN5's ROM, the roulette is not shown: its
  first slot is given at once.

Reasoned with the game-design skill: the BonusPanel stays a lure at a
stub's end, the detour its only price (*prepare or press on*); BN5's
stop by A makes it a small act of timing in a turn-based board, and its
five a choice by need (no harm before a risky phase, HP when low, Order
Points for a LongSwrd, a chip for the run, a guardian off the way).
