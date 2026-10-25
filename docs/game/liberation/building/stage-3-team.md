---
type: Feature
title: "Stage 3, as built: the team"
description: "Stage 3, the team (`src/director/liberation_team.c`): Navis whose BN5 Souls the run holds, two to five by act at BN5's HP, their commands, a teammate's battle fought in his Soul, and boards planned for teams."
tags: [game, liberation, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-25T13:43:58Z }
---

# Stage 3, as built: the team (25 October 2026)

**BN5's team, played** (romlab on the plain ROM, mission 2: MegaMan,
Colonel and KnightMan; [BN5's Liberation Missions](../../../rom/liberation.md), BN5's Liberation board: the
team's turns and HP). Each Navi acts once a phase. R moves play to the
next Navi yet to act (the liberation state's +6, `BN5_LIB_IN_PLAY`), the
camera panning to him; a Navi's action moves it on by itself, and a Navi
who has acted (his unit's +2, `BN5_LIB_UNIT_TO_ACT`, at 0) stands darker.
The Navi in play walks the board as freely as MegaMan does, through his
teammates. His HP shows in a box at the top left, every other Navi's
under him. Pass ("End without doing anything?") gives a fifth of his max
HP back, not the guides' half: MegaMan's 32 came back 52 of 100, Colonel's
72 of 200. Colonel's window holds Liberation, ScrenDiv, Save and Pass;
KnightMan's Liberation and Pass.

**Who comes** (`src/director/liberation_team.c`). MegaMan and the Navis
whose BN5 Souls the run holds, as many as BN5's team of the stage the act
stands for: two at act 2, three at act 3, four at act 4, five from act 5.
Where more Souls are held than there are places, the layer's seed leaves
some out, so a run's boards bring different teams. Each teammate's HP is
BN5's for that stage (`BN5_LIB_NAVI_HP`, missions 1, 3, 5, 7 and 9 for
acts 2 to 6: Colonel 200 at act 2, KnightMan 500 at act 3, ShadowMan 350
and TomahawkMan 350 at act 4). Without BN5's ROM or a Soul, MegaMan goes
alone as before. The board is made for the team; where none fits the
whole team, it is made for one Navi fewer, MegaMan alone at the last.

**On the board.** The team starts on the clean panels nearest the
arrival, out of every unit's reach. The Navi in play is BN6's player
object in his own BN5 overworld sprite: the player's spawn loads the
sprite `BN6_PLAYER_SPRITE_LOAD` is answered with, and the spawn runs
again where he stands (`src/emu/player_body.c`), so he walks as BN6
walks MegaMan, with no movement of ours (AGENTS.md). His sprite and
mugshot are copied from BN5's ROM to one of five list-6 numbers both
versions of BN6 leave empty (`xnavi_mate`, `FREE_LIB_TEAM`). The others
stand as the layer's objects, those who have acted in BN6's own shade
(`BN6_SHADE_DARK` on their sprite's colour shader). BN5's HP box stands
at the top left (`BN5_LIB_HP_BOX`, read from its ROM) and each other
Navi's HP under him in BN5's white digits. Each phase begins with MegaMan
in play; play passes on after each action as BN5's does, BN5's camera
panning to the next Navi, and R passes it by hand. A teammate says a word
in his own habit the first time he comes into play, and after his
battles (`liberation_team_words.c`; [Voice](../../../voice/rules.md#the-guardians), The guardians).

**The commands**, BN5's, an Order Point each, their shapes the players'
guides' (`lib_command_panels`):

| Navi | Command | In the engine |
| --- | --- | --- |
| MegaMan | LongSwrd | two panels in a line, one battle (stage 1's) |
| Colonel | ScrenDiv | three across the panel faced, one battle (BN5's Save not done) |
| TomahawkMan | TomahawkSwing | two deep and three wide, one battle |
| NumberMan | NumberCheck | the two by three ahead: its items taken and its traps disarmed with no battle, the panels left dark |
| ShadowMan | SneakAttack | a guardian one or two ahead struck: its next battle one band easier (our guardians have no HP on the board; on the Darkloid not done) |
| ToadMan | LifeMelody | each teammate beside him yet to act sung for: his Liberation this phase frees five ahead in one battle |
| KnightMan | none | his StoneBody: no harm on the board to him or a Navi beside him |

**A teammate's battle** is MegaMan in that Navi's Soul from the first
turn, with that Navi's HP, written into MegaMan's NaviStats for the
battle and his own put back after it. At the first Custom screen's OK
(`BN6_CUSTOM_OK_HAND`) the Soul is committed as BN5's UNITE commits one
(soulunite.c): the cross record the proxy form's, no chip given, and turns
that never count down (0xFF, which soullife.c leaves alone), so BN6
crosses him as TURN 1 START! shows and he stays in the Soul the whole
battle; no UNITE is offered in it. His HP after it is the battle's as
last seen (BN6's return to the map works MegaMan's NaviStats afresh,
his max among them). A blow that would delete MegaMan in a teammate's
Soul leaves him 1 HP and ends the battle LIBERATE FAILED!: the teammate
is out for the board, as BN5's deleted Navis are, and MegaMan and Lan go
on without him (`--dev matehp=N` tests it). MegaMan's own deletion ends
the run, as in every battle.

**The Darkloids' phase** (`liberation_turn.c`): each unit goes for the
Navi of the team nearest it (BN5's text: closer enemies first) and
strikes him; BigBrute hits every Navi in its reach. KnightMan and the
Navis beside him take no harm, as under a BonusPanel's shield, and the
harm shows over each Navi it hurt.

**The design** (the game-design skill). The experience: "I command a
small team across the dark: who goes where, who spends the Order Point,
who rests." The dialectic stays the run's, *press on or prepare*, now
decided Navi by Navi each phase, and the team adds one of its own:
spread out to work several fronts, or stay together for KnightMan's
shield and ToadMan's song, against the guardians that go for the nearest.
The loop layers: the board's session feeds the run (its prize, its
route), and the run's Souls, won from BN5's guardians earlier, pay a
second time on the board: variety, never power carried between runs
(the Souls are the run's own). The patterns: per-character subsystems
over one chassis (each Navi's one command, sayable in a sentence, over
the shared move, Liberation, Pass and the one pool of Order Points);
loadout as budget (who spends the pool's point); give the defender a
verb (KnightMan, Pass); an honest telegraph (each Navi's HP shown, the
acted ones darker, the stakes box naming the command's panels).

**The generator and the planner for a team.** The planner
(`liberation_plan.c`) plays the team: each phase every Navi in turn,
support first (ToadMan, ShadowMan, NumberMan), each with Liberation, his
command, Pass and fights; the units go for the nearest Navi; a teammate
deleted leaves the play, MegaMan's deletion loses it. The board grows with
its team (`liberation_board.c`): its dark beyond the DarkHoles' rings the
lone board's times two less two for each Navi past MegaMan (a team of
two twice it, of four six times it), set by BN5's pars for its teams (7
for two at mission 1, 9 for three, 9 or 10 for four); BN5's count of
DarkHoles where the floor has room, a lone board's in the last half of its
tries; 60% of a team's decisions dead at most against a lone board's 55%
(each Navi's turn a decision, a support Navi's often his one sensible
move while the rest of his phase decides); a team's sealed route a phase
over the par at least (one Navi's detour while the others press on).
Pass a fifth of the HP for a lone MegaMan too. `LAYER_MAKE` 101.

**The measures** (`tests/test_core.c`, `test_liberation_team`: 16 layers
an act, BN5's own teams of the stage, their HP its table's; and
`test_liberation_board`'s lone boards, 40 an act, before and after Pass
became a fifth):

| Act | Team | Boards | Dark (BN5's) | Par (BN5's) | Dead decisions | Commands a play |
| --- | --- | --- | --- | --- | --- | --- |
| 2 | MegaMan, Colonel | 16 of 16 | 27% (33%) | 6 (7) | 44% | 4.3 |
| 3 | + KnightMan | 15 of 16 | 53% (43-48%) | 8 (9) | 51% | 5.8 |
| 4 | MegaMan, Colonel, ShadowMan, TomahawkMan | 16 of 16 | 60% (51-61%) | 7 (9-10) | 47% | 7.2 |
| 5 | MegaMan, TomahawkMan, NumberMan, ToadMan, ShadowMan | 4 of 16 | 64% (59-60%) | 9 (10-12) | 58% | 11.8 |
| 6 | MegaMan, Colonel, KnightMan, ShadowMan, ToadMan | 2 of 16 | 67% (50-64%) | 7 (14-16) | 55% | 7.2 |

| Lone MegaMan | Boards | Par | Pass the best move |
| --- | --- | --- | --- |
| Act 2, Pass a half / a fifth | 40 / 38 of 40 | 12 / 11 | 7% / 3% |
| Act 3 | 38 / 38 | 14 / 13 | 9% / 1% |
| Act 4 | 39 / 39 | 17 / 14 | 14% / 4% |

What they say: a team's boards read as BN5's of their stage, their dark
thick and their par near BN5's for teams of that size; teammates are
seldom deleted (0.1 a play at act 4), their HP pools being BN5's; each
command is taken several times a board. Acts 5 and 6, which a run does
not meet, rarely fit five Navis on a layer's floor: their boards fall to a
smaller team. A lone MegaMan rests less with BN5's fifth, and his boards
are a phase or three shorter.

**Checked:** the unit tests above; the autopilot through dark boards with
teams (`--dev dark,souls=1`, `0x14` and `0x2B`, `god,onehit`, seed 1):
act 2's board with Colonel won in 7 phases against its par of 8 and the
exit taken, act 3's with NumberMan and KnightMan and act 4's with
Colonel, ShadowMan and ToadMan played on, each teammate in play in his
own body, each of his battles in his Soul (ColonelSoul, ShadowSoul,
ToadSoul) with his HP kept after it, play passing on and the acted
Navis darker; with `matehp=81` Colonel deleted in his battle and the run
going on with MegaMan; over six seeds of act 3 with two Souls held, four
boards for the whole team, one for MegaMan and one teammate, one plain
(its barrier finds no place even alone).

**Left for later:** acts 5 and 6 tuned for their teams with `build.py
pacing` (the owner's decision 5: their boards rarely fit five Navis on a
layer's floor and fall to a smaller team); Colonel's Save; SneakAttack on
the Darkloid (his HP is whole each battle); BN5's own special chip each teammate brings to his
Custom screen (Colonel's C-Cannon), and his own folder; the command
rule's earn check (tools/liberation.py: a board kept only where every
teammate's command earns its point; its spots are built since, The Cross
Navis and the team picked, as built); the second screen's board; the time
a team's board takes to make on the New 3DS (the planner plays every
Navi); co-op over the server, each battle its own player's (stage 5).
