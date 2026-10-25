---
type: Tool
title: "Dev menu: shorter test runs"
description: "Select+R opens the dev menu in battle or on the net (can't die, one-hit enemies, speed, next layer), and `--dev` sets those switches and many with no menu entry (`fragile`, `dark`, `souls=MASK`, `folder=ID`, `hp=N`)."
tags: [devtools]
generated: { by: claude-code/unrecorded, at: 2026-10-25T01:17:25Z }
---

# Dev menu: shorter test runs

Hold **Select** and press **R**, in battle or on the net. The game holds still
while the menu is open; **B** or Select+R closes it.

| Item | Effect |
| --- | --- |
| Can't die | MegaMan's HP stays full, in battle and out |
| One-hit enemies | enemies keep 1 HP: any hit deletes them |
| Random battles | off: no random battles (guardians and challenges still fight) |
| Speed | 1x, 2x, 4x or 8x the game's speed |
| Win this battle | every enemy's HP to 0; they are deleted once the turn runs |
| Heal | MegaMan's HP full |
| +10000 zenny | through the game's own give-zenny script |
| Next layer | the next layer down, at once |
| Next guardian | the layers down to the next guardian, arriving before its room |
| Area | the area of the next layers (Next layer to go there); "the run's" to return to the run's own |

Switches reset when the game restarts. The same switches start on from the
command line:

```bash
python3 build.py shot --scene emu --dev god,onehit,quiet,speed=4
```

Ten more have no menu entry: `fragile` (MegaMan keeps 1 HP in battle, so the
first hit ends the run), `powers` (the five Crosses and BeastOut open from
the first battle on, for a capture of them: 0.3.0's trailer,
`tools/trailers/v0_3_0.py`, plays one),
`gem` (every random battle with a green Mystery Data on the field, to
check it and its reward; BN5's battles too, with BN5's own find),
`visitors` (every random battle the visitors' rules allow takes a visitor
of the other net, not one in four: from act 2, where BN5's ROM is read,
[One net: the design](../multirom/one-net.md); `CYBERWORLD_EMU_DEBUG` prints each roll's foes,
the visitor's family among them, and the run log marks it `+`), `dark`
(every act's second layer from the second a dark layer, a liberation's
board, [Liberation Missions](../game/liberation/overview.md); `CYBERWORLD_EMU_DEBUG` prints the board as
text, and `CYBERWORLD_AUTOPILOT=1` plays it; with `libpar=N` its par is
N, for each tier of its prize; `CYBERWORLD_AUTOPILOT_ROUTE=1` beside it
seeks the sealed region's route first, its key and then the route, and
takes the way it opens at home), `route` (a special route found: with
`--scene home` on the act before's board, from a layer on its own act's,
so the next fork offers its third way, [Special routes](../game/liberation/special-routes.md), Special
routes), `veteran` (a profile that has met seven
guardians, found two Spins and won three of the rival's duels, where it
has none: the PET's mails for `build.py screenshots pet`) and `duels=N`
(the rival's wins made N: `duels=2` brings ProtoMan's netbattle, or his
words naming the third act before it, `duels=1` opens the official gates
of level 1; [The rival: Chaud and ProtoMan](../game/rival.md)), and `hp=N` (MegaMan's max HP made N, and his
HP with it whenever the game moves the max: a later act's guardian, which
a headless start meets at 100 HP, fought at a playtester's HP; `hp=N/H`
holds his HP at H through the run's first 600 frames, a battle begun
hurt), and
`pieces=MASK` (the set pieces in MASK on every layer whose area has them,
where they find room: 1 purple data, 2 a Rush gap, 4 teleports, 8 a Link
Navi obstacle, 16 a security cube, 32 an arrow lane, 64 an invisible
path; `pieces=32` to look at an area's arrow lane and ride it;
[Set pieces](../world/level-design/set-pieces.md)), and `mapall` (the layer's map whole,
as if every panel were seen: a capture of the map and its way, as
`tools/before_after.py` compares two builds' with it), and `xcontrol`
(the control family installed as the core starts: BN6's own Mettaur
moved into the engine's free space as AI 32, ids 0x200-0x205, for the foes
step below and `tools/xcontrol.py`; [Transplanted families](../multirom/families.md), Transplanted
families; it takes the place of BN5's CanGard, which is installed
wherever BN5's ROM is read), and `nofamilies` (no family moved into the
engine, BN5's ROM read or not: BN6's battles as the player's ROM has
them, which `tools/xfamily.py` compares with CanGard installed), and
`xnavicontrol` (the control Navi installed instead of BN5's Navis: BN6's
own HeatMan moved as Navi AI 25, ids 0x1C4-0x1C9, for the foes step and
`tools/xcontrol.py --navi`; [Transplanted Navis](../multirom/navis.md)), and
`nonavis` (no Navi moved into the engine, BN5's ROM read or not: no
Souls then either). BN5's Souls and DarkChips have three:
`souls=MASK` (a new run holding BN5's Souls, bit k Team Colonel's Soul
7 + k: 1 Colonel's, 2 ShadowMan's, 4 NumberMan's, 8 TomahawkMan's, 0x10
KnightMan's, 0x20 ToadMan's; UNITE on BN6's Custom screen in every battle,
the Souls installed wherever BN5's Navis are: [The proxy form](proxy-form.md)),
`darkchips=MASK` (a new run holding DarkChips, bit k BN5's kind k, its id
187 + k: BN6's four 0x2 DrkSword, 0x4 DarkInvs, 0x40 DarkThnd, 0x80
DrkRecov, BN5's own seven 0x1 DarkCirc, 0x10 DrkLance, 0x20 DarkWide,
0x100 DarkMetr, 0x200 DarkDril, 0x400 DarkTorn, 0x800 DrkSonic; the
first layer puts each that plays in BN6's battles in the Pack:
[DarkChips](../game/meta/darkchips.md)) and `worried` (MegaMan held worried by BN5's
mood in BN6's battles once each opens, so BN5's DarkChips come into the
hand's end, for a capture of one offered: with `hp=999/200` the first
screen opens worried too, and one picked frees the hand's end for the
next; `0:chip 214 R 1` puts DarkCirc R in the folder), and `chipai`
(MegaMan put under BN5's chip AI for the player two seconds into each
battle, for 600 frames, as BN5's DarkInvs puts him: its machine,
`src/emu/darkinvs.c`, with no button pressed after the Custom screen's
OK he picks from his folder's chips, walks to where one reaches and uses
it, its end by its time and BN5's as DarkInvs's: [The chips' code](../multirom/chips-code.md), The
chips' code, DarkInvs, moved). The guest
battles' `slowboot=N` and their `gem` went with them. `folder=ID` makes the
run's folder thirty of chip ID in *, or in its first code where its record
has no * (`folder=286`: every hand DrkSword, for a capture of one chip in
BN6's battles; `folder=218`, DarkDril in D: BN6 draws a chip in a code its
record lacks blank, no picture, no code and no effect, its own JustcOne and
FireBrn2 in * too, and the `chip` step says so where it is given one),
`folder=ID/N` its first N
entries alone; `pack=N` puts a copy of each of the first N chips in the
Pack, in its first code (the folder editor's Pack and its scroll on the
second screen, issue #75); `programs=N` gives a copy of each of the first
N NaviCust programs, in the first of its colours the ROM draws, with the
check BN6 lists them by (the NaviCustomizer on the second screen, issue
#76), and `program=P` one of program P besides (a guardian's draft beside
a program held: `program=41` and `--seed 11 --run-depth 3` offer HP+50
again, "Held 1", [The NaviCust](../game/navicust.md) 10); `codes=N` has the profile know the compression codes of the first N
programs that have one (SuperArmor, then Custom1), as if entered in a run
before, kept in its file: the run begins with them compressed and Dad's
Compression mail lists them ([The meta layer](../game/meta/overview.md); `codes=2,programs=2` for a
capture of both on the NaviCust screen); `--talk bugfrag:FRAME` gives one BugFrag, as `bugfrags` gives fifty
(a DarkChip's last). `clock=N` starts the run with the Net's clock at N
notches ([3. Going back, and the Net's clock](../world/home/going-back.md)): the guardians spawn with its HP, a
tenth more a notch. `CYBERWORLD_AUTOPILOT_BACK=N` has the autopilot take
N trips back from Lan's HP, through its older portals, before a way.

The super bosses ([Super bosses](../game/bosses/super-bosses.md)) have three: `side=N`
starts the run on a side layer at `--run-depth`, 1 the Undernet (its
golden gate) or 2 the Secret Area, Bass's once the title's S is held
(`--marks 08`); `bass=N` and `beast=N` make MegaMan's wins over Bass and
over the Cybeast N, their forms and words by that record (`bass=1` Bass
SP, `bass=2,beast=1` Bass BX). `--talk scrtdata:FRAME` gives the three
ScrtData a golden gate wants. With the debug output on, the music's
state every second shows the approach's fade and the silence after the
battle:

```bash
CYBERWORLD_AUTOPILOT=1 CYBERWORLD_EMU_DEBUG=1 python3 build.py shot --scene emu --run-depth 8 --seed 7 --marks 08 \
    --dev god,quiet,hp=800,side=2 --frames 4000 --shot-range 1700:4000:/src/.build/bass/f
CYBERWORLD_AUTOPILOT=1 python3 build.py shot --scene emu --run-depth 19 --seed 7 --dev god,quiet,hp=800 --frames 4000
```
