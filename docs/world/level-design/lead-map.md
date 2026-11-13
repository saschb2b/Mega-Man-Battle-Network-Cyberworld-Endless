---
type: Feature
title: "The lead map"
description: "The lead map: which of an area's original maps a layer's look follows, by the layer's seed (`lead_books`, `NetAreaDef.leads`); an area's own map led every layer, now in twelve areas another of its maps may, each measured as the lead and allowed where its layers drew no worse."
tags: [world, level-design, tiles, variety]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-13T22:30:00Z }
sources:
  - id: owner-concern
    resource: "the owner in the session, 13 November 2026"
    title: "Variety across the whole board, not map by map"
  - id: measure
    resource: "docs/devtools/map-use.md"
    title: "Where the unused tiles lie, every area"
---

# The lead map (built, twelve areas)

Eighteen areas learn their tiles from two to four of the original maps:
Seaside's three, Sky's two, the Judge Tree's four. An area's layers were
all drawn in one look, because one map led:
where two maps' tiles fit a place alike, the first book's won (its panel
in `panels_lay.c`, its pair at the same distance in `tiles_pick.c`), and
the first book was always the area's own map's.

The owner asked for variety across the whole board, not one map at a time
(13 November 2026). The
[measure of what a layer takes from a map](../../devtools/map-use.md)
showed no one kind of art that holds most of what layers leave out, and
[roads](roads.md) reach three areas. Which map leads reaches every area
with more than one.

## What a layer does

A layer takes its lead by its seed: the area's own map, or one of its
other maps that may lead (`NetAreaDef.leads`, a bit a map of its `more`).
The lead's books come before the others' (`lead_books`,
`src/map/netmap.c`); nothing else changes. The layout, the legalizer's
cells and what stands where are as before, and no tile is drawn that the
area did not draw already somewhere: only which of two fitting looks wins.

What it changes to the eye, by the area: Sky's walkways are its glass
catwalks under one map and its framed squares under the other; the
Cybeast's Nest loses its claw marks; Mr. Weather Comp's field is lit or
plain; the Judge Tree's side faces carry its green plates. In others it
is a shade (Seaside's field, End Area's walkways).

## Which maps may lead

Not every map leads well. BN4's ACDC Area 2 as the lead put its one-way
arrows' marks on every walkway. So each map was measured as the lead of
all of its area's layers (the dev atlas with `CYBERWORLD_LEAD=N`, two
seeds a layout) against the area's own, by the breaks in the layers'
pictures ([the tile test](../../devtools/tile-test.md)), and allowed
where they rose by no more than a twentieth; each allowed one was then
looked at.

| Area | Map 2 | Map 3 | Map 4 |
| --- | --- | --- | --- |
| Seaside | -5%, leads | -9%, leads | |
| Sky | -12%, leads | | |
| The Cybeast's Nest | -38%, leads | | |
| Aquarium Comp | +7% | -1%, leads | -1%, leads |
| Judge Tree Comp | -20%, leads | -64%, leads | -11%, leads |
| Mr. Weather Comp | -2%, leads | -5%, leads | +16% |
| BN5's ACDC Area | -3%, leads | | |
| BN5's SciLab Area | +2%, leads | -2%, leads | |
| BN5's End Area | +2%, leads | -3%, leads | |
| BN5's Nebula Area | -4%, leads | | |
| BN4's Town Area | -2%, leads | | |
| BN4's Undernet | -3%, leads | | |
| Central | +15% | | |
| Undernet | +18% | | |
| Robot Control Comp | +30% | +39% | |
| BN5's Undernet | +14% | | |
| BN4's ACDC Area | +12% | +29% | |

(The change in breaks per 100 panels with that map as the lead. BN5's
Oran Area's second map as the lead drew its layers the same.) In five
areas another map leads better than the area's own; the own map still
leads its share of layers, as the area's first look.

## Not done

- **The areas of one map** (21 of 39) have no other lead.
- **A lead by the act or the depth** rather than the seed (an area's
  second visit in another look) is a design call not made.
- **The lists are measured, not derived**: a change to how tiles are
  picked can move a map across the line. `CYBERWORLD_LEAD` measures it
  again.
