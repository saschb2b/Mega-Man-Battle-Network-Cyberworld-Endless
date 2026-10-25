---
type: Status
title: "Status"
description: "How the built progression differs from the first proposal: SP Navis in Servers from act 4, guardian draws and per-act bands, what is not built, and fixes on the way (Colonel as navi 18)."
tags: [game, progression]
generated: { by: claude-code/unrecorded, at: 2026-09-29T00:21:26Z }
---

# Status

Built as described above, with these differences from the first proposal:

- SP Navis in Server challenges wait until act 4, not act 3: at act 3
  MegaMan has about 300 HP against their 1400-1700.
- A guardian is drawn from the navis whose HP suits the act, the area's
  own twice as likely as the others: the three opening areas' own gave
  act 1 BlastMan in every run, and act 1 now meets DiveMan and SpoutMan
  too. Its band was 400-500, BlastMan and DiveMan alone: a playtester met
  BlastMan in seven of eleven runs. Act 1 is the act every run plays, the
  one whose sameness shows first, so its band takes the third navi of the
  400-600 tier; SpoutMan's 600 HP is still under act 2's lightest.
  In act 1 the two are as likely, and a new run avoids the last one's
  first guardian once: a playtester met BlastMan six runs running, and
  the Robot Control Comp's OldStove and Mettaur in every act 1 battle, so
  that area waits for act 2. The Seaside Area opens runs too (five kinds
  of virus in act 1's band), and a new run avoids the last one's act 1
  area: with two opening areas a playtester began in the RoboDog Comp
  four runs running.
- Act 2's guardian band is 600-700: HeatMan, SpoutMan or CircusMan. At
  600-800 it held the 800s (SlashMan, EraseMan, TenguMan, JudgeMan), and a
  playtester with 240 HP and 40-150 damage chips dented EraseMan by 1 of
  800 after a 400-HP BlastMan; they wait for act 3.
- The guardian bands of acts 5 and 6 are 1100-1500 and 1200-2000, so the
  Undernet's and the Graveyard's own Navis (SlashMan, HeatMan, EraseMan,
  DustMan at EX) can hold them.
- A run lost early earns an extra HPMemory before the gift, not a fourth
  choice.

Not built: a second guardian in the Nest on later cycles, and difficulty
levels to choose on the title screen.

Fixed on the way:

- The four random-battle tables are event variants, not story stages
  (`docs/rom/`, `formations.c`).
- Colonel was fought as navi 17 of the enemy table, an unnamed navi with
  4000 HP at V1; he is navi 18 (1200 HP). Saved runs and his rival record
  move over on loading.
- A guardian battle's `no_escape` was never used and is gone; a guardian
  MegaMan runs from waits to fight again, as before.
- A Server challenge on layers 16-19 drew from the next cycle's depth.
