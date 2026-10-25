---
type: Tool
title: "The proxy form: where a Soul stands"
description: "`--dev soulproxy` makes form 8, Falzar's TenguCross that Gregar never offers, an exact copy of HeatCross for BN5's Souls, and `tools/xproxy.py` proves it by one battle fought four ways, every byte traced."
tags: [devtools]
generated: { by: claude-code/unrecorded, at: 2026-10-13T01:53:58Z }
---

# The proxy form: where a Soul stands

```bash
python3 tools/xproxy.py               # four runs: the proxy is HeatCross frame for frame, Gregar's own form 8 is not
python3 tools/xproxy.py --story       # the proxy run's battle, frame by frame
python3 tools/xproxy.py --mutations   # also each part of the install left out: each the battle reaches must part them
python3 tools/xproxy.py --souls       # the four with the Souls installed over the proxy, none held (BN5's ROM beside BN6's)
python3 tools/play.py start NAME -- --scene emu --dev soulproxy   # then: flags 0xE2 0xE2 1, cross 8, and a battle
python3 tools/play.py start NAME -- --scene emu --dev souls=0x3F,folder=140   # every Soul held, a hand of JustcOne: UNITE lights KnightSoul
python3 tools/xdiff.py soul_knight   # a Soul's life in both engines (tests/xdiff/soul_*.json)
```

BN5's Souls stand behind one form of BN6's own (the Souls track's
S1; [Transplanted Souls](../multirom/souls.md)): form 8, Falzar's TenguCross,
which Gregar keeps in every table by form and never offers on
CROSSSELECT. `--dev soulproxy` (`src/emu/soulproxy.c`) makes form 8 an
exact copy of HeatCross in the core's ROM copy as the core starts: entry
8 of each of the eleven tables by form written from entry 1, its NameID's
object id made HeatCross's (in the enemy ids' table the engine reads: the
families' room's grown copy where BN5's ROM grew it, through the room's
own writer, `xg_bn6_entry`), and three hooks where Gregar's code reads the
form itself (HeatCross's chip bonus, his arm flash's look, the summon as
he crosses) answering 8 as 1; every byte is checked as Gregar's first,
and nothing is written where one is not ([MegaMan's forms and BN5's Souls in BN6's engine](../rom/souls.md), MegaMan's
forms). `--dev proxybreak=N` leaves parts out (`SOULPROXY_BREAK_*`), for
the comparator's own test. The dev step `cross F` (`--input "0:cross 8"`,
play.py's `cross 8`) commits the next Cross chosen on CROSSSELECT as form
F, through CROSSSELECT's own commit: its record, the turn that takes it,
the chosen row's used bit. Without the switch nothing of it is in the
game.

`tools/xproxy.py` fights one battle four times from one boot with the
same keys, each traced (`CYBERWORLD_BTRACE`: every byte of EWRAM and
IWRAM after each frame of the battle, as `tools/xcontrol.py` does):
native (no proxy, HeatCross chosen), installed (the proxy installed,
HeatCross chosen), proxy (installed, HeatCross committed as form 8) and
tengu (no proxy, form 8: Gregar's own TenguCross). The battle covers the
crossing (CROSSSELECT, the summon, the overlay), a charged shot
(HeatCross's flame), FireBrn1 (the Fire bonus, a hooked reader), a buster
shot from the row above (the arm's flash, a hooked reader), a step into
a StarFish's row whose bubble breaks the Cross (its weakness), the
buster, and the results; each run is checked to have done all of it.
native and installed must be the same, every byte; installed and proxy
the same but for the places the form is kept (1 against 8: his battle
NaviStats' form, the Custom screen's record, the link packets and both
sides' pending forms it passes through) and his NameID (0x1AC against
0x1B3); tengu must differ. `--mutations` leaves each part out in a run of
its own, and each the battle reaches must part the proxy from HeatCross;
the object id's the battle never reads for him in a Cross (his object is
set up as a battle begins, and a battle of a run begins in no form): it
is reported, not counted. From the proxy,
Beast Out gives Falzar's TenguBeast (form 0x14: the Custom screen's
emblem adds 12 to the form), not HeatCross's: the Souls grey Beast Out
while one is united.

The Souls stand on it from S2 (`src/emu/soulunite.c`, `soullife.c`,
`soulkinds.c`; [MegaMan's forms and BN5's Souls in BN6's engine](../rom/souls.md), UNITE on BN6's Custom screen, and a
Soul's life on the proxy form): wherever BN5's Navis are moved in (BN5's
ROM read beside BN6's, `nonavis` and `xnavicontrol` aside), the proxy is
installed as `soulproxy` does and UNITE over it as the core starts (BN5's
art and its A on UNITE copied from BN5's ROM), every run's battles
taking the run's Souls as each begins; `--dev souls=MASK` gives a new run
those Souls. The second screen's card names UNITE's Soul under the
cursor, and its fight panel the Soul he wears (`--second-shot`). The emblem under OK is UNITE while the last chip picked lights
a held Soul (`--dev folder=ID` deals a hand of one chip: 140, JustcOne,
Break as BN5 has it, KnightSoul's kind); A on it gives the chip up and
shows the Soul, OK unites: the proxy form with the Soul's turns, Beast
Out grey until it ends. Without a held Soul or a lit one the emblem is
BN6's. United, the Soul looks and fights as BN5's (the Souls track's
S3-S5: `--dev souls=0x3F,folder=ID` with a chip of a Soul's kind, 143
RockCube, 177 Invisibl, 175 BusterUp, 116 Boomer, 140 JustcOne, 181
BblWrap, then B held for its charged shot). Its differential is
`tests/xdiff/soul_*.json` (Differential, A Soul's life; A Soul's look,
abilities and passives), each break of its rule `unitebreak=N`, of its
look, abilities and passives `soulbreak=N`. `tools/xproxy.py
--souls` runs the four runs above with the Souls installed (BN5's ROM
beside BN6's in `.build/xproxy-souls/rom`, seed 15, whose layer BN5 does
not dress) and no Soul held, native and tengu with `--dev nonavis` (the
Souls, and the proxy under them, install with BN5's Navis wherever its
ROM is read since issue #82's run round): native and installed the same
every byte
(the Souls' hooks leave BN6's battle alone while none is held; a pointer
into a table the Souls grow, the emotion window's faces and their
palettes, read through its copy, BN6's bytes at the same place; BN5's
mood, which the Souls keep in their own block whoever holds one, its
opening mark, mood and dark latch by side), and the
proxy HeatCross frame for frame through a Soul's life's two stubs, which
its overlay entries 8 then point at.
