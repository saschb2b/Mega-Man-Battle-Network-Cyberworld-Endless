---
type: Design
title: "Encounter groups"
description: "The encounter rework, approved 11 November 2026: one pool of virus families across every ROM tagged by what they do, authored constellations beside each area's own battles, an arc per act around a signature family, rewards from the group fought; what replaces the visitors and the deep's coin, what was measured, and the phases."
tags: [game, progression, multirom]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-12T23:00:00Z }
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
- **Sounds set the one limit** (lifted on 11 November 2026 by the third
  of the five calls: the late families' sounds now have fixed slots,
  [Transplanted families](../../multirom/families.md), BN4's late
  families, so any two share a battle; what follows is what was
  measured): BN4's late families share a pool of song
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
  version may also act faster. The pairs' pass was fought at V1 too.

## The build phase

In three steps, each behind a dev switch until the last, measured before
the next.

1. **The pool and the templates** (`--dev groups`): the tags in the
   engine (the lean, the element, the tricks' table, the pressure, each
   family's stage), the templates and their filling by the act's band,
   the repeat rule's wider memory, the pacing report's danger band. No
   player sees it. **Built** (below).
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

### Step 1, built

The pool, the templates and their filling are in the engine since 11
November 2026, behind a dev switch: no run of a player's holds a group.

- **The module** is `src/core/groups.c`, numbers alone and no ROM: a
  family's lean from its placements, the eleven templates (what a family
  must be to take each slot, what the group must be at its versions), the
  filling under three limits (the act's band, its cap on one hit, the
  pressure cap), a pick by the layer's seed and the battle's index with
  no random number drawn, and the group on the enemy's panels.
  `src/core/group_pool.c` reads the pool from the ROMs (each family's
  versions, where its own game sets it, its stage) and the measured
  tags, `src/core/group_tags.c`, which `tools/encounter_tags.py engine`
  writes from the measuring's files
  ([Encounter tags and danger](../../devtools/encounter-tags.md)).
- **It reproduces the measuring**: `build.py pacing` counts how many ways
  each template fills in every act for each ROM set, and all 308 counts
  (four sets, eleven templates, seven acts) equal the tagging tool's
  ([measured](encounter-groups-measured.md), A cap on pressure). The
  pool by act is the measured one: 33, 39, 53, 53 and 60 families with
  every ROM, 11, 15, 22, 22 and 27 with BN6 alone.
- **The third limit holds**: the pressure cap by act is 220, 360, 620,
  720, 880 and 1040, the Nest 945 (`group_pressure_cap`). The report
  rolls 300 group battles for each layer of each act's areas, 30,000 in
  all, and reads each one's HP, hit and pressure again from the ROM's
  records: none past a limit, none on a shared or special panel. On a
  later cycle the cap grows as the band's cap on one hit does; that is
  assumed from how the prediction is made, not measured.
- **A constellation's field** follows the owner's second call: one of the
  area's own battles' fields on which every panel the group stands on is
  plain or cracked, else the plain field. Two group battles in three
  stood on a field of their area's (four in five in the median area and
  layer; since the fields' pass a field must be fair too, and it is 63%,
  three in four in the median: Easing act 1's pair); the areas whose own fields are special panels where enemies
  stand give the fewest (the Robot Control Comp none, the Park Area one
  in twenty). An older net's area takes the plain field until step 2
  reads its records' fields.
- **The repeat rule sees BN4's families**: a battle's families are a mask
  of 128 bits (`FamilyMask`, `src/core/foes.h`) where 64 held BN6's and
  BN5's alone, and a group battle is kept in mind as an area's own is.
  The memory is its own file beside the run (`battle.sav`), so its
  version changed and the run's did not: an older file is forgotten as a
  first battle's is, and no save is lost.
- **BN6 alone** has its pool: its families' places are tallied without
  another ROM (`src/core/visitor_families.c`), the fourth call.
- **To see one**: `--dev groups` makes every plain random battle a
  group's, `--dev group=N` one of template N (1 the wall and its
  artillery to 11 the nets meeting;
  [Developer tools in the game](../../devtools/dev-menu.md)).

Left of step 1, each measured before step 2 rests on it:

- **The pairs' pass, done** (`tools/encounter_danger.py pairs`, 11
  November 2026): every two families of the pool fought once at V1,
  1,830 pairs ([measured](encounter-groups-measured.md), The pairs'
  pass). Pressure adds up to less than its parts: the median pair presses
  0.72 of the sum of its two families' own. 28 pairs press past 1.25 of
  it, most where the sum is small, and the most any pair presses past its
  sum is 120 HP in 20 seconds. The 27 pairs 40 HP or more past their sum
  are in the tags (`group_pairs`, `src/core/group_tags.c`): a group that
  holds one is predicted that much harder, by the larger of the two
  families' hits at the version fought over their V1's, in the engine
  and in the tool alike. That holds back 165 of 54,617 fillings with
  every ROM, and the caps stay as they are (the acts' own battles'
  three-quarter marks did not move). One family, BN6's 21, is in 11 of
  the 27: alone it presses a standing MegaMan little, beside another it
  does. The pairs were fought at V1; that a pair's excess grows with its
  hit as a family's own pressure does is assumed.
- **The limits are an act's, not a layer's.** An autopilot run with every
  plain battle a group's (`--dev groups`, seed 4) won a swarm on its
  first layer and was deleted from full HP by the next group: BN5's 43
  beside BN4's 64 at V1, 140 HP together, inside the band, the hit cap
  and the pressure cap of act 1. A cap of 220 HP in 20 seconds is the
  harder quarter of act 1's own battles, against a MegaMan of 100 HP, and
  the run's first layers ease their own battles to the band's lower half.
  So step 2 keeps what the arc already asks: on an act's first layer the
  signature alone under the easing rule, a pair on the second, the
  constellation on the third; and act 1's constellation may want the
  median of its own battles' predicted pressure (160) for its cap, which is the
  owner's with the shares.
- **Seven of BN5's families roam from the first act** with no role: no
  record of the areas BN5 lends sets them, and BN5's other maps' battle
  tables are not read yet. The band still holds their HP and hit.
- **Each template is as likely as any that fills.** In act 1 one template
  fills one way, so a share by fillings, or by the arc, is step 2's to
  set with the shares by act.

### Step 2, the arc and the rewards, built

In the engine since 12 November 2026, behind `--dev arc`: no run of a
player's has an arc.

- **The arc** (`src/core/group_arc.c`, no ROM; its rules by act in
  `src/core/group_pool.c`): an act's signature by the run's seed and the
  act alone, so every layer of the act and a CONTINUE have the same. A
  family can be a signature where it stands alone inside the eased
  limits and some template pairs it. The signature is one of the area's
  own in the share the owner set: of 200 runs' arcs an act's area, 75%
  in act 1, 51% in act 2, 34% in act 3, 25% in act 4; the deep takes any
  family. Every act of every area has an arc with every ROM read, from
  13 families in act 1 to 58 in the deep.
- **The arc is the act's, whichever layer asks** (12 November 2026,
  found by the trailer's session with `--run-depth`). Two things it was
  made from were a layer's, not the act's, in an older net's area:
  - Its rules were read at the layer that asked first, and the area's
    own families there are its records' of that layer
    (`xterritory_families`). Played through, the arc was made on the
    first layer and kept; a game started on a later layer (a CONTINUE)
    made it anew. Seed 5's act 2 in BN5's Oran Area says Spikey on all
    three layers played through, and its save continued on the third
    said Armadill. Now the rules are the act's first layer's
    (`group_arc_depth`).
  - A dark layer wears a dark area, which is no territory, so on an
    act's dark layer the area's own were the BN6 area's and the arc was
    made anew for that one layer, in one session too (seed 17's act 2:
    family 32 on its first and third layer, Armadill on its dark
    second). Now the arc reads the area the act wears
    (`loot_arc_territory`, set by the director on a dark layer).

  Of 250 acts (seeds 1-50, acts 2 to 6, every ROM read, each layer
  started on its own) 17 named another family on a later layer, all in
  BN5's areas; none does now. `build.py pacing` asks every act's arc
  first on its second and third layer, in the BN6 area and in each
  older net's area that dresses it, and fails where one differs from
  its first layer's; the unit tests hold `group_arc_depth` to the act's
  first layer.
- **The three beats**: on the act's first layer the signature alone, two
  of it or one, whichever holds more HP inside the eased limits (the
  band's lower half, two thirds of the pressure cap, about the median of
  the act's own battles); on the second a pair, one of the two-family
  fillings that hold it; on the third its constellation, three families
  (with the partner too from act 3), else three of it, else the hardest
  pair with another partner. The Nest's one layer is its constellation.
- **Which battle**: a layer's first random battle; on the run's first
  layer its second, after the run's very first battle. The proposal had
  the first layer's beat after both openers; players fight 1.3 battles a
  layer, so a third battle is seldom met, and the beat alone is an eased
  battle as the second opener is. Never with the Gentle helper, on a side
  layer or a Glitch layer.
- **Act 1 holds every beat to the eased limits**: the band's lower half
  and two thirds of the caps, as its first beat (Easing act 1's pair,
  below).
- **The Server's challenge** is the filling of most predicted pressure
  that holds the signature, inside a challenge's limits (the next act's
  band, its versions, the cap grown as the band's cap on one hit is).
  Every arc has one. Where the area's own challenge rolls a Navi, the
  Navi stays: the Server's rare Navi is no constellation's to take.
- **The rewards** (`src/director/encounter.c`, `loot_paying_id`): the
  reward pick is handed the signature's enemy alone, at the version
  fought, so a beat pays from its row and the busting level picks the
  entry as ever; nothing of the battle's state is written. A signature
  whose row holds no chip leaves the pay to the group's row with the most
  (the fifth call). Played on three seeds: in four of six group battles
  the reward was in the signature's row and not the partner's, in two in
  both.
- **The act card names the signature** (`src/director/story_words.c`):
  "Act 2 - BombCorn signal", in the place "Nets meet" has, a label as
  "Guardian:" is one, the family's name the ROM's own. A signal is what
  MegaMan senses of what waits ("a strong virus signal" is the Server's
  word). It names the family whether or not MegaMan has met it, where the
  guardian's line keeps "???" until he has: the owner's to confirm
  ([What MegaMan knows](../meta/what-megaman-knows.md)). With the arc on
  it also stands where the deep's "Both Nets" and a territory's "Older
  Net" do.
- **The statistics' event** (`act-battles`, `src/analytics/analytics.c`;
  counted in `src/director/runlog.c`, kept with the run's other counts
  in `run.tally`): as an act ends, its random battles on the main path,
  the area's own and the groups' apart: how many were fought, won and
  run from, the HP each cost on average in percent of MegaMan's max, the
  mean busting level of those won; and of the act's arc its signature
  family's number, whether it is one of the area's own, the beats fought
  and the templates. It is sent today too, with the area's own battles
  alone: the measure the groups are held against once they are on
  ([The player's guide](../../GUIDE.md#anonymous-statistics) lists it).
  Checked against a local stand-in for the statistics' server: an act
  played with `--dev arc` sent 2 own battles and 2 group battles, the
  signature and its two beats.
- **What the rows pay** (`build.py pacing`, the acts' arcs: each beat's
  paying row against the rows of the area's own battles, their healthy
  halves): about as many chip entries (3.8 of ten against 3.2 in act 1,
  3.1 against 2.6 in act 4), a fifth to a quarter less zenny a row (0.72
  of the area's own in act 1, 0.76 to 0.88 after), and many more kinds of
  chip: 13 against 3 in act 1, 32 against 4 in act 2, 80 against 12 in
  act 3, about 120 against 26 to 32 in the deep. The variety is the
  point; the zenny is what `build.py economy` must weigh before the arc
  is turned on, since a layer's one group battle is most of the 1.3 a
  layer that players fight.
- **Checked**: `build.py pacing` rolls 200 runs' arcs for every act's
  area and reads each beat again from the ROM's records: none past its
  limits, a signature in each. A unit test holds the arc's rules on a
  made-up pool.

### Easing act 1's pair

Before the arc is turned on. The test autopilot on seeds 1 to 12, fresh
data folders, 45,000 frames a run, deleted in act 1 by viruses:

| The arc | Act 1 deletions | Of them a group's battle | Random battles won | Layers reached, all runs |
| --- | ---: | ---: | --- | ---: |
| Off | 2 | - | 38 of 42 | 36 |
| On, as step 2 built it | 4 | 3 | 33 of 37 | 32 |
| On, the pair in the band's lower half | 2 | 0 | 44 of 47 | 38 |
| On, the pair's partner one of the area's own | 4 | 3 | 35 of 39 | 33 |
| On, a cap on what a group costs | 2 | 0 | 38 of 41 | 35 |
| On, the lower half and the cap on cost | 3 | 1 | 41 of 45 | 35 |
| On, those and fair fields (the rule) | 2 | 0 | 40 of 43 | 35 |

A partner of the area's own changes nothing: the pairs that deleted the
autopilot were inside every limit whoever the partner was. The band's
lower half took them out by their HP (130 and 160 against 125). What the
tally found on the way is what the pressure cap does not see, and it is
two things.

- **The fight.** Pressure is what a standing MegaMan loses in 20 seconds.
  It does not see how long a family takes to delete, nor what lands on a
  MegaMan who lines up to shoot. The two partners that deleted the
  autopilot press a standing MegaMan for 60 and 90, and each costs the
  autopilot 50 HP in 42 seconds fought alone; one family of act 1's pool
  presses for nothing and costs 215. So a group has a second cap, on its
  **cost**: each family's HP lost by the test autopilot deleting its V1
  alone with the starting folder (measured with the tags, 50 seconds at
  most), by its hit at the version fought, added up; held to the
  three-quarter mark of what the act's areas' own battles are predicted
  to cost, as the pressure cap is: 0, 60, 145, 140, 180 and 678, the
  Nest 447 when it was set (`group_cost_cap`;
  [measured](encounter-groups-measured.md), A cap on pressure; 20, 60,
  175 since several of a kind are measured, Step 3). Act 1's was
  nothing: its own battles cost the autopilot nothing alone, so its
  groups were of families that do not either.
- **The field.** With both caps one run was still lost to a group: BN6's
  family 1 beside BN5's 51, 110 HP, which costs the autopilot 0 to 20 HP
  on the plain field in six fights. On one of that area's own fields the
  same pair cost it 80 to 120 HP in four fights, and on three others it
  could not be deleted in 50 seconds. The owner's second call checked a
  field under the group's own panels; what a field does to MegaMan's
  side (holes, poison, ice, panels he cannot stand on) no measure of the
  families sees, and an area's records are authored for their field
  where a group is not. So the fields are measured: that pair, harmless
  on the plain field, fought four times on each of the 120 fields the
  areas' random battles use. 100 are fair (every fight won, 20 HP lost
  at most, as on the plain field), and a group takes an area's field
  only where it is fair (`group_field_fair`; measured, The fields). The
  second call stands with that added.

The rule, then: act 1's pair and constellation in the band's lower half
with the caps' two thirds, as its first beat; the cap on cost beside the
pressure cap in every act; an area's field only where it is fair.

What it costs is variety in act 1: with every ROM four families can be
its signature (BN6's 1, BN5's 32, 34 and 51) where thirteen could, and
its beats are eight battles in all. With BN6 alone act 1 has no arc: no
two of its families pair inside the limits, and the act stays the
area's own. From act 2 the signatures are 21, then 39 to 55.

The area's own battles still delete the weak autopilot in act 1, with
the arc off and on alike (two runs in twelve): that is the game as it
was, and not this rule's to mend.

### Step 3, turned on

On for players since 12 November 2026, the owner's go ("Yes, ease then
on"). Every act of the main path has its arc; `--dev noarc` plays a run
without them.

- **What went**: the one-in-four visitor (`visitor_gate`,
  `visitor_apply`) and the deep's coin between the nets
  (`nets_battle_area`). A layer's own battles are its area's, an older
  net's area's its own records, in the deep too; the older nets' families
  come as signatures and partners. "Nets meet" and "Both Nets" left the
  card for the signature's line; an act without an arc in an older net's
  area still says "Older Net". MegaMan names the first family of another
  net a profile meets in a group's battle, once, in the words he had for
  a visitor. A Glitch layer's battles from its second stage are groups
  of any template, where each took a visitor. A run with the older nets
  switched off draws its arcs from BN6's families alone.
- **The first beat is one virus.** Step 2 let it be two of the family.
  On act 2's first layer two of BN5's family 34, a panel breaker, at
  V1EX deleted the autopilot from full HP: one of them costs it nothing
  in six fights, two cost 40 to 280. One alone is the battle the tags
  measured, and it is what the design says.
- **Several of a kind are measured** (the multiples' pass, 366 battles:
  two and three of each family at V1, standing and fought): together
  they press less than one times their number (the median 120 less) and
  cost more, by 40 HP or more in 57 of 122 cases, 290 at most. A swarm's
  pressure and cost are the measured ones (`GroupMulti`), and so are the
  acts' own battles' that hold two of a kind, which moves the marks the
  caps are: pressure 180, 320, 530, 620, 880, 1040, the Nest 935 (220,
  360, 620, 720, 880, 1040, 945 when one's numbers were added up); cost
  20, 60, 175, 140, 180, 678, 447. The caps are written with the tags
  now (`group_caps`, `src/core/group_tags.c`), so the rule and its
  numbers cannot part. That a later version's cost grows with its hit
  alone stays an assumption, and the breakers above show its limit.
- **The Net Dealer's answer counts the signature**: where the guardian
  is weak to nothing, the signature's weakness is one more kind among
  the area's viruses' (`counter_element`).
- **Layers**: `LAYER_MAKE` goes up by its file
  (`src/net/layer_make.d/`): a layer's Server's challenge and its
  dealer's answer are made otherwise, so a run saved by an older build
  continues its layer afresh. `Run` is unchanged: the arc is the seed's.
- **The economy** (`build.py economy`, the arcs off against on, 300
  runs): an act's battles pay 8% less to 16% more zenny (act 2 +16%,
  act 3 +12%, act 4 -8%) and as many chips to 0.3; an act's whole zenny
  moves 5% at most (act 4 -4.9%, the Nest -4.0%), its chips 0.3 at most,
  its good chips' share 3 points at most. Nothing left its band; the
  centres were set to the report with the arcs on
  ([The guard](../economy/guard.md), Re-centring, which says how a chip
  moved into a reward row sets them again).
- **The tally** (seeds 1 to 12, 45,000 frames): before, with the
  visitors, 4 runs lost to viruses, 38 of 42 random battles won, 36
  layers reached; now 1 run lost to viruses (an area's own battle), 55
  of 56 won, 37 layers reached. No group's battle deleted the autopilot.
- **Checked**: `build.py pacing` 0 past their band, every act's area
  with its arc, the engine's fill counts equal to the tagging tool's in
  308 of 308 cells; the 3DS's heap modelled over long runs with the arcs
  on: 64.4 MB at most with BN6 alone, 85.9 of 96 with BN5's and BN4's ROMs beside it (85.8 before the arcs), nothing refused, no crash, seeds 1 to 6.
- **The pairs' cost, measured since** (the pairs' cost pass, 12 November
  2026: all 1,830 pairs fought once by the test autopilot): the median
  pair costs what its two families' own costs add up to, but 646 cost 40
  HP or more past that, 290 cost 100 or more, 53 cost 200 or more, 400
  at most. Cost adds up to more than its parts far more often than
  pressure does (27 pairs of 1,830). A fight's cost swings with its seed
  and each pair was fought once, so a pair is tagged from 100 HP past
  its sum: 290 pairs (`group_pairs`' cost), added to a group's predicted
  cost by the larger of its two families' hits over their V1's. The
  acts' own battles hold such pairs too, so the cost caps moved with
  them: 20, 70, 200, 160, 270, 888, the Nest 508. Every act's area keeps
  its arc (3 signature families in act 1, 20 in act 2, 42 to 58 after),
  `build.py pacing` and `build.py economy` stay at 0, and the tally is
  one run in twelve lost to viruses, to an area's own battle
  ([measured](encounter-groups-measured.md), The pairs' pass).
- **What it leaves**: act 1 has three signature families with every ROM
  (BN6 alone none: its act 1 stays the area's own);
  the statistics' `act-battles` event now says what the groups cost
  players against the areas' own battles, act by act.

### Calls taken in the build

Four calls of the build were taken by the session that built it, not by
the owner. They were shown to him with step 2's report on 11 November
2026 and he raised no objection:

- **Act 1's beats are eased** (first to two thirds of the pressure cap,
  146 for 220; now the band's lower half too, above).
- **The first layer's beat is the run's second battle**, not its third:
  players fight 1.3 battles a layer.
- **The card names the signature family whether or not MegaMan has met
  it**, where the guardian's line keeps "???" until he has.
- **A Navi in the Server's challenge stays**: where the area's own
  challenge rolls a Navi, no constellation takes his place.

The owner's go for step 3, 11 November 2026: "Yes, ease then on", the arc
turned on for players once act 1's pair is eased, without waiting for
BN4's remaining chips.

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
