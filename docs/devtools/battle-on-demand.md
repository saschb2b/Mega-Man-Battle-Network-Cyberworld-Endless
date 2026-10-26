---
type: Tool
title: "A battle on demand"
description: "Dev steps for `--input` and `tools/play.py`: `battle` (the layer's next random battle), `foes` (named enemies at their panels), `xrecord` (a BN5 record), `status`, `chip`, `folder` and `sig`, with `tools/xcontrol.py` and `tools/xfamily.py`'s battle checks."
tags: [devtools]
generated: { by: claude-code/unrecorded, at: 2026-10-25T00:15:50Z }
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

```bash
CYBERWORLD_EMU_DEBUG=1 python3 build.py shot --scene emu --net-biome 0 --seed 15 --frames 1700 --dev quiet \
  --input "300:,0:chip 204 L 29,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,4:A,56:,0:foes 01@52,260:,4:A,10:,4:START,10:,4:A,150:,4:A,200:" \
  --shot 1150:/src/.build/a.bmp,1365:/src/.build/b.bmp   # WideSht1 L's card on the Custom screen, BN5's picture; its wave at the Mettaur
```
