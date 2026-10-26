---
type: Feature
title: "Over the server"
description: "Two MegaMen a side over the server (issue #181, protocol v15): the challenger asked One by one or Side by side, four seats on the wire, one pair on all four games, the four-game proof."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-22T17:15:21Z }
---

# Over the server (protocol v15)

Two MegaMen a side between four games over the server (issue #181, 22
October 2026): a team challenging a team in the Net Square names its
format, the relay ([Team NetBattles](../teams.md)) or side by side,
and side by side all four play one battle.

**The design.** The game-design lens first. The dialectic is Team
NetBattles' own, mine against ours, pushed to its end: in the relay a
MegaMan is the team's one at a time, side by side he is the team's at
every moment (his shots, his chips, his place on the field next to his
partner's). The loop layer is the session loop's "one thing together",
now with nobody waiting in the stands. The patterns: *subtract* (one
question with two answers, asked of the challenger as his Battle goes;
no settings screen, no order asked side by side: each team's captain
leads, his game the GBA of his side), *telegraph honestly* (the
challenged team reads the format in its question before its Yes),
*diegetic first* (MegaMan asks in BN6's chat box, the way in and the
card are the PET's panels) and *give every player a verb* (side by side
nobody watches; each of the four plays the whole battle). The words are
MegaMan's: "How do we fight 'em,Lan?" with `One by one` and `Side by
side` (B: no challenge), and on the other team's side "NAME's team wants
to fight side by side!", "Take it,Lan?" ([Voice](../../../voice/rules.md#one-beat-a-box): one beat a box,
nothing explained; the formats are named by what they look like, as a
kid would say them). No number about the link is shown anywhere.

**The wire** (shared/online/README.md, protocol v15):

- ARENA's last byte is a team challenge's format (`team_relay` 0,
  `team_side_by_side` 1); ARENA_STATE tells it to the challenged and the
  challenger, TEAM_STATE to all four (a relay's order is asked only for
  the relay).
- Side by side the team match is one battle of four **seats**: seat 0
  the challenger's team's captain (the cable's first GBA), 1 the other
  captain (the second GBA), 2 and 3 their partners. All four send ARENA
  accept when their screens are clear; START goes to all four once all
  four have, each with its seat (ARENA_STATE's side byte, 0 to 3) and the
  four seats' sessions, the delay the longest of the four latencies'.
- What a seat sends (its MegaMan, its keys, its hashes, its ready and
  installs' record, its lag notes) goes to the three others as
  **ARENA_SEAT** (35): the seat, then the message as sent. A partner's
  keys keep their sixteen bits (his Custom screen's picks, bit 15).
- Seat 0 reports the battle (ARENA_STATUS) and sends its end, as the
  cable's first ever did; the winner's team wins, no winner is a draw, and
  a leave, a decline or a dropped link of any of the four ends it broken
  off for both teams (TEAM_STATE over, result 3). All four are busy in
  the Square's marks while it runs.
- The stands keep four seats' MegaMen and keys: WATCH_STATE names the
  partners' seats, WATCH_RELAY and WATCH_KEYS carry the seat, and the
  cable's first's hashes are relayed as ever. STANDS's bout byte says
  side by side (`stands_side_by_side`, 255): the referee calls "TEAM
  NETBATTLE! SIDE BY SIDE!", "ALPHA'S TEAM VS GAMMA'S!" (the leads'
  teams), then the leads' HP as ever; a spectator's end says "Alpha's
  team won the NetBattle.".

**Every game runs the same pair.** Each of the four games (and a
spectator's) builds the pair from the four MegaMen as they came: seats 0
and 1's into the two GBAs as a NetBattle's, seats 2 and 3's into the
partners' own records (`arena_seats.c`: each partner's folder dealt into
his deck, his max HP, base HP and buster in his NaviStats), the dev
flag's two-a-side hooks switched on for the pair (`arena_seats_on`), so
that the pair's hash covers them. The network's link holds four seats
(`arena_link_net_seats`): each other seat's stream, hashes, ready and
record apart; the engine puts my keys in my seat's stream and takes the
three others' (`arena_link_seat`, `poll_player`), each guessed and rolled
back as before. A ready, a record or a hash is said once every other
seat's has come: mine where all three agree with mine, else the first
apart, so that a game apart from any of the others stops before the
first frame or at the next hash. The time sync keeps to the other GBA's
stream (a partner's to the other side's lead's) and its lag notes; a
partner's own keeps to his round trip.

**The partner's own client.** A partner's game shows his side's GBA and
makes his own Custom screen (BN6's, on a plain copy of the pair:
[A partner's own Custom screen](custom-screen.md)) as the
way in ends, before the battle's first frame, so its cost (some 155 ms)
falls outside the battle. Each tick his keys go through his own client
(`team_client_stream`): on his screen while his side's GBA's is open,
his picks crossing as pick messages at his OK (or at the wait's limit),
his battle keys otherwise. The leads wait for him as in the dev runs,
by BN6's own cable, under the turn's one limit for all four (30
seconds from the turn's first OK, `TEAM_TURN_LIMIT`).

**The way in and the card.** The way in's panel shows four plates, my
team's two on the left (its captain first, mine lit), theirs on the
right, each MegaMan's HP as he goes in and his PET's wait (coming,
linking, ready) under it; the band says `Side by side`. The battle shows
my seat's screen (a partner's own Custom screen in his turn, the four
told apart by their seats' marks); a stall names whose PET of the three
it waits for. The result card says the team's result (TEAM WIN!, TEAM
LOSE, TEAM DRAW, TEAM BROKE OFF), against the other team's two names,
each MegaMan's HP left (my team's on the left) and the time; no
standing, no next, no rematch.

**The four-game proof** (22 October 2026, protocol v15, the build of
the four seats before the partners' words rode BN6's packets; this
build's runs follow the table; the player's
Gregar ROM and Team Colonel's in each game's `--rom-dir`, a local server
with seed 120's Square, four host builds headless from `--scene home
--run-depth 7 --seed 3 --dev quiet,arenaboth,arenahp=40`, each MegaMan
40 HP; `arenaboth`: the leads' keys the bot's, the partners' their
stood-in script's, on BN6's Custom screen of each partner's own game and
in the fight; driven by remote play from a harness that reads each
game's log, kept in `.build/2v2/`). Beta, Alpha, Delta, Gamma join;
Gamma asks Delta to team up, Alpha asks Beta, each answered Yes; Alpha's
Battle on Gamma asks its format (DOWN to `Side by side`, A); Gamma's game
takes it; all four go in, seat 0 Alpha, 1 Gamma, 2 Beta, 3 Delta. Every
game's ready said the same pair's hash, `a5bfd374f4a0a3dc`, on all four
and on Epsilon, a fifth game watching from the stands through the
referee.

| Run | End (every game) | Hashes | The two GBAs | The partners |
| --- | --- | --- | --- | --- |
| Side by side, no lag (delay 1) | over at frame 1249, Alpha's team won | 20 agreed on each of the four, 20 checked by the spectator | | each picked on BN6's own Custom screen of his own game |
| Beta's hands off his screen 10 seconds a turn (`teamslow=600` on his game; the last build, twice: the numbers the second's) | over at frame 1850, Alpha's team won (the first: 1849, Gamma's) | 30 agreed on each, 30 checked by the spectator | 970 iterations compared, 0 apart; the partners' keys 0 late, 0 taken apart | side 0's GBA held its packet for Beta 589 frames at most in a turn, side 1's for Delta 10; their picks taken into their own hands on both GBAs |
| Through a local relay, 30 ms and up to 20 ms of jitter each way | delay 5; over at frame 1254 | 20 agreed on each, 20 checked by the spectator; 105, 100, 120 and 51 rollbacks | 952 compared, 0 apart; 0 late | as above |
| Delta's game killed three seconds into the battle | Alpha's and Gamma's battles stopped at frame 189, Beta's at 187: "the other player left the NetBattle" | | | all three told the match broken off (result 3) |
| Delta's game held still from its tick 300 (`arenafreeze=300`, its link kept) | the three others' notes "Waiting for Delta's PET...", B offered at 600 ticks, given up at 1200; the card BROKE OFF, "Delta's PET stopped answering.", TEAM BROKE OFF | | | the match broken off for all four |
| Beta's game never picking (`teamslow=-1`: his client sends nothing) | side 0's GBA held 3600 frames, the wait's limit, his picks none; over at frame 5571 | 92 agreed on each of the four | **apart**: 856 of 1,132 iterations from 73; the partners' keys 107 iterations late, 44 taken apart | (below) |

The last run was the fault [The partners' words in BN6's
packet](partners-words.md) mends: every game and the
spectator agreed on every hash, so the pair was the same everywhere, but
inside it the two GBAs' battle loops ran up to 5 to 8 iterations apart
(read from the rings of then: GBA 1 at iteration 75 found side 0's slot
still holding 67; GBA 0 at 72 found side 1's already holding 80), past
the rings' 4 iterations, in streaks of 20 to 50 iterations about every
60: from GBA 0's iteration 66 of the fight, both leads charging, GBA 0's
count went up one every two frames while GBA 1's went up about one a
frame (a probe of both GBAs' counts after each pair frame).

**A partner's whole MegaMan over the network.** Each game sets the
partners up from the MegaMen they sent (`arena_seats_partners`): his
folder, max HP and buster as before, and now his NaviCust compiled by
BN6's own reload on a core of that game ([His NaviCust, compiled by
BN6](navicust.md)), his record and his screen's hand
size from it, on every game and spectator alike. His own Custom screen
offers his own Crosses and Beast Out as his MegaMan has them
(`team_client_seat`, from the loadout he sent), and his CROSSSELECT's
choice, his Beast Out and a Program Advance his OK makes cross in his
stream as on one machine. `teampa=1` on a partner's own game makes the
MegaMan he carries in a Program Advance's first (Cannon A, B and C his
folder's first chips, Regular chip and TagChips: `arena_seats_pa_folder`),
his own folder as it crosses; the deck override of the stood-in partners
is theirs alone, so that no game's dev flag deals another's deck.

**This build** (22 October 2026, after [The partners' words in BN6's
packet](partners-words.md), the same harness and
games, `--dev quiet,arenaboth,arenahp=40` and each run's flags on the
games named; every game's log and the spectator's compare the pair's two
GBAs iteration by iteration):

| Run | End (every game) | Hashes | The two GBAs, on each game and the spectator | The partners |
| --- | --- | --- | --- | --- |
| Beta's game never picking (`teamslow=-1`), Epsilon watching | over at frame 5810 | 96 agreed on each of the four, the last `d9df79c7251b8482`; 96 checked by the spectator | 1,325 compared, 0 apart on all four; the partners' keys 0 taken apart | side 0's GBA said Beta's picks none at 3,600 frames held, on every game |
| Beta's HeatCross (`teamcross=1` on his game), Delta's Program Advance (`teampa=1` on his), Epsilon watching | over at frame 1020, Gamma's team won | 16 agreed on each (the pair's ready hash `3d4dbab92c15e96d` on all), 17 checked by the spectator | 390 compared, 0 apart on all five | Beta's Cross sent once and taken by BN6's crossing for him on every GBA ("side 0's partner asked to cross 2 times", both GBAs); Delta's Z-Canon1 (`140`, power 300) from Cannon A, B and C, "side 1's partner's hand a Program Advance" on both GBAs of every game; both partners' NaviCusts compiled on all five (HP 40) |
| Beta's hands off his screen 10 seconds a turn (`teamslow=600`), Epsilon watching | over at frame 1849 | 30 agreed on each, 30 checked by the spectator | 964 compared, 0 apart on all five | side 0's GBA held its packet for Beta 593 frames at most in a turn, side 1's for Delta 14 |
| Through a local relay, 40 ms and up to 30 ms of jitter each way; Beta's Beast Out (`teambeast=1` on his game), Delta's Cross 2 (`teamcross=2` on his), Epsilon watching | delay 7; over at frame 1855 | 30 agreed on each, 30 checked by the spectator; 144, 179, 138 and 153 rollbacks | 971 compared, 0 apart on all five | Beta in Beast Out's form (0xB) and Delta in his Cross's (2) from the first turn on both GBAs of every game, BN6's crossing run for each ("asked to cross 2 times", both GBAs); Beast Out's turns counted for them as for the leads |
| Beta's UNITE (`teamunite=1` on his game: NumberSoul held, Atk+10 `*` first), Delta's HeatCross, Epsilon watching | over at frame 1846 | 30 agreed on each, 30 checked by the spectator | 960 compared, 0 apart on all five | Beta's NumberSoul from his own screen, said on every game and taken up on both GBAs, his MegaMan in NumberSoul's look ([A partner's UNITE](unite.md)) |
| Delta's game killed three seconds into the battle | Alpha's, Beta's and Gamma's battles stopped at frame 192: "the other player left the NetBattle" | | 5 compared, 0 apart | all three told the match broken off (result 3) |
| Delta's game held still from its tick 300 (`arenafreeze=300`, its link kept) | the three others waited on Delta's keys and gave up at 1,200 ticks: "the other's keys stopped coming", the match broken off (result 3) | the four agreed to frame 239 | | |

The same runs again on the integration's last build (the games pinned to
it, 22 October 2026): the Cross and the Program Advance frame for frame
as above (over at 1020, the last hash `cbc8963da56d0ffb` on all four);
the partner who never picks over at 5809, 96 hashes agreed and the
spectator's 96 checked, 1,332 iterations 0 apart on all five; Beast Out
and Cross 2 under 40 ms lag and 30 ms jitter over at 1856, 30 hashes,
973 iterations 0 apart on all five; the slow partner over at 1850, 965
iterations 0 apart on all five; Beta's UNITE beside Delta's Beast Out
over at 967, 16 hashes agreed and checked, 390 iterations 0 apart on all
five; the drop stopped at frame 190 on the three, the frozen game given
up at 1,200 ticks, both broken off (result 3). A Team NetBattle one by
one on the same build (the relay of [Team NetBattles](../teams.md),
`teamform=1`, its own harness): bout 1 over at frame 2689 (44 hashes
agreed), bout 2 at 531 (8), bout 3 at 3167 (52), the players waiting
watching each from the stands (44 and 8 hashes checked), the match over
for both teams.

The partner's own client, seen from Beta's seat (seat 2, the pictures
kept out of the repository): the way in's four plates (Alpha and Beta on
the left, his own lit, Gamma and Delta on the right, `Side by side` in
the band), BN6's Custom screen of his own deck while his lead's window
has slid out, his lead's sign (`P1` and a tick), his OK and BN6's
"Sending chip data...", the fight with his own HP box (40, 35, 26, 17 as
he was hit) and the four marks, "MEGAMAN DELETED", and the card: LOSE,
VS Gamma & Delta, Alpha 0 and Beta 0, Gamma 6 and Delta 0, 0:17, TEAM
LOSE; back in the Square, "Aw... Our team lost.". From Alpha's (seat 0):
his window slid out after his OK and his partner's sign (`P3` and its
dots) for the ten seconds, then the same battle from the left GBA. From
Delta's (seat 3): his own Custom screen (Sword first), "Sending chip
data..." while the other side waited for Beta, and the card's WIN, VS
Alpha & Beta, TEAM WIN!. From Epsilon's: "WATCHING Alpha VS Gamma" and
the battle as the cable's first GBA has it. Gamma's game without the bot
asked "Alpha's team wants to fight side by side!" and "Take it,Lan?",
the cursor on No; B turned it down. A NetBattle of two is the battle it
was: `--dev arena,arenaboth,arenalag=4,arenadelay=1 --frames 4200` over
at frame 2690, last hash `86d2f837337a1494`; and `--dev
arena,arenaboth,arena2v2=1,teamhp=500,arenadelay=1,quitend,arenalag=4`
over at 3426, 57 hashes, 2,703 iterations 0 apart, as before (over at
3427 on this build, 57 hashes, 2,696 iterations 0 apart: [Four
streams](four-streams.md)).
