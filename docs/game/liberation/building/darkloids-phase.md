---
type: Feature
title: "The Darkloids' phase as BN5's, as built"
description: "The Darkloids' phase as built after BN5 (`src/director/liberation_turn.c`): the Darkloid first then each guardian, BN5's warp moves, strikes with red reach, cries and yellow harm digits, units' animations via `npc_live`, and the exit pad's drawing."
tags: [game, liberation, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-25T01:28:59Z }
---

# The Darkloids' phase as BN5's, as built (23 October 2026)

The owner saw the 0.15.0 trailer's board: the units stood still and
glided, the Darkloids' phase did not play as BN5's, and the exit's ring
was clipped. The phase is now BN5's as traced above
(`src/director/liberation_turn.c`, a unit's turn a frame at a time):

- **The Darkloid first**, then each guardian, as BN5 orders them; the
  camera's pan and rest to each as before.
- **A guardian's move** is BN5's warp: its steps taken at once by the
  board's rules (`lib_guardian_step`), its object parked off the floor
  for the warp (`LB.hidden`), BN5's puff and lines drawn over the board
  from its ROM (sprite list 7's 59, its medium pair), sounds 0xB4 out
  and in and 0x94 as it shows, the camera's pan between. The object is
  hidden and put back a frame before the effect, as BN6 draws an
  object's new place in its next frame.
- **A strike** where MegaMan stands in a unit's reach: the reach in
  BN5's red look on BG2 (`LB.reach`, look 0x0D; tinted red where the
  board is not in BN5's look), its blink and sound; the unit's line
  (`lib_strike_words`: the guardians' cries and attacks' names,
  BeastBreath, HawkAttack and DarkSlash, BN5's; the Darkloid's in his
  own habit of speech with his mugshot); his strike animation; the blow
  with BN5's sound, BigBrute's flame over MegaMan; the harm in BN5's
  yellow digits over him (`lib_piece_harm`) as his HP drops; BN5's
  frames throughout. MegaMan's end-of-phase "Ow!" is gone: the harm
  shows as it lands.
- **The units' animations** are the board's: each unit's object is a
  standing NPC whose animation follows a byte of its own script
  (`npc_live`), written each frame from `LB.anim`; the guardians stand
  facing down-left in BN5's standing animation (two or four frames) and
  strike toward MegaMan in their own. The Darkloid is a BN6 Navi in his
  overworld sprite: his strike is his title card's pose where he has one
  in list 6 (HeatMan, ElecMan, SlashMan, EraseMan, ChargeMan and BN5's
  six); the others stand, BN6's overworld sprites having no strike.
- **The exit's pad** shows on the Darkloid's panel liberated as he is
  deleted, drawn over BG2 (layer priority 2, `npc_sealed_pad`'s
  `over_bg2`) where BN5's looks lie, still under MegaMan by its depth.
- **The overlays' camera:** while the board holds BN6's camera (the
  Darkloids' phase), the drawing takes the camera's own place for the
  picture's middle; the BG scroll's wrap read it a half turn off away
  from MegaMan.

(The Darkloid's heal and the units' HP digits: The board's HP as BN5's,
below. TinHawk's and Bladia's strikes, the Darkloid's blow and a deleted
guardian's return: below. The team: [Stage 3, as built](stage-3-team.md).)

**Checked:** an autopilot run through seed 1's act 2 board from its
opening to its exit (`--run-depth 5 --seed 1 --dev dark,god,onehit`,
11-12 phases on its par of 12, the exit taken to depth 6); seed 2's
board for a BigBrute's warp and strike (frame 5409 on), seed 1's for
the Darkloid's (frame 13667 on).
