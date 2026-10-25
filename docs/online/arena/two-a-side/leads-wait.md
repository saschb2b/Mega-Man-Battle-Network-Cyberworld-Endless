---
type: Engine Mechanism
title: "The leads wait for their partners"
description: "The leads' Custom screen waiting for their partners (`arena_team_wait.c`): each side's GBA holds back its link packet at `BN6_LINK_SEND`, BN6's Cstmzing... and our signs, and one 30-second turn limit for all four."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-22T17:49:40Z }
---

# The leads wait for their partners

The owner's call (22 October 2026): the leads' Custom screen waits for
a slow partner, his picks waited for; a sign tells each partner his
lead is ready, and each lead that his partner still picks; and an
absent partner cannot hold the others forever.

The game-design lens first. The dialectic is mine against ours again:
each player's turn is his own, but the battle goes on for all four
only together, so the wait must read as a teammate taking his time,
never as a fault. The loop layer is the battle's turn (pick, wait, fight)
inside the session's "one thing together". The patterns: *telegraph
honestly* (the wait is shown where it is, and by whom), *presence over
pressure* (a quiet sign that someone is there and busy; no countdown, no
nagging, no word of explanation: the players know BN6's Custom screen),
*diegetic first* (BN6's own way of showing a GBA still picking before
any sign of ours), and *respect the player's time* (a limit past which
the idle one cannot hold three others: first 60 seconds for a partner
alone, now one of 30 for all four, below).

**How BN6 waits, and where the hold goes.** In a link battle each GBA,
as its lead's OK has slid the window out, sends its packet (bn6f
sub_800B3A2, `BN6_LINK_SEND`: the mark `0x56789123` at `0x0203CBE0`, the
hand its OK built, its battle NaviStats, its Cross's record) and waits in
mode `0x14` (`BN6_CUSTOM_WAIT`, sub_8026DC4) until both sides' packets
have come with that mark (`0x0203F4A0` and `0x0203F5A0`, which every GBA
receives, checked each frame at `BN6_CUSTOM_WAIT_LOOP`); then the screen
closes on both. While it waits, the other GBA shows "Cstmzing..."
blinking over the side still picking (seen in the pair's captures). So the hold is the packet: a
side's GBA keeps its packet back while its partner picks
(`arena_team_wait.c`: its send skipped at `BN6_LINK_SEND`'s entry and
owed, then sent from `BN6_CUSTOM_WAIT_LOOP` once his picks are in). Both
GBAs then wait by BN6's own cable, so the moment the screen closes is
the cable's, one moment every client and spectator agrees on; the hold
itself is read in that GBA's RAM (his turn's state, which each GBA works
out from his words in the packets), never a clock.
The lead's OK is answered as BN6 answers it: the window slides out and
the battlefield shows, as when he waits for the other lead.

**The signs.** On the other side's GBA, BN6's own "Cstmzing..." blinks
over the side held for its partner, as over a lead still picking: no
drawing of ours. On a lead's own GBA, whose partner stands on his own
half where BN6 shows nothing, a quiet plate of ours under the name
plate: his partner's seat (`P3`, `P4`) and three dots, one lit in turn,
while his partner picks (`arena_team_view.c`, `sign`). On the partner's
own screen, the same plate with his lead's seat and a tick once his
lead has pressed OK and waits for him. No words, no count.

**The turn's limit: one for all four** (the owner's call, 22 October
2026, replacing a partner's alone of 60 seconds). Once any player of
the match has pressed OK on a turn, every player still picking has 30
seconds (`TEAM_TURN_LIMIT`, 1,800 frames); then his picks so far are
taken. A battle of two keeps BN6's own Custom screen, which has no
limit.

The game-design lens first. The dialectic is still mine against ours:
a turn is each player's own, but the battle goes on only for all four
together, so one player's thinking is three players' waiting. A limit
that binds only the partner made the two roles unequal (a lead could
hold three others without end, a partner a minute at most); one limit,
started by the first OK and binding whoever is still picking, is the
same for everyone, and it binds nobody who is quick, so a fast turn is
never shortened. The loop layer is the battle's turn (pick, wait,
fight) inside the session's "one thing together": the limit keeps that
loop turning without making the pick itself a race. 30 seconds is
generous for BN6's Custom screen (a turn there is a few seconds; the
players' communities settle a turn in a minute at most), and short
enough that the three who wait are not left watching a still screen;
the owner accepted 60 and asked for it lower where it feels unfair to
the three, so it is a constant to tune, not a rule of BN6's. The
patterns: *telegraph honestly* (the late player alone is told that his
time is running out, in the last 10 seconds, `TEAM_TURN_HURRY`, by a
sign that cannot be misread as anything else, and it never lies: it
shows exactly while the limit approaches and he still picks);
*presence over pressure* (no countdown, no number, no word, nothing on
the others' screens beyond the signs they already have: his own seat's
plate under the team's sign, gold, blinking fast as BN6's own
"Cstmzing..." blinks, `arena_team_view.c`, `hurry_sign`); *diegetic
first* (the others still see BN6's own "Cstmzing..." over a side that
waits, and their own team's sign); and *respect the player's time*
(the idle cannot hold the others past the limit; the late lose
nothing they picked).

**How each one's picks are taken.** Each of the pair's cores keeps the
turn's clock in its RAM (`RAM_TEAM_CLOCK`, `arena_team_custom_clock`,
from the battle's link each frame): frames since the first OK it sees,
its own lead's (his screen past OK, `BN6_MODE_OK`, `BN6_MODE_PA` or
`BN6_MODE_WAIT`) or a partner's (his picks in while his turn is open),
none outside the Custom screen. Each game reads the later of its two
cores' clocks (`team_hurry_clock`) and acts for its own player alone,
in his own stream, so the games need not read it alike:

- A lead's own game, past the limit with his screen still open, presses
  for him: B where CROSSSELECT is open (his Cross not taken), right to
  OK (up from the Beast Out emblem under it), then A, a press every
  eight frames (`team_hurry_lead`, from `square_bout_lead` over the
  network and the local scene's keys under the dev flag). His picks so
  far stay picked: BN6's OK takes them. Nobody's game presses for anyone
  else, and the keys are his own, in his own stream, so every game and
  spectator takes them alike.
- A partner's own client sends the picks he has two seconds before the
  limit (`TEAM_WAIT_SAY`, 120 frames), as it did before the wait's old
  limit.
- A partner whose client has gone quiet: his side's GBA says "no picks"
  in his place at the limit by its own clock (`limit_frame`), in its own
  packet, so both GBAs take it at the same frame as they would his own.
  The hold of the leads' screen for him (the packet held back) is as
  before.

Proved (`--dev arena,arenaboth,arena2v2=1,arenadelay=1,quitend,arenalag=4`,
both ROMs, fresh data folders, 22 October 2026):

| Run | End | The two GBAs | The limit |
| --- | --- | --- | --- |
| `teamhp=500,teamidle=1` (side 0's lead never touches his Custom screen) | over at frame 8780 | 2,818 iterations, 0 apart; 146 hashes | "the turn's limit, 1800 frames after its first OK: side 0's lead's own game presses OK for him", each turn |
| `teamhp=500,teamidle=2` (side 1's lead) | over at 8848 | 2,818, 0 apart | the same for side 1's lead |
| `teamhp=150,teamslow=99999` (a partner who never picks) | over at 10610 | 3,067, 0 apart | his client sends his picks so far at 1,680 frames, each turn |
| `teamhp=150,teamslow=-1` (a client gone quiet) | over at 10370 (16730 with the old 60 seconds) | 3,030, 0 apart | his side's GBA says none at 1,800 frames, each turn |
| `teamhp=500,teamslow=600` (10 seconds a turn, under the limit) | over at 5197, as before | 2,755, 0 apart | none reached |

The hurry sign captured (`teamidle=1,teamseat=0` and
`teamslow=99999,teamseat=2`, kept out of the repository): the idle
lead's Custom screen with his gold `P1` plate under the Square's name
plate, blinking; the idle partner's with his lead's `P1` and its tick
and his own gold `P3` under it. A NetBattle of two is untouched (frame
2639's hash `86d2f837337a1494`, over at 2690).

Over the server (`drive.py`, four games and Epsilon watching, this
build): Alpha's lead idle on his Custom screen (`teamidle=1` on his
game) and Beta, Alpha's partner, never picking (`teamslow=99999` on his):
Beta's client sent his picks so far at 1,680 frames after the turn's
first OK, Alpha's own game pressed OK for him at 1,800, each turn; over
at frame 5570, 92 hashes agreed on each of the four and checked by the
spectator, 1,163 iterations 0 apart on all five. The other four-game
runs on this build as before: the four different checkpoints (over at
1255, 20 hashes), HeatCross and a Program Advance (1020), UNITE and Beast
Out (967), Beast Out and ElecCross over 40 ms and 30 of jitter (1856),
the slow partner (1850), the drop (stopped at 186 on the other three),
the freeze, the relay one by one (three bouts, over at 2690, 531 and
3290, every hash agreeing and the waiting two checking as many, TEAM
WIN for Alpha and Beta); a client gone quiet now ends at 3659 (5810 with the old 60
seconds), his side's GBA saying none at 1,800 frames. On Epsilon's
screen the 2v2's heading: "Alpha & Beta VS Gamma & Delta" (the form
with WATCHING too wide for the picture).

Proved with the old limit, a partner's alone, 60 seconds held
(`--dev arena,arenaboth,arena2v2=1,arenadelay=1,quitend`, both ROMs,
fresh data folders, 22 October 2026; the turn's limit's runs are
above):

| Run | End | The two GBAs, the hashes | The wait |
| --- | --- | --- | --- |
| `teamhp=500,teamslow=600,arenalag=4` (side 0's partner's hands off his screen 10 seconds each turn; 461 rollbacks) | over at frame 5197 (3427 without him slow) | 2,755 iterations, 0 apart; 86 hashes | his picks taken each turn, none dropped |
| ... `arenawindow=0` and `arenaroll=8` | over at 5205 | 2,755, 0 apart; 650 rollbacks of 8 frames, the game's state the same after all 650; the 86 hashes of the two the same | as above |
| `teamhp=150,teamslow=99999,arenalag=4` (a partner who never picks) | over at 7970 | 1,830, 0 apart | his client sends the picks he has at 3,480 and 3,485 frames held, both turns; the screen goes on |
| `teamhp=150,teamslow=-1,arenalag=4` (a client gone quiet, sending nothing) | over at 16730 | 3,230, 0 apart; all 278 hashes the same as `arenalag=0,arenawindow=8`'s | his side's GBA says his picks none at 3,600 frames held, both turns |
| `teamhp=500,arenalag=4` (no one slow) | over at 3427 | 2,696, 0 apart | a partner's OK comes a few frames after his lead's |

The mutation of these runs is the picks' (`teampick=-1`, the table
below: apart from iteration 190): the hold itself cannot part the GBAs,
since it crosses by BN6's cable; held wrongly it would change the
battle on both alike. A NetBattle without the flag is the battle it was
(over at frame 2690, last hash `86d2f837337a1494`).

Captured (`teamslow=600`, kept out of the repository): side 0's lead's
screen with his window slid out after his OK, the battlefield and his
partner's sign (`P3` and its dots) under the name plate; side 1's lead's
screen with BN6's own "Cstmzing..." blinking over side 0 for the ten
seconds; side 0's partner's own screen, BN6's Custom screen with his
deck still untouched, and his lead's sign (`P1` and a tick).
