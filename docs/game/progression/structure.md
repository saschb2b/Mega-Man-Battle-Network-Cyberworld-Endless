---
type: Design
title: "The structure"
description: "The progression structure: depth decides difficulty, the short and endless nets, per-act HP and damage targets, how battles, BN5's records, areas and guardians are chosen, the battle floor, growth, risk the player chooses, the Cybeast, later cycles."
tags: [game, progression]
generated: { by: claude-code/claude-opus-5-5, at: 2026-10-26T17:30:00Z }
---

# The structure

## Principles

1. Depth decides how hard a battle is. The area decides which viruses and
   which Navi, within that limit.
2. Every limit is measured against the max HP MegaMan is expected to have
   at that depth.
3. The player sees a threat before committing to it, and extra risk is a
   choice that pays.
4. Everything is decided from the run and layer seeds, before the game's
   RAM is involved, so layers stay reproducible.

## The short net and the endless net

A new run is the short net ([The meta layer](../meta/overview.md)): layers 1-9 are the endless
net's first three acts, as they are below, and layer 10 is the Cybeast
Nest, paced as a fourth act (its battles in act 4's band, its guardian,
one of the Nest's own, picked and versioned as a fourth act's, never SP).
Its fall wins the run. The endless net, opened by a win, is the whole
structure below. A threat rung ([The meta layer](../meta/overview.md)) adds one constraint to
either: a virus version up from act 2, heals only where an act is sure of
one, dealers half again, guardians at EX from act 2, SP Navis in Servers
from act 1.

## The act

Every act keeps three layers and gets a fixed rhythm:

| Layer | Battles | Services |
| --- | --- | --- |
| First | The easier half of the act's band (the run's first layer: its first two battles) | Mystery Data; a Net Dealer from act 2 on (a shop at 50% on the run's first layer) |
| Second | The full band; a marked Server challenge may appear | Net Dealer and Recovery Mr. Prog, always |
| Third | The full band | Heal and Net Dealer before the arena, then the guardian |

Beating the guardian heals MegaMan fully and gives the reward (see
[Growth](#growth)). The act card names the area and its guardian, so the
player can prepare the folder for it.

## Targets per act

For the first cycle. MegaMan's HP assumes the growth below and no HPMemory
bought; the value before this structure is in brackets.

| Act | Layers | MegaMan max HP | HP per battle | Hardest hit | Versions | Guardian |
| --- | --- | --- | --- | --- | --- | --- |
| 1 | 1-3 | 100 (100) | 90-160 | 30 | V1 | 400-600 V1 |
| 2 | 4-6 | 200 (160) | 150-280 | 80 | V1 | 600-700 V1 |
| 3 | 7-9 | 300 (220) | 200-360 | 120 | V1-V2 | 800-1000 V1 |
| 4 | 10-12 | 400 (280) | 280-420 | 160 | V2 | 1100-1300 (Colonel V1, EX of the 800s) |
| 5 | 13-15 | 500 (340) | 350-500 | 200 | V2-V3 | 1100-1500 EX (the Undernet) |
| 6 | 16-18 | 600 (400) | 400-580 | 240 | V3 | 1200-2000 EX (the Graveyard) |
| Nest | 19 | 700 (460) | 450-650 | 280 | V3, SP | SP |

"Hardest hit" is the damage value of the strongest virus in a formation,
kept to about 40% of MegaMan's HP, so no single hit takes more than half.
At act 1 that keeps Puffy, Trumpy, WindBox and ScarCrow (80-100 damage)
out until MegaMan has grown. The first act allows a third of MegaMan's HP:
with the starting folder a playtester lost the run's first battle to two
viruses hitting for 20.

## Choosing battles

For each formation the area offers, the engine sums its viruses' HP and
takes the highest damage value, reading both from the ROM's enemy table for
each version. It then picks the highest version, up to the act's, at which
the formation fits the act's HP band and damage cap. A formation that does
not fit even at V1 is left out on that layer. If an area leaves nothing,
the band widens by one step before any formation is allowed.

Two more rules from Slay the Spire: the first two battles of a run, and
every battle on the first layer after each guardian, come from the lower
half of the band (a new act eases in: its viruses hit harder and the step
from the last act's easiest is large), with
two thirds of its hardest hit and two viruses at most where the area has
such battles (the run's very first battle keeps the formation's first virus
alone), and the same formation does not come twice in a row.

No battle before the Undernet (act 5) holds two DarkMechs: they teleport
beside MegaMan and slash for more than their damage value says, and a
pair took a playtester from 480 HP to 20 in act 3. CopyBot's comps hold
them only in pairs, so DarkMechs now first come in the Undernet, their
own area in BN6.

A rare virus (one, the others unchanged) may come from act 3 on, 3% of
battles and 2% more each later cycle, and only when the battle still fits
the band with it. BN6's own rare battles (byte 7 of a BattleSettings record)
are left out: the game holds them back until a family has been deleted 16
or 32 times.

Sky HP and Green HP each have a single random battle of their own: every
act 2 battle on Sky HP was Gunner and FgtrPlne. Two battles in three there
come from the area each homepage stands in (the Sky, the Green Area),
inside the act's limits as always; the report shows Sky HP's act 2 battles
across nine kinds of virus since. Two more areas bring too few kinds of
virus: the Aquarium Comp's Piranhas and Quakers take 70% of its battles (a
playtester met nothing else in seven fights) and the ACDC HP has three
kinds, 85% of them two. One battle in three there comes from the town's
other area (the Aquarium HP, Central Area): 11 and 9 kinds of virus in the
report, the two commonest about half. And a battle that shares a family
with the last one is a third as likely as it would be (six of a
playtester's seven act 1 battles in the RoboDog Comp held Gunners): the
area keeps its viruses, but not the same ones fight after fight.

## The battle floor (26 October 2026)

Every layer without a guardian has at least one random battle, Phase 1 of
[the core loop](../core-loop.md) ("A battle floor"): 43% of the layers of
seeds 1-12 had none, and an act could be a walk to its guardian. It serves
the run's own dialectic, preparing for the act's guardian against the walk
that costs time: a layer's battle is where the folder is tested and chips,
zenny and busting levels come from, so a layer without one is all price
and no find. The pattern is bad-luck protection (pity), as loot systems
bias a drop upward after a dry spell, on BN6's own roll and never in its
battle:

- BN6's roll ([BN6's battles](../../rom/battle.md), Encounter odds and classes) checks once
  each 0x40 of the walk since the last battle, its odds rising with that
  walk, scaled by the map's class; the maps the layers take over have
  classes 3, 5 and 6, and class 6 gives nothing before 0x380, longer than
  many a layer's walk. On a layer still without a battle, past 0x100 of
  the walk (about a quarter of a layer's), the walk reads 0x400, BN6's
  best row, so its roll comes at its own moment with its best odds
  (`src/director/battle_floor.c`). Once a battle has been fought there,
  BN6's walk counts as BN6 counts it.
- The few layers it misses hold their exit pad shut, and stepping onto it
  starts the battle the layer owes, the same roll; the pad opens after it.
  Over seeds 1-12 that was 1 layer of 92.
- Not on a guardian's layer (his battle is its own), a dark layer's board
  (its battles are the board's), a trip back (the player's errand home),
  or where BN6's random battles are off.

The battles are the roll's as ever, so the first act keeps its gentler
openings (the run's first two battles and the first layer after each
guardian from the lower half of the band, above). MegaMan's walk and BN6's
battles are untouched; what a layer seed makes is too (no `LAYER_MAKE`).
With it, over seeds 1-12 (the autopilot, `--dev god,onehit`, 60000
frames), layers without a guardian went from 0.27-1.00 random battles a
layer by act and 41 of 106 without one to 1.23-1.50 and none, acts from
1-4 to 2-5; on act 1 played as it is (30000 frames) viruses ended 3 runs
of 12 (2 before) and the guardian 7 (10 before), two runs still going.
`build.py pacing` holds every layer to it ([Measuring it](measuring.md)).

## BN5's battles: one band, floor and cap (4 October 2026)

Where Battle Network 5's areas dress an act, its random battles are BN5's
own records, fought in BN6's engine since issue #82's D1 ([One net: the design](../../multirom/one-net.md),
One net: `xterritory_encounter`, BN6's step counter rolling them as its
own; before it in BN5's engine on a second core, Guest battles; where a
family of its could not be moved in, the BN6 area's own battles).
They were held to the act's band from above only: a record's viruses' HP
together at most the band's top, its strongest hit at most the cap. BN5
places its records by its story, so from act 2 its early areas fell far
under the band: ACDC Area's own at act 2 were 80 to 200 HP against 150 to
280, Oran Area's at act 3 100 to 230 against 200 to 360. The owner's call:
a band with a floor and a cap, as BN6's battles have.

What scaling means here, measured on both games' data:

- **BN6 scales by choice, not by multiplying.** The engine never raises a
  virus's numbers. Depth sets the act; the act sets the band (HP from its
  floor to its top, a cap on one hit) and the versions allowed (V1 to V3,
  one more on later cycles and from threat 1). Each formation an area
  offers is taken at the highest version that keeps it under the band, and
  those that reach the floor are preferred: the formation and its viruses
  stay BN6's own; only which of BN6's own versions comes changes.
- **BN5's viruses come in families of versions too.** Its ids table gives
  each enemy its version, type and AI index, as BN6's does: one AI index
  is one family, six versions each, its ids in a row (the Mettaur's 1 to
  6: 40, 70, 100, 150, 200 and 300 HP, hits of 10 to 200). A record is a
  formation whose viruses can be taken up their families the same way.
- **So BN5's battles scale as BN6's.** Each record of the map's is taken
  at the most versions up (to the act's version, never below its own)
  that keep it under the band's top and cap; those that reach its floor
  are the pool; fewer than four there, the area's other maps' and then
  BN5's other areas' join, each record once; none inside the band at all,
  the same three under the floor; nothing under the cap anywhere, the
  map's own weakest. The battle's copy of the record carries the raised
  viruses (their ids in a copied entity list), so its results screen's
  rewards are theirs. Nothing is multiplied: every virus is one of BN5's
  own, at a version BN5 has.
- **The dialectic** stays BN6's fair challenge against the area's
  identity: a territory's battles are its own formations first, grown to
  the act, and only a thin pool borrows from BN5's other maps.
- **In BN6's engine (D1)** the same pool, the act's version by tier (BN6's
  V1 lets BN5's records go to V1EX, V2 to V2EX, V3 to V3EX), BN6's easing
  (an opening battle the band's lower half, two viruses at most, the run's
  first one virus), BN6's repeat rule and its random numbers; a family
  both games have is BN6's at the tier of its version, measured by BN6's
  stats.
- **Visitors and the deep** ([One net: the design](../../multirom/one-net.md)): from act 2 one
  battle in four takes a virus of the other net at the highest version
  that keeps the battle's HP to the band's top and its hit to the cap;
  the deep's acts take each battle by a coin from BN6's area's own or the
  older net's late areas', each held to the band as above.

`build.py pacing` measures it: every act's territory battles as BN6's
engine fights them (native, opening and challenge rows, with how many of
the older net's), the
deep's acts with the older net's share of their battles, and every act's
battles from the second with a visitor where one fits (the pairs, HP
against the band): none past the band, at threat 0, over every territory
and cycle.

## Choosing areas

Areas fall into three tiers by their own battles:

| Tier | Areas |
| --- | --- |
| Opening | Central, the first home computers (0x8C), Seaside |
| Middle | Judge Tree Comp, Green, Green HP, the homepages (0x88, Aquarium HP's battles among them), the second home computers (0x8D), Sky HP (its battles all 200 HP), Aquarium Comp (its pools and water mazes a hard first map), Robot Control Comp (the battles that fit act 1 are all OldStove and Mettaur; from act 2 Champy and Gunner join them) |
| Late | Sky, Mr. Weather Comp, ACDC HP, CopyBot's comp |

Act 1 draws from the opening tier, act 2 from opening or middle, act 3 from
middle or late, act 4 from late. Every run still varies, and the areas
whose viruses are late in BN6 come late in the run. The battle budget above
keeps any area safe even when a tier runs short.

Acts 5 and 6 changed places: the Undernet's battles are lighter than the
Graveyard's, and BN6 keeps the Graveyard for after the story. Beast Out then
came from the last guardian before the Nest (since issue #109, from the
Cybeast at the Nest itself). Runs saved before keep their order.

## Choosing guardians

Each area keeps its pool of Navis. A guardian is drawn from the Navis of
the area's pool whose HP at the act's version falls in the act's band;
when none does, from all Navis in the band; only then the pool's nearest.
Acts 1-3 fight V1, acts 4-6 EX where it fits and V1 otherwise. Act 1 meets
BlastMan, DiveMan or SpoutMan, alike, and a new run from the title draws
the next seed's run where its act 1 guardian is the last run's (the
profile keeps it). A later act draws again where it met an earlier act's
guardian and another fits (SpoutMan lies in both of the first two bands).
Colonel appears from act 4 on. Over 500
drawn runs every guardian lies in its act's band (`build.py pacing`).

## Growth

MegaMan reaches about 100 HP more per act:

- A guardian gives 5 HPMemory (+100) in place of 3, and heals MegaMan fully.
- A guardian also gives its own Navi chip at the version fought (V1, EX,
  SP). Beating a Navi and getting its chip is Battle Network's own reward.
- From act 2 on, 15% of the rich Mystery Data (the best of three
  qualities) hold an HPMemory, blue as the game keeps them.
- The Net Dealer's HPMemory costs 800 zenny in act 1 and 400 more each
  act, not more each layer. The NaviCust vendor's programs cost a quarter
  of the game's prices (its endgame's) plus 200, and 200 more each act:
  SuperArmor 1000 zenny in act 1, Attack+1 700.
- Every Net Dealer stocks a chip of the element that answers its act: the
  one that hits the act's guardian twice as hard, or with a guardian weak
  to none, the one half or more of the kinds of virus the area's battles
  can hold at that depth are weak to, two kinds at least, each at every
  version the act's battles can roll (Swordy2 is Fire, Swordy3 Aqua;
  `counter_element` in `loot.c`; a mixed area such as CopyBot's comps gets
  no word, and `build.py pacing` lists each act's areas), and says so,
  naming the guardian ("Word is, BlastMan can't stand Aqua chips"). Both
  of BN6's wheels count: Aqua beats Fire, Elec Aqua, Wood Elec and Fire
  Wood; Sword beats Wind, Wind Cursor, Cursor Breaker and Breaker Sword
  (an enemy's traits, `enemy_weakness` in `data.c`; a chip's icon,
  `chip_hits_with`). SlashMan can't stand Breaker chips, EraseMan Wind,
  TenguMan Sword, GroundMan and DustMan Cursor: the dealers had said they
  had no weak element (issue #39), and a Sword's 80 took 160 of
  TenguMan's HP. A guardian weak to none (ProtoMan, CircusMan, JudgeMan,
  Colonel; ElementMan, whose element changes as he fights) is answered
  by the hardest hit on the list instead, and the dealer says so, and
  names the viruses' weakness. The answer is a sword only where Sword is
  the weakness (swords did not reach CircusMan at his back column;
  TenguMan hovers right in front between his dashes).
  It is the hardest hitting of the first eight Standard chips of that
  element the depth rolls that strike outright (the cheapest was
  IceSeed, 10 damage; a Mega chip like DiveMan goes one to a folder) and
  hit the act's guardian for at most a third of its HP, doubled (an
  AquaSwrd, 160 doubled, took 320 of BlastMan's 400 in the first hand),
  and two are in stock: one in a 30-chip folder missed a whole guardian
  fight. Where no chip of the element is that light (act 1's Elec pool
  has Thunder alone), the lightest over it, and only one; the dealer
  says which. `build.py pacing` lists every act's answers per element.
  Reading the act card, buying for it and winning with it is the loop a
  playtester called the best moment of the game.

A start gift adds a choice before the first battle: on layer 1 a Mr. Prog
offers one of three (two HPMemory, a ★3 chip, a NaviCust program). If the
last run ended before the first guardian, he gives an HPMemory first
(`Profile.last_depth`, which took the place of an unused field).

## The economy (issue #154)

What a run finds opens act by act ([Economy](../economy/overview.md), `src/core/economy.c`):
the first act lays half the green Mystery Data, a green data keeps its
chip six times in ten (zenny in its place), the plain finds (green and
blue data, a dealer's shelf, a request) hold a quarter of the good chips'
weight and fewer extra rich data, finds hold half the zenny (a guardian's
chip-entry zenny too) and none leans to the folder's codes; each opens on
a logarithm to what it was by the endless net's sixth act (the short
net's third), never past it. Act 1 gives about 9.5 chips and 2,630 zenny
where it gave 12 and 3,550: at its dealer a run has 930 zenny, the
answer's two copies or the HPMemory. The far detour's rich data, purple
data, Servers' prizes, the gift, the Guardian Data, the dealer's answer
and BN6's own battle rewards are as they were. The traders pick a
prize's code as BN6 does, three in four one the pack lacks.

## Risk the player chooses

- The Server challenge never appears on layer 1 and fights the next act's
  band one version up, at the layer's own depth (not three layers deeper).
  A win pays a chip from the best Mystery Data's roll on top of the game's
  own reward. From act 4 it may hold the area's SP Navi, who keeps his own
  HP: never Bass BX, MegaMan's beast or the place of the Navi the US
  version cut, whose records the Graveyard's and the Nest's tables hold
  beside the story's SP Navis (`navi_challenge`, `super_reserved_ai`): the
  super bosses come at their own places ([Super bosses](../bosses/super-bosses.md)).
- The dark flame into the Undernet stays a side route with tougher viruses
  and better Mystery Data.
- Busting Level stays the game's: a clean, fast battle already earns better
  drops.
- Bass, once the Secret Area has been cleared in any run, waits behind its
  golden gate, and says so before it opens: Bass 1800 HP, Bass SP 2700
  once beaten, Bass BX 3400 once beaten twice with the Cybeast fallen,
  BN6's own records, above every act's band.

## The Cybeast at the bottom of each Net (issue #100)

The endless net's Nest is the Cybeast's den: BN6's own final battle,
Gregar at 2500 HP on the first Net and Gregar SP at 4000 on every Net
after it, where the Nest's SP guardians stood at 1500 to 2000 (SlashMan
SP half the runs, EraseMan SP and ProtoMan SP a quarter each). It stands
above the bands by design, the capstone the Net was built down to, met
with everything the run has gathered: every Cross and GigaChip, five
HPMemory a guardian; BeastOut is its own fall's, for the Nets after
(issue #109), but in a run that brought it. The Net's clock adds its
tenth a notch to it as to any guardian. The short net's Nest keeps its
guardian, paced as a fourth act. `build.py pacing` shows them beside act
6's band.

## Later cycles

After the Nest the band and the damage cap rise by a quarter per cycle and
versions by one, capped at SP; guardians are SP. Two rules change besides
the numbers: rare viruses come 2% more often each cycle, and from the third
cycle the middle layer's heal is no longer certain. The run's first layer
always has a heal too.
