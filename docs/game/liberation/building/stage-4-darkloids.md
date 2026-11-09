---
type: Feature
title: "Stage 4, as built: BN5's Darkloids"
description: "Stage 4, BN5's Darkloids: each mission's, BlizzardMan, ShadeMan and CosmoMan moved into BN6's engine and proven frame by frame, the board's Darkloid picked by act (`run_darkloid`) in his own sprite, fought at the act's band."
tags: [game, liberation, bn5]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-09T11:45:00Z }
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
`_sp`: PASS), and since 9 November CosmoMan (`cosmoman_v1`, `_v1_keys`,
`_sp`: PASS). CloudMan is derived but not yet moved: his three decisions
are settled in writing ([Transplanted Navis](../../../multirom/navis.md),
CloudMan), not built; DarkMegaMan, a MegaMan on BN5's chip AI as the Dark
Soul is, is left.

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

**CosmoMan on the board** (9 November): moved, he is fightable, so the
later acts' dark layers bring him as their own Darkloid (BN5's mission 5,
`run_darkloid`'s fourth), twice as likely as each other one moved, and
before them he is a third choice where BlizzardMan and ShadeMan were the
only two. His sprite and face are list 6's and 8's 78, already copied in.
The act whose own is CloudMan (mission 3) still draws among the moved
three. A dark layer's Darkloid now differs where he is drawn, so
`LAYER_MAKE` is 113.

**Left:** CloudMan and DarkMegaMan in BN6's engine; CosmoMan's attack
below half his HP, which no spec reaches; the
Darkloids' poses (their strike animations: BlizzardMan's in BN5 19-52
frames after the box) and BN5's own strike effects (his Snowball over the
Navi); his entrance's words.
