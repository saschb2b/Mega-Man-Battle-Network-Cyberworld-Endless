---
type: Feature
title: "The Cross Navis and the team picked, as built"
description: "BN6's Cross Navis on a dark board with Team ProtoMan's commands traced in BN5's ROM, each fought as MegaMan in that Cross, and the team picked on arrival (`liberation_team.c`, `liberation_pick.c`); the owner's decisions 9 and 10."
tags: [game, liberation, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-25T16:31:33Z }
---

# The Cross Navis and the team picked, as built (25 October 2026)

The owner's decisions 9 and 10 (below), each as recommended: BN6's Cross
Navis join the board with Team ProtoMan's commands, BN5's chip kept for
the Souls; the player picks the team on arrival.

**Team ProtoMan's commands, traced** (romlab on the plain ROM, mission 1
with a unit's Navi id poked; `src/emu/bn5_liberation_cmds.h`,
[ROM data](../../../rom/overview.md)'s BN5's Liberation commands). Team Colonel runs one code
for both versions: a Navi's commands are a mask (a u16 a Navi at
0x08050048: bit n is command n and entry n of the board's text), each
command's check, step and after-battle step are tables by its number, and
its panels come from a table of shapes by Navi and facing (0x08050344).
Team ProtoMan's Navis are ids 1-6 (ProtoMan, GyroMan, SearchMan,
NapalmMan, MagnetMan, Meddy) and their five commands are all there:

| Command | Cross Navi | In BN5's ROM |
| --- | --- | --- |
| Napalm | HeatMan | four ahead and both sides of the third (shape 0x08050762), one battle |
| MagnetBarrier | ElecMan | no battle, no target: every teammate's flag 0x40 (0x08054518) takes his harm to 0 till the Darkloids' phase's end |
| WideSwrd | SlashMan | ScrenDiv's own shape and steps: three across, one battle |
| PanelSearch | EraseMan | the line from the faced panel over dark and item panels with no enemy, ten at most, cut to its item and trap panels: one battle frees those alone; with none, no battle and the Order Point spent |
| BombLiberation | ChargeMan | no battle: the plain dark panel under the flier freed (an item's will not do); his walk mask lets him stand on dark, item and BonusPanels |

The Cross Navis take them by element, as the recommendation set them out:
Fire's area for HeatMan, Elec's shield for ElecMan, the sword's sweep for
SlashMan, EraseMan's search, ChargeMan's flying bomb. The pairing is ours;
the commands are BN5's.

**Who comes** (`liberation_team.c`). Every Navi the run holds: BN5's Souls
whose bodies stand in BN6 (as stage 3 has them), then Gregar's Cross Navis,
the Cross brought or those won from an earlier act's guardians, read from
the run's state alone (`powers_cross_held`: a checkpoint rebuilds the
board before the game's state is back). The board keeps them all for the
command rule; its team is as many as BN5's of the stage has places, the
layer's seed choosing the team the board is made for, and the player the
team that comes.

**On the board.** A Cross Navi stands in BN6's own overworld sprite, as
BN6's guardians stand on layers (`guardian_sprite`: HeatMan 0x47, ElecMan
0x49, SlashMan 0x4B, EraseMan 0x50, ChargeMan 0x4F, each a plain sprite
the map loads with no list of its own), speaks with his own mugshot, and
in play walks as BN6's player in that body. His HP is the median of BN5's
six at the stage (`BN5_LIB_NAVI_HP`, the mean of the middle two: 300 at
acts 2 and 3, 350 at act 4), as BN5 never sends him. His lines keep the
habit BN6 gives him ([Voice](../../../voice/rules.md#the-guardians), The guardians), in
`liberation_team_words.c`. The commands work as BN5's: Napalm's and
WideSwrd's shapes, MagnetBarrier as a BonusPanel's shield over the team
till the next phase, PanelSearch's items and traps of its line freed by
one battle (its menu line greyed where its line holds none: our board
shows every item, so its search has nothing to find), and BombLiberation
from the flier's own panel. ChargeMan flies: while he is in play the
board's fence lets him onto its dark, item and trap panels (our walls,
not BN6's walk: he walks as BN6 walks its player), and A over a plain
dark panel opens his window; BN5's lets GyroMan onto BonusPanels too,
ours not (a BonusPanel here is taken from beside it).

**His battle** is MegaMan in that Navi's Cross from the first turn, with
the Navi's HP: at the first Custom screen's OK the Cross is committed as
CROSSSELECT's commit writes it (`BN6_CROSS_RECORD`, 0 for its chaos, its
used bit in `BN6_CROSSES_USED`), so BN6 crosses him as TURN 1 START!
shows and its own rules run after: a hit of his weakness breaks the
Cross, and he finishes as plain MegaMan. For any teammate's battle, a
Soul's too, MegaMan's Crosses are off CROSSSELECT (their flags cleared
and set back after it: `powers_crosses_hide`) and UNITE is not offered,
so he fights in the teammate's form alone. Deleted there, the Cross Navi
leaves the board as a Soul's teammate does.

**The team picked** (`liberation_pick.c`, `liberation_pick_draw.c`, its
words `liberation_pick_text.c`). After the board's opening look (the
camera to the Darkloid and back) and before L's briefing and PLAYER PHASE
01, where the run holds more Navis than the board has places beside
MegaMan, the TEAM screen opens: our own in BN6's PET look ([The PET's screens: Battle Network's rules](../../../engine/ui.md)),
as neither BN6 nor BN5 has a team screen. The held Navis in a list, those
picked in gold in the ChipFolder's EQUIP corners, the places filled on
the band ("1/2"); for the cursor's Navi his mugshot, HP and weakness, the
form his battles take (ColonelSoul, HeatCross), and his command with its
kind (1 BATTLE, NO BATTLE, ALWAYS ON) and its shape on a small grid. A
picks or puts back (with one place, A swaps), GO! or START takes the team
once every place is filled, B does nothing: the team must come. It opens
on the run's last pick where its Navis are held, the cursor on GO! when
that fills the places, so one press takes it again. Where the pick
differs from the board's team, its Navis go to the team's places, their
bodies are written into the board's objects (`lib_team_dress`: the
objects' scripts' sprite numbers, BN5's bodies copied in again) and the
map is entered again for them, and the par is the planner's for the team
picked (`lib_plan_par`: the board's own plays again, from the seed it was
made with). The board's note (`run.lib`, "CLB5") keeps the team, its par
and the run's last pick, so a CONTINUE takes the same board, team and par
back, and the next board opens on the same pick.

**The command rule in the engine** (`liberation_board.c`). Every held
Navi's command gets its spot, not only the team's, so any pick has its
use: two spots of a shape command's whole shape of dark beside a clean
panel to stand on (ScrenDiv, WideSwrd, TomahawkSwing, Napalm), one dark
line of five for LifeMelody, the spot that darkens the fewest clean panels
darkened where too few stand; an item in a dark line for PanelSearch; a
cache of Order Points and a heart in a two by three for NumberCheck.
MagnetBarrier, SneakAttack and BombLiberation find theirs on every board
(a unit's reach, a DarkHole's guardians, dark panels). The prototype's
check that each command earns its Order Point is not run in the engine.
`LAYER_MAKE` 103.

**The design** (the game-design skill). The experience: "I read this dark
area, then choose who it needs." The dialectic stays the run's, *press on
or prepare*, and the pick adds a loadout's: BN5's places are the budget
(loadout as budget, opportunity cost: taking one Navi leaves another),
and the board's spots for every held command keep each pick situational,
never a dominant one. The loop layers: the session's board, fed by the
run's Souls and Crosses, feeding nothing past the run (variety, not
power). The patterns: per-character subsystems over one chassis (each
Navi's one command), give the defender a verb (MagnetBarrier), an honest
telegraph (the board seen whole before the pick, each command's shape
and kind on the screen), and the player's time respected (the screen
only where there is a choice, the last pick ready under one press). The
costs of a Cross Navi: his form's weakness breaks his Cross in battle,
and his HP is the stage's middle, not a specialist's.

**The measures** (`tests/test_core.c`, `test_liberation_team`: 12 layers
an act, teams of Cross Navis with more held than placed, their HP the
median's):

| Act | Team (held) | Boards | Par | Another pick's par | Commands a play |
| --- | --- | --- | --- | --- | --- |
| 2 | MegaMan, HeatMan (+ Colonel, EraseMan) | 10 of 12 | 5 | 6 | 3.5 |
| 3 | MegaMan, ElecMan, ChargeMan (+ SlashMan, NumberMan) | 12 of 12 | 7 | 7 | 4.0 |
| 4 | MegaMan, SlashMan, EraseMan, HeatMan (+ ElecMan, ChargeMan) | 12 of 12 | 7 | 6 | 8.1 |

A pick's par is the same twice from the same board and pick. The teams of
Souls alone stand as stage 3 measured them (act 2's 16 boards, act 3's
15, act 4's 16), their spots now made for every held command.

**Checked:** the unit tests above and `build.py lint`; the autopilot (its
pick the screen's first, its window a Cross Navi's command where it can
be taken, MagnetBarrier where a unit's reach covers him, the flier onto
plain dark panels) through dark boards with a Cross brought, `--dev
dark,god,onehit`: act 2 (`--run-depth 5`) seed 1 with ElecMan,
MagnetBarrier twice, the board won and its exit taken; seed 2 with
ChargeMan, BombLiberation twice, then a GAME OVER where one hit passed
MegaMan's 100 HP (`god` refills HP between frames, never against one
blow); seed 3 with EraseMan, his battle as EraseCross, then MegaMan's own
Surrounded battle frozen after an AreaGrab (Left for later). Act 3
(`--run-depth 8`, `souls=0x3F`): seed 1 with Colonel and SlashMan,
WideSwrd three times, ColonelSoul's and SlashCross's battles; seed 2
with Colonel and NumberMan (the seed's team; no Cross picked); seed 3 a
plain layer (no barrier found even alone, as stage 3's seeds met). By
hand: the pick screen at acts 2 and 3, Colonel swapped for NumberMan and
the map entered again with NumberMan and HeatMan on the board, HeatMan's
battle in HeatCross on his 300 HP.

**Left for later:** TwinLiberation and Colonel's Save (not traced);
BombLiberation's flier on BonusPanels; the prototype's earn check in the
engine; an AreaGrab in a Surrounded liberation battle froze the battle
under the autopilot (MegaMan's own, with no Cross in it: lib-battle's
field); the second screen's pick; co-op's pick, one player's each.
