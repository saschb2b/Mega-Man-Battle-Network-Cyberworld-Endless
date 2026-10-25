---
type: Feature
title: "Guardians' strikes and returns as BN5's, as built"
description: "Guardians' strikes and returns as BN5's on the board (`liberation_turn.c`, `lib_guardian_return`): the small warp pair, Bladia's DarkSlash and TinHawk's HawkAttack timed to BN5's frames, the Darkloid's blow, and a deleted guardian rising from its DarkHole."
tags: [game, liberation, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-25T00:07:48Z }
---

# Guardians' strikes and returns as BN5's, as built (25 October 2026)

BN5 as the hard reference (the owner's direction), traced in romlab on
missions 2 and 4 ([The Darkloids' phase as BN5 shows it](../bn5/darkloids-phase.md#the-darkloids-phase-as-bn5-shows-it-play-rom);
[BN5's Liberation Missions](../../../rom/liberation.md), BN5's Liberation board: the guardians' and Darkloids'
strikes) and played on the board (`src/director/liberation_turn.c`):

- **Every warp the small pair** (list 7's 59, animations 4 and 5), as
  BN5's every caller passes; the board's had shown the medium pair.
- **Bladia's DarkSlash:** its pose at 22 from the box's closing, 0x10A at
  33, BN5's blow at 34 (list 7's 58 16 pixels over MegaMan, 13 frames,
  0x6B, the board and the board's sprites shaken: `cinema_shake`, 10
  frames up to 4 pixels), the harm at 35, its pose back at 69, the red out
  at 123.
- **TinHawk's HawkAttack:** its warp out at 22, the camera's pan to
  MegaMan, its warp in beside him in its strike at 97 (BN5's two places,
  20 world units up-left or down-right of him on the screen; BN5's RNG
  picks, ours the phase), shown at 101, the blow at 105, the harm at 106,
  0x10A at 110, its warp out with the digits at 140, the camera back, its
  warp in at home at 216, the red out at 284; it no longer stands where it
  struck from. TinHawk's two strike animations were read the wrong way
  round (25 faces right, as BN5 lands it up-left of its Navi).
- **The Darkloid's blow:** BN6's Navis have no BN5 attack, so his strike is
  his pose where list 6 has one and BN5's own blow, as BN5's Darkloids
  0x15 and 0x16 strike: the burst, 0x6B and the shake at 52, the harm with
  it at 53 (BlizzardMan's Snowball delayed his to 73).
- **A deleted guardian's return as BN5's** (`lib_guardian_return`,
  `src/director/liberation_board.c`, the planner's and the board's): at
  the Darkloids' phase's end, each DarkHole in its order still standing
  with MegaMan within three panels of it on both axes sends its deleted
  guardian to the free dark panel of the eight round it nearest him; the
  camera pans to the hole, the guardian rises on it in BN5's OBJ mosaic
  fading in (ours over the frame's pixels about him, a sprite's told by
  its colours: BN6 draws the object, so its mosaic is approximated, the
  floor under a block its first floor pixel's), with 0x94, then warps to
  its panel. Before, he came back silently beside the hole at the start of
  the Darkloids' phase after next, wherever MegaMan stood, on one of the
  four panels beside it. **The game-design check:** the guardian deleted
  in a player's phase strikes again from the Darkloids' phase after the
  next one, in BN5 as before (the next player's phase is his to strike the
  hole); what changes is that he stands beside the hole in that phase, so
  MegaMan meets him again if he does not take the hole, and that walking
  away from the hole keeps him gone. The planner plays the same rule, so
  the par follows it: `LAYER_MAKE` 101.

**Checked:** BN5 and ours side by side in the pull request (TinHawk's
HawkAttack: seed 1's act 4 board, `--run-depth 11 --seed 1 --dev
dark,god,onehit`, frame 5808 on; the return: frame 2678 on, a TinHawk
rising on its hole and warping beside it); `build.py test`, `build.py
lint`.
