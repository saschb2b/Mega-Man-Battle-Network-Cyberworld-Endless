---
type: Design
title: "The core loop, reviewed"
description: "The core loop reviewed in October 2026 with the game-design skill, a team of six analysts, a devil's advocate and the players' own statistics: what works, what it lacks, what has grown, the next evolution's phases, what it freezes and the owner's decisions."
tags: [game, core-loop]
generated: { by: claude-code/claude-opus-5-5, at: 2026-10-26T21:15:00Z }
sources:
  - id: umami
    resource: "umami.saschb2b.com, site Cyberworld Endless: run-start, run-end, guardian and online events"
    title: "The opt-in statistics on the owner's Umami, 6 to 26 October 2026"
  - id: playtest-log
    resource: ".claude/skills/playtest-loop/lessons.md and .build/play/*/report-s*.md"
    title: ".claude/skills/playtest-loop/lessons.md, sessions 1 to 75"
  - id: deep-acts
    resource: "build/host/cyberworld --headless with CYBERWORLD_AUTOPILOT=1, CYBERWORLD_EMU_DEBUG=1 and --dev god,onehit,hp=999, seeds 1-12 from depths 1, 10, 13 and 16; layer_generate over 200 seeds a depth in the 15 main areas, the Undernet and the Graveyard"
    title: "The deep acts measured headless on dev, 26 October 2026 (the runs and scripts in .build/deep/, not in git)"
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
   11 minutes; two persona acts took 16 and 18 (acts 2 and 3, each played
   again after a CONTINUE: Phase 1, The deep acts). Battles per act
   range from one in three layers to eight in two (BN5's End Area). Nine
   runs in ten carry BN5, so its walk reaches nearly everyone. The rule
   stands: MegaMan walks as BN6 walks him, so the maps change.
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
- **Count the choices**, done (26 October 2026): the run-choices event
  beside run-end and the run log's choices line (docs/devtools/run-log.md),
  in the run's tally with no `Run` change. Counted: trips back, requests
  taken, kept and vowed, orders at AsterLand, drafts left for their
  BugFrags, the Undernet's and dark layers entered, duels and netbattles
  taken and won, folder edits between layers, Virus Battlers' series
  begun, rounds fought on after a win and pots taken, and the run's
  seconds on the map against those in battle. Not yet: an order's chip and price, the
  final folder's codes and how much of it the run found, the seconds per
  layer rather than per run.
- ~~**Freeze** new online social surface, PET extras, meta axes, lock
  kinds and the comps' errands. Defer Virus Battler until Phase 1 has
  landed.~~ Not taken: the owner's decisions, below.
- ~~**Gate each minor release** on one main-path playtest.~~ Not taken:
  the owner's decisions, below.

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
  maze fix showed the share can fall from 44% to 10%. BN5's End Area begun
  (26 October 2026): its rooms drawn whole as its pads and its way kept
  one wide, the walkways' junctions on its walks from 11.4 a layer to
  2.3 ([the generator](../world/level-design/generator.md), Navigation); a persona's session there
  still to measure the share.
- **A battle floor**: at least one random battle a layer, and a
  battles-per-act band in `build.py pacing`. Done (26 October 2026): 43%
  of the layers without a guardian had none over seeds 1-12; now each
  has one, BN6's own roll given its best odds on a layer still without
  one and the exit pad holding the battle the rest owe ([the battle
  floor](progression/structure.md)). The pacing report plays seeds 1-12 and marks a layer
  without one. Watch the statistics' virus deaths: the floor adds about
  half a battle a layer.
- **The deep acts** (16 to 18 minutes) brought toward the early ones'
  ten. The measure is decisions a minute, not minutes an act. Measured
  26 October 2026 on `dev` (the battle floor, BN5's End Area one wide and
  the main path's words in); nothing changed yet:
  - *The figure is not the deep acts'.* "16 to 18" is two persona AREA
    CLEAR cards: act 2 of session 61 (18:23) and act 3 of session 63
    (16:20), each continued from a save with its layer rebuilt and played
    again, the PET's work on the clock (a NaviCust bug fixed, a mail read,
    a chip tagged). No act 4 to 6 has a time from a player or a persona,
    and the statistics count the map's and the battles' seconds per run.
  - *The way, headless.* The test autopilot walks the shortest way to the
    exit and wins each battle at a hit (`--dev god,onehit,hp=999`, fresh
    data folders, seeds 1-12 from depths 1, 10, 13 and 16: 329 layers of
    the first cycle), its time split by `CYBERWORLD_EMU_DEBUG`'s line every
    30 frames (the game's mode and state, the chat box, the map) into each
    layer's walk, talk, battles, menus and loads, the battles counted from
    the run log. It is the floor every run pays, not a person's time:

    | Act | Areas these seeds met | Walk a layer | Random battles a layer | An act on the way: walk, battles, talk |
    | --- | --- | --- | --- | --- |
    | 1 | Central, Seaside, a comp | 22 s | 1.36 | 2.5 min: 1.1, 1.1, 0.2 |
    | 2 | Green, the comps, Sky's HP | 24 s | 1.35 | 3.0 min: 1.3, 1.4, 0.2 |
    | 3 | Sky, the story's comps, the homepages | 25 s | 1.36 | 3.0 min: 1.3, 1.4, 0.2 |
    | 4 | Weather Comp, ACDC's HP | 22 s | 1.17 | 2.1 min: 1.0, 1.0, 0.2 |
    | 5 | The Undernet | 27 s | 1.37 | 3.0 min: 1.3, 1.4, 0.2 |
    | 6 | The Graveyard | 27 s | 1.69 | 3.5 min: 1.4, 1.8, 0.2 |

    Talk on the way stays 2 to 3 seconds a layer in every act (6 to 7 on
    a guardian's), and loads, set pieces' warps and locks' doors under a
    second. A battle won at a hit takes 13 to 14 seconds in act 1 and 17
    to 18 deep, its foes more (their HP together 120 in act 1, 420 to 480
    in acts 5 and 6).
  - *The layer, from the generator* (`layer_generate` over 200 seeds in
    each of the 15 main areas, an act's first two layers): the floor
    grows from 177 panels to 227, the way from 33 to 39, the Mystery Data
    from 4.1 a layer to 7.4 to 7.6 (the Undernet's 8.7), and the walk off
    the way to all of them, there and back, from 69 panels to 111 to 114.
    Each data costs the same walk in every act, 15 to 18 panels (a green
    one about 12, a blue one at a far detour 20).
  - *What makes them long.* For a full clear at the autopilot's two
    thirds of a second a panel, act 1 is about 4.7 minutes of walking,
    battles and talk and act 6 about 7.1: the data's detours 1.4 minutes
    of the difference (more data, each as far), the battles 0.7, the way
    0.3; the battles BN6 rolls on the longer detours come on top. If
    people take the same multiple of this floor in every act (the
    personas took about twice it in acts 1 and 2), a deep act's full
    clear comes to about 14 minutes. Not talk, not loads or locks, not
    the Undernet's layouts (its act on the way is as long as act 2's).
    Battles follow the walk: BN6 rolls by the distance walked, about a
    battle to every 850 to 1000 of the trace's steps in every area, and
    the Graveyard's walk is the deep acts' longest (1472 steps a layer
    against the Undernet's 1244), so 44 of its 68 layers without a
    guardian held two or three; its own map (96:01) also reads BN6's
    encounter class 3 where the other areas' read 5 ([BN6's
    battles](../rom/battle.md), Encounter odds and classes): 1.86 battles
    a layer there, 1.55 on the map it shares (92:00), where the
    Undernet's hold 1.32. People's battles against V2 and V3 are not
    measured by act: a guardian takes them 29 seconds in act 1 and 42 in
    the Nest (the statistics), the folder keeping up with more than twice
    the HP for half again the time. The autopilot cannot stand in (144
    seconds for act 1's guardian with the starting folder).
  - *Through the game-design skill.* The dialectic is the battle floor's,
    preparing for the act's guardian against the walk that costs time;
    the loop layer is the layer's (the walk for Mystery Data and the
    battles on it), feeding the act's folder. A sawtooth steepens late by
    what an act holds, not by its length, and Slay the Spire keeps each
    act's count of floors as its content hardens. By the measure chosen
    the deep acts' main growth is not a fault: their data are decisions
    at act 1's walk each, so a deep act is longer because it holds more
    choices at the same rate. What thins the rate deep is time no one
    chose: the way's six more panels and the battles its walk rolls.
  - *What a small change buys*, each measured on the generator: holding
    the data at act 3's count from act 4 on takes the detours' walk in
    acts 5 and 6 from 111 to 114 panels a layer to 95 to 100, holding
    acts 5 and 6 at acts 3 and 4's layouts (`size` 1 in
    `layer_generate`) to 96 to 102 with a green data fewer or to 103 to
    108 with the data kept: half a minute of a full clear an act at most,
    the first two by taking away a decision a layer. The Graveyard's map
    at class 5 would take about 0.3 of a battle from half its layers.
    None brings a deep act near ten minutes, and the one that would (act
    1's count of data deep) undoes the economy's plan and nearly halves
    the deep acts' decisions.
  - *The proposal, the smallest change:* count before cutting. The
    seconds per layer by kind (map, battle, the rest) on the run log's
    layer line and per act in the run-choices event, Phase 0's open
    count, with no `Run` change, so the statistics show people's acts 1
    to 6 and their battles against V2 and V3. Then judge a deep act by
    decisions a minute, its way and its unchosen battles no larger a
    share of it than in act 1; ten minutes stays the early acts'
    measure. If people's deep acts run past about 14 minutes, the lever
    is the data's count from act 4 (fewer, each richer, through
    `build.py economy`), not the layouts, the walk or BN6's battles.
- **Words on the main path cut**: the Guardian Data chain to one screen,
  arrivals to three or four boxes, one channel per fact at each decision
  point (the warnings that saved runs stay), through [the voice rules](../voice/rules.md) and
  the review dashboard. Begun 26 October 2026 ([the log](../voice/sweep/main-path.md)):
  arrivals at two to four boxes, the next guardian named once a decision
  (the card, the dealer, the portal, his own layer), his tips in Dad's
  mail alone but EraseMan's and DiveMan's warnings; the Guardian Data
  chain from 12 boxes to 9, then to 7 (5 to 6 for a veteran, no mail):
  BN6's own "MegaMan got:" boxes two things each, the call to install in
  the draft's ask, nothing after the pick but BN6's box. Short of one
  screen still; what is left is his last word, the facts in BN6's boxes,
  the draft whole and Dad's first mail.
- **Jobs on BN6's Request BBS** at AsterLand, all three askers' posts in
  one place, the courier naming each offer's kind and pay. The askers and
  the Academy stay as places (decision 10 stands). Built 26 October 2026
  ([Jobs](../world/home/jobs.md), On BN6's Request BBS): BN6's own screen
  lists, reads and takes the three; the statistics' requests taken and
  vows will say whether the draft is read more.

### Phase 2: sharpen (small, one at a time)

- **Guardians that test the plan.** A band for the guardians' HP left
  and seconds in `build.py pacing`, its centre set through the
  game-design skill from today's numbers (a median 81 to 85% HP left),
  and the guardians brought to it with BN6's own versions and HP before
  any new rule. The statistics' guardian event measures it after.
- **The BugFrag workshop** as designed: programs for BugFrags at the
  vendor, the BugFrag Trader folded in, bugged battles paying BugFrags.
  It closes the dead currency and makes the draft's skip real. Built 26
  October 2026 ([the NaviCust](navicust.md), Status); the statistics'
  draft skips will say whether the skip is taken.
- **Lean the code, not the chip**: a find keeps its chip and takes a
  folder code, so variety widens and the folder still converges. Say the
  rule once where the dealer already half says it. Built 26 October 2026
  ([the opening plan](economy/opening-plan.md#the-code-leans-not-the-chip)):
  in act 6 the Mystery Data's chips spread over 219 kinds rather than
  172, a third of the finds fit the folder rather than half, and a median
  run holds 17 chips of one code rather than 19; act 1's dealer adds
  "Deeper in,Mystery Data too!".
- **The Server's prize a pick of one from three** on the vault's screen:
  the riskiest optional fight pays a decision, not a roll. Built 26
  October 2026 ([the structure](progression/structure.md), Risk the player
  chooses): three chips named in the offer, the pick after the win, B
  leaving them at the Server; no skip that pays.
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
- **26 October 2026: no freeze.** Side systems (online, the PET's
  extras, meta axes) may keep growing beside Phase 1 (chosen over the
  freeze this review proposed).
- **26 October 2026: Virus Battler stays** (chosen over deferring it
  until Phase 1 has landed). It had shipped in 0.16.0 (#237) while this
  review read 0.15.1's tree, which called it unbuilt; its series, the
  rounds fought on and the pots taken are among the counted choices.
- **26 October 2026: no release gate.** Releases go out as now, without
  a main-path persona session per minor release; the statistics and the
  players' reports are the check.

The rest of the phases stand as proposed: the choice counts, then pace.
