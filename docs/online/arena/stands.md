---
type: Feature
title: "The stands"
description: "The stands (issue #128, protocol v7): a NetBattle watched for real by rebuilding its pair from both players' keys (`arena_watch.c`, `stands.py`), the catch-up, the referee's report and cheers every game gets, and the three-game proof."
tags: [online, arena]
generated: { by: claude-code/unrecorded, at: 2026-10-22T17:15:21Z }
---

# The stands

The owner's decision on issue #128 (19 October 2026): a NetBattle in the
Square is watched both ways. A game with the NetPass's Arena stamp that
can build the battle's pair **watches it for real**, the same two cores
run from the players' keys a second or two behind them; every game, one
that cannot watch among them (the New 3DS, a game without the Arena
stamp, a machine too slow to catch up), gets **the report**: each
MegaMan's HP, the last chip each used and the winner, from the referee by
the ring and on the Board.

The design (the game-design skill): the Square's dialectic, **gather
against go**, and the session loop (arrive, see who is here, do one thing
together, read what happened, go home). Watching is the one thing to do
together that needs no partner of one's own: it shows other players'
folders and plays (learning by watching, social proof), a diegetic
spectacle in the Arena's wing, the referee its barker. The patterns:
spectating as presence (the stands fill: "2 NAVIS WATCHING!"), cheers
that reach the players afterwards rather than during (a crowd felt, never
a coach), and a report for everyone so no system is left at the door. The
stands feed nothing into a run: no reward, no power, and nothing a
spectator sends reaches the battle.

## Watching

The server writes down every started battle as it relays it
(`server/cyberworld_server/stands.py`): both MegaMen (ARENA_LOADOUT), the
installs' record and the ready (ARENA_HASH at 0xFFFFFFFE and 0xFFFFFFFF),
the cable's first's hash every 60 frames, and both players' keys from
frame 0 (a side's first keys come at its input delay's frame, the frames
before released keys on every game). A player of the channel who asks
(WATCH) is sent the battle's setup (WATCH_STATE: the two sessions and the
delay; v9 dropped the rules, every battle being by full rules), then the players' messages (WATCH_RELAY) and every key
from the first frame (WATCH_KEYS, 240 frames a message at most), at most
2 KB a tick and only while his output is under 4 KB, so a spectator who
came late catches up without the players' relay ever waiting on him. A
gap in a side's keys, or a battle past 20 minutes, stops its watching;
the battle itself goes on.

The spectator's game (`src/online/arena_watch.c`) builds the same pair a
player's does (`arena_boot`, both carried MegaMen as their bytes came,
BN5's installs from its own record), so only a game with the Arena stamp
is offered a battle to watch, and the server refuses a WATCH from one
without the full set (NOTICE 28). At the Custom screen its pair's hash must be the players'
ready, or it stops ("the two PETs here came out apart from theirs";
"this PET's BN5 is not theirs" where the installs' records differ). Then
it runs the pair as `arena.c` does, frame N once both sides' keys for N
and the frame after it are in (`watch_keys.c` keeps the whole
battle's keys, about 240 bytes a second), each core on its own frame
clock. More than 110 frames behind the frames whose keys it has (BEHIND_KEPT and
BEHIND_LEAST), it catches up, silent: as many frames a tick as 12 ms allow (eight
a tick headless, so a capture's frames are the same on every machine),
until it is 90 behind; from there it runs one a tick with the watched
side's sound, and within 20 it waits. So it stays a second and a half
behind and rarely stalls; the keys' end (the battle over) runs the rest
as fast. Catching up that
gains nothing in ten seconds gives up ("this PET is too slow to catch
up"), and the referee does not offer that battle again: the next one.
Every 60 frames its pair's hash is checked against the cable's first's
for the same frame; a difference stops the watching ("the games went
apart by frame N"). The keys spent once the server has said the battle
is over, without both cores out of it, end it as broken off.

In the Square (`src/online/square_watch.c`, its words in
`stands_words.c` and `stands_text.c`): the referee's archive is his
words, then the channel's battle as the report says it, then, where this
game can watch it, "WANNA WATCH?" with the cursor on Yes. To a game
without the Arena stamp he says "NO ARENA STAMP ON YOUR NETPASS!", calls
the battle all the same and asks nothing. Yes sets the
Square's flag `0x14C4`; as the chat closes the game sends WATCH and, on
WATCH_STATE, holds the Square's core as a battle of its own does
(`online_visit_held`): the screen is the pair's picture, the cable's
first's side (its heading and keys shown four seconds: "WATCHING ALPHA VS
BETA", "R: other side  L: cheer  B: leave"; "Catching up..." while far
behind; two a side, both teams: "WATCHING Alpha & Beta VS Gamma &
Delta" where it fits the picture's width, else "Alpha & Beta VS Gamma &
Delta", else each team by its lead, "Alpha's TEAM VS Gamma's TEAM",
long leads' names cut to fit, never inside a character: `square_watch.c`'s
heading, `stands_team_heading`). R shows the other GBA's picture, L sends the Square's Cheer
emote, B leaves at once; the end shows "ALPHA won the NetBattle." until A
or B, or five seconds, and MegaMan is back where he stood. A spectator is
busy for the server: a challenge to him is refused (NOTICE 16), and his
game sends nothing of the battle.

## The report and the cheers

The cable's first game reads both MegaMen off its pair's first core at
the last settled frame each tick (`arena_reading` in `arena.c`, the
report in `arena_report.c`; `stands_report.c` the reading, ROM-free): each
side's player object (`BN6_BATTLE_PLAYERS`, the same on both GBAs, which
number the sides alike), its HP and max HP (`BN6_T1_HP`, `BN6_T1_MAX_HP`),
and a chip counted as used where his action turns to a chip's
(`BN6_T1_ACTION` 0x12-0x16) and the chip he uses next (`BN6_T1_CHIP`, the
hand's first) gives way the same frame: seen in `--dev arena` (MiniBomb,
54, picked and used: the field 0x36 until the action's 0x12, 0xFFFF
from that frame; a charged shot's action is 0x16 with no chip, not
counted). It sends what changed every 30 frames at most (ARENA_STATUS)
and the winner as the battle ends, before ARENA's over; `--dev arena`
prints it at the end ("arena: the report: HP 100/100 and 0/100, chips 1
and 0 (last 54 and 0), winner 1"). It only reads.

The server tells each channel its battles (STANDS) as they begin and end
and at most once a second as they change: who fights whom, how many
watch and cheer, the report. The referee says it in BN6's
Mr.Prog voice ("A NETBATTLE'S ON! ALPHA VS BETA!", "ALPHA HAS 40 HP!
BETA HAS 20!", "ALPHA USED MINIBOMB!", "2 NAVIS WATCHING!", then "ALPHA
BEAT BETA! WHAT A BOUT!" and the cheers), and the Board posts it
(">NetBattle now! Alpha vs Beta", ">Alpha beat Beta! 3 cheers."). A
chip of BN5's is named only by a game with BN5's ROM; another leaves its
line out.

A phrase or emote said in the channel while a battle is on is a cheer:
the server counts it for the battle and holds its EVENT from the battle's
two players, and STANDS too until their battle is over. Back in the
Square, the referee tells them: "THE STANDS CHEERED FOR YOU! 3 CHEERS!".
A spectator cheers with L while he watches, or with the phrase chat in
the Square as anyone does.

## The three-game proof

20 October 2026, the player's Gregar ROM (and Team Colonel's where
named), protocol v7, a local server with seed 120's Square, three host
binaries headless (`SDL_VIDEODRIVER=dummy`), each with its own data
folder and `network.ini`, from `--scene home --run-depth 7 --seed 3`, the
harness of [the two-client proof](from-square/two-client-proof.md) grown to three
(kept in `.build/`): Beta placed at 28,404 and Alpha beside him; Alpha
challenges, Beta says Yes, both pick a chip and OK and fire charged
shots, `arenahp=150`; Gamma, BN6's ROM alone, walks to the referee
(at -428,44: MegaMan at -428,28 facing him) and presses A through his
pages. These runs were before v9: the players' battles in the table were
by BN6 rules, which Gamma could watch with BN6's ROM alone. Under v9
every battle is by full rules and Gamma would get the report alone, as
in the full-rules run below.

| Run | Gamma came in | Gamma's pair | Caught up | The battle's end | Hashes |
| --- | --- | --- | --- | --- | --- |
| Watched from the way in | as the players' pairs reached their Custom screen | the players' ready (`0f5e9abea55dced4`) | from 708 frames behind to 90 at frame 773 | frame 2,506 on all three, Alpha's win (the cable's first's result 1) | 41 agreed between the players, 41 checked by Gamma |
| The same, an earlier build | the same | the same | from 785 to 90 at frame 863 | frame 2,915 on all three | 48 and 48 |
| A spectator late | 28 seconds into the battle | the same | from 1,686 frames behind to 90 at frame 2,028, live after | frame 2,506 on all three | 41 and 41 |
| Gamma's copy changed (`arenadesync=1200` on Gamma) | at the way in | the same | at frame 841 | Gamma stopped at frame 1,260 ("the games went apart by frame 1259"), the players fought on to 2,506 | 41 agreed between the players |

On Gamma's screen: the referee's "A NETBATTLE'S ON! ALPHA VS BETA!",
"WANNA WATCH?" with the cursor on Yes, "TO THE STANDS! ENJOY THE BOUT!",
then the battle's Custom screen with "WATCHING Alpha VS Beta" and
"Catching up...", the battle with its keys shown four seconds, R turning
the picture to Beta's GBA and back (the HP in the corner each player's
own), and "Alpha won the NetBattle." at the end; back in the Square on
the referee's panel. The players stalled 0 to 6 ticks a battle, as
without a spectator, but for one run while the machine built other
targets for other work (402 ticks on Beta's game, the spectator's own
pace unchanged). Gamma's three L's while watching came back as three
cheers: the Board's "Alpha beat Beta! 3 cheers." and, to Alpha at the
referee after his battle, "LAST BOUT: YOU BEAT Beta!", then "THE STANDS
CHEERED FOR YOU! 3 CHEERS!" (the server holds a cheer's EVENT from the
two players while they fight: `server/tests/test_stands.py`). Alpha's
report, where his MiniBomb came up: HP 10 of 150 and 0 of 150, MiniBomb
(54) used once, the winner Alpha.

A battle by full rules (Alpha and Beta with Team Colonel's ROM beside
BN6's, Gamma with BN6's alone): Gamma's referee called it, "A
NETBATTLE'S ON! ALPHA VS BETA!", "ALPHA HAS 150 HP! BETA HAS 150!", and
asked nothing; the server refused a WATCH from a game without BN5's ROM
for such a battle (NOTICE 19 then; under v9 NOTICE 28 refuses any WATCH
from a game without the full set, `server/tests/test_stands.py`). Watched
instead by Gamma with Team Colonel's ROM beside BN6's (the same three
games, Gamma asking 1,368 frames into the battle): his pair made with
the same installs' record as the players' (1,913,709 bytes and 132
hooks, `1efe82b0894607e2`) and their ready (`7eaaa466008086ab`), caught
up at frame 1,592, the battle over at frame 3,080 on all three, 51
hashes checked against the players' 51.

The browser build before 20 October 2026, when the Arena was compiled
out there (`CW_ARENA` 0):
`tools/online_browser.html`'s probe served beside a local v7 server
allowing its origin, in headless Chrome (agent-browser), its MegaMan put
before the referee and A pressed through his pages while two host builds
fought (to frame 2,918, 48 hashes agreeing): "SORRY! NO NETBATTLES FROM
THIS PET!", then "A NETBATTLE'S ON! ALPHA VS BETA!", "ALPHA HAS 150 HP!",
later "ALPHA HAS 120", "ALPHA USED MINIBOMB!" and "ALPHA HAS 10 HP!" as
the cable's first's reports came, and no question.
