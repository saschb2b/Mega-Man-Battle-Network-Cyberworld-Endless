---
type: Design
title: "Identity"
description: "Layer identity (issue #98): what repeated before, each layer's signature room drawn from its area's pool (`net_signature.c`), the way run through it and its landmark, and an act's three layers kept apart (`net_plan.c`)."
tags: [world, level-design]
generated: { by: claude-code/unrecorded, at: 2026-10-06T13:05:29Z }
---

# Identity (built, issue #98)

The owner, after several layers: random, but it soon feels the same; each
layer should be one to remember, as Warframe's tilesets are.

## What repeated

Measured over 60 runs' acts per area (three layers each, depths 4-6: 180
layers an area; a study kept out of the repository) and the tile test's
layers:

- **No room stood out.** The biggest room of a route, hub, ladder, web,
  comb, trail or catwalks layer was 18 to 30 panels; only fields, slabs
  and crosses held one of 49 or more. Nothing on most layers was a place
  to name.
- **No landmark.** The areas' set pieces (the giant tree, the statue, the
  monument) stood on none of Central's, Seaside's, Sky's and the Nest's
  layers, 3% of the Undernet's, 2% of the Secret Area's and 41% of
  Green's; only the Graveyard's monument came often (95%).
- **Acts that repeat.** Where an area has two layouts (Seaside, Sky,
  Green, the Graveyard, the Nest, Robot Control) every act repeated one,
  its first and third layers the same kind; the Aquarium's, the Judge
  Tree's and Mr. Weather's comps have one layout, all three alike.
- **The same outlines.** A layer's floor on the screen and the most alike
  other layer of its area covered the same cells to 56-79% (their
  intersection over their union); every hub 99-100%, crosses 87-91%.
- **The same walk.** MegaMan arrives at the top of the screen on 87-100%
  of layers, the exit in the bottom 40% on 83-98%.

The variety was in the wiring (which platform joins which, where a
walkway bends), which a player does not see. Kate Compton's bowls of
oatmeal: every one different, none of them memorable.

## What Warframe teaches

Sources: the [Tile Sets](https://wiki.warframe.com/w/Tile_Sets) and
[Orokin Moon](https://wiki.warframe.com/w/Orokin_Moon) articles of the
Warframe wiki; Digital Extremes' level designer and layout artist [on the
Grineer Sealab](https://www.warframe.com/news/let%E2%80%99s-meet-tim-and-erwin)
and [on the ice planet](https://www.warframe.com/news/building-warframe-ice-planet-1);
Daniel Brewer's [GDC 2013 talk](https://mcvuk.com/development-news/gdc-13-handling-ai-in-procedural-levels/)
on its procedural levels; Kate Compton [on generators](https://www.gamedeveloper.com/design/practical-procedural-generation-for-everyone-).

- A tileset is a few dozen built rooms with roles (the start, connectors,
  big intermediate rooms, the objective, dead ends with loot, the exit)
  strung on a template: a path that is mostly a line, with branches. A
  set is told by its big rooms and their features (the Gas City's
  reactors, the Galleon's rusted halls), which players learn and name;
  the connectors carry the variety.
- A set grows from one idea the eye holds: the Sealab from pods built
  round rock pillars, so that it would not become the Galleon again,
  hallway after hallway; the ice planet from a crashed ship that is its
  base and its outline at once.
- Special rooms recur: the Moon's challenge rooms turn up at random in
  its missions, now and then two in one, and each is known on sight.
- What is not built by hand is steered by measures: a map of distances
  to the objective paced the walk where placing by hand could not.
- Compton: a million things that differ where nobody looks are worth
  less than a few that look different; a big thing with smaller ones
  beside it reads as made.

For the Endless Net, in its own terms:

1. **One room to remember per layer, in its area's language** (Warframe's
   big rooms; BN6's [one landmark per map](originals.md)): its *signature*.
2. **The way runs through it**, so it is seen, and the layer can be told
   by it: past the statue, round the ring.
3. **A small pool that recurs**: two signatures or so per area, after
   its own maps, known on sight from run to run, never twice in an act
   while the area has another.
4. **Hierarchy, not more variety**: the signature is the layer's biggest
   room; the rooms round it stay what they were.
5. **Measured**: the tests hold the signature, the way through it and
   the act's variety.

## The design

Through the game-design skill:

- **Dialectic**: the familiar and the fresh. Every layer reads as its
  area, BN6's own maps; each one reads as this layer, not the last.
- **Loop layer**: the walk (finding the way by a landmark) and the act
  (three layers, three places).
- **Patterns**: the handcrafted-PCG hybrid (shapes after BN6's landmarks,
  its own props, placed by rules), landmarks for wayfinding (a place seen
  from the way, no marker), a shuffle bag for an act's signatures, with
  memory across the act's border.
- **Smells checked**: no new rule to learn and no screen; no power, so no
  treadmill; nothing BN6 lacks (each signature is one of its maps'
  landmarks, its props the originals' objects); MegaMan walks as BN6
  walks him, the layers stay joined as Capcom's are (Navigation), and a
  layer is still made from its seed alone.
- **Left to the owner**: a name for a layer (an epithet MegaMan or L
  says, a name on its card) is a call of voice and fiction (MegaMan
  senses; he does not name what he has not seen, [Voice](../../voice/rules.md)); a
  signature with play of its own (a fight, a reward) changes how the game
  plays. Neither is built.

## Signatures (built)

`net_signature.c`: each layer has one (`layer.sig`, its room
`layer.sig_room`), drawn from its area's pool:

| Area | Signatures (weight) | After |
| --- | --- | --- |
| Central | crater 50, plaza 50 | Central Area 3's field round a pit; Central Area 1's big field |
| Seaside | field 50, lagoon 50 | its one great field with ragged edges; the same round a pool of the void, where Seaside Area 2's field holds a field of its arrow panels ([Arrow lanes](arrow-lanes.md)) |
| Sky | ring 50, plaza 50 | Sky Area 3's ring road (round its coloured fields; here round a sunken middle, a pad in it on a bridge); its pods grown to an octagon |
| Green | grove 60, plaza 40 | Green Area 2's field with the giant cybertree at its back; Green Area 1's grass field with holes in it (seen in its map) |
| Graveyard | slab 60, ring 40 | its great slab with a line of holes, the monument past its back; a slab round one hole (its loops are holes punched in its slabs) |
| Undernet | court 60, cross 40 | a square plateau holding the statue between its braziers; a great plus |
| Secret Area | court | |
| Nest | cross 50, slab 50 | the Underground's plus round its raised block; its slabs |
| Comps, homepages | plaza 50, cross 50 (Robot Control 60, 40) | a plaza square, octagonal on the homepages, ragged on the Judge Tree; the Aquarium's a glass pool its pads' size, Mr. Weather's and CopyBot's a plaza alone (a hole or a plus in Mr. Weather's slab its art draws in pieces) |

- **Its size**: 8 to 13 panels a side by the layer's size (a ring 11, a
  plus 11 or 13, Seaside's fields 10 to 12).
- **None where the art has no big room**: BN5's Science Labs and Oran
  Isle draw every room as their small hubs (`LayerKit.hub_rooms`). A
  signature bigger than those came out flat and in pieces of other
  surfaces, and one of seven a side drew its tiles off on a fifth more of
  the floor (over 18 layers each, 59.0 tiles off per 100 panels against
  47.4 in the Science Labs, 26.0 against 21.0 on Oran Isle) and stood out
  from nothing: their layers keep none.
- **Its place, the layout's heart**: a route's middle platform (on a
  guardian's layer one sooner, the arena past it), the field itself, a
  field across every plank of a ladder where the way's rungs cross, a
  hub's centre, one of the slabs, a web's middle plateau, the lattice's
  middle block, a hole in the catwalks' maze, the plaza at a comb's
  middle catwalk, a room on a trail's middle leg. The first eight tries
  at a layer keep its layout until the signature fits it; then any of
  the area's layouts, with one where it fits.
- **The way runs through it**: the exit, or a guardian's arena, is the
  room farthest from the arrival of those every shortest walk reaches
  only through the signature (`ng_walk_past_signature`).
- **Its props**: the landmark goes to the signature first (Props, Sets):
  the giant tree to a grove, the statue to a court (past its back rim,
  else in a walled hole in its floor, the braziers in holes beside it, as
  BN6 sets most of its statues, gravestones and boards: 33 of 39 in
  walled holes, Props), the monument to the Graveyard's. Central
  and Sky frame their signature with an avenue of cybertrees past its
  back rim, mirrored with a panel's gap in its middle, as Green Area 2
  frames its giant tree; Seaside's BBS stands at its field. No counter
  stands in the landmark's room, and the landmark is set before the
  services.
- **Its open floor is no detour's end**: a plaza's corners and a ring
  road's far side are seen from the way (No long detour left empty).
- **Drawn**: where an area draws its small platforms and arena as pools
  (the Aquarium), the signature is one too.

`build.py atlas` names each layer's signature in its report, the dev
step `sig` (`--input "0:sig"`, play.py's `sig`) puts MegaMan in it, and
play.py's state says where it lies.

## An act's three layers (built)

`net_plan.c`: an act draws its layers' signatures from the area's pool
without replacement, by their weights, as it draws its layouts; where an
act would open in the layout or the signature the act before closed in,
its draw turns on by one; and where it would close in its first layer's
layout and signature both (two layouts and two signatures, each drawn in
turn), its second layer takes the first's signature and the third the
other, so the guardian's layer differs from the one before it in both
should its layout not fit and another be built. All of it from the run's
seed and the depth, as before; side layers (an Undernet detour, the
Secret Area, a trip back) draw theirs from their own seeds.

## Measured

| | Before | After |
| --- | --- | --- |
| A signature (the tests' 40 runs of 18 layers) | | 720 of 720 layers, the way through it on 682, the most floor of the layer's rooms on 708 |
| The biggest room's box (the study) | 18-30 panels but on fields, slabs and crosses | 49-112 panels, the signature (in the catwalks' mazes 32-49) |
| A layer's layout and signature twice in an act (the study) | Seaside, Sky, Green, the Nest, Robot Control 1.0 an act, the Graveyard 0.98, the Undernet 0.12 | 0, 0, 0.02; the Judge Tree 2 to 1; the Aquarium and Mr. Weather still 2 (one layout, one signature) |
| Two layers in a row alike (test runs) | | 4 of 720 (a layout that did not fit, rebuilt as another) |
| The landmark (the study) | Green 41%, Graveyard 95%, Undernet 3%, Secret 2% | 82%, 100%, 49%, 98%; at the signature on 173 of 188 test layers that hold one |
| The Undernet's statue (the tests' 30 Undernet layers) | 1 | 16 |
| Outlines alike (the study) | 0.56-0.79 | 0.57-0.82 |
| The exit in the bottom 40% (the study) | 83-98% | 79-96% |
| Tiles (build.py tiles, all areas) | 2.7 off and 82.4 seams per 100 panels | 2.4 and 75.1; Comp B's off 0.7 to 1.5 (a plus's inner corners, which its maps never show, drawn with the nearest) |

The outlines barely moved: identity rides on the signature, not on the
floor's outline. MegaMan still arrived at the top of the screen (since
built: [Arrivals](arrivals.md)), and the areas of one layout still repeat it
(left for later, below).

Set pieces move with the floor. Over every layer of the study's runs,
arrow lanes laid 1124 before and 1312 after (the Undernet's 375 to 319),
Rush's gaps 3407 and 3228 (Sky's 655 to 529), teleport pairs 1067 and
1562.

## Left for later

- **Where MegaMan arrives**: always the pad nearest the top of the
  screen, so every layer's walk ran down it. Built since (Arrivals,
  below).
- **More layouts** for the areas of two (an act still repeats one) and
  of one (the Aquarium, the Judge Tree, Mr. Weather).
- **A name per layer**, and signatures with play of their own: the
  owner's (The design, above).
