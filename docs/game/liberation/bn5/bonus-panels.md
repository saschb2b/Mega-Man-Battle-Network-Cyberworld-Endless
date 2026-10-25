---
type: Research
title: "BonusPanels as BN5's"
description: "BN5's BonusPanel traced in romlab (`BN5_LIB_BONUS_*`): freed by the Liberation command without a battle, its lead-in flashes, the roulette's timing, sounds and sprites, and its five slots from phase invincibility to Major Hit."
tags: [game, liberation, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-25T00:18:16Z }
---

# BonusPanels as BN5's (ROM, play)

Traced in BN5's own board (`tools/romlab`'s BN5 lab, mission 1 from its
board's state, the faced dark panel made a BonusPanel in RAM and the
mission's first BonusPanel record moved to it; each slot forced by its
byte and played to its end; `src/emu/bn5_liberation.h`'s
`BN5_LIB_BONUS_*`):

- **Freed by command.** A BonusPanel is not walked (the walk mask of
  MegaMan's unit, 0x1001, is clean and start alone); facing it, the
  command window offers Liberation, which frees it **without a battle**:
  the player's phase's step `0x0804C468` sends kind 9 to the roulette's
  steps, and the Navi's actions left (his unit's +2) are untouched: the
  same Navi's menu opens again after it.
- **The lead-in:** MegaMan's line ("A BonusPanel!"); the panel dissolves
  (19 frames) and shines (31); three white flashes over the whole picture,
  sound 0xD7 each, two frames white then down by 2/16 a frame, the second
  26 frames after the first, the third 10 after that; the band opens 30
  frames after the third.
- **The roulette** (its state `0x0200ABB0`, ten routines from
  `0x08054EF4`): the phase banner's band (BG0 filled with one tile of the
  phase words' palette, WIN1, blended over the board at half) opens in two
  frames to rows 56-87; 16 frames on, sound 0xD6, the slot's icon slides
  in from the right, 32 pixels a frame, to the middle (x 120, y 72); 8
  frames later "PRESS A BUTTON" shows under it (y 98) and A is heard. The
  slot steps every 4 frames (each BonusPanel record's u16) from a random
  first one, sound 0x80 at each step once the icon is in place; the icon
  shown is the slot's, one frame behind it, so the slot stopped on is the
  one on screen as A is pressed (sound 0x92). Its name replaces the prompt
  for 88 frames, then the icon slides out to the left, the band shuts in
  two frames, and the slot is given (`0x0804C754`'s five handlers).
- **The icons and words** are BN5's sprites of list 9: the icons
  0x3F, 0x3C, 0x3D, 0x3E and 0x40 (`BN5_LIB_BONUS_ICONS`: a flexing
  green Navi, a heart, an Order Point, a chip, a skull), the words list
  9's 25 (`BN5_LIB_PHASE_WORDS`' sprite): animation 4 "PRESS A BUTTON",
  6-10 the names "1PHS INVIN", "HP Rec!", "Order Pt!", "BattleChip!",
  "Major Hit!".
- **The slots, in BN5's order** (each mission's BonusPanel records,
  `BN5_LIB_PTR_BONUS`, the same five in all nine):
  0. no harm on the board for the rest of the phase: every Navi's unit
     flagged 0x80 (sound 0x93, the team tinted green), cleared at the
     next player's phase;
  1. the acting Navi's HP full: an item find of a heart of 100 (x 10),
     his alone (30 of 100 to 100; Colonel's 50 of 200 untouched);
  2. five Order Points (3 to 8), an item find;
  3. a chip, an item find: one of the mission's four (mission 1: four
     standard chips, one of them a "*" code), their weights alike (0x20);
  4. "Damages the closest enemy the most!": the nearest guardian (BigBrute,
     TinHawk or Bladia, by squared distance from the Navi) deleted, the
     camera on it ("Gyaaaaahh!!", its explosion); with no guardian left,
     the nearest Darkloid loses a fifth of his max HP (400 to 320, 1000 to
     800, 300 of 400 to 220: his max's, not his HP's).
