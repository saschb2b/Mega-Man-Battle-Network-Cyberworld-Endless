---
type: Tool
title: "The Arena on one machine"
description: "`--dev arena` runs BN6's Practice NetBattle between two linked cores on one machine, with switches for delay, lag, rollback, seeds, desyncs, two MegaMen a side (`arena2v2`) and mutations, and `--dev rollbench` timing a rollback's cost."
tags: [devtools]
generated: { by: claude-code/unrecorded, at: 2026-10-22T17:15:21Z }
---

# The Arena on one machine

`--dev arena` starts on a Practice NetBattle of BN6's own between two
linked cores (issue #120), by full rules as every NetBattle is, so with
BN5's ROM beside BN6's (without it the way in stops, saying so); you the
first GBA and a script the second,
whose keys come over a local link held back as a network's would be;
`arenadelay=N`, `arenalag=N` and `arenahp=N` set its input delay (1 to
60), the link's one way and the carried MegaMan's HP; `arenacarry=N`
sends him in with N HP of his max, written on both GBAs at the Custom
screen as a Team NetBattle's winner's is. `arenaseed=N` gives the bout
the seed N, BN6's random words written on both GBAs as Practice's last
confirm is pressed (a Net Square bout's seed is its battle ID; without
it none, the battle as before), and `arenaagain=N` fights N bouts more
after the first, each from the pair kept as the last one began, its seed
one more: a rematch in a second, whose hands and hashes must be those of
the whole way in with that seed ([A rematch in a second](../online/arena/from-square/rematch-in-a-second.md), A rematch in a
second). `arenaseedoff=N` moves a Net Square bout's seed by N on this
game alone, a mutation: both games' readies apart, the bout stopped
before its first frame. `arenawindow=W`
sets the rollback window (0: the delay alone, a frame run once its keys
have all come; without it, what a frame costs here affords, as a Net
Square battle picks it), and `arenabusy` makes the second player step,
shoot and OK the Custom screen by the frame's number alone, so a run that
rolls back and one that does not hand him the same keys: with
`arenadelay=1,arenalag=4` every frame's other keys are a guess, and the
game's hashes must be those of `arenawindow=0,arenalag=0`. `arenaboth`
makes the first player a script too (the Custom screen's OK, then
charged shots from where he stands), so the battle plays itself on a
device with no hands on it; in the Net Square it takes a challenge at
once (Lan's Yes, the first folder that fits) and plays the battle the
same way, for a browser tab or a device against another game. `quitend`
quits the game once `--dev rollbench` and `--dev arena` are done (a run
sent over 3dslink gives the Homebrew Launcher back). In a Net
Square battle `arenawindow=W` sets the window too and asks with it, and
`arenadelay=N` asks the server for N frames. [The Arena](../online/arena/overview.md)
says what runs, how to script a battle headless and what it measured.

`arenaroll=W` makes it roll back on purpose (the window then none of its
own): the pair's snapshot kept before each
frame and every W frames (1 to 30) the last W run again from the first's,
the battle going on from there, each time compared with the state run
straight (`arena: frame N: core ...'s state apart` where not), and every
60th frame the game's hash said (`the game's hash`, the pair's hash,
which leaves the sound's fields out), which a run with `arenaroll=0` says
too, for the two to be compared ([The Arena](../online/arena/overview.md),
Rollback). A battle's end says its rollbacks: `arena: window W, delay D:
N rollbacks, F frames run again, M the most at once; a frame of the pair
... ms, a tick at worst ... ms, K past a 60th`.

A battle from the Net Square between two games takes `arenahp=N` too
(each game's own MegaMan, so both pairs stay the same), and
`arenadesync=N` changes the other player's keys from frame N for a
second on this game alone: the hashes' stop seen on both
([The Arena](../online/arena/overview.md), The two-client proof), and
`arenafreeze=N` holds its battle still from its tick N for 40 seconds,
its link to the Square kept, as an app sent to the background: the
other game's stall note, its way out and its end seen
([The Arena](../online/arena/overview.md), The battle's screen). On a game
watching a battle from the stands, `arenadesync=N` changes the cable's
first's keys from frame N in its own copy: its watching stops at the
next hash, the players' battle goes on ([The stands](../online/arena/stands.md)).
`--dev arena` prints the stands' report at the end ("arena: the report:
HP ... chips ... winner"), as the cable's first sends it from the Square.
The Square's Mr.Progs' places are in the log ("online: Mr.Prog 2 at
-428,44", the referee), for a harness's `place`.

`arena2v2=1` gives the Arena's Practice NetBattle two MegaMen a side
(epic #167, [Two MegaMen a side](../online/arena/two-a-side/overview.md)): each side's
partner a stream of the Arena's input of his own, scripted by the
frame's number, and the two GBAs compared iteration by iteration at the
battle's end (`arena2v2:` lines, with each hit by MegaMan); `arena2v2=N`
(N > 1) flips one key of side 0's partner on the second GBA alone from
iteration N, a mutation that must part them; on the Custom screen each
lead takes the chip under the cursor before the script's OK; each
partner picks on BN6's own Custom screen on a copy of the pair his
stood-in client keeps (the cursor right and A every 20 frames), his
picks crossing in his stream ([A partner's own Custom screen](../online/arena/two-a-side/custom-screen.md), A partner's own Custom
screen). `teamseat=N` shows seat N's screen (0 and 1 the GBAs' players,
2 and 3 side 0's and side 1's partners: his side's GBA, his own Custom
screen while it is open, his HP box), and `teampick=-1` writes each
partner's picks into his hand a chip short on the second GBA alone, the
picks' mutation; `teamslow=N` keeps side 0's partner's hands off his
screen for the first N frames of each turn, a slow partner the leads
wait for (`-1`: a client gone quiet, which never picks nor sends, so the
wait's limit ends it); `teambuster=N` (1-5) gives both partners' records their
buster's Attack N, as their own loadouts would. Where the data folder
has a checkpoint, side 0's partner's NaviCust is compiled by BN6's own
reload over the way in's own state ([His NaviCust, compiled by BN6](../online/arena/two-a-side/navicust.md), His NaviCust,
compiled by BN6), and `teamcust=1` gives side 1's partner the
checkpoint's MegaMan's NaviCust too (`-1`: the second GBA's records their
sides' copies, the mutation; `-2`: the NaviCusts compiled over this
game's own checkpoint's state instead, as before issue #181's review, a
mutation that parts the games of different runs). `teamidle=N` keeps the lead of side N - 1 (over the network, this
game's lead) off his Custom screen, so that his own game presses OK for
him at the turn's limit ([The leads wait for their partners](../online/arena/two-a-side/leads-wait.md)).
`teamcross=N` (1-5) offers each partner Cross N on
his own CROSSSELECT and has his stood-in client take it at his first turn
(`-N`: none on the second GBA, the mutation); `teambeast=1` offers each
partner Beast Out on his own Custom screen and has his client take its
emblem at his first turn (`2`: at every turn; `3`: the leads too, by the
same keys, the partners' control; `-1` none on the second GBA, the
mutation), each turn's form check saying each MegaMan's form and
BeastOutCounter in the log; `teampa=1` opens side 0's
partner's folder with Cannon A, B and C, his first hand Z-Canon1, a
Program Advance (`-1`: the PA left out of his hand on the second GBA, the
mutation); `teamunite=1` gives each partner's own Custom screen the
carried MegaMan's Souls (BN5's installs' hooks on his side's GBA of his
copy) and has his client take the chip under the cursor and UNITE at his
first turn (`2`: each one's folder opening with Atk+10 `*`, a Plus chip:
NumberSoul's unison of three turns where the Souls held have it; `3`:
RockCube `*` first and ColonelSoul held, ColonelSoul's unison; `-1`:
his Soul left out on the second GBA, the mutation; with
`CYBERWORLD_EMU_DEBUG` a partner's Soul taken up and ended are said by
core: "side 0's partner united with Soul 9 ..."). On a partner's own
game in a side-by-side match from the Square (with `arenaboth`, his
client the stood-in script), `teamcross=N` and `teambeast=1` offer and
take his Cross or Beast Out on his own screen, `teamunite=1` gives the
MegaMan he carries in NumberSoul and Atk+10 `*` as his folder's first
chip and takes UNITE, and `teampa=1` makes the MegaMan he carries in a
Program Advance's first (his folder's Regular chip and TagChips Cannon A, B and
C, as his own folder crosses to every game): a test partner whose
choices cross the server as a player's do. `teamform=N` makes a team
challenge from the Square name its format without asking (1 one by one,
2 side by side). `teamhp=N` gives each partner his own
battle NaviStats' max HP N (`-N` the second GBA's alone, the records'
mutation), and `arenaother` shows the second GBA's screen instead of the
first's (the game the same: a picture drawn changes no state), where side
0's partner's HP shows under him. The two GBAs' first iteration apart is
also said as it runs ("the two GBAs part at iteration N, said as it
ran", with the partners' keys each took there), for a mutation whose
battle the two no longer end together, and the partners' words in the
GBAs' link packets are counted at the end ("the partners' words in
BN6's packets": handed to each side's GBA, packets built, messages sent
and read on each GBA, the wait's limit said in a partner's place).

The full-rules audit's runs ([What a partner meets under full rules](../online/arena/two-a-side/full-rules.md), What a partner meets
under full rules; issue #182): `arenapeer=MASK` makes the sides in MASK
(1 side 0, 2 side 1) play their folders' first chips, the chip under the
cursor taken before OK and used in the fight every four seconds (side 0
a second before side 1, his charged shots let go first), and side 1
(MASK 2) carries the checkpoint's MegaMan as side 0 does, so a
checkpoint of thirty DarkInvs (`--scene emu --dev folder=289`) puts
either side under BN5's chip AI. Under `arena2v2`, `chipaibreak=N` makes the core of hook
slot N (the debug lines' "core N"; the second GBA's is 2) take no room
of BN5's chip AI for anyone but side 0's player, and `partnerbreak=N`
makes it take no object for a side's partner (his side's Soul, mood and
NaviStats his, as before the audit): two mutations that must part the
GBAs. With `CYBERWORLD_EMU_DEBUG` the rooms ("seat 2's AI data ... moved
into his room ..."), each MegaMan's AI data as the chip AI takes one, a
Chaos Unison's release and a partner's own charged B, and a Navi made
dark by a DarkChip (his mood and NaviStats) are said by core.

`--dev rollbench` measures what a rollback of the Arena costs on the
machine it runs on (issue #128), on the player's ROM, and says it on the
screen and in the output (`rollbench:` lines; log.txt on a handheld): the
game's core alone from power-on (every build), then, where the Arena is
built and BN5's ROM beside BN6's, the pair at a Practice NetBattle under full rules (every NetBattle's since v9; without BN5's ROM it says so and measures the core alone) for 600 frames
of scripted keys on both sides: a core's state and the pair's snapshot
in bytes, their save and load and a pair frame's emulation (mean and
worst), a rollback of eight frames timed, whether it came back to the
same state as the frames run straight, and the largest rollback window a
frame holds with a quarter of it kept for the rest, with the lag of the
other's keys it covers at delays 1, 2 and 3, then the way in's frames
timed and the window the Arena picks from them and from the battle's;
the battle's frame cost is kept in the data folder (`arena.ini`) for the
delay the next challenge asks. A pair frame is timed three ways: with my
GBA's picture drawn (a battle's frame), with none (a frame a rollback
runs again) and with both; the window counts the frames run again at the
second. On the 3DS it says the heap and the linear heap with the pair
made. A or B quits; a headless run ends by itself; with `arena` too
(`--dev rollbench,arena`) the Arena's battle follows on the same core,
and `quitend` ends the game after it. [The Arena](../online/arena/overview.md),
Rollback, has its numbers on this machine, the Retroid Pocket Nova, the
browser and the New 3DS:

```sh
python3 build.py shot --dev rollbench --frames 20000      # here
SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy ./cyberworld.aarch64 --headless --frames 20000 --rom-dir ROMS --data-dir /tmp/rb --dev rollbench   # a handheld, over SSH
3dslink -a 3DS-ADDRESS -s build/3ds/cyberworld-endless.3dsx -- --data-dir sdmc:/3ds/cyberworld-endless/test \
    --dev statistics=off,rollbench,arena,arenaboth,arenabusy,arenadelay=1,arenalag=4,quitend   # a New 3DS: both, its output back
```
