---
type: Design
title: "The core loop, reviewed"
description: "The core loop reviewed in October 2026 with the game-design skill, a team of six analysts, a devil's advocate and the players' own statistics: what works, what it lacks, what has grown, the next evolution's phases, what it freezes and the owner's decisions."
tags: [game, core-loop]
generated: { by: claude-code/claude-opus-5-5, at: 2026-10-26T17:30:00Z }
sources:
  - id: umami
    resource: "umami.saschb2b.com, site Cyberworld Endless: run-start, run-end, guardian and online events"
    title: "The opt-in statistics on the owner's Umami, 6 to 26 October 2026"
  - id: playtest-log
    resource: ".claude/skills/playtest-loop/lessons.md and .build/play/*/report-s*.md"
    title: ".claude/skills/playtest-loop/lessons.md, sessions 1 to 75"
---

# The core loop, reviewed

A step back after fifteen releases in a month: is the run still the
direction, what does it lack, and what has grown that should not have.
Reviewed on 26 October 2026 with the game-design skill (the loop stack,
the critique smells, the pattern catalog) and its dissections of Hades,
Hades II and Cult of the Lamb, by a team of six (Hades, Cult of the Lamb,
the run's build against One Step From Eden and the deckbuilders, the
audience, a systems audit, the playtest record) and a devil's advocate
who argued against the first verdict and changed it, then checked
against the players' own statistics, which changed it again. Their
reports, with every source, are in `.build/research/loop-review/` (not
in git). The owner's decisions go at the end.

## The verdict

**Keep the direction. Fix the pace and put the danger where the run's
peaks are, before anything else; count the choices before cutting; and
stop growing sideways until the main path is played again.**

The minutes between BN6's battles have grown faster than the decisions
in them, the act's climax has stopped testing the plan it asks for, and
the project ships faster than it is played.

## What works, and must not move

Every source agrees on one small core:

- **BN6's battle, untouched, with a folder the player builds.** It is what
  BN fans name first, what they say One Step From Eden lost (random cards,
  no pause, no folder of one's own), and what players of this game praise:
  "BN6 itself", a folder coming together run after run.
- **The act's chain.** The card names the guardian (or "???"), the dealer
  has heard what he can't stand, a pick in the folder's codes, MegaMan's
  battle data, the guardian beaten on that plan. It is the high point in
  about 30 of 87 playtest reports and in nearly every 9/10 session. Hades'
  known boss at a biome's end, done for a folder game: the portals preview
  the test, not the reward, and "you choose the fight your folder answers".
- **Memory as the pull.** Guardians who remember, first meetings as "???",
  Chaud's record, a town and a lab that keep count. The reasons a persona
  gave to keep playing name these, never an unlock count. It is Hades'
  death-as-story: the copies learn from every fight, so repetition is the
  plot.
- **Variety, not power, at the meta.** Folders, Cross starts, the threat
  staircase, helpers as the chosen exception. Hades II is the warning
  for the other road: its most cited friction was gathering, many
  currencies and features behind recipes, and Supergiant spent patches
  reducing it. No gathering, no crafting gates, no meta currency, ever.
- **Lan's HP between acts.** The run's fork, its checkpoint and a priced
  way back in about ten seconds. Cult of the Lamb's hub turned into a
  chore through upkeep that grows, a clock that runs while you are away,
  and no overview; home here has none of the three.

## What it lacks

1. **Pace.** Walking is the most repeated complaint in the whole record
   (the top wish in about sixteen reports over forty sessions, about a
   third of a persona's calls), the one human verdict on pace says "it
   just felt so slow", and BN fans call traversal the slog of replays. It
   is friction on every path (walkway mouths, junctions, shops at the ends
   of parallel walkways), not a count of detours. Acts 1 and 2 take 9 to
   11 minutes; deeper acts 16 to 18. Battles per act range from one in
   three layers to eight in two (BN5's End Area). Nine runs in ten carry
   BN5, so its walk reaches nearly everyone. The rule stands: MegaMan
   walks as BN6 walks him, so the maps change.
2. **A climax that bites.** Players win 93% of guardian battles, in the
   short net with a median 81 to 85% of MegaMan's HP left after about
   half a minute (the Nest: 62% after 42 seconds). The chain players love
   prepares for a test that has stopped testing; "I got better" needs a
   test that could have gone the other way.
3. **Danger in the right places.** What ends runs is attrition and one
   rival: viruses end 23 of 45 lost short nets, spread over layers 1 to
   9, and ProtoMan ends 30 of 88 lost runs, from 24 different players,
   12 of them on layer 8. [The rival](rival.md) promises "a lost duel costs only
   the HP spent". Either that promise is broken by a bug or the duel is
   mistuned; both sit on the main path.
4. **Choice counts.** The statistics measure the economy, the guardians
   and the run's end; not one decision (trips back, jobs, orders, drafts
   skipped, folder edits).
5. **Variety a veteran feels.** "The net doesn't feel random enough"
   after 23 hours; the same town, comp and guardian four runs running; the
   one long design post by a player asks for risk trades, floor modifiers
   and an optional second guardian. The code lean's reroll (up to eight
   times, swapping the chip itself) narrows which chips appear, which fits
   "too many S and A codes".
6. **A dead currency.** BugFrags have a sink only past a dark flame or in
   the Graveyard, so in most short nets the NaviCust draft's skip pays
   nothing and is never right: a take-everything draft. The workshop that
   spends them is designed ([the NaviCust](navicust.md)) and unbuilt.

Not a lack, on a closer look:

- **A price on the layer.** The audit found the full clear dominant; the
  personas do clear everything. Humans don't: they open a median 3.25
  Mystery Data a layer of the 5 to 8 a layer holds, 7 of act 1's 15. They
  already price the walk, by skipping. The layer needs no new cost; the
  walk needs to cost less.
- **The build "decided at the setup".** The lean reads the folder as each
  layer is made, so editing the folder already steers it. What is true is
  that the lean is silent.

The zenny surplus is real for winners (a median 17,431 earned in a won
short net, above the economy report's full clear), on seven runs; act 1
lands where the opening plan put it (2,730 by its end against the
model's 2,720).

## What has grown that should not have

Bloat is where the player meets it: on the main path.

- **Words.** About 19 boxes of Guardian Data, 12 to 16 of arrival talk,
  seven channels that name the next guardian. AI-sounding text is the
  loudest public criticism of the project (a dozen of the launch thread's
  65 voices, four refusing to play; one player who liked the game called
  the dialogue awkward). Every line that changes no decision is time and
  surface.
- **Rules without decisions.** Eight lock kinds that each guard one
  Mystery Data on a spur: one decision, eight rule sets. Add no more kinds.
- **The vow behind a walk.** The best-rated job sits with three askers in
  two buildings; the courier says where requests wait, not what they are.
  A home visit measured 2.5 minutes for one purchase against a design of
  60 to 90 seconds.
- **Collapsed decisions.** The draft's skip (never), a trip back (never
  worth a tenth of every later guardian's HP for what the run already has
  too much of), the dark flame's Yes. (The "every Mystery Data" job looked
  like one too, paying for what players do anyway; the statistics say
  people skip half the data, so for them it is a real ask.)

Off the path (the Square, Trade Central, NetBattles, the stands, the
DeepNet, the Jukebox, the Bass chain) the systems cost a player almost
nothing. They cost the project its attention: by changelog text,
0.1.0 to 0.8.0 was about 83% core loop, 0.13.0 to 0.15.1 about 48%
online, 19% PET and presentation and 7% core, while the walking stayed
open. Most systems since 0.8.0 have not been played by anyone the record
can see. Public demand for online was two voices, and nobody asked for
the Square's social surface; the statistics agree (31 players visited
the Square, a median minute; eight NetBattles by four players; one
battle watched). That is a focus finding, not a design one: freeze
growth there, keep what works.

The persona's falling scores (8.0 early, 3.0 for the 0.13.0 candidate)
are not evidence of bloat. The lowest two were a pad that never warped
and BN5's End Area junctions, both main-path failures; the same session
called the NetPass "the best screen in the game"; and on one build the
veteran persona scored 6 where the newcomer scored 9.

## What the statistics say

The opt-in statistics ([the player's guide](../GUIDE.md), Anonymous statistics) from 6
October (0.11.2) to 26 October 2026, read on Umami: 119 players started
251 runs and ended 125, with 561 guardian battles and 66 visits to the
Square. Players are Umami's monthly hash, so one on two devices counts
twice. Release builds and the browser page's builds alike; the owner's
own runs are among them. The guardian figures stand on hundreds of
battles; the economy's on seven won runs.

| Question | Answer |
| --- | --- |
| How long is a run? | A won short net: a median 64 minutes (53 to 87). An endless run, as it ends: 88 |
| How often is it won? | 37 short nets of 82 (45%); at threat 0, 22 of 55 |
| What ends a short net? | Viruses 23 of 45 losses (layers 1 to 9), guardians 16, ProtoMan 6 (all on layer 8) |
| What ends an endless run? | ProtoMan 24 of 43, viruses 9, Gregar 3 |
| Do guardians test the plan? | 522 battles won of 561. The short net's first guardian falls 91 times in 95, MegaMan at a median 85% HP after 29 seconds; acts 2 and 3: 81%, 33 and 32 seconds; the Nest: 62%, 42 seconds. ProtoMan staged as a fight: 7 won of 20 |
| Is the full clear the rule? | No: a median 3.25 Mystery Data opened a layer (2.7 to 4.4) of the 5 to 8 it holds; 7 of act 1's 15 |
| Is zenny in surplus? | Act 1 as planned (2,730 by its end); a won short net earns a median 17,431 (seven runs) |
| Do players climb the staircase? | 62 of 119 never left threat 0, 31 stopped at 1, 26 went higher |
| Do they come back? | 63 of 119 started one run only |
| Where do they play? | Of 166 release runs: Android 90, Windows 40, the Steam Deck 16, Linux 7, the 3DS 7, iOS 6; the browser page 74 more |
| Is BN5 a side system? | No: 147 of 166 release runs carry it (89%) |
| What do they bring? | No Cross in 129 of 166 runs, no helper in 139; Standard 96, Storm 39, Blade 31 |
| Online | The Square: 31 players, 66 visits, a median minute, 63 visits under three. NetBattles 8 by 4 players, a battle watched once, DeepNet posts 8 by 6 |

What it changes:

- **The act's climax does not bite**, and the danger sits in attrition
  and one rival (Lacks 2 and 3).
- **The personas play differently from people.** They clear every layer
  and never leave threat 1; people skip half the data and, half of them,
  never leave threat 0. A persona's view of exploration and of the
  staircase is not evidence.
- **The first run is the game for half the players**, mostly on a phone,
  at about an hour. Its first act is where they decide.
- **BN5 is the main path.** Its areas' walking and its Navis' tuning
  reach nine runs in ten.
- **The staircase is barely climbed**, so the default run's peaks come
  before recognition for climbing it.

## The next evolution

One phase at a time, each measured before the next. A change that
tightens the run (prices, heals, sinks) lands alone, so its effect can be
seen; the opening plan of 21 October is still unplayed by a person.

### Phase 0: read, count, freeze (days)

- **Read the statistics**: done (above), to be read again after each
  phase.
- **Count the choices** in the run's tally and the run-end statistics (no
  `Run` change): trips back, jobs taken and kept, orders (the chip, its
  price, zenny left), drafts skipped, dark flames and dark layers taken,
  duels, Liberation boards, folder edits, the final folder's codes and
  how much of it the run found, and each layer's seconds walking against
  fighting.
- **Freeze** new online social surface, PET extras, meta axes, lock kinds
  and the comps' errands. Defer Virus Battler until Phase 1 has landed.
- **Gate each minor release** on one main-path playtest; patch releases
  as now.

### Phase 1: pace and the run's peaks (the next release)

- **ProtoMan's wall**, first. Done (26 October 2026): not a bug. His
  netbattle, the third rung after two races won, first waits on act 3's
  duel layer, layer 8, and BN6's GAME OVER ended the run as MegaMan fell;
  his hits (90 to 150, a 400 HP MegaMan gone in 12 seconds standing
  still) were never capped with his HP. Now his deleting blow leaves
  MegaMan 1 HP and ends the battle as BN6's draw ends one: Lan pulls him
  out ([the rival](rival.md), The netbattle). Watch the statistics for the other
  side, a loss too cheap to fear.
- **Navigation in the generator**, gated per area on walk share with
  `build.py tiles` and a play measure, BN5's End Area first (#164). The
  maze fix showed the share can fall from 44% to 10%.
- **A battle floor**: at least one random battle a layer, and a
  battles-per-act band in `build.py pacing`.
- **The deep acts** (16 to 18 minutes) brought toward the early ones'
  ten. The measure is decisions a minute, not minutes an act.
- **Words on the main path cut**: the Guardian Data chain to one screen,
  arrivals to three or four boxes, one channel per fact at each decision
  point (the warnings that saved runs stay), through [the voice rules](../voice/rules.md) and
  the review dashboard. Begun 26 October 2026 ([the log](../voice/sweep/main-path.md)):
  arrivals at two to four boxes, the next guardian named once a decision
  (the card, the dealer, the portal, his own layer), his tips in Dad's
  mail alone but EraseMan's and DiveMan's warnings; the Guardian Data
  chain from 12 boxes to 9, short of one screen, its rest BN6's own
  "MegaMan got:" boxes and the draft.
- **Jobs on BN6's Request BBS** at AsterLand, all three askers' posts in
  one place, the courier naming each offer's kind and pay. The askers and
  the Academy stay as places (decision 10 stands).

### Phase 2: sharpen (small, one at a time)

- **Guardians that test the plan.** A band for the guardians' HP left
  and seconds in `build.py pacing`, its centre set through the
  game-design skill from today's numbers (a median 81 to 85% HP left),
  and the guardians brought to it with BN6's own versions and HP before
  any new rule. The statistics' guardian event measures it after.
- **The BugFrag workshop** as designed: programs for BugFrags at the
  vendor, the BugFrag Trader folded in, bugged battles paying BugFrags.
  It closes the dead currency and makes the draft's skip real.
- **Lean the code, not the chip**: a find keeps its chip and takes a
  folder code, so variety widens and the folder still converges. Say the
  rule once where the dealer already half says it.
- **The Server's prize a pick of one from three** on the vault's screen:
  the riskiest optional fight pays a decision, not a roll.
- **MegaMan reads the next layer at a fork** (an exit pad beside a dark
  flame, a Liberation way): a strong virus signal, someone selling
  programs. Hades' doors, with no new screen.
- **Guardian lines keyed to the last fight**, on the path every dive
  takes (the arena's intro, the first arrival in Lan's HP): he deleted
  MegaMan last run, what finished him, the Cross MegaMan wore.
- **The three designed folders** (Z-Canon, Busting, Lean): horizontal
  options on the strongest meta axis, known to this audience.

### Phase 3: variety, from what Phase 0 shows

- **Risk the player chooses on the layer**, never in BN6's battle: a deal
  of max HP for power, a layer modifier, an optional second guardian. The
  "press on" half of the dialectic, asked for by players.
- **BN5 as the variety engine**, admitted area by area once its walk
  share passes the gate; its Liberation take rate measured, since with
  BN5 two forks in three hold a dark layer behind one way.
- **Random battles as a choice** (a route or an item: fewer battles,
  fewer finds), the series' most named tedium, answered as a decision.

### Deferred until the counts exist

- The Order Service's list (this run's chips always, the Library on a
  short shelf), if orders prove to be the same best chip every visit.
- Trips back: pay something scarce, or go, if none are taken.
- The draft's three offers as deepen, answer and wild.
- Recognition per start (the highest threat won with each folder and
  Cross, Hades' per-weapon bounty with no reward but the record), once
  more than a few players climb.

### Not proposed

- **One Cross slot a run.** Found Crosses number 0 to 2 in a short net,
  none before layer 6; switching Crosses by element is a skill this
  audience owns.
- **Naming a code at Lan's HP.** The folder already declares it.
- **A price on the layer** (threat 2's heals as the default, or any other
  cost). People already skip half the data, it would remove about half a
  heal an act, delete a rung, and the full heal before the arena refunds
  layer 3 anyway.
- **Folding home's SubChip seller and Chip Trader.** They cost a skipping
  player nothing and are BN6's own AsterLand.
- **StS card removal, a smaller folder, code remaps, chip drafts after
  battles, power meta, any change to BN6's battle.**

## Outside the loop, deciding who reaches it

- **The ROM.** Four or more players could not get a ROM recognized: the
  most common failure in the public record.
- **The first ten minutes.** 63 of 119 players started one run only.
  Broken tiles in first impressions, a first home visit of 94 calls. A new system meets a player gentle or opt-in
  first (the first rival duel, KnightMan in act 1 and BN5's blank chips
  each cost a run before they were tuned).
- **Returning players.** Recent releases draw 22 to 70 downloads: a few
  dozen people who notice when a shipped thing goes. A cut needs a reason
  they feel, and a `Run` change costs their runs in progress.

## The owner's decisions

- **26 October 2026: a lost netbattle costs the HP and the clearance,
  never the run** (chosen over halving ProtoMan's hits or moving the
  netbattle to the endless net): his deleting blow ends the battle with
  MegaMan at 1 HP, Lan stopping the battle.

The rest pending.
