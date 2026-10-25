---
type: Feature
title: "10. AsterLand and the Cyber Academy"
description: "AsterLand and the Cyber Academy (issue #96): BN6's maps taken over (`aster_land.c`, `academy.c`), AsterLand's clerk, Request BBS and Chip Trader with the Number Trader off, class 6-1's NetBattle club."
tags: [world, home]
generated: { by: claude-code/unrecorded, at: 2026-10-06T00:18:57Z }
---

# 10. AsterLand and the Cyber Academy

The town's doors to AsterLand (`0x01:04`, the town's warp 4) and the
Cyber Academy (its group `0x02`; the gate is the town's warp 2, into the
foyer `0x02:06`) open, as Lan's front door does: BN6's own maps, their
furniture, checks, doors and songs kept, no people or story scripts of
BN6's, the run's own people in them.

- **AsterLand**: the counter's clerk (the home shop, piece 6), the Request
  BBS (the jobs, piece 5) and BN6's own Chip Trader (the trade screen has
  AsterLand's prize list, map key `0x104`: 70 chips of rarity 1 to 3,
  [Chips, folders and the Library](../../rom/chips.md), Chip Traders).
- **The Cyber Academy**: the foyer, the 1F hallway (`0x02:04`), the 2F
  hallway (`0x02:05`) and Lan's class, 6-1 (`0x02:00`): classmates who
  post jobs and speak of the run (the run's own Lan of class 6-1), the
  NetBattle club. The other rooms' doors stay shut.

Verified in Gregar's own tables (6 October 2026): Central Town's warp
lists (its group's loader `0x0804F4B0`, lists `0x0804F210`: the town's
five entries, AsterLand's one back to the town at -150, -16) and the
Academy's (loader `0x08053508`, lists `0x080530C4`: the foyer's gate back
to the town at 92, -174, its way to the 1F hallway, the hallways' doors
to the classrooms two each and their stairs); BN6's machines in
AsterLand by its check dispatch (bn6f `sub_8034E88`, the same addresses
in Gregar: the Chip Traders' maps at `0x08034E74`, AsterLand's check
`0xF7`; the Number Trader's at `0x08034E80`, AsterLand alone, check
`0xF9`; at `0x08034E84` AsterLand's chip table, check `0xF8`, a text).

As built (issue #96): both taken over as Lan's house is (`indoors.c`),
each in a part of free space of its own, their doors out where the
planned town has them; their people and what they say are
`place_lines.c`'s, sprites of the game's people that Central Town's own
crowd does not use.

- **AsterLand** (`aster_land.c`): BN6's checks read as BN6 has them (the
  virus panel, the showcases, the magazines, the locked register). The
  request board opens BN6's own Request BBS, which holds none of BN6's
  requests in a run: the jobs (piece 5) are to post there. The Chip
  Trader trades from AsterLand's own list in the folder's codes, as the
  layers' traders do (theirs moved to the tables' second entry, Sky
  Town's, a map the run never visits). The **Number Trader is off**: its
  codes are no secret, and every run would take BN6's prizes from it, a
  power no run earned (variety, not power: [The meta layer](../../game/meta/overview.md)); its check reads
  an "Out of order" sign (BN6's own taken off its map list). The clerk
  stands behind the counter where BN6 stands its own and points at the
  Chip Trader; a NetBattler waits at the empty board; a shopper gulps at
  the rare chips' prices.
- **The Cyber Academy** (`academy.c`): the foyer's way to the 1F hallway,
  its stairs up, the 2F hallway's two doors into class 6-1, and each way
  back; the other classrooms, the teachers' room and the principal's
  office shut (their warp-off flags held each frame). No class today, as
  Lan's classmate says in town: the NetBattle club meets in class 6-1 (a
  tip on matching codes, a Navi beaten by a Mettaur), a first grader
  waits in the 1F hallway. The classroom PCs and the foyer's jack-in do
  nothing.
- L in either names the way out; R has MegaMan point it and remind Lan
  his PC is at home. The map's label and the second screen name the
  place.

A's reach in the real world is BN6's own again: the probe ahead of Lan
that A reads checks along is the one the layers widen (24 units ahead
for navis on platforms, [Scripts, text and shops](../../rom/scripts-and-shops.md), Talking reach), and widened it
read past every check a cell deep, the request board, the Chip Trader
and all of Lan's room's. It is widened only in the Net now.
