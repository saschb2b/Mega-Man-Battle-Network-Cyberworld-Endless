---
type: Design
title: "Battle Network 4 in a run"
description: "What BN4 Red Sun's ROM could lend a run (measured), what players said of BN4 and what follows from it, the design checked against the core loop (BN4's Navis and viruses first, its tournament as an act's climax), its viruses as visitors in the nets' one-in-four, BN4 whole in players' runs since 5 November 2026 (its viruses, areas and music: the owner's call 10), and a phased plan with the owner's calls (the tournament an act's climax, areas shared by the seed, BN4's net the older Net, its visitors out once six were proved)."
tags: [multirom, bn4, design]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-05T13:55:00Z }
sources:
  - id: bn4-rom
    resource: "Mega Man Battle Network 4: Red Sun (USA), B4WE, SHA-1 a97e96a7da03abd70f7953328e511b9fa29179f1"
    title: "The owner's BN4 Red Sun ROM, read as docs/rom/bn4.md says"
  - id: players
    resource: "Reviews from 2004 to 2024, Steam's Legacy Collection Vol. 2 discussions, community wikis; each in docs/research/sources.md, Battle Network 4"
    title: "What players and critics said of BN4, researched 3 November 2026"
---

# Battle Network 4 in a run

Battle Network 4: Red Sun as a third, optional ROM beside BN6's, lending a
run what its data holds as BN5's does ([Other games' ROMs](overview.md)).
Researched, nothing built: its tables are in [BN4 Red Sun's tables](../rom/bn4.md),
what players said of it in [Sources](../research/sources.md) (Battle Network 4).
Reasoned with the game-design skill and checked against
[the core loop](../game/core-loop.md), as a new axis must be. The calls at the end
are the owner's.

## What its ROM holds

Counted from the ROM; "new" is what neither BN6 Gregar nor BN5 Team
Colonel has, by name, with the community's lists as the check
([BN4 Red Sun's tables](../rom/bn4.md)).

| Pool | In BN4 Red Sun | New to a run | Cost, as [Data, not code](data-not-code.md) counts it |
| --- | --- | --- | --- |
| Net areas | 22 large maps in five groups: ACDC Area 1-3 (lime), Town Area 1-4 (teal), Park Area 1-3 (lavender), Netfrica, Netopia, Sharo and YumLand (one each), the Undernet 1-6 and Black Earth 1-2 (dark green circuits) | All five looks; each map draws whole through the engine's own map reader as it is | moderate, BN5's road |
| Music | 34 songs, 0x00-0x21 | All: its net theme (0x12), Undernet (0x13), battle (0x14), boss (0x15), Duo's (0x16) and the tournament's (0x17); its battle and boss themes are what players rate best | low: ready, every song copied into BN6 and proved against BN4's own playing (Its music, below) |
| Viruses | 22 families, six versions each | 17: VolGear, Billy, Gaia, Spikey, Shrimpy, Boomer, MagTect, Melody, Dharma, Kilby, Weather, Spidy, CirKill, Moloko, Walla, Elemperor, and Black Earth's sword-only NavBlack and NavShado | a recipe each, as [Transplanted families](families.md) |
| Navis | 25 bosses with Duo and Bass | 18: Red Sun's Soul Navis GutsMan, FireMan, Roll, WindMan, ThunderMan and SearchMan; Blue Moon's AquaMan, WoodMan, MetalMan and JunkMan, whom Red Sun's tournaments fight too; TopMan, BurnerMan, ColdMan, SparkMan, LaserMan, KendoMan, VideoMan; Duo | a recipe each, as [Transplanted Navis](navis.md), by a BN4 path of its own: each an object type of its own, the Soul Navis' attacks MegaMan's own states, 64 objects to move ([BN4's Navis in BN6's engine](bn4-navis.md)) |
| Souls | Red Sun's six (Guts, Fire, Roll, Wind, Thunder, Search), by BN5's rule (a chip of the Soul's kind) | All twelve: Red Sun's six, and Blue Moon's six, refused only by their flags, whose forms run in Red Sun's ROM when given one (each seen united and its charged shot fired in romlab) | high, as [Transplanted Souls](souls.md); measured Soul by Soul in [BN4's Souls on BN6's proxy form](bn4-souls.md) |
| Chips | 389 records | About 225 names neither game has: some 60 Navi chips of the Navis above (V1, SP, DS), its Program Advances (MagShok, PitHoky, BstFang, FlmCros, PitRng, DarkNeo and more), the Gigas RedSun, HolyDrem, BugCharg and BlakBarr; many others BN4's numbering of chips the run has (Thunder1-3, AirHoc1-3), to be told apart by behaviour | a unit each where BN6 lacks the code; [the chip registry](chip-registry.md) grown past 512 |
| DarkChips | 8 | DrkCanon, DarkBomb, DrkVulcn, DrkSpred and DrkStage by name | a unit each |

What it costs beside BN5: the data travels as BN5's did, but the code is
further from BN6's, and its viruses are built otherwise. BN5's and BN6's
run every virus as one object type through per-family tables, which a
recipe fills; BN4's runs each family as an object type of its own, its
variables in the object and no AI data, its collision data laid out
otherwise ([BN4 Red Sun's tables](../rom/bn4.md), Virus families). A BN4 family's
recipe needs a path of its own in `tools/xrecipe.py` (its states, its
looks and its numbers read from its type's code and its records), a map
of its collision data and object fields onto BN6's, and the 71 engine
routines its families call that BN6 has no twin of copied or ported
once; the 40 that BN6 has the same are paired as BN5's were. A third of bn6f's functions are found in BN5 by their
bytes and an eighth in BN4; of the 69 routines of BN6's that the moved
families and Navis call, BN4 shows 46 by bytes, BN5 59. Each transplant
has more of its engine to pair before its recipe is whole, and its
differential needs BN4's own battle on the guest core first, which reads
BN4's save block through eToolkit, as BN4 moves it in RAM at each change
of mode ([BN4 Red Sun's tables](../rom/bn4.md), The battle in RAM): about a day or two
for battles that start, end and report, a week more for the differential's
view of its objects.

Three families are whole so far, Billy, Spikey and Shrimpy
(`src/emu/xvirus_bn4.c`, `--dev bn4families`), and each after the first
cost audits more than code. Spikey and Shrimpy took 14 more pairs in
`tools/bn4_twins.py` (six of them the collision routines their attacks
call, read side by side with BN6's), two routines copied with their
tables, and two ways the generator now has for every family: what an
init or update does past BN6's own work runs as a hook (a routine of its
own, or its tail entered through a shim), and an attack BN6 has no object
of moves with its family (its code, its sprite into list 4, its spawn
through the second-level dispatch at numbers apart from BN5's). Three
interims every family met were open then: BN4's effect table not mapped
(its effects drawn as BN6's of the same ids: Shrimpy's splash showed as
BN6's wide sweep), its sounds played as BN6's of the same numbers, and
the can-move check's panel type 0xA with no BN6 type known; the sounds
are BN4's own since (Its sounds, below).

## What players said, and what follows

The signals, strongest first (their sources in [Sources](../research/sources.md),
Battle Network 4):

1. **Three clears to see everything** is the most cited flaw, from 2004's
   reviews to 2023's. A run answers it by its shape: everything BN4
   lends is reachable in one run's draw, behind no clear and no version.
2. **The tournaments**: their fights are the part reviewers liked, the
   same pattern of filler around each round the part they did not. Take
   the fights as a set piece; leave the scenarios, the errands and the
   story.
3. **Random brackets and version-locked Souls** frustrate where chance
   hides what the player wants; players reset at the bracket to steer it.
   The draw is the run's seed and shows before it matters: the act's card
   names the bracket, "???" for a Navi never met, as guardians are. Red
   Sun's ROM fights Blue Moon's Navis as well, so no version waits.
4. **Double Soul** is the best-liked idea, held back by its chip's cost,
   its three turns and weak Souls; BN6's Cross is named as the fix. A
   run already has both: BN5's Souls by BN5's rule beside the Crosses.
   BN4's Souls come by the same rule (UNITE, a chip of its kind, the turns
   its game gives), the cost the bonus with a drawback the run keeps;
   whoever wants none takes a Cross.
5. **Dark Chips** are liked as risk and reward, and missed in BN6, but
   their cost felt hollow. The run's price already bites (20 max HP a
   dark battle, [DarkChips](../game/meta/darkchips.md)). BN4's own twist fits it: a BN4 Navi
   beaten with dark power gives his DS chip in place of his own, as BN4
   does below its karma line.
6. **The story and its filler** are the most disliked part after the
   clears. None of it comes, and its Navis speak as few lines as the
   run's guardians do (the core loop's Words).
7. **Difficulty** split: later playthroughs' bosses too hard, some story
   bosses too easy. The act's band scales BN4's Navis as it does BN5's,
   from BN4's own ladder (GutsMan 500 at V1 to 1700 at SP).
8. **Music** middling, its battle themes the best; it follows the areas.

## The design

- **The experience:** the old guard. A veteran meets Navis and viruses
  last fought in 2004, in one run, without a story played three times.
- **The dialectic** stays the run's: what you bring against what you
  find, the folder planned for the fight that is shown. The tournament
  restates it three times in a row.
- **The loop layers:** the moment (17 families to read in BN6's
  battle), the act (BN4's Navis as guardians and the tournament as a
  climax), the run (Souls and DS chips carried). The meta loop is
  unchanged: variety, not power.
- **The patterns:** hand-made units in a generated run (BN4's own
  formations, taking a visitor as BN5's do), an honest telegraph (the
  bracket shown, first meetings as "???"), a bonus with a drawback
  (Souls, dark power), meta as variety.
- **Against the core loop:** it serves its fifth lack, variety a
  veteran feels, in every battle, and through the tournament its second,
  a climax that bites (93% of guardian battles won, about 80% of HP
  left). It risks the first, pace: BN4's areas bring their walk, so each
  waits for the walk's gate as BN5's areas do (Phase 3 of the review),
  and comes after the viruses.

### The tournament

In some acts the arena holds a bracket of three Navis instead of one
guardian: two rounds below the act's band, the final at it, MegaMan's
HP carried from round to round with the arena's heal before the first
alone. The act's card shows the bracket; the prize is the final's
Guardian Data (his Navi chip, his Soul where Red Sun gives one). It keeps
BN6's battle untouched and makes the act's last test one that can go the
other way, which one guardian at 80% HP left no longer is.

It needs no BN4 Navi to begin: its first bracket can draw from the Navis
the run already moves (BN6's and BN5's), so its structure ships and is
measured before BN4's are moved, and BN4's join its pool one by one.

Its numbers went through the game-design skill on 5 November 2026, in a
concept of their own, [The tournament](../game/tournament.md), nothing
built. In brief, as recommended: the short net's act 3 in half the runs
and one of acts 3 to 5 each endless cycle, once a profile has reached the
Nest; round 1 at about 55% of the act's guardian band and round 2 at
about 75% (in act 3, act 1's band and then about act 2's), each Navi at
his own version's HP; no heal between the rounds; a round lost ends the
run as a guardian does; the rounds paying their results screens and
memory, the final his Guardian Data; the card naming all three, "???" for
a Navi never met; the rounds drawn from every moved Navi of the games
present, each as likely, ProtoMan never; the arena's door the only save.
Projected for act 3: the final begun at about 75% of MegaMan's HP, one
bracket in five lost where one guardian in fourteen is, against a target
of 75 to 90% won. Six calls there are the owner's: how often, from a
profile's first run, a heal between, a lost round, what the rounds pay,
and BN4's own bracket screen.

Not chosen: a tournament at home beside the Virus Battler, which would
repeat its press-on-or-cash-out off the main path and add a walk and
words where the review asks for fewer.

What BN4 itself shows of a tournament (its bracket screen, driven in romlab)
and the two ways to show it in a run: [BN4's tournament screens](bn4-tournament.md).

### Not taken

The three playthroughs and their version locks; the tournaments'
scenarios and the story; BN4's NaviCust rules (the run's NaviCust is
BN6's); its karma, Full Synchro and Dark Soul states (BN6's battle stays
BN6's); Black Earth's gates (five evil chips, then every Navi chip),
gathering gates the meta layer refuses (its maps may still dress the
deep).

## Its viruses as visitors

Phase 1's way into runs, before BN4 has an area of its own: its moved
families visit BN6's and BN5's battles as BN5's visit BN6's
([One net](one-net.md), the visitors' rule). Decided by this session on
4 November 2026, reasoned with the game-design skill: the dialectic stays
what you bring against what you find; it feeds the moment (a family last
fought in 2004, read in BN6's battle) and the session (a layer's battles,
one in four with a visitor); its patterns are the hand-made units of a
procedural run (a battle stays its game's record, a visitor takes one
place in it), an honest telegraph and meta as variety. Against
[the core loop](../game/core-loop.md) it serves the fifth lack, variety a
veteran feels, in battles a run fights already: no walk, no battle and no
line is added, so pace, the first lack, is untouched. Built behind BN4's
research switch (`--dev bn4families`), then turned on for players on 5
November 2026 (Turned on, below).

- **One slot, shared.** The gate stays one random battle in four from act
  2, with the easing rule and the exclusions as they are (no opening
  battle, Server's, duel's or Gentle helper's battle). The visitor of that
  battle is drawn among every fitting family of the two other nets, each
  family as likely as any: BN6's areas meet BN5's and BN4's viruses, BN5's
  areas BN6's and BN4's. Not chosen: a slot of BN4's own, which makes nearly one
  battle in two foreign, so an area's own viruses stop being its identity
  and "Nets meet" stops naming a seam; and a coin between the nets first,
  which with one BN4 family moved puts Billy in one battle in eight from
  act 2, a family repeated where variety was meant. Drawn by family,
  BN4's share grows with its families moved: with Billy alone about one
  visitor in eleven (`build.py pacing` with `--dev bn4families`: 739 of
  8412 rolled), about half with its 17 beside BN5's 18 that may visit.
- **Roles from BN4's own records**, by the rule BN6's and BN5's take (the
  back where three of four placements are in the last column, the front
  where most are in the first enemy column, else either), over its net's
  random records: the groups from 0x80 that have maps, each list of
  viruses once, as its story stages and spare map indices repeat them
  (587 lists). Measured (a scratch tally of BN4 Red Sun's ROM, the same
  count `src/core/visitor_families.c` makes): Weather the front (21, 14,
  6); Billy (28, 32, 17), VolGear, Gaia, Spikey, Shrimpy, Boomer, MagTect,
  Melody, Dharma, Kilby, Spidy, CirKill, Moloko, Walla and Elmperor
  either; BomBoy and Trumpy the back, but BN6 has both by name.
- **Stages** by where its records set a family first: its net areas,
  comps and homepages the surface's (0), the Undernet (group 0x94's maps
  0 to 5) 1, Black Earth (its maps 6 and 7, whose records alone hold
  NavShado and NavBlack) 2. Spidy and Elmperor are the Undernet's, the
  other new families the surface's ([BN4 Red Sun's tables](../rom/bn4.md),
  Battle records).
- **Versions by the act's band**: BN4's six (V1, EX, V2, EX, V3, EX) meet
  the act's cap by BN5's tiers (BN6's V1 allows V1EX, and so on), the
  highest that keeps the battle's HP to the band's top and its hit to the
  cap. BN4's ladder runs higher than BN5's (Billy 50, 120, 160, 200, 250
  and 300 HP), so the band takes lower versions of it.
- **Panels**: BN4's records name a field the project has not read (a
  record's +1, assumed), so no BN4 family counts as bound to special
  panels; a visitor still takes only a plain or cracked panel of the
  battle it visits.
- **Only what is new visits.** A family BN6 or BN5 has by name (Mettaur,
  Trumpy, Lark, BomBoy, WindBox) is not moved; the 17 above are, so every
  BN4 visitor is a virus a run had not met.
- **The deep acts draw as now**: BN6's and BN5's late areas by the coin,
  no visitor, as they mix already and the card says "Both Nets". BN4's
  records would join the deep with its areas; the areas came on 5
  November 2026 without them (call 10), so the deep draws as before.
- **The telegraph**: the act's card says "Nets meet" where either other
  net's families can come, its words unchanged. MegaMan names a profile's
  first visitor once, as before. A BN4 visitor counts as the older Net's
  (the owner's call 5): one "older Net" beside BN6's, BN5's and BN4's
  families both of it, so a BN4 visitor in a battle of BN6's has BN5's
  words, named by its own family ("Lan,that Walla back there...", "It
  came from the older Net!"), and spends the same once (the profile's
  `visitor_taught`): whichever of the two comes first is named, and a
  later visitor of the other is not. A BN4 visitor in a battle of BN5's
  is the older Net's in the older Net: no other net met, so MegaMan says
  nothing and the once is kept for a visitor that crosses. The words for
  BN6's in BN5's ("one of ours ... In the older Net!?") would be false of
  it, and a line naming BN4's as apart from BN5's would split the one
  older Net the call keeps. Built 4 November 2026 (`visitor_telegraph`
  in `src/core/visitors.c`, read by `src/director/nets_words.c`; the
  unit tests' visitors), no line changed; seen in a fresh profile with
  `--dev bn4families,visitors` at depth 5: WallaEX in a battle of BN6's
  named, then PowieEX of BN5's in the next, nothing said.
- **The record**: the run log marks a BN4 visitor as any (`+v64.0`,
  Billy, BN6's AI index 64, at V1). A battle's families (a bit each, the
  repeat rule's memory) stop at 63, and no battle of a run's own holds a
  BN4 family, so its visitor sets none and the rule is as before.
- **What stays as it was**: with BN4 not read, the nets' families, roles
  and draws are byte for byte as before (the same random numbers drawn,
  `LAYER_MAKE` and its hash unchanged). With BN4 read, a visitor's pick
  is drawn among more families, so a visited battle's foes differ from a
  run without BN4 from the first visitor on; layers (their layouts,
  objects and loot) do not change.

Before BN4's visitors reached players (the owner's call 6: once the six
families moved are proved): Billy, Spikey, Shrimpy, Walla, Elmperor and
CirKill proved by `build.py xdiff` and their reward rows read, both
done (call 6's condition met, 4 November 2026: Its viruses' rewards,
below), and a BN4 visitor named as the older Net's (call 5), built.

### Turned on, whole

On 5 November 2026 the owner turned BN4 on for players whole, its
viruses and its areas with their music at once (call 10, below): "just
enable everything. no need to held them back". BN4's ROM is offered and
read as BN5's is: a third cartridge on the ROMs screen (red, Duo's
face), the phones' pickers, the browser, the desktops' dialogs, the
title's and the PET's found lines, and the New 3DS where BN5's is not
read. Its layout's `research` mark is gone ([How it is
built](how-it-is-built.md), 1 and 6), and its five areas' `held` with
it, so they dress BN6's by the seed area by area among the games
present (call 3): `run_dress` gives each other game's area a coin of the
seed, BN5's by the bits it always took, BN4's by a second word's (bits
past 31 of the first are none), BN5's first where both dress one BN6
area. With BN6 and BN4, each of the five BN6 areas BN4's are like is
BN4's in half the runs (x15 ACDC Area to x19 the Undernet, each over
the BN6 area its `like` names; seed 5's first act, BN6's Seaside Area,
is BN4's Town Area); with BN5's too, an area BN5 dresses as well is BN4's in a
quarter. Their themes (BN4's net and Undernet songs)
and battle themes (`BN4_SONG_BATTLE`, `BN4_SONG_BOSS`, named in its
layout now) are copied as BN5's are, their battle backgrounds BN4's own;
their battles, guardians and bystanders stay BN6's (no records of BN4's
in a run yet), a guardian's prelude BN6's. `--net-biome x15`-`x19`, the
atlas's `x1` and `--dev showbn4` work as before. Its six families
install wherever its ROM is read, after BN5's where BN5's is read and
alone where it is not (`src/emu/xvirus.c`'s `xvirus_install`; before,
only beside BN5's and with `--dev bn4families`), and the visitors'
families are made with BN4's alone beside BN6's
(`src/core/visitor_families.c`), so with BN6 and BN4 every visitor is
BN4's. `--dev bn4families` asks nothing of the ROMs now: the
differential still counts its families by it.

Checked that day:

- **BN4 absent**: autopilot runs from seeds 1-4 with BN6 alone and with
  BN6 and BN5 (`--dev god,quiet,visitors`, the layer's next battle asked
  for every 2500 frames, 40000 frames, 12 BN5 visitors among them) the
  same as the build before, run log and last picture; the unit tests,
  `LAYER_MAKE` and its hash unchanged. With BN4 a layer seed makes what
  it made in the BN6 area it dresses, as BN5's areas do, and a layer saved
  in one tiles and continued in another starts again (`run.area`), so
  `LAYER_MAKE` stays.
- **Visitors**: autopilot runs from depth 4, seeds 1-4, BN6 and BN4 and
  all three, a battle asked for every 2500 frames (200000 frames,
  MegaMan kept alive; the visitors' build before the areas came): 281
  battles, every one won, 8 BN4 visitors (Billy 2, Shrimpy 1, Walla 1,
  CirKill 4), 3 of BN5's, none lost or stuck; again with the areas in
  (100000 frames): 200 battles, every one won, 7 BN4 visitors, 3 of
  BN5's, 2 of BN6's in BN5's battles. `build.py xdiff billy_v1` still
  PASSES.
- **Areas**: each of x15-x19 walked by the autopilot in a run (seed 1,
  30000 frames): two layers left each, 2 to 5 battles won; seed 4 with
  BN6 and BN4 fought act 2 in BN4's Park Area, its layers walked and
  left, its guardian (CircusMan) in BN4's battle background. `build.py
  tiles`: 278 layers against the baseline, 0 worse; BN4's tiles drawn
  with other floors than they show, per 100 panels, ACDC Area 5.4, Town
  Area 24.5 (its pale squares' floor in a near shade), Park Area,
  Countries and Undernet 0. `build.py pacing`: 0 battles or guardians
  past their band.
- **Saves**: a run saved at home and at an arena's door after three BN4
  visitors, continued without BN4: the arena's door again, the guardian
  fought, on to layer 12. A run quit in a battle of a BN4 visitor,
  continued without BN4: the layer's checkpoint, battles fought, BN6's
  foes in the visitor's place. A run in BN4's Town Area (seed 5, BN6's
  Seaside Area dressed) quit in a battle, continued without BN4: "From
  the layer's start", in BN6's Seaside Area, battles fought; with BN4
  still there it went on in BN4's Town Area.
- **The New 3DS** (`python3 tools/heap3ds.py --cases bn5,bn4,all --seeds
  1-6`, 120000 frames a run, BN4's families and areas in its runs): BN6
  and BN4, 0 allocations refused in 6 runs, the heap's peak 95.8 MB of
  96, no crash; BN6 and BN5 as before (9 refused in 4 runs, the tiles'
  speed cache); all three read at once 50 refused in 5 runs, the maps'
  mirror images, which is why a 3DS reads one extra ROM, BN5's first
  ([The 3DS's memory](../engine/emulation/3ds-memory.md), A third ROM).

## Its viruses' rewards

A moved BN4 family pays from BN4's own reward rows (`BN4_REWARD_ROWS`,
BN6's format: [BN4 Red Sun's tables](../rom/bn4.md), Its rewards, how
they were found and proved in romlab), read for its six versions' ids as
it installs and written into the rows of its ids in BN6's grown copy, as
BN5's families' are ([Transplanted families](families.md), Its rewards):
zenny and HP as they are, a chip as BN6's own of its name and kind
(standard, Mega or Giga), else 200 zenny (`xchips_reward_bn4`, BN5's
stand-in). Before, their ids read BN6's filler row (100 or 60 zenny,
else Cannon \*).

None of the six families' chips is BN6's (Thunder1-3, HeatShot, Heat-V,
HeatSide, Bubbler, Bub-V, BublSide, TwnFng1-3, ElemFlar, ElemIce,
ElemLeaf, ElemSand, ElemDark, CircGun1-3), so each pays zenny and, at
low HP, HP: Billy V1 30 to 100 zenny and 200 for its Thunder1, its V3EX
30 to 650; CirKill's rows 30 to 700. One reward pick of a visitor at the
economy's busting mix (`econ_bust_weight`, 10% at low HP) paid 2 zenny
and 0.98 chips (Cannon \*) from the fill; it pays 178 (Billy V1) to 438
zenny (CirKill V2 on) and no chip now, 316 the mean of the 36 versions
(a scratch tally of both ROMs by the report's rule). `build.py economy`
does not see visitors (its runs start no core with the families
installed): with `--dev bn4families` its report was the same before and
after, 0 numbers outside their bands.

An HP entry of 0 (`0x8000`) is in 23 of the six families' entries:
ShrimpyV3EX's 7, WallaV3's and WallaV3EX's 8 each, all in a row's half
for MegaMan's HP at 3/8 or less (82 in BN4's whole table, 108 in BN5's,
none in BN6's: a scratch tally of the three ROMs' rows). BN6's own
results screen reads it as **HP MAX**, with its recovery's picture, and
fills MegaMan's HP: WallaV3 alone in BN6's engine, MegaMan at 700 of
2000 (`--dev hp=2000/700`), deleted at Busting 4, reward `8000`, his HP
2000 after (5 November 2026, captured). BN5's visitors pass theirs as
they are (`xchips_reward_bn6` keeps every entry but a chip), so BN4's do
the same, nothing changed. That BN4's own results screen reads it so is
assumed (its format is BN6's), not seen. A full heal at low HP is BN's
own reward, and only V3-tier versions carry it, which the act's band
gives in the late acts.

The design (the game-design skill): the dialectic is the nets' what you
bring against what you find, at the battle's reward in the session loop.
The patterns: parity across the older nets (a BN4 visitor pays by BN5's
visitors' rule), an honest telegraph (the results screen never shows a
chip that cannot play, nor a BN6 chip as a BN4 virus's), and a stand-in
until the content comes (Phase 4 brings BN4's chips). Its cost: where a
row's best entries are chips, the stand-in pays a fast busting less than
the zenny below it (CirKill V3: 600 to 700 zenny for A, 200 for its
CircGun3). The owner kept the stand-in before Phase 4 (call 9, below), over a
chip BN6 lacks paying its row's best zenny or its nearest BN6 chip by
hand.

## The plan

Each phase ships on its own; each transplant proved against BN4's own
battle frame by frame, as BN5's were.

0. **The ROM and the harness.** Done, for research (3 November 2026):
   BN4 an extra ROM's entry read only when a dev tool asks (the atlas's
   `x1` draws its maps), `src/emu/bn4.h` and BN4's symbol files, its
   battle records, encounter check, enemy and chip records, sprite lists,
   battle backgrounds, map routines, music, Souls and the battle in RAM
   found ([BN4 Red Sun's tables](../rom/bn4.md)), and `tools/bn4_lab.py`
   starting any BN4 battle in romlab. Built behind the switch (4
   November 2026): everything a player needs to bring BN4's ROM, on every
   platform, its research mark the one line that offers it
   ([How it is built](how-it-is-built.md), Platforms): its cartridge on
   the ROMs screen (red, Duo's face), the phones' pickers, the browser,
   the desktops' dialogs, the title's and the PET's found lines, and the
   3DS (the mark gone on 5 November 2026: Its viruses as visitors,
   Turned on, whole), whose heap holds one extra ROM beside BN6's (`tools/heap3ds.py`:
   BN4's alone fits, BN5's and BN4's together do not), so BN4 is read
   there where BN5 is not. The guest core runs BN4's battles as the
   differential's reference.
1. **Its viruses.** The 17 families by `tools/xrecipe.py`, BN4's engine
   paired first, each proved by `build.py xdiff`; visitors at the seams
   as BN5's, sharing their one battle in four from act 2 (Its viruses as
   visitors), in players' runs since 5 November 2026 (Turned on, whole),
   with its areas.
2. **Its Navis and the tournament.** The tournament on the Navis already
   moved; BN4's Navis moved, guarding and joining its bracket; BN4's
   Souls, Blue Moon's six with Red Sun's as the ROM runs them all
   (mapped in [BN4's Souls on BN6's proxy form](bn4-souls.md): ThunSoul
   first, then AquaSoul and GutsSoul, the dearest after the BN4 tracks
   they lean on); DS chips for dark wins. The Navis are mapped Navi by Navi, nothing built
   ([BN4's Navis in BN6's engine](bn4-navis.md)): two builds (ten Soul Navis
   on their side's AI data and MegaMan's own states, eight on their own
   states and the object's variables), about 70 KB of code with their 64
   objects, 24 engine routines to port once; GutsMan first, then
   ThunderMan, WindMan, FireMan, SearchMan, Roll and LaserMan, each adding
   one mechanism.
3. **Its areas and music**: each dresses the BN6 area it is like, as
   `run_dress` does for BN5's, its themes and battle backgrounds with it.
   Prepared and held for review (4 November 2026), their 256-colour maps
   shown so, their backdrops and battle backgrounds BN4's own ([Status](status.md),
   BN4 Red Sun's areas); in runs, all five at once, since 5 November 2026
   (call 10, over admitting them area by area after review). Its own
   battle records, guardians and bystanders are still to come.
4. **Its chips**: the registry past 512, those BN6 has the code for
   first, then its Program Advances and DarkChips by BN4's code moved.

## Its music

In runs since 5 November 2026 (call 10), the net and Undernet themes on BN4's areas and its
battle and boss themes in their battles: `src/audio/xsong.c` copies each
of BN4's 34 songs into BN6 as it copies BN5's, BN4's one drum kit for the
keys a song's drum tracks play alone (its keys 0-35 hold other data), 11
to 63 KB a song. Each was proved by `python3 tools/mp2k_verify.py x1/all`
([The engine's sound against the game's](../devtools/sound-check.md)): the copy played by BN6's own sound
engine in romlab beside BN4 playing the song from its title, every part
of the sound engine's state compared frame by frame for 900 frames, the
copy's pointers moved back to BN4's. 31 are the same in every part; two
differ in a counter of a channel already let go, the PSG's registers the
same every frame; the boss theme (0x15) differs in its first 3.3 seconds,
where two square waves swell in from silence in BN4 and stay silent in
BN6, as BN6's newer sound engine lets go a PSG channel the hardware has
stopped ([BN4 Red Sun's tables](../rom/bn4.md), Its sound engine against BN6's). BN4 also mixes its
music at master volume 12 where BN6 mixes at 15: a copy plays 1.8 dB
louder in BN6 than in BN4. Both are for the phase that plays them: the
swell kept or not, BN4's level matched or BN6's kept. `--render-song
x1/0x12:60:PATH` (0x14, 0x15) renders one through the engine's own sound
engine for listening, its envelope following BN4's own recording (r 0.92
to 0.97).

## Its sounds

Heard in runs since 5 November 2026, wherever BN4's ROM is read: the sounds BN4's six
virus families play are BN4's own in BN6's engine. Each sound their code
plays (`BN4_PLAY_SOUND`, r0 a `movs`) that is the same song as BN6's of
its number (`tools/xrecipe.py`'s `song_sig`) stays BN6's; the other 12
are no song of BN6's under any number (Spikey's 0xBC and 0xC4, Shrimpy's
0x9F, 0xC9 and 0xCA, Walla's 0x70, 0x9A, 0xA6 and 0xB7, Elmperor's 0x9D
and 0xFC, CirKill's 0xF9). As a family installs, each of its sounds is
copied from the player's ROM (`xsong_pack`, its sequence, voice group and
samples, none of it in the repository) into a room of the families' art
below their sprites (from `XV_ART + 0x5A000`, the sprites' own room full)
and listed at one of BN6's empty song slots fixed for it whichever family
plays it (`src/audio/xsong.c`'s `XSOUNDS`, `tools/xrecipe_bn4.py`'s
`SOUND_SLOTS`: 0x3F down to 0x34 in install order, the top of the
per-run themes' range, whose 12 themes copied today fit below; the
sounds' own 0x40-0x62 are BN5's, its chips' and Darkloids' still to come
in 0x41-0x4B), on the player BN4's entry names (players 8, 9, 19, 28 and
29: BN6's 32 players answer to BN4's numbers), and the `movs` names the
slot. Played on BN4's player, Elmperor's 0x9D no longer falls on BN6's
player 0, which the Custom gauge's sound holds at t513.

Each copy was proved as the songs were, by `python3 tools/mp2k_verify.py
x1/9D` (and the other 11): the copy played by BN6's own sound engine in
romlab beside BN4 playing its song, both on the music player (whose state
romlab records; the sound's own player is the differential's to check),
every part of the engine's state compared frame by frame for 900 frames:
all 12 the same in every part and their PSG registers the same, each
playing for the same 13 to 61 frames in both. In the differential every
copy is begun on BN4's player at BN4's frame ([The
differential](../devtools/differential.md), The BN4 trace, the families'
copied sounds). One is left short: CirKill's 0xF9 has two tracks and
BN6's player 29 one (BN4's two), so BN6 plays its first track alone (13
frames, its second's 23 not), an interim the recipe names; and the slots BN4's
sounds take from the themes' range are the owner's to confirm.

## The owner's calls

1. **Decided, 4 November 2026: BN4 comes, its viruses first** (chosen over
   its areas first). Phase 1 begins with the guest core for BN4's battles
   and its families moved and proved, visiting BN6's and BN5's battles
   before BN4 has an area of its own.
2. **Decided, 4 November 2026: the tournament as an act's climax** (The
   tournament, above), over leaving it out. Its numbers still go through
   the game-design skill and `build.py pacing` when Phase 2 builds it.
3. **Decided, 4 November 2026: by the seed, area by area.** With three
   games read, the run's seed picks each BN6 area's dressing among the
   games present (BN6's own, BN5's or BN4's), as `run_dress` picks BN5's
   now, so nothing more is saved and a run continued without a ROM goes on
   in BN6's tiles. Chosen over one other game a run and over BN4 only
   where BN5 has none.
4. The 3DS: BN4 left out where its heap has no room (built so: one extra
   ROM a 3DS, BN5's first, `tools/heap3ds.py`'s numbers in
   [The 3DS's memory](../engine/emulation/3ds-memory.md)).
5. **Decided, 4 November 2026: the same "older Net" as BN5's.** MegaMan
   names the older Net once, at a profile's first visitor of BN5's or
   BN4's, and the deep's card stays "Both Nets", BN6's and the older
   one. Chosen over a third Net, older still, and over leaving BN4's
   unnamed until its areas come. Built the same day (Its viruses as
   visitors, the telegraph): a BN4 visitor in BN6's battle has BN5's
   words and spends the same once; one in BN5's says nothing.
6. **Decided, 4 November 2026: once the six moved are proved.** BN4's
   visitors reach players when Billy, Spikey, Shrimpy, Walla, Elmperor
   and CirKill all pass `build.py xdiff`, their reward rows read; the
   other eleven join as each is proved. Chosen over waiting for all 17
   and over Billy alone (one visitor in eleven, the same family each
   time). Its condition met the same day: all six PASS and pay from
   BN4's own rows (Its viruses' rewards); the visitors still wait on
   call 5's naming. Turned on for players 5 November 2026, call 5
   built (Its viruses as visitors, Turned on, whole).
7. **Decided, 4 November 2026, then revised the same day: the comparison
   is aligned, the game unchanged.** First decided as "BN4's moved viruses
   enter with BN4's own appear", on the lead's claim that the two games'
   appears differ. Measured before it was built, the claim was wrong: BN4's
   appear (`0x0800FE00`, from its generic state 0 `0x0800FCAC`) runs BN6's
   twin's steps (only its sound and a battle-kind check differ), and the
   one sprite update Shrimpy V3 parts by comes from the battle's opening,
   whose appear bit BN4 sets a frame later after the virus's init than BN6
   does ([BN4 Red Sun's tables](../rom/bn4.md), the appear). Revised: the
   BN4 specs run BN6's opening a frame longer, as they run BN6's battle a
   frame behind, the engines' difference named in the comparison; a BN4
   visitor keeps BN6's appear and opening in play. Chosen over a game shim
   that delays BN4 visitors a frame and over porting BN4's appear anyway.
   Built the same day: the harness writes BN6's first blink step a frame
   longer into every BN4 spec against BN6's (`tests/xdiff/legit.json`'s
   `bn4` `align`), every BN4 spec PASSES and the opening's allowance is
   gone ([the differential](../devtools/differential.md), The BN4 trace).
8. **Decided, 4 November 2026: BN4's sounds keep slots 0x3F-0x34**
   (Its sounds, above), the top of the per-run themes' range, over taking
   BN5's reserved 0x41-0x4B first.
9. **Decided, 5 November 2026: a chip BN6 lacks pays 200 zenny**, BN5's
   visitors' rule (Its viruses' rewards, above), until Phase 4 brings
   BN4's chips; over a missing chip paying its row's best zenny and over
   a BN6 chip mapped to it by hand.
10. **Decided, 5 November 2026: enable everything.** "just enable
   everything. no need to held them back": BN4 comes into players' runs
   whole at once, its ROM offered, its six families visiting and its five
   areas dressing BN6's by the seed with their themes and battle
   backgrounds (Its viruses as visitors, Turned on, whole). Chosen over
   its visitors alone with the areas held for Phase 3's review, area by
   area as the walk's gate admits them. What the areas still lack (BN4's
   own battle records, guardians and bystanders, the boss theme's swell
   and BN4's mixing level, Its music) comes as before, phase by phase.
11. **Decided, 5 November 2026: BN4's Navis' sounds by battle.** Their
   sounds BN6 has no song of take a small shared pool of BN6's empty song
   slots, copied from the player's ROM (xsong's copy) for the Navi a
   battle holds; the families keep their fixed slots 0x34-0x3F. One
   battle holds one BN4 Navi (a tournament's three rounds are three
   battles, as a run's guardians are), so the pool is sized by the most
   sounds one Navi plays: Duo's 10 (measured by the generator's walks of
   all eighteen, approximate while their recipes are open). The pool is
   BN6's 16 empty slots no other track takes, without BN5's reserved
   0x41-0x4B ([BN4's Navis in BN6's engine](bn4-navis.md), GutsMan
   built). Of them only five are at or below 0xFF (0x32, 0x33, 0x63,
   0xD0, 0xE0), which a sound its code names by one `movs` or a byte
   needs, so the themes' top two stay the pool's: TopMan and VideoMan
   name six sounds so, one more than the pool holds. **The owner's call
   of 5 November 2026 for those:** a Navi's sound named by a single
   `movs` or a table's byte whose pool slot is past 0xFF is routed
   through a small shim that plays its 16-bit number, so every sound
   plays right and no theme slot is lost; built in the recipe, for any
   BN4 Navi. Built for a `movs` (`XvSoundCall`, the generator's
   wide_sounds), proved on GutsMan's pool given in another order; a
   table's byte past 0xFF stays a decision of the generator's until a
   Navi needs it ([BN4's Navis in BN6's engine](bn4-navis.md), GutsMan
   built, his sounds).
