---
type: Feature
title: "Team NetBattles"
description: "Team NetBattles (epic #167, protocol v12): relay teams of two, Team up and Split up on a card, the captains' blind order, the winner staying on with his HP, the wire (`teams.py`), the four-game proof."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-21T18:22:09Z }
---

# Team NetBattles

Epic #167's multi-battles, the owner's decision of 21 October 2026: a
**relay** first. Teams of two; each team's order picked blind; the leads
fight today's NetBattle; the winner stays on with the HP he has left and
the loser's partner comes in; a team is out when both its MegaMen are
deleted; whoever waits watches live from the stands. Two MegaMen a side
on one field is a separate engine track ([Two MegaMen a
side](two-a-side/overview.md)) and shares none of this code.

## The design

The game-design skill's lens, before the numbers and screens:

- **The dialectic: mine against ours.** Every MegaMan is a run's (its
  folder, its NaviCust, its HP), and a team asks what he is for the
  pair: who goes first (the lead spends HP the anchor never gets back,
  the anchor sees the other team's folders before he goes in), and
  whether the HP a win leaves is the team's gain or its next loss. Under
  it the Square's own pull, gather against go: a team pulls hard toward
  gather, so every step lets a player go (a No on the question, Split up
  on the partner's card, a leave that breaks the match off for nobody's
  loss).
- **The loop layer: the session loop** (arrive, see who is here, do one
  thing together, read what happened, go home), its "one thing together"
  grown from two players to four. Nothing comes home: no reward, no
  power, no record kept but the referee's words (the Square's rule since
  issue #120).
- **The patterns:** *a blind pick* (each captain names his lead without
  seeing the other's, KOF's order select: a small guessing game between
  lead and anchor); *attrition as the brake* (the winner's HP carried, no
  heal: a sweep costs the sweeper; KOF heals a little, ours starts with
  none and watches whether anchors sweep); *give the waiting player a
  verb* (he watches the bout live, with stakes: the folder he may meet
  next); *telegraph honestly* (the order shown to all four before the
  first bout, the result card's MegaMen standing and who is next, the
  way in's HP of his max); *subtract* (two a side, the relay before one
  field, BN6's own two-Navi engine untouched); *name failure in the
  fiction's voice* (a stop broke off, never a loss). Canon: BN5's
  Liberation Missions, a team that fights one Navi at a time.

The numbers: the team card stays up 6 seconds (`TEAM_CARD_TICKS`) while
the match goes on (the next bout's way in follows it; A closes it
sooner), 10 at the match's end as a stop's card does; a captain's order
waits 30 seconds (`ORDER_WAIT`), after which he leads himself; a bout
waits 30 seconds for both its fighters (the Arena's `PENDING`), after
which the match breaks off; a proposal to team up waits 30 seconds
(`ASK_WAIT`). No heal on a carried HP.

## Forming a team

On another player's card, where both passes hold the Arena stamp, the
chat box offers `Battle`, `Team up` and `More...`, then `Wave`, `Mute`,
`More...`, then `Block` and `Back` (BN6's box holds three; Team up beside
Battle). Team up asks him (TEAM propose): MegaMan says "Let's see if NAME
teams up with us!"; the other's MegaMan asks, once no chat is open,
"NAME wants to team up with us!" and "Team up,Lan?" with the cursor on
No. Yes makes them a team ("Alright! We're a team with NAME!" on both);
No, B, or 30 seconds without an answer turn it down ("Aw... NAME won't
team up this time."). On the partner's card the choice reads `Split up`,
and either one's Split up, his leaving the channel or the server, or a
dropped link split the team ("OK... Our team with NAME split up."). The
one who asked to team up is the team's **captain**. A player on a team
asked to team up again is answered by MegaMan ("Lan,we're with NAME
already!"), and an ask the server refuses is MegaMan's word too, never
the corner's notice: NAME on a team already (NOTICE 29, "Huh? NAME's on a
team already..."), busy or gone (16, 17) as a challenge's are.

A Battle between players of two teams is a **team challenge**: the
server checks all four free (in no battle, trade, watch or match;
NOTICE 16 otherwise); the challenged's MegaMan asks "NAME's team wants a
Team NetBattle!" and "Take it,Lan?". A Battle from a team's player to a
lone one, or the other way, is refused (NOTICE 29, `notice_team`),
MegaMan saying why ("Huh? NAME has no partner...", "Huh? NAME's on a
team..."); one on his own partner's card MegaMan answers at once, as the
server would refuse it ("Huh? NAME's on our team,Lan!"). A lone
player's Battle with a lone player is today's NetBattle, unchanged.

## The match

The challenge's Yes begins the match (TEAM_STATE match to all four; the
challenge's own battle is dropped without an ENDED). Each captain's
MegaMan asks "Who goes first,Lan?" with `We do` and the partner's name
(B: we do); the server keeps the first pick in and shows nothing until
both are, then sends all four the order (TEAM_STATE ordered), and each
MegaMan says it ("We're up first,Lan! Against NAME!", or "NAME goes
first! Go get 'em,NAME!").

Each **bout** is an ARENA battle (`arena.py`'s Match, its id the bout's)
the server calls (TEAM_STATE bout to all four): its two fighters' games
go into it once nothing is on their screen (MegaMan's word read, the
last card closed, a watch over) by ARENA accept, and the server STARTs
both once both have. From there it is today's NetBattle: the way in's
panel (its band says Team), the Custom screen, the battle, the hashes,
the report. As it starts (TEAM_STATE fight), the two others' games ask
the server to watch it, unasked, as the referee's Yes would
(`square_watch_team`): the waiting partner, and the MegaMan already
deleted.

The bout's winner is the cable's first's report (ARENA_STATUS's winner,
`stands.py`): the loser's team's next comes in; a bout with no winner
puts both out. The staying MegaMan carries **the HP he has left**: his
own game's tally at the battle's end (`square_arena_last_hp`), logged
beside the server's report ("we stay on with 7 HP (the server's report:
7)"), and carried in his MegaMan's bytes (ARENA_LOADOUT v2, `hp` beside
max HP). BN6 makes HP whole again from the NaviStats in its menus on the
way in, so the carried HP is written where the battle has made its
objects: as the pair reaches the first Custom screen (`carried_hp` in
`arena_boot.c`), into that side's battle object's HP (`BN6_BATTLE_PLAYERS`,
`BN6_T1_HP`) on both GBAs of the pair, read back from both, before the
pair is anchored and hashed. Both games and a spectator build the same
pair, so the ready's hash covers the carried HP: a game that carried
another number stops before the first frame. Seen with `--dev
arena,arenacarry=40` (the carried MegaMan going in with 40 of 100): the
HUD's 40 from the Custom screen through three turns, on both GBAs.

The **result card** of a team bout shows, in the rematch's place, the
MegaMen standing ("Standing 2 - 1", ours first) and who is next ("Next:
Alpha vs Delta"); no rematch is asked. It closes itself after 6
seconds while the match goes on, and the next bout's way in follows (or
the stands, for a game whose MegaMan waits). At the match's end the card
says the team's result (TEAM WIN!, TEAM LOSE, TEAM DRAW where both
teams' last MegaMen went down in one bout, TEAM BROKE OFF), and MegaMan
says it back in the Square ("Alright!! Our team won,Lan!!", "Aw... Our
team lost.", "Phew... Both teams went down at once!", "Whoa! The Team
NetBattle broke off!"). A leave or a dropped link mid-match, a bout
broken off or never begun, ends the match **broken off** for both teams:
no result for anyone, as a lone bout broken off counts neither won nor
lost. The teams stay teams afterwards, for another.

The referee calls a team's bout ("TEAM NETBATTLE! BOUT 2!", then "ALPHA
VS DELTA!") and the Board posts it (">Team NetBattle, bout 2! Alpha vs
Delta"), from STANDS's bout byte.

## On the wire (protocol v12)

TEAM (33, client → server: an action, a session, a value) and
TEAM_STATE (34, server → client: the stage, the partner, the match, the
bout under way, the four players in order, each team's MegaMan up now
first, the MegaMen standing each side, the bout's number, the result,
the staying MegaMan's HP as the report left it); STANDS's entry grows a
byte, the team bout's number; ARENA_LOADOUT's MegaMan, its own version
2, carries his HP; NOTICE 29 refuses a challenge between a team and a
lone Navi. `server/cyberworld_server/teams.py` keeps the teams and the
matches; `server/tests/test_teams.py` the relay from the ask to the last
bout, a blind order, an order not picked, a draw, a drop, and a team
split between the challenge and its Yes (the challenge ends, no match).
[shared/online/README.md](../../../shared/online/README.md) has the bytes.

## The four-game proof

21 October 2026, the player's Gregar ROM and Team Colonel's in each
game's `--rom-dir`, protocol v12, a local server with seed 120's Square,
four host builds headless from `--scene home --run-depth 7 --seed 3
--dev quiet,arenaboth,arenahp=40` (`arenaboth`: the battles play
themselves, the bouts' fighters' keys a script), driven by remote play
(`--remote`) from a harness that reads each game's log (kept in
`.build/team/`): Beta, Alpha, Delta, Gamma join; Alpha asks Beta, Gamma
asks Delta, each answered Yes; Alpha challenges Gamma, who takes it;
Alpha picks Beta to lead, Gamma himself.

| Bout | Fighters (side 0 first) | Carried | The ready's hash, on both players and both spectators | Over | Hashes | Winner, HP left |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | Beta vs Gamma | none | `85776fd9af3f0dc2` | frame 1,250 | 20 agreed, 20 checked by Alpha and Delta | Beta, 6 of 40 |
| 2 | Beta vs Delta | Beta 6 of 40, "written on both GBAs" on all four games | `ae970b1e7c3951ca` | frame 655 | 10 agreed, 10 checked by Alpha and Gamma | Delta, 29 of 40 |
| 3 | Delta vs Alpha | Delta 29 of 40 (side 1), on all four games | `eb579513b4c5f1d2` | frame 1,010 | 16 agreed, 16 checked by Beta and Gamma | Alpha, 17 of 40 |

Each staying MegaMan's own tally matched the server's report ("we stay
on with 6 HP (the server's report: 6)", 29 and 29), and the HUD showed
the carried HP from the Custom screen on (6 for Beta in bout 2). The
loser of a bout watched the next one, unasked, as the waiting partner
did (Beta watched bout 3). The match ended TEAM WIN on Alpha's and Beta's
cards ("Standing 1 - 0") and TEAM LOSE on Gamma's and Delta's; between
bouts, Beta's card after bout 1 read "WIN VS Gamma", "Standing 2 - 1",
"Next: Beta vs Delta"; back in the Square, "Alright!! Our team won,Lan!!".

**A drop mid-match** (the same harness, `--drop`): Delta's game killed
three seconds into bout 2 (Beta, 7 of 40 carried, against Delta). Beta's
battle stopped at frame 192 ("the other player left the NetBattle"), his
card BROKE OFF with "Delta left the NetBattle.", "Standing 2 - 1" and
TEAM BROKE OFF; Alpha's and Gamma's watching stopped at frame 185 ("the
NetBattle broke off"); all three were told the match broken off (result
3), Gamma's team split with Delta gone, and MegaMan said "Whoa! The Team
NetBattle broke off!" back in the Square.

**The referee** (the same harness, `--referee`): Epsilon, a fifth
game, at the referee during bout 1: "TEAM NETBATTLE! BOUT 1!", "BETA VS
GAMMA!", "BETA HAS 40 HP! GAMMA HAS 30!". The match ran again after the
team's course moved under the Arena's own frame (`square_arena_update`,
`square_arena_held_frame`): three bouts, every hash agreeing, TEAM WIN for
Alpha and Beta.

**Split up** (the same harness, `--split`): Alpha's Battle on Beta's
card, his partner, answered at once ("Huh? Beta's on our team,Lan!"),
then `Split up` on the same card ("OK... Our team with Beta split up.";
both games told).

**The words** (`--words` and `--question`, a fifth game, Epsilon, alone):
Epsilon's Team up on Alpha ("Let's see if Alpha teams up with us!", then
"Huh? Alpha's on a team already...") and Battle on him ("Huh? Alpha's on
a team...", "We'd need a partner too,Lan."); Alpha's Battle on Epsilon
("Huh? Epsilon has no partner..."); Gamma's game without the bot asked
"Alpha's team wants a Team NetBattle!" and "Take it,Lan?" with the cursor
on No.

**The way in with a carried HP** (`--dev arena,arenacarry=40`, the
local scene): the plate reads "HP 40/100", the Custom screen's HP 40.
