---
type: Feature
title: "Two MegaMen a side"
description: "Two MegaMen a side (epic #167, issue #181): four MegaMen in one BN6 battle via hooks at `BN6_SPAWN_ENTITIES` and `BN6_KEY_COPY`, `--dev arena2v2`, and the rule that both GBAs run the same battle iteration by iteration."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-22T14:54:41Z }
---

# Two MegaMen a side

Four MegaMen on one field in BN6's own engine, two a side (epic #167,
issue #181): a side-by-side team match from the Net Square ([Over the
server](over-the-server.md)), and its research and proofs
behind `--dev arena2v2=1` (`src/online/arena_team*.c`) with `--dev
arena` (its local link) or `--dev rollbench`. A hook at the battle's
spawn (`BN6_SPAWN_ENTITIES`) trades a NetBattle's list for one of four
MegaMen; a hook at the battle loop's copy of the sides' keys
(`BN6_KEY_COPY`, side 0's call) counts the loop's iterations in the
core's RAM (`RAM_TEAM_TICK`) and writes each partner's keys into his AI
data as BN6 writes a player's. `arena2v2=N` (N > 1) flips one key of
side 0's partner on the second GBA alone for six iterations from
iteration N: a mutation that must part the GBAs.

The rule everything here keeps: both GBAs of the pair run the same
battle, compared iteration by iteration (the four MegaMen's panels, HP,
actions and AI data's keys, the random number, the battle's timer: the
log's "arena2v2: iterations ... differing"), and whatever a partner
does reaches both at the same iteration. A lead's pad does so by BN6's
own link; since [The partners' words in BN6's
packet](partners-words.md) a partner's keys and choices
ride the same packets, so they do so by the same construction. Under
full rules BN5's chip AI gave both of side 0's MegaMen its one block of
AI data (`RAM_PLAYER_AI`), so the flag keeps it the first's
(`darkinvs_two_a_side`), and each other MegaMan takes a room of his own
as the chip AI first takes him (issue #182: [What a partner meets under
full rules](full-rules.md)). The research notes,
with the harness's whole battle and its fields compared byte by byte,
and the link's timing traced, are kept out of the repository
(`.build/research/battles/engine-2v2.md`, `linkdelay.md`).

## Contents

- [Four streams](four-streams.md)
- [The partners' words in BN6's packet](partners-words.md)
- [The partners' own NaviStats](navistats.md)
- [The partners' own hands](hands.md)
- [A partner's own Custom screen](custom-screen.md)
- [The leads wait for their partners](leads-wait.md)
- [The partner's seat: his HP box and the four told apart](seat.md)
- [His loadout in his record](loadout.md)
- [His NaviCust, compiled by BN6](navicust.md)
- [His messages checked](messages-checked.md)
- [His Cross](cross.md)
- [A Program Advance and his hand's size](program-advance.md)
- [What a partner meets under full rules](full-rules.md)
- [A partner's UNITE](unite.md)
- [Proved: the partners' own folders and screens](proved.md)
- [Over the server](over-the-server.md)
