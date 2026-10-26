---
type: Feature
title: "A fork's read"
description: "MegaMan reads the layer behind each way at a fork (an exit pad beside a dark warp, the warp itself, Lan's HP's portals): a Server's signal, the NaviCust vendor, a trader, the Net Dealer, a dark board, from the seed before the layer is built (`layer_holds`, `fork_words.c`)."
tags: [world, level-design, design, voice]
generated: { by: claude-code/claude-opus-5-5, at: 2026-10-26T19:56:00Z }
sources:
  - id: core-loop
    resource: "docs/game/core-loop.md, Phase 2: MegaMan reads the next layer at a fork"
    title: "The core loop, reviewed"
  - id: code
    resource: "src/net/net_services.c, src/layer/fork_words.c, src/director/director_way.c (fork_pad_read), tests/test_core.c (test_fork_read)"
    title: "The fork's read as built"
---

# A fork's read

[The core loop's review](../../game/core-loop.md) found the run's choices
sparse between its act-level decisions, and proposed reading the next layer
at the forks a layer already has rather than adding forks (a second exit
pad would double the walking): Hades' doors, which show the kind of reward
behind them, with no new screen. Built 26 October 2026.

## Where he reads

| The fork | Its ways | What MegaMan says, and when |
| --- | --- | --- |
| A layer with a dark warp (from layer 4, 12% of normal layers before a guardian's, 40% in the Graveyard) | The exit pad to the next layer; the warp to an Undernet layer whose exit leads to that same next layer | Within three panels of the exit pad, once a layer: "Past this pad..." and the next layer's two strongest calls (`fork_pad_read`, `director_way.c`). At the warp, its own talk adds "And a strong virus signal!" where the Undernet behind it holds a Server (`ta_undernet`, `service_words.c`) |
| Lan's HP after an act | Each lit portal's area, its act's first layer (one way of them may hold the act's [dark layer](../../game/liberation/overview.md) second) | Beside a portal, after the area's feel and its guardian's signal ([Lan's HP](../home/lans-hp.md)): "Out there..." and its first layer's Server, vendor or trader, nothing more where it holds none (`home_port_words`) |

The read at the pad is the exit's next layer: where that layer is its act's
dark board, "Lan... It's gone dark past here. Dark panels,like Nebula's!",
as the portal says of the way that holds one; where it is the guardian's,
"A strong Navi's signal!" first. With nothing of note: "Past this pad...
It's quiet."

## The design

Run through the game-design skill. The dialectic is the run's own: press on
or get ready (what the next layer offers the folder against what it
costs), restated at the layer's one fork. The loop layer is the session's:
a layer to a layer, where the act's ways are already chosen at home.
Patterns: **branching-dag-map** (Hades' one chamber ahead: the kind of
reward behind each door, never its contents) and an **honest telegraph**
(a read that lies is worse than none).

- **Two calls at most, strongest first**: the guardian's signal, a Server
  (the riskiest optional fight, now a pick of three: [the
  structure](../../game/progression/structure.md), Risk the player
  chooses), the NaviCust vendor, a trader (Chip or BugFrag), the Net
  Dealer. One box, two where a second call follows ("...,too!"). The words
  budget of the main path (Phase 1 cut it) holds.
- **The kind, never the contents**: no chips named, no prices; the Server's
  own talk names its three.
- **At a portal, only what differs**: every act's first layer after the
  first holds the Net Dealer, so the portal reads Server, vendor and trader
  alone, and says nothing more where the layer holds none. The guardian is
  the portal's own line already.
- **Senses, never names** (the fiction's fit, [the voice rules](../../voice/rules.md)):
  signals and calls, as BN6's MegaMan reads a Navi's signal; never a
  place he has not seen.
- **Never promises what is not built**: a guardian's last stop (its dealer
  and heal) is left unread, the last stop telling of them on the layer
  (`last_stop`, `briefing_words.c`): its dealer's counter found no room on
  one guardian layer in a few hundred.

## Known from the seed

A layer stays reproducible from `run.layer_seed`, and generation never
reads the game's RAM (AGENTS.md), so the read must be the roll itself.
`src/net/net_services.c` rolls a layer's services and gates (dealer, heal,
traders, vendor, Server, ProtoMan's duel, the dark warp, the gates) on
numbers of their own, seeded from the layer's seed and its area, with the
generator's numbers put back after; `layer_holds(seed, depth, biome,
kind)` (`net.h`) returns them as `HOLDS_*` bits without building anything.
Before, the rolls followed the floor's in one stream, so nothing short of
building the layer knew them. The area is in the seed's mix so that
Lan's HP's two portals, whose first layers share a depth and seed, read
apart. The director's `fork_holds_in` (`director_layer.c`) adds the run's
side: the layer's seed (`run_layer_seed`, `run.h`), its area, and whether
it is a dark board (`lib_layer_dark_at`), which holds none of a normal
layer's services.

This changed what every seed makes: `LAYER_MAKE` 109 and its hash
(`src/net/layer_make.h`). `tests/test_core.c`'s `test_fork_read` builds
300 layers (normal and Undernet, every act) and holds the read to the
objects built: the Server, vendor, traders, dealer and dark warp, none
read and not built, none built and not read. In the game (seed 30 at layer
4; seed 7's Lan's HP before act 3) the Undernet behind the warp held the
Server it read, and the portal's way the trader.

## Its words

`src/layer/fork_words.c` holds the calls and their openers, a words file
of its own ([the voice rules](../../voice/rules.md), Where the lines live);
the Undernet's line is in `service_words.c` with the warp's other words.
Each call is one box of six words or fewer, no flags in the catalog.
