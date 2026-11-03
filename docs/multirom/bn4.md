---
type: Design
title: "Battle Network 4 in a run"
description: "What BN4 Red Sun's ROM could lend a run (measured), what players said of BN4 and what follows from it, the design checked against the core loop (BN4's Navis and viruses first, its tournament as an act's climax), and a phased plan with the owner's open calls."
tags: [multirom, bn4, design]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-03T22:20:00Z }
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
| Music | 33 songs | All; its battle and boss themes are what players rate best | low, once a copied song is checked against BN4's own playing (its sound engine is another build) |
| Viruses | 22 families, six versions each | 17: VolGear, Billy, Gaia, Spikey, Shrimpy, Boomer, MagTect, Melody, Dharma, Kilby, Weather, Spidy, CirKill, Moloko, Walla, Elemperor, and Black Earth's sword-only NavBlack and NavShado | a recipe each, as [Transplanted families](families.md) |
| Navis | 25 bosses with Duo and Bass | 18: Red Sun's Soul Navis GutsMan, FireMan, Roll, WindMan, ThunderMan and SearchMan; Blue Moon's AquaMan, WoodMan, MetalMan and JunkMan, whom Red Sun's tournaments fight too; TopMan, BurnerMan, ColdMan, SparkMan, LaserMan, KendoMan, VideoMan; Duo | a recipe each, as [Transplanted Navis](navis.md) |
| Souls | Red Sun's six (Guts, Fire, Roll, Wind, Thunder, Search) | All six; the ROM names Blue Moon's six too, and whether they run in Red Sun is a lead to check in romlab | high, as [Transplanted Souls](souls.md) |
| Chips | 389 records | About 225 names neither game has: some 60 Navi chips of the Navis above (V1, SP, DS), its Program Advances (MagShok, PitHoky, BstFang, FlmCros, PitRng, DarkNeo and more), the Gigas RedSun, HolyDrem, BugCharg and BlakBarr; many others BN4's numbering of chips the run has (Thunder1-3, AirHoc1-3), to be told apart by behaviour | a unit each where BN6 lacks the code; [the chip registry](chip-registry.md) grown past 512 |
| DarkChips | 8 | DrkCanon, DarkBomb, DrkVulcn, DrkSpred and DrkStage by name | a unit each |

What it costs beside BN5: the data travels as BN5's did, but the code is
further from BN6's. A third of bn6f's functions are found in BN5 by their
bytes and an eighth in BN4; of the 69 routines of BN6's that the moved
families and Navis call, BN4 shows 46 by bytes, BN5 59. Each transplant
has more of its engine to pair before its recipe is whole, and its
differential needs BN4's own battle run on the guest core first, its enemy
tables not yet found ([BN4 Red Sun's tables](../rom/bn4.md), Enemy ids table).

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

## The plan

Each phase ships on its own; each transplant proved against BN4's own
battle frame by frame, as BN5's were.

0. **The ROM and the harness.** Found beside BN6's on every build (an
   extra ROM's entry, the ROMs screen, the phones' pickers, the browser,
   the 3DS only where `tools/heap3ds.py` shows a third 8 MB fits);
   `src/emu/bn4.h` and BN4's symbol files; its enemy ids, stats, battle
   records, sprite lists and Navi tables found; the guest core running
   BN4's battles as the differential's reference.
1. **Its viruses.** The 17 families by `tools/xrecipe.py`, BN4's engine
   paired first, each proved by `build.py xdiff`; visitors at the seams
   as BN5's (one battle in four from act 2), so they reach runs before
   BN4 has an area of its own.
2. **Its Navis and the tournament.** The tournament on the Navis already
   moved; BN4's Navis moved, guarding and joining its bracket; Red Sun's
   six Souls (and Blue Moon's, if the ROM runs them); DS chips for dark
   wins.
3. **Its areas and music**, area by area as the walk's gate admits
   them: each dresses the BN6 area it is like, as `run_dress` does for
   BN5's, its themes and battle backgrounds with it.
4. **Its chips**: the registry past 512, those BN6 has the code for
   first, then its Program Advances and DarkChips by BN4's code moved.

## The owner's calls

1. Whether BN4 comes at all beside the core loop's main-path work, and
   if so whether its viruses go first (recommended) or its areas.
2. The tournament as an act's climax (recommended), or not at all.
3. With three games read, how a run shares BN6's areas between BN5's and
   BN4's dressings.
4. The 3DS: BN4 left out where its heap has no room.
