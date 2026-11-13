---
type: Design
title: "Side pieces"
description: "What an original map hangs on its floor's sides (CopyBot Comp's claws, BN4's Undernet's flames), cut whole from its second tile layer with the panel it grips (`src/map/sides.c`) and set on a generated floor's panel of the same neighbourhood; the survey of what layers still miss (issue #505)."
tags: [world, level-design]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-13T21:45:45Z }
---

# Side pieces (built for two areas, issue #505)

The originals do not stop at their floors. CopyBot Comp's walkways are
gripped by purple claws at their bends, flames stand beside the catwalks of
BN4's Undernet, the Judge Tree hangs planters on its walls. A generated
layer drew its floor from the maps' tiles and none of that: a survey of 24
areas (13 November 2026, issue #505) found the layers drawing 41% of the
maps' different tile pictures, some 6,800 of the rest lying beside a floor.
The free-standing kind, which touches no floor, is the scenery
[the generator](generator.md) sets in the void (`decor.c`); this page is
the kind fixed to the floor.

## What a side piece is (`sides_learn`, verified on the maps)

A map's second tile layer holds what it sets in front of its floor. A group
of its tiles joined eight ways is a side piece where it both grips ground
floor and hangs off it: at least 8 of the pixels it draws lie on a ground
panel's top, and three tenths or more lie off every ground panel's. That
tells the three kinds on that layer apart:

- what lies on the floor alone is a pad's centrepiece or an emblem, which
  marks a warp and is set by rules of its own ([Props](props.md));
- what touches no floor is scenery;
- what does both is a claw, a flame, a lamp on a rim.

A group is refused where the map's edge cuts it, where it is smaller than 4
tiles or larger than 160 (20 by 14 at most), where a tenth of it lies over
floor of another height (a bridge, a raised floor's own tiles) and where the
panel it grips is an [arrow lane](arrow-lanes.md)'s (the lane's frame goes
with the lane).

It is kept with the panel it grips, the one most of its floor pixels lie
on, and that panel's neighbourhood in its map: which of the nine panels
round it are ground floor. Each look is kept once, 24 an area at most.

## Where a layer sets one (`nm_paste_sides`)

On a ground panel of the layer whose neighbourhood is exactly a piece's, so
a claw cut at a bend stands at a bend and reaches into the same void. Not
on raised floor, a pad, a lane or the arena nor beside them, and not within
a panel of anything that stands on the layer (Mystery Data, a Navi, a
warp). Of the panels that fit, half are taken by the layer's seed, none
within two panels of another, 12 a layer at most; a piece goes down whole
or not at all (on the map, on its tile lattice, over nothing the second
layer already holds). It is the second layer's tiles alone: the floor under
it is the layer's own, and nothing about walking changes.

## Who is in front (`priorities`, `nm_sides_extra`; verified in the game)

BN6 draws MegaMan behind a claw on the panel it grips: its maps hold layer
priorities (section 2 of the coordinate data) there, 27 to 48 cells within
a panel of each of CopyBot Comp's claws, of the map's 434. A piece is cut
with the cells on its panel and the eight round it and they are written
with the layer's own, so he passes behind a claw on a generated walkway as
he does in the comp (placed on a claw's panel in a CopyBot layer, the claw
covers his legs; a panel back, he stands clear of it).

## Which areas (`net_area.sides`)

Taken per area as looked at, as every laying rule is
([the tile test](../../devtools/tile-test.md), Each area's laying):

| Area | What the rule cuts | Taken |
| --- | --- | --- |
| CopyBot Comp | its claws, 3 looks | yes |
| BN4's Undernet | the flames beside its catwalks, 8 looks | yes |
| Central Area | the Net Dealer's capsule counter, which means a shop | no |
| Aquarium Comp | one piece, nothing to see | no |
| every other area | nothing | - |

## Not built

- **The first tile layer's pieces.** Robot Control's pipes ending in red
  fists, the Aquarium's pillars under its glass pads, the Judge Tree's
  planters, BN5's Undernet's dark figures and Nebula Area's dripping pods
  lie on the first layer, beside or under the floor (probed on the maps),
  and the Undernet's, the Nest's and Mr. Weather's maps hold their
  background in the second, so theirs lie there too. Scenery refuses a
  piece that touches other art; the rule for these is theirs to get.
- The other kinds the survey found (floor marks, the exit arrows and warp
  discs, stairs) are listed in issue #505.
