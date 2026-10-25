---
type: Design
title: "The meta layer's design"
description: "The meta layer's design: what you bring against what you find, the loop stack, variety not power, and its pieces: starting folders, Cross starts, the threat staircase, helpers, the Library, gates, routes, run length."
tags: [game, meta]
generated: { by: claude-code/unrecorded, at: 2026-10-25T17:18:46Z }
---

# The meta layer's design

## Where it stands

A run resets fully. What persists today is little and mostly invisible:

- the best depth, shown on the title;
- `rivals.sav`: per guardian, meetings and who won, which picks its lines;
- one-time tips (the NaviCust board, the map's violet marks) and the last
  run's first guardian, so the next run meets another;
- (since built: programs found join later runs, [The NaviCust](../navicust.md), 7).

A run is long. The playtester's last run reached layer 9 in about an hour
of game time, over five sessions, and ended at ChargeMan on a summary of
numbers. Nothing of that hour carried over but a best depth.

## The experience

"Every run teaches me something and leaves me a new way to play." After a
loss the player should already be thinking about the next start: which
folder, which Cross, one more rung of threat, the gate they are one code
away from.

Not: getting stronger by grinding, a new player shut out of the fun by
missing unlocks, or a wall of options before the first jack-in.

## The dialectic

The run restates one tension at every scale: **prepare or press on**. Every
detour (a Server, the Undernet, a trader) trades HP and time now for power
at the guardian.

The meta layer restates it once more: **what you bring, against what you
find**. Before a run the player chooses what to bring (a folder, a Cross,
helpers, how hard the net is). Inside it they adapt to what they find.
Unlocks widen what can be brought; the threat staircase narrows the margin
for what is found.

## The loop stack

| Loop | Time | Today | With the meta layer |
| --- | --- | --- | --- |
| Moment | seconds | A BN6 battle | Unchanged: BN6's core stays frozen |
| Session | minutes | A layer: explore, fight, spend | Unchanged |
| Progression | an hour | An act: prepare for its guardian | Plus a choice of the next act's route |
| Run | hours | Acts to the Nest | Starts from a chosen folder, Cross and threat |
| Meta | weeks | A best depth | Unlocks, the Library, threat rungs, gates that stay open |

Today the run's output feeds nothing: the meta loop is hollow. The fix is
not more numbers but more options that the next run can use.

BN itself worked this way: the series froze its battle system and iterated
the systems around it for five games (the skill's iterative-refinement
pattern). Cyberworld Endless should freeze BN6's battles and grow the meta.

## The stance: variety, not power

Runs one and a hundred share the same baseline strength (Slay the Spire,
Balatro). What persists is options and knowledge: never max HP, chip power
or zenny. Permanent power causes difficulty drift, widens the new-player
gap and makes runs blur together.

The one exception is chosen, not earned: **helpers**, toggled per run, off
by default, named on the summary (Dungeons of Infinity's head starts,
Hades' God Mode). They are an accessibility dial, not progression. One
helper is earned all the same, by the owner's call: BeastOut from the
first battle, once the endless net's own Nest has fallen ("BeastOut from
the start, a fifth helper", below).

## The pieces

Each piece is its own axis, so none makes another obsolete (the skill's
nested progression).

### 1. Starting folders (Balatro's decks)

At NEW GAME the player picks a folder. Each is a legal 30-chip BN6 folder
with an identity and a cost, never simply better. Each is unlocked by a
milestone that teaches its style, never by a count to grind.

| Folder | Identity | Cost | Unlocked by |
| --- | --- | --- | --- |
| Standard | BN6's own starting folder | None | Always |
| Blade | Swords, WideSwrd, LongSwrd, StepSwrd | No ranged chips: guardians in the back row need planning | Finish a guardian with a sword |
| Storm | Elec chips around DolThdr and ElcPuls | One element: only Aqua foes take double | Delete an Aqua guardian with an Elec chip |
| Z-Canon | Cannons in codes A, B and C: BN6's Program Advance ready to draw | Few answers to viruses behind cover | Use any Program Advance in a run |
| Busting | Heavy hitters, no Recovery chips | Every heal comes from the net | Clear an act without a Recovery chip |
| Lean | Few kinds of chip, codes aligned for big hands | Little coverage | Reach layer 10 |

A folder is what the run brings: the pack starts as BN6's would have after
NEW GAME gave that folder (its chips counted, marked as given), not with
the Standard folder's chips as spares, which let a Blade run slot in
CrakShot and Cannons and so lose its cost (session 31).

### 2. Starting as a Cross (MegaMan's axis)

A Navi deleted in any run lets later runs start in its Cross (HeatCross,
ElecCross, SlashCross, EraseCross, ChargeCross), chosen beside the folder.
The cost is BN6's own: the Cross's element makes MegaMan weak to its
counter (HeatCross and ChargeCross to Aqua, ElecCross to Wood, SlashCross
to Breaker, EraseCross to Wind, as BN6's own Cross tutorials say), which
the setup's Cross row names, as MegaMan does when a guardian's Cross is
won. The Cross still comes from its guardian mid-run as today; a Cross
start only moves when. BeastOut comes with the Cybeast's fall at the
endless net's own Nest, for the rest of that run ("BeastOut from the
Cybeast", below); once that Nest has fallen, a helper brings it from the
first battle ("BeastOut from the start, a fifth helper", below).

### 3. The threat staircase (Ascension, Heat, Stakes)

Ten rungs, each one new constraint, each unlocked by reaching the Nest on
the rung below. A rung is a lesson, not just a bigger number. Examples,
all engine-side, none touching BN6's battles:

1. Viruses one version up from act 2
2. Recovery Mr. Progs only on each act's second layer
3. Net Dealers charge half again
4. Guardians at EX from act 2
5. Servers may hold SP Navis from act 1
6. Mystery Data holds chips only, no zenny
7. Chip Traders come half as often
8. The NaviCust draft offers two programs, not three
9. One HPMemory fewer in each Guardian Data
10. A second guardian waits in the Nest after the first

The title shows the highest rung cleared.

### 4. Helpers (Dungeons of Infinity's head starts)

Per run, off by default: two HPMemory at the start, a heal on every layer,
the act 1 dealer's pick as the start gift, slower virus versions. The
summary names a helped run. Whether a helped run counts for unlocks is a
decision below. A fourth, All * (every chip in *), came with issue #18:
"All *, a fourth helper", below; a fifth, BeastOut from the first battle,
the only one a profile earns, with issue #99: "BeastOut from the start, a
fifth helper", below.

### 5. The Library (the collection)

Every chip MegaMan ever holds enters the profile's Library, BN6's own word
for it. The summary names the new entries of a run. The Library feeds:

- rarer stock at dealers and traders once a chip has been seen;
- later, a custom starting folder built from Library chips under a
  budget (the skill's loadout-as-budget);
- the programs pool of [The NaviCust](../navicust.md), 7, on the same rule.

Beside it the BtlrCard, BN6's own card of virus data (the owner's
decision of 25 October 2026; [The Virus Battler](../../world/level-design/virus-battler.md)):
the first Virus Battler met gives it, with Mettaur and Gunner, and each
Rare virus deleted in any run joins it. It is variety, never power: its
viruses fight for MegaMan only at a Battler, where a bigger card plays
longer series.

### 6. Gates that open over runs (BN's security cubes)

BN hid paths behind cubes and doors that opened with codes found later.
Here they open across runs:

- **Sealed gates** stand on layers from the first run, with the code they
  need named ("It needs a HeatCode"). The telegraph comes first; opening
  comes later.
- **Codes** are earned by milestones: beat HeatMan three times, clear an
  act on threat 3. They persist.
- **An opened gate stays open** in later runs and leads to one of:
  - a vault of rare chips;
  - a Navi's rematch room, whose SP version pays its SP chip;
  - a route branch: another area and guardian for the next act.

The engine already runs one gate: the golden gate that three ScrtData
open. This makes the same object persistent.

### 7. Routes the player chooses

At an act's end, two exit pads, each naming the next area and its guardian
(Hades' doors, Slay the Spire's map). Opened gates add branches to this
choice. This is where the net grows: every new area or guardian in the ROM
becomes a branch that some runs take.

### 8. Run length

A full cycle takes about two hours of play, and a loss resets it all. The
meta layer softens a loss (each run pays some progress) but does not
shorten it. Options:

- keep one long run;
- a short net (three acts and the Nest, about an hour) as the default,
  with the endless cycle as an unlock;
- a checkpoint at each act start that a run can use once.

## Where the choices live

First, a run setup screen after NEW GAME, drawn in the PET's style: folder,
Cross, threat, helpers (Balatro's deck and stake screen). Later, the town
as the hub: Higsby's counter for folders, the lab for threat and helpers,
Lan's PC for the Library. The town's doors lead nowhere today; this gives
them a use.

## Teaching it

- The run summary names what the run unlocked (on its RECORDS screen,
  four lines at most, each marked NEW: the setup's new entries first,
  then the title's marks) and what is closest
  ("One more sword finish: the Blade folder"). A Cross that won't fit
  beside the run's says, at its first win, that the next dive can start
  with it.
- The first unlock comes in the first or second run (front-load the first
  reward).
- An unexpected unlock (a code found in a vault) pays more than an
  expected one: the skill's overjustification guardrail. Milestones that
  certify mastery, not counts to grind.

## What could go wrong

- **Power creep:** a folder strictly better than Standard. Each gets a real
  cost, and pacing runs check each folder at every threat rung.
- **Grind:** unlocks by counts ("delete 500 viruses"). Use milestones that
  teach, as Balatro's decks do.
- **Complexity:** the setup screen becomes a wall. Ship three folders,
  three helpers and five rungs, and add from there.
- **The new-player gap:** nothing locked is required. A first run is
  complete without unlocks.
- **Saves:** the profile grows append-only, as it does now.

## Measuring it

- Which folders, Crosses and rungs players pick; a folder nobody picks
  needs a reason to exist.
- The playtester's restart judgement: whether a new run feels fresh or a
  chore.
- Unlock pace: the first in run one or two, then about one every one or
  two runs.

## Phases

1. Starting folders (three), helpers (three), threat rungs 1 to 5, the
   setup screen, the unlock line on the summary. Uses BN6's folder data
   and the engine's existing pacing knobs; needs the folder's RAM layout
   (bn6f). Built.
2. The Library, the programs pool, Cross starts (reasoned below), and
   rewards in the folder's codes. Built.
3. Gates and route choice, with new areas and guardians as branches.
   The route choice, the Navi gates, the vaults and the dark way are
   built.
4. The town as hub, more starting towns, rungs 6 to 10. The rungs are
   built (Mystery Data of chips, half the Chip Traders, drafts of two,
   four HPMemory a Guardian Data, and a second guardian below the short
   net's Nest: its first guardian's exit leads to a layer 11 with a heal,
   a dealer and another of the Nest's guardians, drawn from the run's
   seed, none of the acts' nor the first; the endless net's Nest rebuilds
   the net, so the rung says it is the short net's). Seaside and Green
   Towns are the third and fourth starting towns, copied whole with their
   own walls and heights, since each stands on several; Sky Town would
   come the same way ([The overworld](../../world/overworld.md#further-towns-planned), further towns). The town as hub
   (the setup's choices made in the town, at its people and shops) waits
   on the playtests: the setup screen has raised no complaint. The town
   as home between acts (5 October 2026, [Home](../../world/home/overview.md)) keeps the setup a
   screen and gives the town the run's act ends instead: its ports, its
   people's jobs, its shop.
