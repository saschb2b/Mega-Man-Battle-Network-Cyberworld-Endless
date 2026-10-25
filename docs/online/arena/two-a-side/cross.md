---
type: Feature
title: "His Cross"
description: "A partner's Cross and Beast Out (`arena_team_cross.c`): his own CROSSSELECT on his screen copy, `TEAM_CROSS` in his stream, three hooks on BN6's crossing controller for his object, and his Beast Out's turns."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-22T14:54:41Z }
---

# His Cross

BN6's crossing controller (`BN6_CROSS_SIDES`, bn6f dword_20367F0) keeps
a record a side, copied from the turn's packets as the Custom screen
closes, and crosses those records' objects alone: each side's player
([What a partner meets under full rules](full-rules.md), read in bn6f; now seen).
A partner's Cross (`src/online/arena_team_cross.c`, 22 October 2026):

- **His choice** is his own CROSSSELECT's: his screen copy holds his own
  Crosses' flags (his lead's off) and the Crosses he has chosen this
  battle (`BN6_CROSSES_USED`, kept by his client from his last OK), so
  BN6 lists his. At his OK the form his CROSSSELECT committed
  (`BN6_CROSS_RECORD` +0) goes in his stream before his picks
  (`TEAM_CROSS`: the pick message's count 7, the form), and both GBAs
  keep it as his pending form for the turn (`RAM_TEAM_CUSTOM`), before his
  side's GBA can send its packet: the leads wait for him, so every GBA
  has it before either can begin the turn's crossing.
- **BN6's crossing for his object**, by three hooks: the turn's form check
  (`BN6_TURN_FORMS_SET`) sends the controller to its crossing where a
  partner on the field has a Cross, as where a record has one; the
  crossing's step (`BN6_CROSS_ACTIVATE`) asks his object to cross at its
  first frame as BN6 asks a record's (`BN6_AI_CROSS_ASK` into his AI
  data) and holds BN6's wait while he still crosses (`BN6_AI_CROSSING`);
  and the crossing's phases, which read the pending form by the side
  (`BN6_CROSS_PENDING_GET`), are answered his own where the crossing
  object is his. His form is then written into his own record (the
  NaviStats answered by object), so his element, his buster and the
  weakness that breaks it are his; the leads' Crosses go as before.
- **What we met**: the step is reached by `mov lr,pc; bx r1`, which leaves
  lr's Thumb bit clear; a hook's return there ran the game on in ARM and
  stopped it on the crossing's black field. The wait returns by a jump to
  lr | 1, as the routine's own `pop {pc}` does.

Proved with fresh data folders, `--dev
arena,arenaboth,arena2v2=1,teamcross=1,teamhp=500,arenadelay=1,quitend`
(each partner's client takes HeatCross on his CROSSSELECT at his first
turn), this build:

| Run | End | Hashes | The two GBAs |
| --- | --- | --- | --- |
| `arenalag=4` (379 rollbacks) | over at frame 4800 | 80, all the same as `arenalag=0,arenawindow=8`'s (0 rollbacks) | 3,499 iterations, 0 apart |
| `arenalag=4,arenaroll=8` | over at 4810 | all 80 the same as the delay alone's (`arenawindow=0`) | 3,499, 0 apart; 601 rollbacks of 8 frames, the game's state the same after all 601 |
| `teamcross=-1,arenalag=4` (no Cross on the second GBA, the mutation) | | | apart from iteration 1: the crossing comes before the fight's first iteration, and the second GBA's battle begins its frames earlier |

His form is folded into the GBAs' comparison (the records' form). Seen:
his CROSSSELECT listing HeatCross alone, his emblem taken; on the first
GBA (`teamseat=2`) after the first turn's crossing both partners in
HeatCross and both leads as they were, "TURN 1 START!" over them; on the
second (`teamseat=3`) BN6's black crossing field with both partners
crossed. The log: "the partners' Crosses: the turn's crossing begun for a
partner alone 4 times, side 0's partner asked to cross 2 times and side
1's 2, BN6's wait held for a partner still crossing 204 frames, their own
pending forms answered 210 and 210 times, every core's" (each GBA's, the
rollbacks' frames run again among them). His pending form is taken as the
crossing's last mode begins (`BN6_CROSS_RESTORE`), as BN6 takes the
records' once their objects have crossed: before that, kept to his next
turn, the turn's check as a turn's change begins (`0x0800840C`) met it
again and began the crossing anew for him (asked 6 times where 2 is right,
the first proof's runs).

**His Beast Out, and its turns as a lead's.** Gregar's emblem under OK
commits a Beast Out at OK into the same record as CROSSSELECT
(`BN6_BEAST_COMMIT`, `0x08029344`: form 0xB, 0xD on from a Cross, 0x17
BeastOver), so it crosses in his stream and through BN6's crossing as his
Cross does: `teambeast=1` sets Beast Out's flag on his screen copy (his
record's BeastOutCounter, 3, as his side's), his client takes the emblem
at his first turn (RIGHT five times, DOWN, A, then UP and A on OK), and
both partners turn into Gregar's beast at the first turn's crossing while
their leads stay as they are, on both GBAs. A Beast Out's turns BN6
counts for each side's player alone: his AI data's turn count as each
Custom screen opens (`BN6_FORM_TURN_DOWN`, called with the side-player
getter's object; two of its call sites are the Souls' hooks,
`BN6_TURN_COUNT`), his BeastOutCounter as the crossing controller is done
(`BN6_BEAST_TURN`), its end in the turn's form check for the records'
objects (`BN6_FORM_END_CHECK`, then `BN6_FORM_LEAVE_ASK`) and the wait
while he leaves it in the controller's mode 8 (`BN6_CROSS_RESTORE`). For a
partner each is BN6's own code run for his object: the two counts' ends
hooked (`BN6_FORM_TURN_DOWN_END`, `BN6_BEAST_TURN_END`) and their bodies
entered again with his object where the one just done was his side's
player; the end's check and the leave asked from the turn check's end
(`BN6_TURN_FORMS_SET`) for each partner without a form this turn, BN6's
routines called and come back to the hook (the run kept a core, never past
the frame); mode 8 waiting while he leaves his form. (The controller BN6
sets up as each screen opens for the two players, `BN6_TURN_SIDES`, asks a
routine that answers 0 in Gregar: nothing there for him.) All four
MegaMen run the same code each turn, a partner's own forms aside, so the
partners' pair hashes differ from the first proofs' while every battle
ends at the same frame (`teamcust`, `teampa` and the flag alone; `teamcross`
by its pending form above).

The control: `teambeast=3` has the leads take Beast Out by the same keys
(its flag on both GBAs as the field spawns), `teambeast=2` the partners at
every turn. At each turn's form check (the log's "the turn's check at
iteration ..." lines) each lead and his partner are in the same form with
the same BeastOutCounter: form 0xB and 3 at the first, 0xB and 2 at
iteration 1084, 0xB and 1 at 1897 in the delay alone's run (the first
GBA's; with the window of 8, 0xB and 2 at 1088 before the battle ends).
Proved, fresh data folders, `--dev
arena,arenaboth,arena2v2=1,teamhp=500,arenadelay=1,quitend`:

| Run | End | Hashes | The two GBAs |
| --- | --- | --- | --- |
| `teambeast=1,arenalag=4` (260 rollbacks) | over at frame 3169, side 1 won | 52, all the same as `arenalag=0,arenawindow=8`'s (0 rollbacks) | 2,167 iterations, 0 apart |
| `teambeast=1,arenalag=4,arenaroll=8` | over at 3169 | all 52 the same as the delay alone's (`arenawindow=0`) | 2,160, 0 apart; 396 rollbacks of 8 frames, the game's state the same after all 396 |
| `teambeast=-1,arenalag=4` (no Beast Out on the second GBA, the mutation) | | | apart from iteration 1 |
| `teambeast=3,arenalag=4` (the leads too; 522 rollbacks) | over at 6266, side 0 won | 104, all the same as `arenalag=0,arenawindow=8`'s | 2,896, 0 apart |
| `teambeast=3,arenalag=4,arenawindow=0` (the delay alone) | over at 7075 | 117 | 2,376, 0 apart |
| `teambeast=3,arenalag=4,arenaroll=8` | over at 7075 | all 117 the same as the delay alone's | 2,376, 0 apart; 884 rollbacks of 8 frames, the game's state the same after all 884 |

(This build's; the build before the packets parted the last two at
iteration 2305, 71 of 2,375 iterations apart: in that schedule, with all
four in Beast Out, the two GBAs' battle loops came more than 4
iterations apart in time and the partners' keys, queued 4 on, were taken
late on one GBA, side 1's partner's B held on the second GBA and none on
the first. With the partners' turn hooks switched off it parted at the
same iteration, so it was not a partner's form: [The partners' words in
BN6's packet](partners-words.md) has the cause and its
end.)
