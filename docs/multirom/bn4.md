---
type: Design
title: "Battle Network 4 in a run"
description: "What BN4 Red Sun's ROM could lend a run (measured), what players said of BN4 and what follows from it, the design checked against the core loop (BN4's Navis and viruses first, its tournament as an act's climax), its viruses as visitors in the nets' one-in-four, and a phased plan with the owner's open calls."
tags: [multirom, bn4, design]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-04T01:55:00Z }
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
interims every family meets stay open: BN4's effect table is not mapped
(its effects draw as BN6's of the same ids: Shrimpy's splash shows as
BN6's wide sweep), its sounds play as BN6's of the same numbers (BN4's
have no slots among BN6's empty songs yet), and the can-move check's
panel type 0xA has no BN6 type known.

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
other way, which one guardian at 80% HP left no longer is. Its numbers
(how often, the rounds' bands, a heal between) go through the game-design
skill and `build.py pacing` when it is built.

It needs no BN4 Navi to begin: its first bracket can draw from the Navis
the run already moves (BN6's and BN5's), so its structure ships and is
measured before BN4's are moved, and BN4's join its pool one by one.

Not chosen: a tournament at home beside the Virus Battler, which would
repeat its press-on-or-cash-out off the main path and add a walk and
words where the review asks for fewer.

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
research switch (`--dev bn4families`): no player's run meets one until the
owner turns BN4 on.

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
  records join the deep with its areas (Phase 3), under the owner's third
  call.
- **The telegraph**: the act's card says "Nets meet" where either other
  net's families can come, its words unchanged. MegaMan names a profile's
  first visitor once, as now, BN5's or BN6's; a BN4 visitor is not named
  and does not spend that once, until the owner names BN4's net in the
  run's fiction (a call below): BN5's is "the older Net", the copy the
  Nest made, and BN4's is older still.
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

Before BN4's visitors reach players: its families proved by `build.py
xdiff`, their reward rows read (BN6's fill stands in for them now), and
the owner's calls 5 and 6 below.

## The plan

Each phase ships on its own; each transplant proved against BN4's own
battle frame by frame, as BN5's were.

0. **The ROM and the harness.** Done, for research (3 November 2026):
   BN4 an extra ROM's entry read only when a dev tool asks (the atlas's
   `x1` draws its maps), `src/emu/bn4.h` and BN4's symbol files, its
   battle records, encounter check, enemy and chip records, sprite lists,
   battle backgrounds, map routines, music, Souls and the battle in RAM
   found ([BN4 Red Sun's tables](../rom/bn4.md)), and `tools/bn4_lab.py`
   starting any BN4 battle in romlab. Moved to Phase 1, where BN4 first
   lends a run something: the ROMs screen, the phones' pickers, the
   browser and the 3DS's heap (`tools/heap3ds.py`), and the guest core
   running BN4's battles as the differential's reference.
1. **Its viruses.** The 17 families by `tools/xrecipe.py`, BN4's engine
   paired first, each proved by `build.py xdiff`; visitors at the seams
   as BN5's, sharing their one battle in four from act 2 (Its viruses as
   visitors, built behind the research switch), so they reach runs before
   BN4 has an area of its own.
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
3. **Its areas and music**, area by area as the walk's gate admits
   them: each dresses the BN6 area it is like, as `run_dress` does for
   BN5's, its themes and battle backgrounds with it.
4. **Its chips**: the registry past 512, those BN6 has the code for
   first, then its Program Advances and DarkChips by BN4's code moved.

## Its music

Ready ahead of Phase 3, shown to no one: `src/audio/xsong.c` copies each
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

## The owner's calls

1. **Decided, 4 November 2026: BN4 comes, its viruses first** (chosen over
   its areas first). Phase 1 begins with the guest core for BN4's battles
   and its families moved and proved, visiting BN6's and BN5's battles
   before BN4 has an area of its own.
2. The tournament as an act's climax (recommended), or not at all.
3. With three games read, how a run shares BN6's areas between BN5's and
   BN4's dressings.
4. The 3DS: BN4 left out where its heap has no room.
5. What MegaMan calls BN4's net when its first visitor is met: one more
   "older Net" beside BN5's, or a third, older still; and whether the
   deep's "Both Nets" counts three once BN4's areas join it.
6. When BN4's visitors reach players: with the first of its families
   proved, or once all 17 are.
