---
type: Design
title: "BN4's Navis as guardians"
description: "Nine of BN4 Red Sun's proved Navis guarding the areas BN4 dresses, as BN5's six guard theirs: where its net's records set each, how often, the version and HP band, the staging in his own sprite and face, and his Guardian Data (his Navi chip, a BN6 kin's while his own sits out; no Soul until the owner's call)."
tags: [game, bosses, bn4, design]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-11T11:06:19Z }
sources:
  - id: records
    resource: "BN4 Red Sun's net battle records, tallied by tools/bn4_lab.py (docs/rom/bn4.md, Battle records): which Navis each net group's lists hold"
    title: "BN4's net records"
  - id: pacing
    resource: "build.py pacing on dev at 3e424c81 with this change: .build/pacing.txt, Guardians of the older net"
    title: "The pacing report"
  - id: owner
    resource: "docs/multirom/bn4.md, The owner's calls 3, 5 and 10"
    title: "The owner's BN4 calls"
    author: human:saschb2b
---

# BN4's Navis as guardians (12 November 2026)

Until now a BN4-dressed act ended at a BN6 guardian, BN4's moved Navis
meeting a player only in a tournament's rounds ([BN4's Navis in BN6's
engine](../../multirom/bn4-navis.md); [The tournament](../tournament.md)).
This brings them to their own areas exactly as [BN5's Navis](bn5-navis.md)
came to BN5's: the same rules, the same staging, the same band. Reasoned with
the game-design skill:

- **The dialectic** is the meta layer's, what you bring against what you
  find, met at an act's end; here it is mostly the variety half of it. The
  core loop's lack 5 names "the same guardian four runs running"
  ([The core loop](../core-loop.md), What it lacks): an area of BN4's now
  ends at a Navi from BN4's net in half the runs.
- **The loop layer** is the progression loop: the act's last fight.
- **The patterns:** parity across the nets (his band, staging and Guardian
  Data BN6's guardians'), the honest telegraph (BN4's tiles, music and
  backdrop already say whose net this is), and meta as variety: nothing he
  gives carries to the next run.
- **What to watch:** his Guardian Data has no power yet (below), so an act
  he guards gives one Cross fewer than a BN6 guardian's would; the owner's
  call on BN4's Souls fills it.

## Where, and how often

BN4's net records place its Navis (the lists of each net group,
`tools/bn4_lab.py`'s tally in [BN4 Red Sun's tables](../../rom/bn4.md),
Battle records). Of the nine moved and proved, each guards the area BN4 sets
him in (`NetAreaDef.xguard`, `src/core/rom.c`):

| BN4's area (it dresses) | Its records hold | Guardian (by a coin of the seed where two) |
| --- | --- | --- |
| ACDC Area (Central Area) | AquaMan, GutsMan, SparkMan | GutsMan or AquaMan |
| Town Area (Seaside Area) | FireMan, NumberMan, Roll, TopMan | FireMan or Roll |
| Park Area (Green Area) | BurnerMan, VideoMan, WoodMan | WoodMan |
| The countries (Sky Area) | ColdMan, KendoMan, ThunderMan, WindMan | ThunderMan or WindMan |
| The Undernet (Undernet) | JunkMan, SearchMan, and others not moved | SearchMan or JunkMan, in the deep's pool |

**How often**, as BN5's: on the guardian layer of a surface act its area
dresses, in half the runs (a coin per area of its own, beside the dress's).
The Undernet is a deep act, whose guardian comes from one pool by band
([BN5's Navis](bn5-navis.md), How often): where BN4's Undernet dresses it,
its two join BN5's six and BN6's own in that pool, each as likely, none
that already guards a surface act of the run. Without BN4's ROM, or on a 3DS
whose heap leaves it unread, BN6's guardian stands; nothing is saved for
him. `--guardian 34` on (34 plus his seat: GutsMan 35) puts one in every act
another game's areas dress. SparkMan, TopMan and BurnerMan, whom BN4's
records also set there, are moved and proved since and fight in the
tournament's rounds ([The tournament](../tournament.md)), but guard no
area yet: an area names two guardians at most (`xguard`). Those not yet
moved (NumberMan, VideoMan, ColdMan, KendoMan, MetalMan, LaserMan) can
join as each is proved.

## His band

BN5's rule ([BN5's Navis](bn5-navis.md), His band): the lowest of V1 to V3
whose HP reaches the act's band, V2 at least from act 4 and from act 2 on
threat 4, SP on later cycles; his HP above the band's top capped as he
spawns. Every placement lies in its band (`build.py pacing`, 0 past their
band):

| Guardian | Acts (band) | Version, BN4's HP, fought at |
| --- | --- | --- |
| GutsMan | 1, 2 | V1 500 at 500; V2 900 at 700 |
| AquaMan | 1, 2 | V1 500 at 500; V2 1000 at 700 |
| FireMan | 1, 2 | V1 500 at 500; V2 1000 at 700 |
| Roll | 1, 2 | V1 700 at 600; V1 700 at 700 |
| WoodMan | 2, 3 | V1 1000 at 700; V1 1000 at 1000 |
| ThunderMan | 3, 4 | V1 800 at 800; V2 1200 at 1200 |
| WindMan | 3, 4 | V1 800 at 800; V2 1200 at 1200 |
| SearchMan | the deep's 5, 6 | V2 1200 at 1200 |
| JunkMan | the deep's 5, 6 | V2 1100 at 1100; V3 1500 at 1500 |

## The fight

Staged as BN5's: the arena, the door's save, his log-in, title card with
BN4's version's mark (V2, V3, SP) and intro in our own words (a first
meeting's "the older Net's Navis" line, BN4 being the same older Net as
BN5, the owner's call 5). He stands in his own BN4 overworld sprite and
speaks with his own BN4 face, copied in from the player's BN4 ROM at his
list-6 number (`BN4_OW_*`, [BN4 Red Sun's tables](../../rom/bn4.md), Sprite
lists), as BN5's are; the battle is BN6's engine's at his moved id and
version, the area's boss theme and BN4's backdrop, his art and sounds loaded
for the battle as a tournament round's are. Lost, the run ends as at any
guardian.

## His Guardian Data

Five HPMemory, a full heal and the NaviCust's draft, as every guardian's,
and his Navi chip: his own from BN4 where the registry's row plays in
BN6's battles (V1 in the first three acts, his SP from the fourth, as
BN5's), else a stand-in, the Navi chip of the BN6 Navi of his element, or
of his kind where he has none, at the version he is fought at (V1, V2 and V3
as EX, SP):

| BN4's | BN6's kin | Why |
| --- | --- | --- |
| FireMan, ThunderMan, AquaMan, WoodMan | HeatMan, ElecMan, SpoutMan, TomahawkMan | the same element |
| WindMan | TenguMan | wind |
| GutsMan | GroundMan | a brawler who breaks the field |
| JunkMan | DustMan | scrap |
| SearchMan | EraseMan | the unseen hunter's aim |
| Roll | CircusMan | a summoner |
| TopMan | ChargeMan | no element: a charge down the row, his top's |
| SparkMan | ElecMan | the same element |
| BurnerMan | HeatMan | the same element |

On 12 November 2026 every one of BN4's Navi chips is still `XREG_PENDING`
(another session moves them, [BN4's chips in a run](../../multirom/bn4-chips.md)):
each comes in its guardian's place by itself once its row plays.

**No power yet.** BN6's guardians give their Cross, BN5's their Soul; BN4's
Souls are being built (ThunSoul first, [BN4's Souls](../../multirom/bn4-souls.md)),
and which a run gives is the owner's call. Until then his Data names no
power and gives no Cross: a BN6 Cross from a BN4 Navi would say a power
came from him that did not.
