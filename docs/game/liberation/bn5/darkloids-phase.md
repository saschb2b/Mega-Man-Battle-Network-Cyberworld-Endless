---
type: Research
title: "The Darkloids' phase in BN5"
description: "BN5's Darkloids' phase, from text, guides and romlab recordings: guardian kinds (BigBrute, TinHawk, Bladia), the Darkloid's heal by dark panels, warp moves, five-beat strikes, and a deleted guardian's return from its DarkHole."
tags: [game, liberation, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-25T00:07:48Z }
---

# The Darkloids' phase in BN5

## The Darkloids' phase (text, guides)

Guardians "attack closer enemies first" (**text**), and one is best
deleted before the Darkloids' phase or its DarkHole liberated in one shot.
A DarkHole's liberation is a fight with its guardian; won, it frees the
eight panels around it; while the hole stands, a deleted guardian comes
back (**guides**; when and where read in BN5's code: The Darkloids' phase
as BN5 shows it, below). The kinds (`0x11`-`0x13` in the records): BigBrute
moves two and hits every Navi beside it; TinHawk hits one within three;
Bladia teleports, darkening the clean panel it lands on (**guides**, with
HP and damage by mission). The Darkloid attacks Navis in his reach and
heals for each dark panel around him (100 a panel, **guides**). All down
means a retreat (**text**).

## The Darkloids' phase as BN5 shows it (play, ROM)

Recorded in romlab on mission 1 (both Navis' Pass taken, then MegaMan's
place, his unit's world place at `0x0203E000` +0x1C and +0x20, poked
two panels from a BigBrute, and in a second run two from BlizzardMan),
frame by frame, each OBJ traced to its sprite and each sound to its
caller ([BN5's Liberation Missions](../../../rom/liberation.md), BN5's Liberation board: the Darkloids' phase):

- **The order.** The camera pans to the Darkloid, DARKLOID PHASE, and he
  acts first; then the camera visits each guardian in turn (its pan 22
  frames, its rest 15) and back to the Navi for PLAYER PHASE. A
  guardian with no Navi near does nothing at all.
- **The Darkloid's heal.** Right after the banner where dark panels stand
  round him: rising lines over him (effect 0x78), sound 0x8A, his HP up
  at once (204 to 400 in the recording).
- **A guardian's move is a warp, not a walk.** All its steps at once: a
  puff and then a beam's lines where it stands (sprite list 7's 59, 2
  frames each; the guardian gone with the puff), sound 0xB4, the camera's
  pan to the panel it takes, then the lines and the puff there, sound
  0xB4 again 46 frames after the first, the guardian back 4 frames into
  it, sound 0x94.
- **A strike in five beats.** Where a Navi stands in the unit's reach:
  every board panel in the reach turns red (look 0x0D, BG palette 7; on
  9 frames, off 7, on 9, off 7, then on, sound 0x91 as each light
  comes); 68 frames on, the unit's line in a chat box, its attack's name
  ("Grrrowl! BeastBreath!" with no face for a BigBrute, BlizzardMan's
  with his mugshot); once it closes, the unit's strike animation
  (BigBrute's 12-27 frames after the box closed, BlizzardMan's 19-52),
  the blow on the Navi with sound 0x6B (0x6E where nothing harms him):
  BigBrute's breath of fire over him (sprite list 7's 56: rising,
  burning, falling, 27 frames), BlizzardMan's Snowball falling on him;
  the harm in yellow digits over the Navi (BN5's HP digits, palette
  `0x087E4678`, 29 pixels over his feet) as his HP under him drops (at
  the blow +30 for BigBrute, +21 for BlizzardMan), held 60 frames; the
  red put out (123 and 127 frames after the box closed) and the next
  unit's pan.
- **The units stand animated.** The guardians in their standing
  animations by the way they face (list 6's 83-85: 0-7 by the eighth,
  two or four frames looped), facing down-left on the board; their
  strikes their own animations (BigBrute's 33-40, TinHawk's 25-26,
  Bladia's 25-28).
- **Every warp the small pair.** A unit's move, TinHawk's attack and a
  guardian's return all spawn list 7's 59 in its animations 4 (out) and
  5 (in), read in each caller's r1 (missions 1, 2 and 4).
- **Bladia's DarkSlash** (mission 4, Colonel beside it): its pose (25 + its
  eighth / 2) 22 frames after the box closed, sound 0x10A at 33, the blow
  at 34: BN5's burst (list 7's 58, 16 pixels over the Navi, 13 frames;
  the board event 0x08051904) with 0x6B, the board shaken (BG1's scroll,
  up to 4 pixels for 10 frames), the harm at 35, its pose back at 69, the
  red out at 123 as BigBrute's.
- **TinHawk's HawkAttack** (mission 2, MegaMan two from it): it warps out
  where it stands (22), the camera pans to MegaMan, it warps in beside him
  already in its strike (97), 20 world units up (25, facing right) or right
  (26, facing left) of him by the RNG, shows four frames on with 0x94;
  BN5's burst and the shake at 105, the harm at 106, 0x10A at 110; it warps
  out with the harm's digits at 140, the camera back to its panel, in there
  at 216 (0x94 at 220), the red out at 284.
- **A Darkloid's strike** is his own pose and, for two of BN5's Darkloids
  (kinds 0x15, 0x16) as for Bladia, BN5's burst (the same event); the
  others' own effects (BlizzardMan's Snowball, 0x080517F4; 0x0F's
  0x08051874; 0x10's 0x08051914), one (0x0D) warps beside the Navi as
  TinHawk does.
- **A deleted guardian's return** (0x0804E700-0x0804E858; romlab, mission
  2, a BigBrute poked deleted beside its DarkHole): at the Darkloids'
  phase's end, after every unit's turn, while fewer than eight enemies
  stand, each DarkHole in its order still standing with a Navi within
  three panels of it on both axes (the 7 x 7 round it) sends back one of
  its deleted guardians (at random, while fewer than three of its own
  stand) to the free dark panel (dark or an item's, no unit) of the eight
  round it nearest the Navi nearest the hole, with full HP. The camera
  pans to the hole; 20 frames on the guardian rises on the hole in OBJ
  mosaic, 15 down to 0 as it fades in over 16 frames, with 0x94; 40 frames
  after the rise began it warps out of the hole, 22 later in on its panel
  (0xB4 each, 0x94 as it shows); 20 frames, the next hole. Deleted in a
  player's phase, a guardian is back for the next one where a Navi stayed
  near its hole, and strikes from the Darkloids' phase after.
