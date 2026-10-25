---
type: Feature
title: "Guardians"
description: "Guardian Navis' bodies and indexes (`guardian_body`): Gregar's sprites and mugshots, Falzar's Navis in battle sprites with `portrait.c` faces, BN5's copied in, navi indexes 1-29, and how an act's HP band picks the guardian."
tags: [game, bosses]
generated: { by: claude-code/unrecorded, at: 2026-10-06T11:34:42Z }
---

# Guardians

Mugshots share their index with the Navi's overworld sprite in list 6:
HeatMan 0x47, ElecMan 0x49, SlashMan 0x4B, ChargeMan 0x4F, EraseMan 0x50,
BlastMan 0x51, DiveMan 0x52, Colonel 0x53, CircusMan 0x54, JudgeMan 0x55,
ElementMan 0x56, ProtoMan 0x3B, MegaMan 0x37. Every guardian stands on
the net in his own shape (`guardian_body`):

- Gregar's overworld sprite where it has one that faces every way.
- BlastMan's and ElementMan's, which Gregar draws only facing right
  through down-left (animations 2-5; its story never turns them from the
  camera), turned down-right or down-left where the arena would face them
  up into the screen.
- Falzar's Navis (SpoutMan, TomahawkMan, TenguMan, GroundMan, DustMan),
  whom Gregar has no overworld sprite or face of, in their battle sprites
  (sprite list 0, `0x2E` + the navi), standing in animation 0, logging in
  by BN6's battle warp-in (their animation 3) and mirrored to face right.
  They speak with a portrait from MegaMan's battle data (`portrait.c`): a
  40 x 48 window of the same battle sprite, in the frame of a BN6 face, on
  a ground of their title card's colour.
- BN5's Navis (below), in their own BN5 overworld sprites and faces, copied
  in from the player's BN5 ROM at Gregar's list-6 and mugshot number 78,
  one guardian at a time (`xnavi_guardian`); their sprites lay out their
  animations as Gregar's Navis' do, so they log in by animation 25 and
  strike their pose, 24, for the title card.

A guardian's navi index is its ai in the enemy table: HeatMan 1 .. ElementMan
16, and Colonel 18. BN5's take 24-29 (Colonel, ShadowMan, NumberMan,
TomahawkMan, KnightMan, ToadMan: their AI index in BN5's table, 7-12, plus
17), each with a rival record of his own. Bass is 19 and the Cybeast
Gregar 20 ([Super bosses](super-bosses.md)).
Index 17 (and 0, 22) is an unnamed navi with 4000 HP at V1, drawn as
MegaMan: the place of a Navi the US version cut (What the ROMs hold,
below), whose SP record a Server's challenge in the Graveyard could roll
until the super bosses held it back; runs saved before Colonel moved to 18
have their 17 changed on loading, and his rival record follows.

Which guardian an area gets depends on its act ([Progression](../progression/overview.md)): from
the area's pool the Navis whose HP at the act's version lies in the act's
band (400-600 in act 1 up to 1200-2000 in act 6), else another Navi in the
band, else the pool's nearest. Acts 1-3 fight V1, acts 4-6 EX where it
fits, the Nest, the Secret Area and later cycles SP.
