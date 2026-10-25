---
type: Proof
title: "Proved: the partners' own folders and screens"
description: "The proof of the partners' own folders and Custom screens (22 October 2026): runs with rollback, the delay alone and a mutation, both GBAs taking each partner's picks at one iteration, his screen's costs."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-22T14:54:41Z }
---

# Proved: the partners' own folders and screens

The player's Gregar ROM and Team Colonel's, a fresh data folder each,
`--dev arena,arenaboth,arena2v2=1,teamhp=500,arenadelay=1,quitend`, both
partners' clients stood in for on the machine (their screens' keys a
script by the frame: the cursor right and A every 20 frames), side 0's
lead tapping R every 600 frames in the fight so that the Custom screen
opens again once its gauge is full (three turns; with `teamhp=150` the
partners are deleted before the second), 22 October 2026, with the leads
waiting for their partners (below):

| Run | End | Hashes | The two GBAs | His picks |
| --- | --- | --- | --- | --- |
| `arenalag=4` (285 rollbacks, 1,140 frames run again) | over at frame 3427, side 0 won | 57, all the same as `arenalag=0,arenawindow=8`'s (0 rollbacks) | 2,696 iterations, 0 apart | three turns each, set as his side's screen closed, taken by both GBAs at the same iteration |
| `arenalag=4,arenaroll=8` | over at 3427 | all 57 the same as the delay alone's (`arenawindow=0`) | 2,689, 0 apart; 428 rollbacks of 8 frames, the game's state the same after all 428 | as above |
| `teampick=-1,arenalag=4` (a chip short in his hand on the second GBA alone, the mutation) | over at 3836 | 63 (the local link echoes them) | apart from iteration 1 (2,696 of 2,696) | |

Each partner's hand's chip up next moves from his own picks alone:
side 0's at iterations 166 and 190, 1361 and 1393, 2145 and 2625, side
1's at 418, 818, 1373, 1773 and 2397, both back to 0 at 1028 and 1983 as
their picks of the next turn go in. (Before the wait, the same runs:
2,725 iterations, 57 hashes, the chips 11 iterations later; with
`teamhp=150`, one turn each, 1,314 iterations 0 apart, 30 hashes the
same with 173 rollbacks and with none, `arenaroll=8` 230 of 230.) A
NetBattle without the flag is the battle it was:
`--dev arena,arenaboth,arenalag=4,arenadelay=1` over at frame 2690, the
last hash `86d2f837337a1494`.

His client's screen: a frame 1.3 to 1.5 ms (the worst 2.1); the pair's
copy into it 0.14 ms at the first turn and some 20 ms at the later ones
(the worst 31: a frame or two of his own screen held as a turn begins).
Timed apart: the pair's save 0.05 to 0.08 ms every turn, its load into
the copy 0.08 ms at the first and 17 to 19 ms at the later ones, inside
mGBA's state load (not yet looked into). Made once at the first turn,
some 155 ms (the battle's worst tick then, 3 past a 60th in a battle).
His screen offered no Cross and no Beast Out then (their flags cleared on
the copy's GBA); since His Cross it offers his own Crosses and Beast Out
(`teambeast`).

Captured (`teamseat=2` and `3`, the pictures kept out of the
repository): BN6's own Custom screen on each partner's client with his
own deck (side 0's: BusterUp *, Sword S, Cannon B, Atk+10 *, MiniBomb L,
where his lead's GBA offers a Cannon A first; side 1's: WhiCapsl *,
Sword S and three chips of code D), his picks in BN6's column as he
takes them, his OK and BN6's "Sending chip data..." after it; his second
turn opening on the chips he left (Cannon B and MiniBomb L first, then
Cannon A, Sword S and an L chip), as BN6 keeps the chips not taken at
the deck's front; in the fight his HP box (150), the four marks, his
lead's HP under his lead, and his own chips' icons over him.
