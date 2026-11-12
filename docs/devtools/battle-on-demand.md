---
type: Tool
title: "A battle on demand"
description: "Dev steps for `--input` and `tools/play.py`: `battle` (the layer's next random battle), `foes` (named enemies at their panels), `lab` (the same foes as the Battle Lab's test battle, inside a snapshot of the game), `swap` (a Navi swapped mid-battle for the next), `xrecord` (a BN5 record), `bracket` (BN4's tournament bracket moved into BN6), `status` (a panel's type too), `chip`, `folder` and `sig`, with `tools/xcontrol.py` and `tools/xfamily.py`'s battle checks."
tags: [devtools]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-12T21:00:00Z }
---

# A battle on demand

```bash
CYBERWORLD_AUTOPILOT=1 python3 build.py shot --scene emu --net-biome x0 --dev quiet --input "300:battle" --shot 1100:/src/.build/a.bmp
python3 tools/play.py do NAME "battle; wait 200"
```

The `battle` step (`--input`'s `0:battle`, which `300:battle` waits 300
frames before; play.py's `battle`, beside `place` and `flags`) starts the
layer's next random battle at the first moment MegaMan stands free on its
map: no chat, the director's words or a scene, no warp, no guardian's
staging or challenge under way. It waits that long: an act's arrival
words must be paged first (the autopilot pages them above; A in the
script does too).
It is the record the roll would have handed out next, BN6's own or, on a
layer of BN5's (its territory, [Other games' ROMs](../multirom/overview.md)), one of its records in
BN6's engine, forced past the roll as the guardians' are (with `--dev
quiet` too).

The `foes` step (`--input`'s `0:foes 200@52/01@43`, play.py's `foes
200@52 01@43`) starts a battle of the foes it names, at the same first
free moment: each an enemy id in hex at its panel, column 1-6 then row
1-3 (`mm@22` puts MegaMan at column 2, row 2, where he stands without; columns 1-4),
on a plain field with nothing else on it (`field@66` on BN6's field 0x66
instead: BN5's sea field 0x42 as `src/emu/xfields.c` installs it; `sides@33`
with its columns shared as BN5's Surrounded liberation battles share
them, the enemy's 1-2 and 5-6, `mm@32` putting MegaMan between, and
`sides@30` or `sides@3C` MegaMan's four or two: [The battle's field, as BN5's](../game/liberation/building/battle-field.md), The
battle's field), in
BN6's engine whatever the layer. It names the families moved into the
engine ([Transplanted families](../multirom/families.md)) beside BN6's own: with
BN5's ROM read, BN5's CanGard is AI 32 with ids 0x200-0x205 (its six
versions, the families' ids from 0x200), and `0:foes 200@52/01@43`
fights CanGard beside a Mettaur (four foes at most: of six named, BN6's
battle spawned the first four); `--dev xcontrol` installs the control
in its place, BN6's own Mettaur moved as AI 32 with the same ids. BN5's
Team Colonel Navis moved into the engine ([Transplanted Navis](../multirom/navis.md), Transplanted
Navis) take ids 0x1C4-0x1DB, four a Navi (V1, V2, V3, SP): Colonel
0x1C4, ShadowMan 0x1C8, NumberMan 0x1CC, TomahawkMan 0x1D0, KnightMan
0x1D4, ToadMan 0x1D8, so `0:foes 1D4@52` fights KnightMan V1 in BN6's
engine (`--dev xnavicontrol` puts the control HeatMan at 0x1C4-0x1C9).
The `onehit` step (`0:onehit`) holds every enemy at 1 HP from there on,
as `--dev onehit` does: a long fight's end, the control Navi's `delete`
spec. A
layer dressed as BN5's area has longer arrival words, and its random
battles are BN5's: seed 15's first area is BN6's own.

```bash
CYBERWORLD_ROM_DIR=$PWD/.build/xcontrol/rom python3 build.py shot --scene emu --net-biome 0 --seed 1 --dev xcontrol \
  --input "300:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,0:foes 200@52" \
  --shot 1100:/src/.build/a.bmp
python3 tools/xcontrol.py --story     # the control against BN6's own Mettaur, frame by frame
python3 build.py shot --scene emu --net-biome 0 --seed 15 --frames 1400 \
  --input "300:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,0:foes 200@52/01@43,260:,4:START,10:,4:A,400:" \
  --shot 1378:/src/.build/a.bmp   # CanGard's reticle striking MegaMan, its muzzle's blast
python3 tools/xfamily.py              # CanGard in BN6's battles: fight, statuses, deletions, BN6's own left alone
python3 tools/xfamily.py --spec families --family zomon   # every family's (or some): its record, statuses, deletions
```

The `lab` step (`--input`'s `0:lab 01@52/01@62`, play.py's `lab 01@52
01@62`) fights the `foes` step's battle as the Battle Lab's test
(`src/director/lab_battle.c`; [the Battle Lab](../game/battle-lab.md), The
battle): the game's whole state is kept as it begins and put back when
BN6 is on its map again, the blow that would delete MegaMan ends it, L on
the Custom screen leaves at the first try, and nothing of it is counted.
With `CYBERWORLD_EMU_DEBUG` it prints how it ended, its time, busting
level and HP lost, the frames of the game it took and the work RAM's hash
before and after ("lab: won in 96 frames, busting level 9, 0 HP lost;
... (RAM 6b3927d7, then 6b3927d7), the run untouched"). The proof that
nothing is left of it: the same run without the step, stopped that many
frames earlier, ends with the same picture and the same save files (the
Battle Lab's page, Built).

`0:lab open` opens the Battle Lab's builder instead (its pages over BN6's
own battle of the set, held with its field up: FOES, FIELD, STAGE, SETS
and BATTLE!; leaving it prints "lab: left with no fight; the game put
back (RAM ..., then ...)" with `CYBERWORLD_EMU_DEBUG`), listing the families the
profile has battle data on; `0:lab all` lists every family the engine
holds, each in every version, for a look at the screen without a
profile's history.

The `swap` step (`--input`'s `0:swap 101:300/107:300/10D:300`, play.py's
`swap 101:300 107:300 10D:300`; `src/dev/swap_dev.c`) fights one Navi
swapped mid-battle for the next, the Glitch layer's copy
(`src/emu/battle_swap.c`; [Navis moved into BN6's battle engine](../rom/navis.md),
A Navi swapped mid-battle): two or three forms, each a Navi of BN6's by
its enemy id in hex (0x101 HeatMan V1, then six ids a Navi: ElecMan
0x107, SlashMan 0x10D, ProtoMan 0x13D, BlastMan 0x143) and its HP after
the colon (none or 0 its own). The first form's battle starts as the
`foes` step's at column 5, row 2; deleted, it glitches out in BN6's
sprite mosaic, uncounted, and the next arrives on its panel through
BN6's mid-fight spawn and fights as itself; the last form's deletion
wins the battle as BN6 wins any, its results the first form's reward.
With `CYBERWORLD_EMU_DEBUG` it prints each form found, taken off,
spawned and arrived, and every 30 frames the form's state, HP, panel
and the side's counts. A step names a form that is no Navi of BN6's, or
one whose deletion is not BN6's generic Navi's, and fights nothing.

```bash
CYBERWORLD_AUTOPILOT=1 CYBERWORLD_EMU_DEBUG=1 python3 build.py shot --scene emu --net-biome 0 --seed 15 --dev god,onehit,quiet --frames 4900 \
  --input "300:,0:swap 101/107/10D" --shot 1468:/src/.build/out.bmp,1500:/src/.build/in.bmp   # HeatMan glitching out, ElecMan logging in
```

The `bracket` step (`--input`'s `0:bracket 0 1e4000/004001/034002/044003/094004/114005/014006/024007`)
shows BN4 Red Sun's tournament bracket, moved into BN6 (`src/emu/xbracket.h`;
[BN4's tournament screens](../multirom/bn4-tournament.md), Moved into BN6), from the
map MegaMan stands on: the tournament (0 The Den Battle Tournament to 3 Free
Tournament), then its eight entrants in bracket order, each its id, flags
and seed in hex (`BN4_ENTRANT_*`: 1E MegaMan, 40 still in, 80 out this
round, 1 to 4 the rounds won), '/' between them. It needs BN4's ROM beside
BN6's; A, B or START fades it out and the map is entered again.
[The bracket's differential](bracket-diff.md) runs it against BN4's own.

The `xrecord` step (`--input`'s `0:xrecord 811DE10`, play.py's `xrecord
811DE10`; issue #82) starts one of BN5's battle records at the same first
free moment, in BN6's engine whatever the layer, as BN5's territories
will fight them ([One net: the design](../multirom/one-net.md); `src/core/xbattles.c`): the
record's address in BN5's ROM in hex (`build.py xrecords` lists them,
below), then the versions up its viruses go (`0:xrecord 811DE10 1`: V1EX
from V1, BN5's own index). Its foes by BN6's ids (a family both games
have by name BN6's, at the tier of its version; a family moved into the
engine its installed ids), its field as BN6's, its green Mystery Data by
BN5's rule with BN5's finds, its rocks; it prints what it fights, or why
the record cannot be fought yet (a family BN6 lacks, a field with panels
BN6 has none of, a Navi) and starts nothing.

```bash
python3 build.py shot --scene emu --net-biome 0 --seed 15 --frames 1500 \
  --input "300:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,0:xrecord 811DE10,260:,4:START,10:,4:A,400:" \
  --shot 1378:/src/.build/a.bmp   # BN5's ACDC Area record: two CanGards and a Mettaur on BN5's field 0x15, holes and all
CYBERWORLD_AUTOPILOT=weak python3 build.py shot --scene emu --net-biome 0 --seed 15 --frames 1200 \
  --input "300:,0:xrecord 8121898" --shot-range 300:1200:/src/.build/r_   # won, its results: 200 zenny, then the Mystery Data's 2000
python3 build.py xrecords          # every territory record: fightable in BN6's engine, or why not
```

The `status` step (`--input`'s `0:status freeze 90`, play.py's `status
freeze 90`) gives the battle's enemies a status at once, as a hit leaves
it in their collision data for BN6's engine to act on as their next frame
starts: `paralysis`, `freeze` (an ice block round it), `bubble`, `drag`,
`slide` (pushed along a panel), `confuse` or `blind` (a mark over it at
its arm point), for the frames named (60 where none; drag and slide take
none), and with an enemy id in hex after them only that
id's (`0:status paralysis 60 BB`). It prints how many it struck, and
names none outside a battle. It puts a family moved into the engine
through BN6's generic states, which the fight alone never reaches
([BN6's battles](../rom/battle.md), Statuses as a hit leaves them).
`status panel C R TYPE` (`0:status panel 2 2 15`) sets the field's
panel at column C, row R to BN6's type TYPE instead, its type and the
type it is drawn as: the engine's own types and those it is given past
12, BN5's lava (13) and sea (14) and BN4's Hole (15), to see a panel
drawn and stood on before a chip that makes it moves
([BN4's chips](../multirom/bn4-chips.md), The dark units).

`tools/xcontrol.py` copies BN6's ROM alone into `.build/xcontrol/rom`
(no family, Navi or chip of BN5's in its battles) and runs its
specs, `fight` and `status` (`--spec` one of them), three battles each,
with `CYBERWORLD_BTRACE=1` ([Testing](../engine/emulation/testing.md)). With `--navi`
it proves the Navis' installer on BN6's own HeatMan (`--dev
xnavicontrol`): HeatMan V1 native (0x101), installed beside the control,
and moved (0x1C4), each spec's three battles the same byte for byte
(`fight`, 4000 frames of his attacks; `status`; `delete`, every enemy
held at 1 HP by the `onehit` step to the results).
`tools/xfamily.py` copies both ROMs into `.build/xfamily/rom` and runs
its specs, `fight`, `status`, `delete`, `native` and `families` (`--spec`
one of them), each battle traced and its checks printed, pictures of
CanGard's first shot and of each status in its folders ([Testing](../engine/emulation/testing.md),
Testing). The status step prints the battle's frame it struck on, which
the `status` spec reads. `families` runs CanGard's checks for every family
installed (`--family NAME ...` some of them; four runs at once, each in a
container of its own): its V1 alone with MegaMan idle (`--dev god`), its
AI index and V1's HP (BN5's struct2, read from the player's ROM where
`tests/xdiff/families.json` names it) and its CurActions within its state
table; each of paralysis, drag, freeze and bubble given where the plain
run first showed it holding an object (its AttackVars naming one in play,
or one naming it its owner) while it runs a state of its own (not a
chip's family, CurAction 0x10 on: Zomon's AreaGrab freezes time, which
holds every status until it ends), each taken the frame after (CurAction 4, 5,
6, 7), what it held let go and forgotten (no AttackVars word left naming
an object let go), back in its own states and acting again, the battle on
to its end; confusion and blindness given in its first state of its own
(CurAction 8), each one's mark (the fourth kind's 6) over it every frame
after at its arm point 5 as BN5's record has it ([Virus families moved into BN6's battle engine](../rom/virus-families.md), their
arm points; Appley wears no blindness mark, as in BN5: its pre-step takes
the hit's status bits before the status step reads them); and each version
alone at (5,2), MegaMan shooting along his
row and the rows beside it, then along the middle row alone (two steps
up and one down first, wherever a hit left him; TinHawk, as in BN5, is
hit only as it dives along his row), `--dev onehit,god,hp=9999`
(a later version's hit takes more than his 100 HP at once), the rare
path's gate set: its MaxHP its struct2 row's, deleted, the results, BN6's
delete counts and the rare path's byte untouched through every frame.

The `glitch` step (`--input`'s `0:glitch`, play.py's `glitch`) puts
MegaMan two panels from the layer's Glitch warp, or a Glitch layer's
copy ([the Glitch layer](../game/glitch-layer.md)); `--talk glitch:FRAME` and
`--talk copy:FRAME` open their talk.

The `sig` step (`--input`'s `0:sig`, play.py's `sig`) puts MegaMan in the
layer's signature, the room it is remembered by ([Identity](../world/level-design/identity.md),
Identity), on the free panel nearest its anchor; play.py's state (with
`CYBERWORLD_STATE_POS`) names the signature and where it lies, in panels
and in the world. The `identity-*` screenshots and the `identity` clip
are made with it.

The `folder` step (`--input`'s `0:folder 2`) copies MegaMan's first
folder into Folder2 (its Regular chip and TagChips too) or, with 3, the
Extra Folder, as a run that handed out more folders would hold them: no
run does yet, and the Net Square's NetBattle asks which folder goes
where more than one holds chips ([One rule: full rules](../online/arena/from-square/one-rule.md), One rule: full
rules). Before a `chip` step, the copy keeps BN6's chips while the first
folder takes the chip.

The `chip` step (`--input`'s `0:chip 204 L 29`, play.py's `chip 204 L
29`; issue #82) puts chip ID (1-511: BN6's, or one of BN5's in BN6's
tables, [The chip registry](../multirom/chip-registry.md)) in code CODE (a letter or
`*`) into the folder's first N entries (one where none), owned as BN6
marks a chip it gives, and gives it to the pack N + 1 times through
BN6's GiveChips (outside the folder editor the pack's counts hold the
folder's copies: one is the pack's own). It prints the chip's name and
its four counts in the pack (an id from 320 counts in the pack's
extension: `0:chip 411 M 1`, MrkCan1, reads "its counts 2 0 0 0"). A
battle then deals the folder as BN6 does, with the chips that sit out
taken out of the deck (`CYBERWORLD_EMU_DEBUG=1` prints how many).
`chip ID` alone changes nothing and prints what the game holds of it:
the pack's four counts and order stamps, the folder's copies and whether
the profile's Library has it (`chip 700`: "held: the pack's counts 6 0 0
0, its stamps 7fff 0000 0000 0000, 5 in the folder"), an id to 1023.

```bash
CYBERWORLD_EMU_DEBUG=1 python3 build.py shot --scene emu --net-biome 0 --seed 15 --frames 1700 --dev quiet \
  --input "300:,0:chip 204 L 29,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,0:foes 01@52,260:,4:A,10:,4:START,10:,4:A,150:,4:A,200:" \
  --shot 1150:/src/.build/a.bmp,1365:/src/.build/b.bmp   # WideSht1 L's card on the Custom screen, BN5's picture; its wave at the Mettaur
```
