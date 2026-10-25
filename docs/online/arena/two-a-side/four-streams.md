---
type: Engine Mechanism
title: "Four streams"
description: "The partners as players of the Arena: four key streams (`arena_input.c`, `arena_link_local.c`, `arena_pair_step_all`), each partner's sample put in his side's GBA's link packet, and proofs that both GBAs agree under rollback."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-22T14:54:41Z }
---

# Four streams

The partners are players of the Arena's like the two GBAs' (21 October
2026):

- **The input** (`arena_input.c`) carries `ARENA_PLAYERS` streams: 0 and
  1 the GBAs' pads as before, 2 and 3 side 0's and side 1's partners
  (`arena_input_players`). A frame settles once every stream in play is
  known for it and the one after; a partner's key not yet come is
  guessed as his last held and rolled back where it came otherwise, as
  the other player's are. Two streams leave the partners' columns
  released, so a NetBattle without the flag takes the same keys as
  before.
- **The local link** (`arena_link_local.c`) queues each remote stream on
  its own, after the same latency: the peer's (player 1) and, with
  `arena_link_local_partners`, both partners', by the frame as the
  peer's. `--dev arena,arena2v2=1` gives them `arena_team_script`, by the
  frame's number alone (side 0's partner steps up to row 1 and fires
  charged shots along it; side 1's fires along row 1 and steps down and
  back between), so a run that rolls back and one that does not hand
  them the same keys. The link's `poll_player` takes them; the
  network's link has none (below).
- **The pair** (`arena_pair_step_all`) samples a side's partner's keys
  on that side's GBA's frames, as a GBA's pad is read: before a core's
  first run loop in a frame of its own clock, his keys for that frame
  (a core as far ahead as the keys go samples once the next frame of
  the pair gives it its own). The frame last sampled is each core's in
  the pair and in its snapshot.
- **The packets** (`arena_team_sample`, `arena_team_link.c`): the
  sample is handed to its side's GBA alone, as its pad is, and goes into
  the link packet that GBA sends for the frame, beside its lead's pad;
  both GBAs take it from the exchange that brings the lead's pad, at the
  same iteration ([The partners' words in BN6's
  packet](partners-words.md), which replaced the first
  design's rings: the sample queued in both cores' RAM at the sampling
  GBA's iteration plus 4, which two GBAs further apart than that took at
  different iterations). The spawn empties them with the battle; they
  are core RAM, so a rollback puts them back with the state and the
  pair's hash covers them.

Proved with BN5's ROM beside BN6's (full rules), delay 1, fresh data
folders, `--dev arena,arenaboth,arena2v2=1,teamhp=500,quitend` (the
partners' scripts, records, hands and own Custom screens as they stand
since, each partner's keys in his side's GBA's packets), on this build
(22 October 2026, the integration of issue #181):

| Run | End | Hashes | The two GBAs |
| --- | --- | --- | --- |
| `arenalag=4` (window 8: 285 rollbacks, 1,140 frames run again) | over at frame 3427, side 0 won | 57 agreed | 2,696 iterations, 0 apart; the partners' keys 0 iterations taken apart |
| `arenalag=0,arenawindow=8` (0 rollbacks) | over at 3427 | all 57 the same as the run above, frame for frame | 2,696, 0 apart |
| `arenalag=4,arenawindow=0` (the delay alone) | over at 3427 | 57 | 2,689, 0 apart |
| `arenalag=4,arenaroll=8` (8 frames rolled back every 8) | over at 3427 | all 57 the same as the delay alone's | 2,689, 0 apart; 428 rollbacks of 8 frames, the game's state the same after all 428 |
| `arena2v2=300,arenalag=4` (the mutation) | over at 3427 | 57 (the local link echoes them) | 6 of 2,696 apart, from iteration 300: the flip's six |
| `--dev rollbench,arena2v2=1` | 600 frames | | the battle held at its first Custom screen for partners who have no client there (the leads wait for them); rollbacks of 8: 11 of 11 the game's state the same |

(The delay alone stalls, and `arenaboth`'s first player is a script by
the tick, so its battle is another than the rolling one's; the rollback
tried runs with no window and comes back to its hashes.) A NetBattle
without the flag is the battle it was:
`--dev arena,arenaboth,arenalag=4,arenadelay=1` over at frame 2690 with
the last hash `86d2f837337a1494`, on this build as before the change.

Each MegaMan's HP is his own object's (`BN6_T1_HP`): in the battle
above both partners stand on the field's purple panels of columns 1 and
6 (poison, as the picture and the drain read: 1 HP each 7 iterations
from iteration 62, the leads beside them on plain ones untouched), side 0's partner's
charged shot takes side 1's partner from 40 to 30 at iteration 477 and
side 1's takes side 0's from 50 to 40 at 407, side 0's player's charged
shots take side 1's player from 100 to 0 (iterations 184 to 2344), and
side 0's player, whom nothing hits, stays at 100 throughout, his
partner deleted at iteration 685; both GBAs alike at every iteration.
On the first GBA's screen side 1's MegaMen carry their own HP under
them (89 and 51 at the battle's frame 600); a GBA's own partner has no
label.
