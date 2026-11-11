---
type: Design
title: "Encounter groups"
description: "The encounter rework, approved 11 November 2026: one pool of virus families across every ROM tagged by what they do, authored constellations beside each area's own battles, an arc per act around a signature family, rewards from the group fought; what replaces the visitors and the deep's coin, what was measured, and the phases."
tags: [game, progression, multirom]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-11T17:45:00Z }
sources:
  - id: owner
    resource: "The owner's calls of 11 November 2026, given in the project's sessions and written down here"
    title: "The owner's calls"
    author: human:saschb2b
  - id: measured
    resource: "tools/encounter_tags.py and tools/encounter_danger.py on dev at b04cdad6, BN6 Gregar, BN5 Team Colonel and BN4 Red Sun (USA) read"
    title: "The tags, the fill counts and the danger runs of 11 November 2026"
---

# Encounter groups

A random battle today is one game's own formation, taken at the highest
version that fits the act's band ([The structure](structure.md), Choosing
battles). From act 2 one battle in four swaps one virus for a visitor of
another net, and the deep acts pick whole battles by a coin ([One
net](../../multirom/one-net.md), the visitors' rule; [Battle Network 4 in a
run](../../multirom/bn4.md), Its viruses as visitors). The owner's word for
what the added ROMs bring so far: "sprinkled here and there". With 65
families in the engine and the coming version's goal "everything from
everything", a wider pool should lift the whole run, not decorate it.

This page is the design as approved, what the measuring phase found, and
what the build phase rests on. Nothing here is built: a run's battles are
as they were. The numbers are in [Encounter groups,
measured](encounter-groups-measured.md), from the tools of [Encounter tags
and danger](../../devtools/encounter-tags.md).

## Through the game-design skill

- **The dialectic** is the nets' own, what you bring against what you
  find ([The meta layer](../meta/overview.md)), at the scale of one
  battle: the folder built for the act's guardian against battles that
  ask it something else.
- **The loop layers**: the moment (a battle that is a question, not a
  head count) and the session (an act that teaches one family before its
  guardian). The run's layer gets the rewards. The meta loop gets nothing:
  variety, not power.
- **The patterns**: the hand-made units of a procedural run (a template is
  authored, its filling rolled, as a formation is its game's own record);
  introduce, develop, twist, test (the arc: alone, a pair, a
  constellation, then the guardian); an honest telegraph (the card names
  the family the act is about); one verb deepened (BN6's battle untouched,
  each group a new question about the same folder); defense paid in
  resources (the busting level still picks the reward's quality).
- **The smells it must survive**: complexity over depth (the templates are
  no rule a player learns: one line on a card is the whole surface);
  treadmill (no number is raised: every virus is one of its game's own, at
  a version its game has); a dominant strategy (a battle can be run from
  as now, and a group pays no more than its signature's own row).

Against [the core loop](../core-loop.md): it serves lack 5, variety a
veteran feels, inside battles a run fights already. It adds no battle
(BN6's roll and [the battle floor](structure.md) decide how many, as now),
no walking, and no words beyond one line the act card already has room
for ("Nets meet" goes). Lack 3 is its risk: viruses end half the lost
short nets by attrition, so a constellation may not cost more than the
area's own battles do (Danger, below). And the first act is the game for
half the players: it stays the area's own.

## The design

Approved by the owner on 11 November 2026.

1. **One pool** of virus families across every ROM present, tagged by
   what a virus does: where it stands (`visitor_role`'s front, back or
   free, from its own game's records), its element, and a trick (breaks
   panels, summons, guards, denies area).
2. **Constellations beside each area's own battles**: about ten authored
   templates, each filled from any ROM's families that fit the area and
   the act's band. The area's own battles stay the majority in act 1; the
   mixed share rises by act.
3. **An arc per act**: each act draws one or two signature families by
   seed. Layer 1 meets one alone, layer 2 pairs it, layer 3 puts it in
   its constellation before the guardian.
4. **The Server's marked challenge** becomes the act's hardest
   constellation. No second elite system.
5. **Rewards from the group fought**: the reward comes from the group's
   signature family at the version fought, and the busting level still
   picks its quality.
6. **The act card names the act's signature family** in one line.

### The owner's calls (11 November 2026)

- **Constellations beside native battles**, the mixed share rising by act
  (over replacing the areas' own records, and over a wider visitor rule).
- **The signature family pays**, the busting level picking the quality.
  This revises "BN6's battle drops stay as BN6 has them"
  ([Decided](../economy/decided.md), 4): the rows and the levels stay
  BN6's and each game's own, uncapped; what changes is whose row a group's
  battle reads.
- **The act card names the signature family.**

### What it replaces

- **The one-in-four visitor** (`visitor_gate` and `visitor_apply` in
  `src/core/visitors.c`): a foreign family no longer takes one place in a
  finished formation by chance. It comes as an act's signature or as a
  constellation's partner, in a battle shaped for it. The roles the
  visitors measured (`src/core/visitor_families.c`) become the pool's
  first tag.
- **The deep's coin** (`nets_battle_area`): the Undernet's and the
  Graveyard's acts and the Nest draw from the one pool as every act does,
  their own records and the older nets' late ones the "area's own" there.
- **"Nets meet"** on the act card: the signature's line says more with
  as many words. MegaMan still names a profile's first family of another
  net once (`visitor_taught`).

What stays: the bands, caps and versions of [the structure](structure.md);
the easing rule (a run's first two battles and the first layer after a
guardian from the band's lower half, two viruses at most); the repeat
rule; no two DarkMechs before the Undernet; the Gentle helper's battles
their own net's; a family bound to special panels stays on them; BN6's
battle code and MegaMan's walk.

## What the measuring phase found

Measured on `dev` at `b04cdad6` with all three ROMs read; the tables are in
[Encounter groups, measured](encounter-groups-measured.md).

### The pool

65 families are in the engine: BN6's 29, BN5's 25 and 11 of BN4's (nine
that fight on their own, and Mettaur and Melody moved for a Navi's summon
alone, which stay out). BN4's other families join as they are moved; the
tool counts whatever the engine holds.

| ROM set | Act 1 | Act 2 | Act 3 | Act 4 | Acts 5, 6 and the Nest |
| --- | --- | --- | --- | --- | --- |
| BN6 alone | 11 | 15 | 22 | 22 | 27 |
| BN6 and BN5 | 25 | 31 | 44 | 44 | 51 |
| BN6 and BN4 | 19 | 23 | 31 | 31 | 36 |
| All three | 33 | 39 | 53 | 53 | 60 |

A family counts for an act where one of its versions the act allows (the
older nets' by tier, as the visitors take them) hits under the cap and has
no more HP than the band's top, its stage is reached, and it is not bound
to special panels. A New 3DS reads one extra ROM, so its pools are the
second or third row.

What the tags are worth:

- **The visitors' role is too strict to build on.** It asks three
  placements in four for the back and most for the front, which gives 7
  families of 65 a role (two the front, five the back). A **lean** (two
  placements in five in the first or last column, and no column more)
  gives 5 the front and 22 the back with all ROMs. The build takes the lean for the
  templates; the visitors' role stays where a foe's exact column matters.
- **Seven of BN5's families have no role**: they are in the engine but
  none of the records of the areas BN5 lends a run sets them, so
  `visitor_families.c` never tallies them. The build reads every map of
  BN5's for the roles (its battle tables past the lent maps are not
  mapped yet: a reverse-engineering step, [BN5's battles,
  records and rewards](../../rom/bn5-battles.md)).
- **Tricks show only where the battle shows them.** Read from a battle
  against a standing MegaMan, 4 families change his panels, 4 take or
  reshape the field, one summons, and 16 are tough (the autopilot's
  starting folder does not delete their V1 alone in 50 seconds). Guards
  and area denial that need MegaMan to move do not show this way; the
  build's tag table starts from these and is finished by hand, each
  hand-set tag a claim about its game verified in [the
  sources](../../research/sources.md).
- **The hardest hit misses the danger**, as two DarkMechs taught. A
  standing MegaMan loses 440 HP in 20 seconds to one DarkMech V1 (its hit
  50), 540 to BN5's family 36 (hit 60) and 320 to BN6's family 15 (hit
  10), while six families whose hit is 5 to 100 cost him nothing. The
  **pressure** measured this way is the pool's fifth tag.

### The engine's room

A battle of BN6's spawns four enemies at most (of six named, the first
four: [A battle on demand](../../devtools/battle-on-demand.md)), and that
is the limit for foreign families too, with one exception.

- **Art**: every moved family's sprites, effects and attack types are
  installed at fixed numbers as a core starts ([Transplanted
  families](../../multirom/families.md), the engine's room), so a battle
  loads nothing per family. Three and four different foreign families of
  two older nets were fought in one battle and drawn whole (BN5's CanGard
  with BN4's Billy and Spikey; a BN5 family with Kilby and two more of
  BN4's): captured, the sprites and palettes of each as in its own
  battles.
- **Sounds set the one limit**: BN4's late families share a pool of song
  slots a battle (`xv_bn4_pool`, pointed each frame at the first pool
  owner in play: `src/emu/xnavi_bn4_sounds.c`), so **one pooled BN4 family
  a battle**. Today that is Kilby or MagTect; the six families still to
  move (Spidy, Boomer, Dharma, Gaia, Weather, Moloko) are all pooled. BN5's
  families and BN4's first eight have fixed slots and no limit. The lift:
  BN6's song table is grown to 1024 songs with slots free past the chips'
  bands (`src/emu/song_table.h`), so the pooled families can take fixed
  slots there; until then the templates fill with one of them at most,
  which the fill counts already do.
- **The repeat rule's memory** is a 64-bit mask of families (`Encounter.
  from_families`, `LootMemory`), and BN4's start at 64: it cannot see
  them. It widens with the build, which changes the save beside the run.
- **The New 3DS's heap**: the families' battle path allocates nothing
  (their only allocations are at install), so a battle of three foreign
  families costs the heap what today's costs. Read, not run:
  `tools/heap3ds.py` plays what a run holds, and no run holds a
  constellation yet. The build runs it once they do.

### The templates

Eleven, each a question for the folder. A slot is filled by tag; a group's
versions are the highest that keep its HP inside the act's band and each
hit under the cap, and it must reach the band's floor.

| Template | Slots | What it asks |
| --- | --- | --- |
| Wall and artillery | A front or tough family; a back-row family | Reach the back while the front holds |
| Panel war | A family that changes MegaMan's panels; a roamer | Fight on the ground you are left |
| Summoner and guard | A summoner; a front or tough family | Delete the right one first |
| Clashing answers | Two elements next to each other on the wheel | One answer chip is the other foe's own element |
| One element | Two families of one element | The act's answer chip hits the whole battle double |
| Pincer | A front, a back and a roamer | Three rows of fire at once |
| Swarm | Three of one family | The arc's own shape: alone, a pair, three |
| Held ground | A family that takes or reshapes panels; a back-row family | Take the field back under fire |
| Glass cannons | Two families hitting near the cap, their HP in the band's lower half | Delete before they act |
| Bulwark | One family three fifths of the HP hitting softly; one hitting near the cap | Ignore the wall or go through it |
| Nets meet | A roamer of each of two nets | Two games' habits at once |

How they fill, by ROM set (the counts per act and area are in the
measured tables):

- **All three ROMs**: every template fills from act 3; act 1 lacks glass
  cannons, bulwark and the summoner (the cap of 30 leaves no one near
  it), act 2 the summoner. From act 2 at least 38 of 39 families fill
  three templates or more, so nearly any family can carry an act.
- **BN6 and BN5**: the same, on a smaller pool.
- **BN6 and BN4**: no summoner at all (BN5's family 52 is the pool's one)
  and no panel war before act 5.
- **BN6 alone**: no summoner, no nets meet by its nature, no panel war
  before act 5, and glass cannons and bulwark only in some acts. Six
  templates fill in every act from the second and two more in most, which
  is still a rework for a run without another ROM: BN6's own families in
  groups its tables never set.
- **Act 1 is thin everywhere**: with BN6 alone one family of 11 fills
  three templates. The first act cannot carry the arc on constellations,
  which agrees with keeping it the area's own.
- **An area's own families** stand in most fillings from act 3, but not
  everywhere: several areas of acts 2 and 3 have templates none of their
  own families fills (the zeros in the measured tables). Where the build
  asks for one of the area's own in a group, those areas take the template
  without, or another template.

### The rewards

BN6's reward pick takes one of the enemies a battle spawned and then an
entry of that enemy's row by the busting level ([BN6's
battles](../../rom/battle.md), Battle rewards: verified in Gregar), so
today a mixed battle pays by chance for whichever virus the pick lands
on. The engine already answers that pick by a hook, where it recodes a
row's chips to the folder's codes; handing it the signature's ids alone
makes the group pay as its signature, with nothing of the rows or the
levels changed.

What the rows hold, read from the engine's copy: every family of BN6's 29
and BN5's 25 has chips in its row, and seven of BN4's nine (two pay zenny
and HP until their chips are moved); 173 kinds of chip between them (85,
67 and 21), two of a row's ten entries a chip at a family's top version
in the median. A signature drawn from the whole pool therefore opens rows
an area's own battles never pay from, which is the reward pool the owner
asked for; the chip sessions' work decides how many of the older nets'
kinds play.

### Danger

Measured on five of each act's own battles and two fillings of every
template in acts 2 to 5, every ROM read (116 battles, each fought twice:
by a standing MegaMan for its pressure, and by the test autopilot with
the starting folder and 2000 HP, so none ends the run).

| Act | Own battles: pressure (median) | Templates filled to the band alone | Harder than the hardest own battle sampled | Autopilot loses the act's max HP: own | ... templates |
| --- | --- | --- | --- | --- | --- |
| 2 | 120 | 175 | 8 of 20 | 0 of 5 | 8 of 20 |
| 3 | 120 | 405 | 17 of 22 | 0 of 5 | 14 of 22 |
| 4 | 230 | 540 | 4 of 22 | 0 of 5 | 13 of 22 |
| 5 | 840 | 720 | 3 of 22 | 3 of 5 | 17 of 22 |

- **The HP band and the hit cap do not hold a constellation.** Filled to
  them alone, a group presses three times as hard as an area's own battle
  in act 3 and costs the autopilot the act's whole HP in 14 fights of 22,
  where the area's own cost it none of 5. The reason is in the data: an
  area's own formations pass the band at lower versions and softer
  families, and a filling takes every slot to the highest version that
  fits.
- **Pressure adds up, mostly less than its sum.** A battle's pressure
  against the sum of its families' own (each by its hit at the version
  fought) has a median of 0.62 over 115 battles and stays at or under 1 in
  96: one hit's flinch covers the next. Twelve go past 1.25, the most 2.5
  (two of BN6's family 2 in an act 1 battle of its own) and 2.3 (BN5's
  family 53 with BN4's 64): standing together does add something for a
  few pairs, as it does for two DarkMechs.
- **The Undernet's own battles are the outlier the other way**: their
  median pressure is 840 against a max HP of 500, and the autopilot loses
  that HP in 3 of 5. The statistics' virus deaths spread over every layer
  agree that attrition, not one hit, is what the band misses.

So the build adds a third limit, and `build.py pacing` holds it:

- **A pressure cap per act**: a group's predicted pressure (the sum
  above) is at most the three-quarter mark of the predicted pressure of
  its act's areas' own battles: 220, 360, 620, 720, 880, 1040 by act, 945
  in the Nest. Versions step down to meet it, as they do for the band. No
  number of ours: the area's own battles set what a battle there may
  cost, and a constellation is never the hardest thing an area holds.
  The Server's challenge takes the next act's cap, as it takes its band.
- **It leaves the templates their fillings**: with all ROMs every
  template that filled still fills in acts 1 to 6 (wall and artillery 113
  ways in act 2 where 138 fit the band alone; the measured tables hold
  both); in the Nest glass cannons and bulwark fall out, three fillings
  and one. The cap costs choices, not templates.
- **Pairs that add up to more are measured, not guessed**: before the
  build turns on, `tools/encounter_danger.py` fights every pair of
  families once (about 2,000 battles, an hour of runs side by side) and
  the pairs whose pressure passes 1.25 of their sum go into a table the
  filling reads, each counted at what it measured. Two DarkMechs before
  the Undernet stays the rule it is.
- **The report** rolls 300 groups an act and area for every ROM set and
  prints the predicted pressure beside HP and hit, OVER past the cap, 0
  expected; a unit test holds the cap's rule without a ROM; the measured
  pass stays local, as everything that needs the game does.
- **Pressure is measured at V1 and scaled by the hit.** That a family's
  pressure grows with its hit alone is assumed, not measured: a later
  version may also act faster. The pairs' pass measures the versions the
  acts take for the families it flags.

## The build phase

In three steps, each behind a dev switch until the last, measured before
the next.

1. **The pool and the templates** (`--dev groups`): the tags in the
   engine (the lean, the element, the tricks' table, the pressure, each
   family's stage), the templates and their filling by the act's band, the
   one-pooled-family rule, the repeat rule's wider memory, the pacing
   report's danger band. No player sees it.
2. **The arc, the rewards and the card**: an act's signature by the run's
   seed, the three beats on its layers, the Server's challenge as the
   act's hardest constellation, the reward pick reading the signature's
   row, the card's line through [the voice rules](../../voice/rules.md),
   the statistics' event.
3. **Turned on**: the visitors' gate and the deep's coin removed, the
   words that named them gone, the report's bands at zero, an autopilot
   tally over seeds 1 to 12 against today's.

Where each beat goes (the shares as the owner set them, below):

- **One group battle a layer**, its first random battle that is not one of
  the run's two openers: on an act's first layer the signature alone (one
  virus, or two of it, inside the easing rule as it stands), on the second
  a pair, on the third its constellation. A layer's other battles are the
  area's own.
- **The signature's net by act**: one of the area's own families in three
  runs of four in act 1, in half of act 2, a third of act 3, a quarter of
  act 4; in the deep any family of the pool. So act 1's arc is mostly the
  area's own records chosen to show one family, and the mixed share rises
  as approved.
- **Two signatures from act 3**: the second is the pair's partner and
  stands in the constellation.
- **Everything from seeds**: the signature from the run's seed and the
  act, a layer's group battle from its layer seed and the battle's index,
  as the visitors' gate is, so a checkpoint rebuilds the same.

## The five calls, decided

The measuring phase left five calls to the owner, each with this page's
recommendation. The owner took all five as recommended on 11 November
2026 ("Yes, all five"):

1. **The shares by act** (above): the signature is one of the area's own
   families in three quarters of act 1's runs, half of act 2's, a third of
   act 3's and a quarter of act 4's, the deep free; one group battle a
   layer. The statistics' event then says whether act 1 should open
   further.
2. **A constellation's field**: the area's own records' field where the
   group's panels are plain or cracked there (the visitors' rule), else a
   plain field, so an area's panels stay part of its battles.
3. **The pooled BN4 families' sounds** go onto fixed slots of the grown
   song table before the build, so any two share a battle; the families
   still to move take theirs there too, since each new one is pooled.
4. **BN6 alone** gets the rework with the templates it fills: BN6's own 29
   families in new groups is the same variety, and one in ten release
   runs has no BN5.
5. **The reward where a signature's row holds no chip that plays** (a
   moved family whose chips are not in the engine yet pays the stand-in's
   zenny): the group's best row pays instead, so the rule never costs a
   player a chip.

The build's order, the owner's too: call 3 first, then the build phase's
step 1, each a change of its own; steps 2 and 3 wait for a report after
step 1, since the reward rows are still filling with BN4's chips, and go
through the game-design skill and `build.py economy` first.
