---
type: Engine Mechanism
title: "The partners' words in BN6's packet"
description: "Why a partner's keys and picks ride his side's GBA's own link packet (`arena_team_link.c`): the first design's queue drifted, while BN6's link reads the same exchanges in order at any distance in time."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-22T17:15:21Z }
---

# The partners' words in BN6's packet

**The fault.** The first design queued a partner's keys in both GBAs'
RAM at the iteration the sampling GBA's battle loop was at plus 4, and
set his picks there from that GBA's frame. That holds while the two
GBAs' loops run within 4 iterations of each other in time, and they need
not: two runs met it. Over the network with a partner who never picks
(his client sending nothing, the leads waiting to the limit), the two
GBAs parted from iteration 73 though every game and the spectator still
agreed (the hashes cover both GBAs alike, so all four games ran the same
two diverging GBAs). With all four MegaMen in Beast Out under the delay
alone (`teambeast=3,arenalag=4,arenawindow=0`), 71 of 2,375 iterations
apart from iteration 2305, side 1's partner's B held on the second GBA
and not on the first, 46 iterations taken late. A bigger margin would
only have moved the line.

**The cause, in BN6's link.** A lead's pad never meets this, because of
how BN6's link battle keeps its two GBAs together (the link study,
`.build/research/battles/linkdelay.md`). Each frame of a battle, each
GBA reads the exchange the cable delivered into its two receive buffers
(`BN6_LINK_IN`, side 0's packet then side 1's, every GBA alike), takes
both sides' pads from them into the key array, and writes its own pad
into its packet (`BN6_LINK_OUT`); the link's frame stages that packet,
the library queues it, the cable exchanges it with the other GBA's, and
four frames later both GBAs read that exchange. The library never drops
a packet, so a GBA that falls behind (a frame of work past its VBlank,
a hold) keeps a packet standing in its queue: the two GBAs then run
apart in time, by as many frames as stand, but each still reads one
exchange a frame of its battle, the same exchanges in the same order.
Frame n of one GBA's battle and frame n of the other's read the same
exchange: the two run the same battle at any distance in time. The
queue's depth is all the drift is; four MegaMen in Beast Out (more work
a frame) and a held packet (the wait) are what deepen it.

**The fix.** A partner's word now rides in his side's GBA's own packet.
BN6 writes eight halfwords a packet and leaves two bytes that nothing
of its own writes or reads: +5 and +7, the other halves of the word
stream's index (+4) and the ready flags (+6), each written and read as
a byte (every writer of `BN6_LINK_OUT` and reader of `BN6_LINK_IN` in
bn6f, matched to Gregar's bytes: [Online: the Square, the Arena and the link battle](../../../rom/online.md), A link battle's
packet). The library carries all eight halfwords. So:

- The pair hands a side's GBA its partner's value for each of its
  frames before the frame runs, as it hands it its pad (`team_link_put`,
  into that GBA's RAM alone, `RAM_TEAM_LINK`): keys kept as the last held
  for its next packet, a message of his Custom screen (a pick, his
  Cross, a Program Advance) queued in an outbox of twelve
  (`team_outbox.c`: a turn's messages are nine at most, his Cross, his
  Soul, ColonelSoul's chip, a Program Advance and five picks, so a
  whole turn with room to spare), one a packet. A message past it is
  refused and said in the output ("his outbox full"), never lost unsaid;
  no run has queued more than two.
- Where the battle reads both sides' received packets and stores its own
  pad into its packet, a hook reads each side's partner's word out of
  that side's received packet (where its pad's mark is set, as BN6 asks
  before it takes the pad; side 1's whatever its mark where the link's
  state says so, `BN6_LINK_TAKES_ANY`, the routines' own test), then
  writes this GBA's own partner's word into +5 and +7 of its packet.
  sub_801FF18 reads and stores together (`BN6_LINK_PAD_STORE`,
  `0x0801FF8C`: both at once). sub_801FFD6, the other routine of the
  battle's link table, skips the read with bit 3 of its argument and the
  store with bit 4 (`0x08020052`; the library sets bit 4 where its send
  queue cannot take the packet, `0x0803EC28`), so there the words are
  taken at the end of its read path (`BN6_LINK_READ_B`, `0x0802001C`)
  and put at its store (`BN6_LINK_PAD_STORE_B`, `0x08020062`) each as
  BN6 runs it: a read without a store takes the partners' words with the
  pads, and a message waits in the outbox for the next store. The
  cable's GBAs run sub_801FF18 (the library's mode 4; sub_801FFD6 runs in
  modes 8 and 12), and every run below says "the link's other routine
  read 0 and 0 times".
- Each GBA works the partner's turn out from those words on its own
  (`arena_team_custom_link`): his deck dealt as its Custom screen first
  opens (seeded by its random number, the same on both at that frame),
  his turn opened and closed with BN6's phase, his picks in as their
  last message comes, his hand set at the battle loop's next iteration
  after his side's screen has closed with them in. Where his client has
  gone quiet, the turn's limit is said by his side's GBA in his place:
  at `TEAM_TURN_LIMIT` frames after the turn's first OK ([The turn's
  limit](leads-wait.md)) it queues "no picks" in its own outbox (marked said only once queued:
  were the outbox full it tries again the next frame), and both GBAs
  take it from its packet as they would his own. No byte of his turn is written into one
  core by the other any more.
- At `BN6_KEY_COPY` each partner's keys are those the last packet of his
  side brought (`team_link_keys`), none on a Custom screen or in a
  message's frame, as before.

**Why it cannot drift.** Both GBAs take a partner's word from the same
exchange as his lead's pad, in the same frame of their battles: the
invariant a NetBattle of two already rests on. A distance between the
two GBAs in time that parted their partners would part their leads
too, and BN6's own battle with them. The words keep BN6's four-frame
pipeline, the timing a lead's keys have (the owner's call: the link's
timing kept), and the wire is unchanged: a partner's stream crosses the
server as before, and only inside each game's pair does it ride the
cable. A NetBattle of two never writes the two bytes (no hook without
two a side). The runs on the cable traced it: a partner's pick written
at +5/+7 of one GBA's packet came out of both GBAs' receive buffers four
frames later with the same sequence (`AF` on both), and the outbox
held at most a message or two.

Proved, 22 October 2026, fresh data folders, both ROMs (the runs before
the fix as their branches reported them):

| Run | Before | Now |
| --- | --- | --- |
| `--dev arena,arenaboth,arena2v2=1,teamhp=500,arenadelay=1,quitend,teambeast=3,arenalag=4,arenawindow=0` (all four in Beast Out, the delay alone) | 71 of 2,375 iterations apart from iteration 2305 | over at frame 7075, 117 hashes, 2,376 iterations 0 apart, the partners' keys 0 taken apart |
| ... `arenaroll=8` | as above | 884 rollbacks of 8 frames, the game's state the same after all 884, its 117 hashes the delay alone's |
| Four games and a spectator through a local server, Beta's client sending nothing (`teamslow=-1`, the leads waiting to the limit) | the GBAs apart from iteration 73 on every game, the hashes agreeing | over at frame 5810 on all four, 96 hashes agreed and the spectator's 96 checked, the GBAs 0 apart over 1,325 iterations on every game; side 0's GBA said his picks none at 3,600 frames held |
| `--dev arena,arenaboth,arena2v2=1,teamhp=150,teamslow=-1,arenadelay=1,quitend,arenalag=4` (the same, one machine) | over at 13010, 3,155 iterations 0 apart | over at 16730, 278 hashes the same as `arenalag=0,arenawindow=8`'s, 3,230 iterations 0 apart |

Every battle of the flag ends otherwise than before (a partner's keys
now as late as BN6's pipeline makes a lead's, frame for frame rather
than iteration for iteration, and his picks in his hand at the next
iteration rather than four on), so the tables below are this build's.
Run again on the integration's last build (with [A partner's
UNITE](unite.md) and the Souls' modules by object), the local
runs give the same hashes, frame for frame: all four in Beast Out with
the delay alone and with `arenaroll=8`, the Cross, the Program Advance,
the slow and the gone partners, the NaviCust from the playtest's
checkpoint, the DarkInvs rooms from the rules branch's, the plain run.

Run again on issue #181's review's build (the outbox of twelve,
sub_801FFD6's read path, [His messages
checked](messages-checked.md), the NaviCust over the way in's state),
22 October 2026, fresh data folders, both ROMs: every local run ends as
it did, on the same frame, with the same iterations, rollbacks and
number of hashes, the two GBAs 0 apart, no message refused, no outbox
full, the link's other routine never run: all four in Beast Out with the
delay alone and with `arenaroll=8` (over at 7075; 884 rollbacks of 8
frames, the game's state the same after all, the 117 hashes the delay
alone's), the Cross (4800), the Program Advance (2035), UNITE with
`teamunite=1`, `2` and `3` (4846, 7766, 6204), the slow and the gone
partners (5197, 16730), the NaviCust from the playtest's checkpoint
(3530; `teamcust=-1` apart from iteration 1), the DarkInvs rooms (2261),
the plain run with `arenawindow=0` (3427). The hashes' values are new,
`RAM_TEAM_LINK`'s layout being new; a NetBattle of two is untouched
(frame 2639's hash `86d2f837337a1494`, over at 2690). Four games and a
spectator through a local server (`drive.py`):

| Run | End | The hashes, the GBAs |
| --- | --- | --- |
| Four players of different runs, three of them on other runs' checkpoints, and a spectator with none (`--diverse`) | over at 1255 | one ready hash, `3cf71bb612a0215c`, on all four; 20 agreed, 20 checked by the spectator; 949 iterations 0 apart on all five |
| ... `teamcust=-2` on Beta's game (the mutation) | stopped before the battle on all four, "went apart" | Beta's ready hash `1d33c68692f9c1c4`, the others' `3cf71bb612a0215c` |
| Beta's client sending nothing (`teamslow=-1`) | over at 5810 | 96 agreed; 1,325 iterations 0 apart on all five; the limit said in his place |
| Beta's HeatCross, Delta's Program Advance | over at 1020 | 16 agreed, 17 checked; 390 iterations 0 apart on all five; the Cross asked twice, the PA in his hand |
| Beta's UNITE, Delta's Beast Out | over at 967 on all four | 16 agreed; 390 iterations 0 apart on the four (the spectator, at the referee, met a line of his more than the harness pressed through, and did not watch; it watched every other run) |
| Beta's Beast Out, Delta's ElecCross, a relay of 40 ms and 30 of jitter between the games and the server | over at 1855 | 30 agreed, 30 checked; 973 iterations 0 apart on all five |
| Beta slow, 10 seconds a turn (`teamslow=600`) | over at 1850 | 30 agreed, 30 checked; 965 iterations 0 apart on all five |
| Delta's game killed three seconds in | "stopped at frame 190: the other player left the NetBattle" on the other three | 3 iterations 0 apart |
| Delta's game frozen 300 frames (`arenafreeze=300`) | "match 2 stopped: the other's keys stopped coming" on the other three | as before |
| One by one, the relay (`relay_drive.py`) | three bouts, over at 2690, 655 and 2450; TEAM WIN for Alpha and Beta | 44, 10 and 40 agreed, the others' games watching each and checking as many |
