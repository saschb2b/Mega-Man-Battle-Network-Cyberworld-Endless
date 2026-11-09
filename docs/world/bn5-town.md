---
type: Design
title: "A BN5 town on the way"
description: "A town from BN5 Team Colonel's ROM as a stop on the way into an act, designed with the game-design skill and not yet built: where it stands and how often, how Lan gets there, what it holds and pays, what BN5's ROM has of its towns (read on 9 November 2026), Oran Isle first, the build units, the checks, its pace cost against the core loop and the owner's questions."
tags: [world, design, bn5, town]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-09T19:45:00Z }
sources:
  - id: owner
    resource: "The owner's call of 9 November 2026, recorded in the gap audit of 9 November 2026 (on the bn5-gaps branch: multirom/gaps.md, The owner's calls)"
    title: "A BN5 town: a place visited, a stop on the way as a side path is"
    author: human:saschb2b
  - id: bn5-rom
    resource: "Mega Man Battle Network 5: Team Colonel (USA), SHA-1 5f472f78d8de2df01d5039e045c043cb40969a39: its map lists read by a scratch script with tools/xrecipe.py's readers, its maps' coordinate data counted by build/host/cyberworld --atlas DIR:x0, its songs' starts by tools/bn5_songs.py"
    title: "The owner's BN5 Team Colonel ROM"
  - id: stats
    resource: "docs/game/core-loop.md, What the statistics say and What it lacks (Umami, 6 to 26 October 2026)"
    title: "The players' statistics and the walk's measures"
---

# A BN5 town on the way

The owner's call of 9 November 2026, for the content filler after
0.20.0: a town from Battle Network 5 Team Colonel as a place visited. Not
a dressing of home, which stays BN6's Central Town after every act
([Home](home/overview.md)), but a stop on the way, as a side path is:
off the main way, now and then, skipped at no cost. This is the design,
reasoned with the game-design skill and checked against [the core
loop](../game/core-loop.md); nothing of it is built.

Held back on 9 November 2026: built on its branch and seen in a run, the
cut from MegaMan deep in the net to Lan in another city read as odd to
the owner (discussion #323). [Step out](step-out.md) proposes another
way there, MegaMan through a CopyBot from Oran Area's own layer, and
keeps this page's island, stall and job.

## What BN5's ROM has of its towns

Its real world is six map groups ([BN5's maps, songs, text and objects](../rom/bn5-world.md),
BN5's real world, group by group), read on 9 November 2026. The maps'
lists lie one after another, so each group ends where the next begins:

| Group | Place | Maps | Its first map | Floors | Its song |
| --- | --- | --- | --- | --- | --- |
| 0x00 | ACDC Town | 8: the town, six houses, Higsby's | 132 x 72 | one (no height cells in any of the 8) | its map lists: 0x03 the town, 0x04 the houses, 0x05 Higsby's |
| 0x01 | SciLab | 5 | 68 x 56 | its second map on several | its map lists: 0x05 |
| 0x02 | Oran Isle (beach, mine, dock) | 12: the island, two large maps, nine smaller ones | 128 x 96 | several (990 height cells on the first) | Beach Day, 0x06, started by the first map's own scripts each frame (`tools/bn5_songs.py`) |
| 0x03 | the harbour (a raised quay, the ship) | 6 | 152 x 96 | several | none: in no map list, its scripts start their scenes' songs |
| 0x04 | a castle town on stone terraces | 5 | 100 x 78 | several | Challenge, 0x23, from its second map's script |
| 0x05 | Nebula's base | 5 | 72 x 72 | several | none |

- **Tiles**: 256 colours, as BN6's real world's ([The overworld](overworld.md));
  a tile set a map but where rooms share one: ACDC Town's 8 maps on 8
  sets, Oran Isle's 12 on 4, its nine smaller maps sharing one.
- **People**: BN5's sprite list 5 holds 34 sprites (by the lists'
  bounds); that they are its townsfolk, as BN6's list 5 is, is assumed
  from BN6's layout until unit 1 reads them. BN5's
  bystanders on its net areas, from list 6, are already copied in
  ([Other games' ROMs: status](../multirom/status.md)).
- **What a town would take** (the status's own estimate for ACDC Town,
  **L**): its tile set and 256 colours copied into a free range of their
  own (the town is built while the next layer's tiles may be another
  game's), BN6's interface colours 208-255 kept; its map copied whole with
  its own walls and heights, as Seaside Town is ([The overworld](overworld.md),
  Seaside Town); its jack-in point and its buildings' checks. The others
  then come as Seaside and Green Town did, **M** each.
- **Not run: BN5's map scripts.** They are BN5's own script engine (the
  song finder decodes 50,580 of its commands), which BN6 does not run, and
  BN5's text is never copied. The people are BN6's NPCs with BN5's
  sprites; what they say is ours.
- **The heights**, counted by the atlas through each group's coordinate
  lists, which run on into the next group's as the maps' do: each count is
  to be confirmed group by group as its town is built.

## The design

**The experience.** "We went in from Oran Isle this time." A trip, like
BN6's own trips to Seaside Town or Green Town, to a place a BN6 player has
never walked in this engine: the dock, the beach, BN5's own people and its
song. It is the older Net's world in the flesh, where the run's BN5 areas
are only its wires.

**The dialectic** is the run's: prepare or press on. The town is
preparation (the act's guardian named before the dive, a chip picked for
the folder, a job vowed) for a minute of the player's time; pressing on
is the portal as it always was. It feeds the act (the session loop), not
the meta.

**The patterns.** *The side path* (a set piece off the way, taken or not,
as the [Virus Battler](level-design/virus-battler.md) is); *Hades' doors*,
the fork read before it is taken (the question says where the stop
leads and what is there); *respect the player's time* (one stop, its pay
on its arrival's line, the way out a short walk); *the honest telegraph*
(people who live by a net know who guards it, [What MegaMan knows](../game/meta/what-megaman-knows.md)).

### Where it stands, and how often

- **On the way into an act**, from act 2 to the last before the Nest,
  whose area BN5 dresses: the town is the older Net's, and from its PC
  the older Net's areas are reached, as BN5's own net areas join one
  another. Never act 1 (the first ten minutes decide half the players:
  63 of 119 started one run only), never the deep acts, the Nest, a trip
  back or a dark way.
- **Once a run at most, in half the runs** that hold such an act: a coin
  of the run's seed, as the Glitch layer's and the Virus Battler's are,
  and it stands before the first act that fits. A unit test over 300
  seeds holds the share, its band set from the first count.
- **Which town**: the act's own area's where BN5's ROM has one (Oran Area
  Oran Isle, ACDC Area ACDC Town, SciLab SciLab), else the seed's among
  those built. Built first, Oran Isle stands for all of them.

### How Lan gets there: out of the net

In BN a Navi does not walk the real world; Lan does, PET in hand, and
jacks MegaMan in wherever there is a port. So the stop is Lan's trip,
made where the run already forks:

1. **In Lan's HP**, on the portal of that act, MegaMan asks before it
   warps: the older Net runs from Oran Isle too, so they could go in from
   there. BN6's own yes and no, starting on No. No is the portal as
   always (two seconds).
2. **Yes**: BN6's own transition from the internet to the real world,
   as the jack-out home makes it, sets Lan down at Oran Isle's dock, its
   name shown as a town's is. The run is saved there ("From Oran Isle"),
   as it is in Lan's HP and at home.
3. **The town**, its own map whole, its song, its people.
4. **Its jack-in point** by the dock starts the act's first layer, as the
   portal would have. After the act, Lan's HP and home are as ever: a
   jack-out from Lan's HP sets Lan down in his room, the act's hour moved
   on ([Time of day by act](home/time-of-day.md)).

Who knows what: Lan and MegaMan have been to Oran Isle before (BN5's
story takes them there: assumed from the game's place names, not
verified here), so Lan's line is a return, not a discovery; the people
of the island live by the older Net and pass on what is said on it;
MegaMan senses nothing he could not sense from Lan's HP.

### What is there, and what it pays

Three things on the walk from the dock to the jack-in point, each a
decision, nothing that is not:

- **Word of the guardian.** One of the island's people names the act's
  guardian and his manner, as the act's first bystander does today
  ([What MegaMan knows](../game/meta/what-megaman-knows.md): hearsay, never
  his moves): the act's "???" answered before the dive, so the folder can
  be edited for him before its first battle.
- **The older Net's chips.** A stall: three of BN5's own chips (the
  registry's, [The chip registry](../multirom/chip-registry.md)) of the act's area's catalogue,
  in the folder's codes where they come, one at a Net Dealer's price; one
  of them of a held Soul's kind where the run holds one. BN6's own pick
  screen, as the Server's vault shows its three.
- **A job.** One request, as the Request BBS's ([Jobs](home/jobs.md)): a
  vow for this act, paid by the courier in Lan's HP after its guardian.

Not there: a battle (the act's first layer is the next one), the shop's
whole stock, more people with lines. The island's other paths stay
open to walk and hold nothing to collect.

### Its pace cost

The review's first lack is pace ([the core loop](../game/core-loop.md), What it lacks),
and its measure is decisions a minute, not minutes:

- **Taken**: the target is 90 seconds from the question to the jack-in
  (a home visit measured 2.5 minutes for one purchase against a design of
  60 to 90 seconds; this stop holds three decisions where that visit held
  one, on one line). A won short net takes a median 64 minutes; one stop
  is about 2% of it, once.
- **Not taken**: one question, two seconds.
- **Its rate**: a layer's full clear holds its data's decisions at 15 to
  18 panels of walk each ([the core loop](../game/core-loop.md), The deep acts); three
  decisions in 90 seconds is that rate or better.
- **Measured**: the seconds in the town on the run log and the run-choices
  event (towns offered, taken, the stall's pick, the job taken), the
  core loop's count of choices. If people take it and its median runs
  past 90 seconds, the walk shortens (the jack-in point nearer the dock)
  before anything is added.

## The build, in order

| Unit | What | Size |
| --- | --- | --- |
| 1. Oran Isle read | Its dock (the arrival) and a jack-in point from its coordinate data's triggers, its buildings' checks, which of list 5's 34 people stand there, Beach Day's size; their rows in [BN5's maps, songs, text and objects](../rom/bn5-world.md) | S |
| 2. A BN5 town in BN6's real world | Its 256 colours and tiles in a range of their own, its map copied whole with its walls and heights as Seaside Town's (`coords_write_raw`), BN6's interface colours kept, its people by BN6's NPC commands; Oran Isle, then each other town as Seaside and Green Town came | L, then M a town |
| 3. The stop | The question on the portal, the transition, the town's checkpoint, the act begun from its jack-in point; once a run in half the runs (the unit test), a dev switch to force it; the run log and run-choices counts | M |
| 4. What is there | The word of the guardian, the stall on BN6's pick screen, the job paid by the courier; the lines in a words file through [the voice rules](../voice/rules.md) | S-M |
| 5. Its song | Beach Day copied by `xsong.c` as the island's (the songs' free space: 768 KB, about 231 KB used) | S |
| Later | ACDC Town (its songs in its own lists, one floor), SciLab, the harbour, the castle town; never Nebula's base | M each |

## The checks

- `build.py town`'s captures of the BN5 town for its seeds (drawn, misses
  marked), and an autopilot run from the question through the town to the
  act's first layer, `layer>=2`.
- A CONTINUE from the town's checkpoint, Lan at the dock, the stall as the
  quit left it.
- `build.py test` (the stop's share over 300 seeds), `build.py lint`,
  `python3 tools/voice_catalog.py`.
- `tools/heap3ds.py` with BN5: the town's tiles on the New 3DS.
- The seconds in town on the run log, headless and in one persona's
  session, against the 90 seconds.
- After release: towns offered and taken, and seconds there, in the
  statistics.

## The owner's questions

**Answered on 9 November 2026: as designed.** Oran Isle first, reached by
a question on the portal of an act BN5 dresses from act 2, once a run at
most in half the runs that hold such an act, with the guardian's word,
the stall and the job, and no battle.


Each with the recommendation the data gives:

1. **Which town first.** Recommended: Oran Isle. BN6 has no island, so a
   veteran meets something new (the review's lack 5), its own scripts
   start its own song, and it comes as Seaside Town came. ACDC Town is
   the cheaper map (one floor, its songs in its lists) but BN6's own
   prologue already shows ACDC Town at the same 132 x 72.
2. **How it is reached.** Recommended: a question on the act's portal in
   Lan's HP, starting on No. The other ways: from home, which every
   player does not visit and which the owner keeps BN6's; or a port at a
   side path's end mid-act, which would put Lan on a trip while MegaMan
   is still in the layer.
3. **How often.** Recommended: once a run at most, in half the runs that
   hold an act from 2 on in a BN5-dressed area.
4. **What it pays.** Recommended: the guardian's word, a pick of three of
   the older Net's chips and a job; no battle in the town.
