---
type: Design
title: "Step out: the towns through the CopyBot"
description: "A proposal for the real world in a run (discussion #323), designed with the game-design skill and not built: MegaMan leaves a layer through a CopyBot link into its area's own town, walks it as BN6's CopyBot walks him, and goes back into the net through one of its devices; what BN6 has for it (read in bn6f on 9 November 2026), the visit, its doors, its pace cost, the depth that can follow (machines, keys, links, the endless reach), the build units and the owner's questions."
tags: [world, design, town, copybot]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-09T19:45:00Z }
sources:
  - id: discussion
    resource: "https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/discussions/323"
    title: "What should we do with the real world? (towns, not the net), 9 November 2026"
    author: human:saschb2b
  - id: bn6f
    resource: "dism-exe/bn6f (Cybeast Falzar) at ~/.cache/mmbn-ref/bn6f: asm/ow_player.s, asm/asm28_0.s, asm/asm00_1.s, data/dat27.s, constants/enums/GameAreas.inc and ewram_flags.inc, data/textscript/; Gregar's addresses from .build/symbols/ (build.py symbols)"
    title: "The bn6f disassembly, read on 9 November 2026"
  - id: stats
    resource: "docs/game/core-loop.md, What it lacks and What the statistics say (Umami, 6 to 26 October 2026)"
    title: "The players' statistics and the walk's measures"
---

# Step out: the towns through the CopyBot

The owner asked what the real world should be in a roguelike that lives
in the net (discussion #323). BN6's ROM holds 37 maps outside the net and
a run uses eight of them, all at home; BN5's holds six more places.
[Oran Isle](bn5-town.md) was built as a stop on the way and held back:
"MegaMan is deep in the copied net and suddenly we're with Lan in
another city".

This is a proposal for that question, reasoned with the game-design
skill and checked against [the core loop](../game/core-loop.md). Nothing
of it is built, and its questions at the end are open.

**The proposal in one line: MegaMan leaves the net, not Lan.** A CopyBot
link on a layer puts MegaMan in that area's own town, walking it himself
as BN6's CopyBot walks him, and every device there is a door back into
the net. Lan stays at his PC on the PET, so Central Town stays home and
Lan's HP stays the warp zone ([the owner's decisions](home/decisions.md),
4 and 8).

## What BN6 has for it

Read in bn6f on 9 November 2026; Gregar's addresses by `build.py
symbols`. Unit 1 below puts what the build uses into the project's map
(a name in `src/emu/bn6.h`, its row in [ROM data](../rom/world.md)).

| Claim | Status |
| --- | --- |
| BN6 walks MegaMan through its real-world maps. With event flag `0x1729` set (bn6f's `EVENT_COPYBOT_ACTIVE`), the overworld player's sprite on a real-world map group is `0x37`, MegaMan, where it is Lan's `0x00` otherwise | Verified: `owPlayer_toggleUsingCopybot_809e4bc` (Gregar `0x0809F9F4`) and the player's sprite load beside it in `asm/ow_player.s` |
| BN6's own cutscenes switch the walker five times | Verified by count: five `cs_toggle_ow_player_using_copybot` commands in bn6f's cutscene data. Which scenes they are was not traced |
| BN6's people answer a Navi in a CopyBot: about twenty of its map text archives and the LevBus's test the flag | Verified by a search of `data/textscript/` for the flag. None of that text is used; it shows that BN6's towns expect him |
| With the flag set, L in the real world runs the map's own script, not MegaMan's talk from the PET | Verified: `RunLMessageTextScript` in `asm/asm28_0.s` |
| What R does in the real world while the flag is set | Not traced. A Navi jacking himself in from a CopyBot is ours until it is (question 2) |
| 43 jack-in destinations: 30 device comps of groups `0x8C` and `0x8D` (the doghouse, the vending machine, the popcorn shop, the punish chair, the oxygen tank and 25 more), five homepages (Lan's, ACDC's, the Aquarium's, Green's, Sky's), the four big comps' first maps and the Pavilion's five | Verified: the destinations' table (Falzar `0x080984C8`, [BN6's net maps](../rom/net-maps.md), Jack-in) decoded against `GameAreas.inc` |
| Which town each device comp belongs to | Not traced one by one: each real-world map's jack-in table says it (unit 1) |
| The towns' street maps hold few points of their own: Seaside Town two, Green Town one, Sky Town one. The rest stand indoors | Verified in [the overworld](overworld.md) (Seaside Town, Green Town, Further towns) |
| Interiors a run does not use: the Aquarium's two maps, its auditorium and control room; the court's foyer, the courtroom, the punishment room and an underground room; Sky Town's operator and force rooms; the Expo's five maps | Verified: `GameAreas.inc`'s map lists |
| A LevBus warp is one of BN6's nine warp transitions | Verified: the jump table in `asm/asm00_1.s`. Not used by this proposal |
| Where BN6 stands its CopyBots in each town | Not traced |
| BN6's story sends its villains' Navis into the towns through CopyBots | Assumed, from memory of the story |
| BN5 and BN4 have no CopyBot | Assumed: the CopyBot is BN6's story's machine. No search of their ROMs was run |

## The design

**The experience.** "There's a way out here." Deep in an area's layers
MegaMan finds a link to the outside and steps into the town that area
belongs to: Seaside Town from Seaside Area, Green Town from the Judge
Tree's comp. He walks a place BN6's players know, reads which of its
devices are worth going into, picks one, and is back in the net a layer
further on. The player never leaves MegaMan.

**The dialectic** is the run's, prepare or press on, restated as inside
or outside: the exit pad is the short line through the layer, the link
the way round that comes back in with something chosen.

**The loop layer.** The visit is part of an act (the session loop): what
its door pays goes into the folder that meets the act's guardian. It
feeds no meta but a record (the jack-in log, below).

**The patterns.** *The side path* (a set piece off the way, taken or
not, as the [Virus Battler](level-design/virus-battler.md) and the
[Glitch layer](../game/glitch-layer.md) are); *Hades' doors*, each door
read before it is taken ([a fork's read](level-design/fork-read.md));
*the draft with a skip* (one door of three, or the layer's own exit);
*respect the player's time* (the link is a second exit, not a detour).

### Why this and not Lan's trip

The stop at Oran Isle cut from MegaMan to Lan in the middle of a dive,
at a place the act had no tie to. Home had the same fault once: Lan
jacked in at a doghouse to go on through the Endless Net, and the owner
moved every way on into Lan's HP ([Home's design](home/design.md)). A
second trip of Lan's, by LevBus from Central Town between acts, would
bring that back and add its walk to a home visit already measured at 2.5
minutes. With the CopyBot the trip starts in the net, belongs to the
act's own area and keeps the player's hands on MegaMan.

### Where it stands, and how often

- **On a layer of an act whose area has a town**, at a side path's end,
  never on the guardian's layer:

  | The act's area | Its town | Stands today |
  | --- | --- | --- |
  | Seaside Area, the Aquarium's comp and homepage | Seaside Town | Whole in `town.c` |
  | Green Area, the Judge Tree's comp, Green's homepage | Green Town | Whole in `town.c` |
  | Sky Area, Mr. Weather's comp, Sky's homepage | Sky Town | Planned: its start needs a height |
  | ACDC's homepage | ACDC Town | As a cut style |
  | The Pavilion's comps | The Expo Site | Not built |
  | BN5's Oran Area | Oran Isle | Built on the held branch (question 1) |

  Central Area, Robot Control's comp and the home computers have none:
  their town is home, and home is Lan's.
- **Never act 1** (the first run's first act decides half the players,
  [the core loop](../game/core-loop.md)), never the Undernet, the
  Graveyard, the Nest, a trip back or a dark way.
- **Once a run at most, in half the runs** that hold such an act: a coin
  of the run's seed, as the Virus Battler's and the Glitch layer's. A
  unit test over 300 seeds holds the share.

### The visit

1. **The read.** At the side path's fork MegaMan reads a signal from
   outside, one call, as he reads a dark warp today.
2. **The link.** A pad at the path's end. Stepping on it asks once, BN6's
   own yes and no, starting on No.
3. **Step out.** A fade, and MegaMan stands in the town where its CopyBot
   stands, the town's own song playing, the act's hour on it ([Time of
   day by act](home/time-of-day.md)). The run is saved there.
4. **The town.** Three of its devices are live this run, each marked.
   Up close MegaMan reads each by its kind before he goes in.
   The town's people each have a line for a Navi walking by. L asks Lan,
   who answers from his room in a line.
5. **Back in.** R at a live device takes him into its comp: one small
   layer in that comp's own look. The other two go dark. The comp's exit
   pad leads to the act's next layer, as the left layer's would have.

**Who knows what.** Lan sees the town only through the CopyBot's eyes,
so he asks and MegaMan tells; MegaMan reads a device's signal up close,
never from the net. The townsfolk know nothing of the Endless Net beyond
what home's people say ([What MegaMan
knows](../game/meta/what-megaman-knows.md)).

### The doors

Three live devices: the town's own jack-in points first, then ports at
its checks' ground, which `town_ports.c` already makes for the jobs'
errands ([Jobs](home/jobs.md)). A device's comp is BN6's own where it
has one (unit 1's reading), else the nearest in kind, which
[Fidelity](../game/fidelity.md) names. Three kinds, one of each a visit:

- **Data.** Mystery Data of the act's tier, one behind a lock the run
  already knows.
- **A dealer.** A Net Dealer with three chips in the folder's codes.
- **A signal.** One battle a version above the act's, its prize chip
  named in the read.

43 comps are 43 looks, not 43 rule sets ([the core
loop](../game/core-loop.md), Rules without decisions). The numbers come
at the build through the game-design skill and `build.py economy`: a
door pays what the rest of the layer left behind would have paid a
median player, so the plan in play stays inside its bands.

### Its pace cost

Pace is the review's first lack, measured in decisions a minute.

- **The link is the layer's second exit.** Taking it gives up what is
  left of the layer and its walk; the comp's exit pad holds the battle
  the layer still owes ([the battle
  floor](../game/progression/structure.md)). A visit replaces a walk; it
  adds none back.
- **Taken**: a target of 60 to 90 seconds from the link to the comp, for
  one draft of three. The towns are large, so the CopyBot stands within
  a short walk of all three devices, measured per town by `build.py
  town`.
- **Not taken**: one fork read. The side path can be left unwalked, as
  people leave half a layer's Mystery Data.
- **Measured**: links offered and taken, the door picked and the seconds
  in town, on the run log and the run-choices event.

Taking the link whenever it shows is fine, as taking a Virus Battler is:
the decision is which door.

## The depth that can follow

Each a unit of its own, after the visit is played and counted. Each
names the smell it has to survive.

- **The town's machine.** One lever a town on the rest of the act, set
  from inside its own device. It is where the core loop's Phase 3 asks
  for a place: Sky Town's weather sets the act's battle rate (fewer
  battles and fewer finds, or more of both: random battles as a choice);
  Seaside's tanks pick which of two virus families the next layers
  field (the fight the folder answers); Green Town's Judge Tree hands
  down a vow for pay, as a job's. What each machine runs in BN6's story
  is assumed. Smell: complexity over depth. One lever a town, two
  settings, each a trade.
- **Keys and interiors.** A Mystery Data in the area can hold the town's
  pass, a key item in the PET's KeyItem list ([the PET](../game/pet.md)).
  It opens an interior (the Aquarium, the court, Sky Town's rooms) and
  the big comp's entrance in it: an optional second guardian, which
  Phase 3 asks for and [Team ProtoMan](../game/team-protoman.md) already
  designs for BN5's areas. Smell: a walk no one chose. The interior is
  one room to the door.
- **Links that stay.** A town visited lights its homepage's link in
  Lan's HP for the rest of the run ([Lan's HP](home/lans-hp.md)). Going
  there again is a trip back, priced by the Net's clock ([Going
  back](home/going-back.md)). Smell: the farm. Each device pays once a
  run.
- **A number in town.** One of the town's checks hides a number of the
  run's seed, and AsterLand's Number Trader, off today because BN6's
  codes are no secret ([AsterLand](home/asterland-and-academy.md)),
  takes the run's numbers alone, for a prize of the run's tier. Smell:
  power no run earned. The prize is the run's, never the profile's.
- **The jack-in log.** The PET keeps every device MegaMan has been
  inside, per profile: 43 in BN6, more with the other games' towns.
  Recognition, never power, and a reason to pick another door next run.
- **The endless reach.** Each Net rebuilt after its Nest surfaces
  farther from home: BN6's towns in the first, the Expo after, then the
  older Net's side (Oran Isle, the harbour, the castle town) and BN4's.
  From the second Net a copied guardian may stand in a town in a CopyBot
  of his own, its devices dark until he falls. Smell: a treadmill. A
  farther town is a new place and a new machine, never a larger number.

## What must not move

- **Home.** Central Town, Lan's room and his PC, Lan's HP and its
  portals stay as they are. Lan never travels in a run.
- **MegaMan's walk.** He walks the towns as BN6's CopyBot walks him.
- **A required town.** A run that never takes a link loses nothing it
  was owed.
- **BN6's battle and the act's guardian.** No door changes either. The
  machines change the layers, and say so before they are set.
- **Words.** One read a door, one line a person, a line from Lan on L;
  ours, in a words file, through [the voice rules](../voice/rules.md).

## What could go wrong

- **The towns are big.** Seaside Town is 132 x 96 tiles and Green Town
  184 x 104. If no spot keeps three devices within the target's walk,
  the visit lights two, or the CopyBot stands by the town's own point.
- **A second hub.** With links that stay, a long run collects towns.
  Lan's HP already shows the newest ways first.
- **The first visit teaches three things at once** (the link, the
  town, the doors). The first one a profile sees lights one door only.
- **Saves.** A checkpoint in a town rebuilds it and its doors from the
  layer's seed before it loads the state; the visit adds to `Run`
  (`RUN_MAGIC`) and to what a seed makes (`LAYER_MAKE`).
- **The New 3DS.** A town's 256 colours and tiles beside a layer's:
  `tools/heap3ds.py` before anything ships.
- **Fidelity.** A Navi jacking himself in, a CopyBot where BN6 stands
  none, and a device with a comp not its own are the engine's;
  [Fidelity](../game/fidelity.md) names each.

## The build, in order

| Unit | What | Size |
| --- | --- | --- |
| 1. The CopyBot read | The flag and the walker's sprite named in `src/emu/bn6.h` with their rows in [ROM data](../rom/world.md); what R, START and a door's warp do with the flag set; where BN6 stands its CopyBots; each town map's jack-in table against the 43 destinations | S |
| 2. The experiment | The flag set on Seaside Town (`CYBERWORLD_TOWN_STYLE=2`) and on Oran Isle's branch: captures of MegaMan walking both, all eight directions drawn, a chat opened | S |
| 3. The link | The side path's pad, its read and question, the step out, the town's checkpoint; once a run in half the runs (the unit test), a dev switch to force it | M |
| 4. The doors | Three live devices with their reads, the three kinds, the comp's layer and its exit to the next layer; the lines; the run log's counts | M |
| 5. Green Town, then Sky Town and ACDC Town | Each as Seaside came; Sky Town's start height first | S to M each |
| Later | The machines, keys and interiors, links that stay, the number, the log, the endless reach: one at a time, each through the game-design skill | M each |

## The checks

- `build.py town` with the CopyBot's spot and the three devices marked,
  the walk to each counted.
- An autopilot run from the layer through the link, the town and a door
  to the act's next layer, `layer>=2`; a CONTINUE from the town's
  checkpoint with the doors as the quit left them.
- `build.py test` (the link's share over 300 seeds, `LAYER_MAKE_HASH`),
  `build.py lint`, `python3 tools/voice_catalog.py`.
- `build.py economy` and `build.py pacing`: the plan in play inside its
  bands with the doors' pay in.
- `tools/heap3ds.py`.
- After a release: links offered and taken, the door picked and the
  seconds in town, in the statistics.

## The owner's questions

Open. Each with the recommendation the reading gives:

1. **Oran Isle.** Recommended: reuse the held branch's island with the
   arrival changed (MegaMan steps out of Oran Area's layer; its stall
   and its job become two of its doors), after Seaside Town proves the
   visit. It needs a reason for a CopyBot on the island, or it waits for
   the endless reach, where the Net surfacing under the older Net's
   places is the reason.
2. **MegaMan jacking in by himself.** Recommended: yes, as the
   engine's, unless unit 1 finds BN6 does it already. The other way,
   Lan pulling him back to the PET and sending him in from home, puts
   the cut to Lan back.
3. **How often.** Recommended: once a run at most, in half the runs
   that hold an act with a town, as the Battler's coin.
4. **The machines.** Recommended: after the visit has been played and
   its take rate read, Sky Town's first, since random battles as a
   choice is the series' most named tedium answered.
5. **Home's own town.** Recommended: no link to Central Town. If a
   CopyBot at home is wanted, it is a later question for
   [Home](home/overview.md).
