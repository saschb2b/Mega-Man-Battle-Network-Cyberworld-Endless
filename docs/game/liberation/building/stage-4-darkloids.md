---
type: Feature
title: "Stage 4, as built: BN5's Darkloids"
description: "Stage 4, BN5's Darkloids: each mission's, BlizzardMan and ShadeMan moved into BN6's engine and proven frame by frame, the board's Darkloid picked by act (`run_darkloid`) in his own sprite, fought at the act's band."
tags: [game, liberation, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-25T00:29:32Z }
---

# Stage 4, as built: BN5's Darkloids (25 October 2026)

**Which Darkloids BN5's missions use (ROM).** The nine records' Darkloid
(their units' first, `[4]` +5 the kind, +0x12 the enemy id, read with the
names archive and the ids table): mission 1 BlizzardMan V1 (`0x14F`, AI
14), 2 ShadeMan V1 (`0x149`, AI 13), 3 CloudMan V1 (`0x155`, AI 15), 4
DarkMegaMan (`0x18F`, a MegaMan of the ids table's type 2, AI 23), 5
CosmoMan V1 (`0x15B`, AI 16), 6 DarkColonel (`0x126`: Colonel's V2,
already moved), 7-9 ShadeMan, CloudMan and CosmoMan in their third
versions (`0x14B`, `0x157`, `0x15D`). LarkMan (AI 17) and GridMan (18)
are BN5's but no mission's.

**In BN6's engine** ([Transplanted Navis](../../../multirom/navis.md), The Darkloids):
BlizzardMan and ShadeMan, each proven against BN5's own battle frame by
frame (`blizzardman_v1`, `_v1_keys`, `_sp`; `shademan_v1`, `_v1_keys`,
`_sp`: PASS). CloudMan and CosmoMan are derived but not yet moved (their
object types, a RAM table of CloudMan's and a sprite loaded by a number
made otherwise are open decisions); DarkMegaMan, a MegaMan on BN5's chip
AI as the Dark Soul is, is left.

**On the board.** A dark layer's Darkloid (`run_darkloid`, the guardians'
navis 30-33 here: ShadeMan, BlizzardMan, CloudMan, CosmoMan) is BN5's own
where its ROM brought him into BN6's engine: BN5's missions' order makes
the act's own the likeliest (act 2 BlizzardMan, act 3 ShadeMan, act 4
CloudMan, then CosmoMan), each other one moved half as likely, by a coin
of the run's seed and the act; where none is moved, a Navi of BN6's as
before. He stands in his own overworld sprite and speaks with his own
mugshot, BN5's lists 6 and 8 copied in as BN5's guardians' are
(`xnavi_guardian`): BlizzardMan's 76, traced on BN5's own board (mission
1 in romlab, his OBJs' tiles found in his sprite), ShadeMan's 70 (by its
palette beside the six's, then seen on our board). His strike says his
line in his habit (`lib_strike_words`). His battle is his moved id at the
version the act's band takes (`xguardian_version`, the pacing's band for
BN5's Navis), his HP held to the band as BN5's guardians' are (the board's
HP is the board session's), with BN5's rules of the Darkloid's battle as
before (no turns to count, the late double).

**The design** (the game-design skill): the Darkloid is the dark layer's
climax and the board's identity; BN5's own order is a staircase a BN5
player knows (BlizzardMan's 400 HP at the first mission, ShadeMan's 600
the next), and the act's band picks the version, so strength follows the
run, not the story. Variety stays: a second Darkloid moved means half the
boards meet the other one.

**Checked:** the six specs above PASS, and all 32 of the Navis' specs
with the comparator's change (only the spec's own Navis' code is the
foe's); `build.py test`, `build.py lint`, `build.py pacing` (0 past their
band); the autopilot (`--dev dark,god,onehit`) through seed 1's act 2
board (BlizzardMan V2, deleted at par 12, the exit taken) and seed 2's
act 3 board (ShadeMan V2, deleted two under par), each meeting his strike
in DARKLOID PHASE.

**Left:** CloudMan, CosmoMan and DarkMegaMan in BN6's engine; the
Darkloids' poses (their strike animations: BlizzardMan's in BN5 19-52
frames after the box) and BN5's own strike effects (his Snowball over the
Navi); his entrance's words.
