---
type: Design
title: "Home's design"
description: "Home's design by the game-design skill: the old town's faults, two homes (Central Town for Lan, Lan's HP for MegaMan), the prepare-or-press-on dialectic, the loop stack, its patterns and what must not move home."
tags: [world, home]
generated: { by: claude-code/unrecorded, at: 2026-10-06T01:42:31Z }
---

# Home's design

## Where it stands

A run's town (Central, ACDC, Seaside or Green Town) was the run's first
minute: Lan walks from where he starts to the landmark's port and jacks
in, and the run never sees it again. Its people say a line about the
place, its checks describe the houses, a Mr. Prog explains R. The owner:
"just a theme park, a stark short walk to the jack-in and then
forgotten".

The skill's rework procedure names what is wrong:

- **No dialectic.** The town restates none of the run's tensions: it is
  decoration, not design.
- **No decision.** The walk has one way and one end.
- **A hollow loop.** Nothing the net gives reaches the town, and the town
  gives the net nothing.
- **Seen once.** Ten to thirty seconds a run.

The first answer (issues #85 and #86, built) brought Lan home to the
town after every act and made the act's ways ports in town. The owner
played it and named the problem: MegaMan jacks out, Lan stands in the
town, and then jacks in at a doghouse or a fountain to go on through the
Endless Net. In BN6 a jack-in point leads to its object's own comp; the
net beyond is reached from Lan's PC, through Lan's HP. The Endless Net
wants one place of its own.

## The experience

**Two homes, one for each of them.** Central Town is Lan's: his house,
his errands, people who have heard what MegaMan did. Lan's HP is
MegaMan's: the place every jack-in arrives, where the run's portals light
up as its guardians fall, and where he chooses to press on or go back.

Not: a chore between acts, a second setup screen, a walk that taxes every
restart, a hub of menus.

## The dialectic

The run's own, **prepare or press on**, made spatial. In Lan's HP the
newest portals press on; an older one goes back to an area already won,
to prepare, at a price: the Net's clock. In town, a job taken bends the
next dive, zenny spent at home is not carried to the net's dealers.

## The loop stack

| Loop | Before | With home |
| --- | --- | --- |
| Progression (an act) | Ends on its guardian and "Which way?" | Ends in Lan's HP: the next act's portals light up, and the run is saved there |
| Run | The town once, then only the net | Lan's HP between acts, the town when Lan has errands; the HP grows with the run |
| Meta | The town forgets | The town remembers the runs (recognition and variety, never power) |

## Patterns

- **The waypoint hub** (Diablo's waypoints, Darkest Dungeon's hamlet and
  its regions): one place the reached destinations hang from, the newest
  the push, the older ones the farm. The farm is the pitfall (below).
- **The time-difficulty clock** (Risk of Rain): the longer a run takes,
  the harder what lies ahead. Going back is allowed and priced, not
  forbidden.
- **The bonfire rest point** (Dark Souls, Hollow Knight): one safe stop
  per act, with a checkpoint; its spacing is the act.
- **The hub as the same place** (Hades' House, Cult of the Lamb): a home
  works when it is the same each run, so its people and changes can be
  noticed.
- **The dual-life loop** (Moonlighter, Cult of the Lamb): the net
  supplies what home asks for, home gives the net its goals. The halves
  feed each other or one becomes a chore; the town stays short and
  optional against an act.
- **The draft with a skip** (Slay the Spire, Hades): a job from one of up
  to three people, or none.
- **The branching map** (Slay the Spire's map, Hades' doors): the next
  act's ways as portals side by side.
- **Flavor as meta-narrative**, and its candidate the diegetic newspaper
  (Mina the Hollower): people and the news report the run.
- **The bonus with a drawback**: a job's constraint is its price.

## What must not move home

- **Power across runs.** Portals are the run's; pay is the run's; the
  profile keeps lines.
- **The setup screen.** Walking to pick a folder would tax every restart;
  the setup stays a screen.
- **A required town.** The town is for errands; a dive straight through
  Lan's HP never needs it.
- **Long chats.** One line a person, shorter for returning players.
- **A heal that matters.** The trips heal as BN6's jack-out and jack-in
  do, after a guardian who healed already.
- **Free farming.** Going back is priced by the clock; one job at a time.

## Time

Lan's HP: about 10 seconds for a player who steps on the newest portal.
The town: nothing for one who skips it, 60 to 90 seconds for one who
takes a job and shops. Three to five minutes in a short net of about an
hour, at most.

## What could go wrong

- **Two hubs, twice the walking**: the HP small and the newest portals
  beside the arrival pad; the town only when wanted.
- **The farm**: going back made the best play. The clock's notch is the
  dial, checked against `build.py pacing`.
- **A maze of portals**: a long endless run collects many; the HP shows
  the newest ways first, the older ones in rooms behind.
- **The economy**: jobs, the shop and going back add zenny, chips and
  items. Pacing checks every act's band with them.
- **A dominant order**: the Order Service always buying the best chip.
  One order a visit, at a premium.
- **Saves**: a checkpoint in Lan's HP or the town rebuilds them, and the
  layer behind each portal, before it loads the state.
- **Fidelity**: Lan's HP's rooms beyond BN6's one and time of day are
  the engine's; [Fidelity](../../game/fidelity.md) names them.

## Phases

The epic's stories, in order: home after every act (#85, built); the
ports in town (#86, built, moved); Lan's HP as the warp zone, the act's
exit there and the town optional (#93, built); Central Town as home,
with Lan's room and PC (#94, built); going back and the Net's clock
(#95, built); a town that remembers (#87, built); AsterLand and the
Cyber Academy as places (#96, built); jobs (#88, built), at the BBS, from the townsfolk and the
classmates, with the objects' comps; the home shop at AsterLand's
counter (#89, built); time of day (#90, built, to be judged); the second screen (#91, built); the captures, docs and notes.
