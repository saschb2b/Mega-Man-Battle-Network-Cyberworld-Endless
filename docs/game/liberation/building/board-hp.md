---
type: Feature
title: "The board's HP as BN5's, as built"
description: "The board's HP as BN5's: digits under units, the Darkloid's HP carried into and out of his battle and kept in `run.lib`, and his heal, a tenth of his max per dark panel round him."
tags: [game, liberation, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-25T01:16:13Z }
---

# The board's HP as BN5's, as built (25 October 2026)

**BN5's rules, read in its ROM and played in romlab** (mission 1;
`src/emu/bn5_liberation.h`, [ROM data](../../../rom/overview.md)'s row): each board unit
keeps its HP, its max and the HP its digits show. The digits stand under
every unit in play but the Navi in play (his HP is the HP box's): white,
8x16 (BN5's HP digits), centred on the unit's place on the floor, their
top there; the HP shown steps 4 a frame toward the HP. A unit fought
lends its battle's marked enemy its HP and max as the enemy spawns, and
takes back the HP that enemy has as the battle ends, so one not deleted
keeps on the board what its battle left (one left at none by a battle
not won gets a quarter of its max). The Darkloid heals as his turn
begins, as DARKLOID PHASE's band closes, where his HP is below his max
and dark panels stand round him: a tenth of his max for each (dark, an
item's, a DarkHole, his ring: BN5's weights), his max at most, the
digits with it at once; rising lines over him (list 7's 120, in the
palette its variant names) with sound 0x8A, then 20 frames before his
turn goes on. (The guides' "100 a panel" is a tenth of a 1000 HP
Darkloid's.) A Navi's rest heals by BN5's story's progress, a fifth of
his max on mission 1 to a half (its ROM); ours is the guides' half.

**On our board** (`liberation.c`, `liberation_battle.c`,
`liberation_turn.c`, `liberation_pieces.c`, `liberation_draw.c`):

- **The Darkloid's HP is the board's**: his own battle's (the version
  make_boss gives him) as the board begins, kept across a CONTINUE
  (`run.lib`, its note `CLB3`), shown under him in BN5's digits, rolled
  as BN5 rolls them, hidden under a chat box. His battle begins from it:
  the director gives his object the board's HP as it is first in play,
  his max his own (BN6's MaxHP, as BN5 gives the board's max), and
  reads his HP each frame of it; a battle that leaves him standing
  leaves the board that HP.
- **His heal** is BN5's, at the start of his turn in the Darkloids'
  phase, its effect and sound BN5's, read from its ROM, then BN5's wait.
- **The guardians show no HP**: our guardians' battles are the act's
  viruses, not one foe of theirs, so they have none to keep.

His battle times up at three turns as every liberation battle does (The
battle's field, as BN5's), so a battle that leaves him standing leaves
the board his HP, his heal comes into play, and the next battle with him
begins from what he has; so do the team's strikes (stage 3: ShadowMan's
SneakAttack) and the BonusPanel's Major Hit, a fifth of his max.
`--dev libhp=N` begins him at N HP to see them.

**Checked:** BN5 and ours side by side (the pull request's pictures):
the digits under BlizzardMan and a guardian in BN5, under the Darkloid
on our board; the heal with `--dev dark,god,onehit,libhp=100` (seed 1,
depth 5): 100 to 660 at once (his ring's eight dark, 70 each, frame
2266), BN5's lines over him in its purple, 700 the next phase;
`test_liberation_heal` (the weights, the cap, none at full HP or with
none dark round him); the autopilot through seed 1's board to its exit.

**Left:** a guardian's own HP (BN5's guardian battles' marked enemy not
traced: which of its battle's enemies the record marks); the rest's heal
by BN5's table (a game-design call: ours is the guides' half); the HP
box and the team's digits with stage 3.
