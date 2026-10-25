---
type: Feature
title: "The Arena, in brief"
description: "The Arena, the stands and Team NetBattles in brief: a challenge, BN6's Practice NetBattle on two linked cores per game by full rules, rematches, the referee, watching for real, relay teams and side by side."
tags: [online]
generated: { by: claude-code/unrecorded, at: 2026-10-22T15:15:49Z }
---

# The Arena

Battle on a card, offered where both games' passes hold the Arena stamp
(above), sends a challenge; if the other player takes it, both
games run BN6's Practice NetBattle on **two linked cores each**, the two
MegaMen carried as they are (folder, NaviCust, HP, buster, Crosses,
Souls), and only the players' keys cross the server. Every NetBattle is
by **full rules** since v9: BN5's installs in both pair cores of each
game, the installs' record compared, BN5's chips in the folder and a
Soul MegaMan unites with carried to the other GBA
([Full rules](arena/from-square/full-rules.md)). Where more than one of
Folder1, Folder2 and the Extra Folder holds chips, Lan picks the one
carried ([One rule: full rules](arena/from-square/one-rule.md)). The
Square's core holds still
meanwhile; after the battle each MegaMan stands where he stood, says
how it went and asks Lan whether to go again with the same player (the
cursor on No): when both say Yes they fight again with the same
folders, over the same ARENA messages (a challenge answered at once),
the rematch in about a second: each game goes in from the pair it kept as
the last bout began, each bout's hands its own from the bout's seed
(protocol v13, [A rematch in a
second](arena/from-square/rematch-in-a-second.md)). The
referee by the ring calls the bout while it runs and as it ends, and keeps the count.
Before anyone commits, the Square shows who is busy (a mark over a player
in a NetBattle, watching one or at a trade), an open challenge on the
challenger's screen (whom it rings, then his time to answer running
down, and Call off on that player's card), the challenged player's PET
ringing with a "!" over the challenger and the challenger's card read
before the question, and the link between two players as bars, never a
number ([The Square before a
NetBattle](arena/from-square/square-before.md)). A hash of the
pair every 60 frames catches two games apart and stops both. Nothing is
won or lost. It runs on every system but the New 3DS, the browser
among them; the New 3DS builds it but does not offer it (`CW_ARENA`: a
frame of the pair takes half again a 60th of a second there): Battle is
not offered there, whatever the pass, and a challenge is declined. [The Arena](arena/overview.md) has the engine, the wire,
the lockstep and the two-client proof.

## The stands

[Issue #128](https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/128),
the owner's decision of 19 October 2026: the Arena's wing has stands. A
NetBattle in the channel is **watched for real** by a game that can build
its pair (the Arena built there, and the Arena stamp on its pass): the
referee by the ring asks "WANNA WATCH?", and Yes turns the screen to the
battle, its two cores built and run here from both players' keys as the server hands them on,
a second and a half behind them, from the battle's first frame however
late the spectator came (he catches up faster than real time). R shows
the other GBA's side, L cheers, B goes back to where MegaMan stood. The
spectator checks the players' hashes and stops, saying so, where they
disagree. Every game, the New 3DS and a game without the Arena stamp
among them, gets **the report**: the referee calls the channel's battle (who fights whom, each
MegaMan's HP, the last chip each used, how many watch, the winner and
the cheers) and the Board posts it, from what the cable's first game
reads off its own pair and sends now and then. A phrase or emote said
while a battle is on is a cheer, held from the two players until their
battle is over; the referee then tells them. The stands give nothing
toward a run. How it is built, and its design: [The
stands](arena/stands.md); the wire (protocol v7: STANDS,
ARENA_STATUS, WATCH and the server's three WATCH messages; v9 took the
rules byte out of STANDS and WATCH_STATE and refuses a WATCH from a game
without the full set, NOTICE 28):
[shared/online/README.md](../../shared/online/README.md).

## Team NetBattles

Epic #167, the owner's decision of 21 October 2026: a relay of two teams
of two. `Team up` on another player's card asks him to team up (his
MegaMan asks Lan, the cursor on No); `Split up` on the partner's card
ends it, as leaving does. A Battle between players of two teams is a
team challenge; its Yes asks each team's captain (the one who asked to
team up) who goes first, blind, and shows all four the order once both
are in. Each bout is the Arena's NetBattle as above: the winner stays on
with the HP he has left (carried in his MegaMan's bytes and written into
the battle on both GBAs as it reaches the Custom screen, so the pair's
hash covers it), the loser's partner comes in, and the team whose
MegaMen are both deleted is out. Whoever waits watches the bout from the
stands, unasked. The result card shows the MegaMen standing and who is
next, and at the end the team's result; a leave or a dropped link breaks
the match off for both teams, with no result. Nothing is won or lost.
How it is built, its design and the four-game proof: [
Team NetBattles](arena/teams.md); the wire (protocol
v12: TEAM, TEAM_STATE, STANDS's bout byte, a carried HP, NOTICE 29):
[shared/online/README.md](../../shared/online/README.md).

Since protocol v15 (issue #181) a team challenge names its format:
MegaMan asks the challenger "How do we fight 'em,Lan?", `One by one`
(the relay above) or `Side by side`, and the other team's question says
which. Side by side no order is asked: each team's captain leads, his
game the GBA of his side, and the match is one battle of four, every
one of the four games running the same pair from the four players'
keys, each partner picking his own chips on BN6's own Custom screen of
his own game while the leads wait for him. Each partner is his whole
MegaMan: his folder, HP and buster, his NaviCust's programs (worked out
by BN6's own code on every game from the MegaMan he sent), his Crosses
and Beast Out on his own screen, a Program Advance his OK makes, and
UNITE with his own Souls. His buttons and choices cross the server in his own stream
and, inside each game's two linked GBAs, ride his lead's GBA's own link
data, so both GBAs take them in the same frame as the lead's. The way
in's panel shows the four, the result card each MegaMan's HP left and
the team's result; whoever watches from the stands runs it from the
four streams. How it is built and the four-game proofs: [
Two MegaMen a side](arena/two-a-side/overview.md) and [Over the
server](arena/two-a-side/over-the-server.md).
