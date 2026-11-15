---
type: Design
title: "Heights"
description: "How a layer climbs: every original ramp surveyed (one or two panels wide, two to four long, rising 32 or 64, half a cell off the panels towards its foot), the stairs twelve areas learn from all their maps (`stairs.c`), and the floor raised on them, a room the rest meets only from its front or a terrace behind a line across the screen (`net_height.c`)."
tags: [world, level-design, rom]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-15T18:30:00Z }
---

# Heights

The owner saw a mock-up of a net map in CopyBot Comp's kit, its plateaus
reached by railed ladders, and asked why the layers were mostly flat. Two
of 39 areas learned a stair, and a layer raised at most a dead end. This
page is the survey of BN6's (and BN5's and BN4's) ramps, what a layer
learns of them, and where it raises floor. The formats are in
[the ROM's net maps](../../rom/net-maps.md) (Height); how a raised floor is
drawn and walled is in [the generator](generator.md).

## The originals' ramps (verified on the maps, 15 November 2026)

A ramp is a run of coordinate-data section-1 cells of type 0x13 (climbing
towards world +X) or 0x14 (towards world -Y), 4 units higher each 8-unit
cell. Every map of the three games' net areas was read (`stair map` lines
of `CYBERWORLD_EMU_DEBUG=1 ... --atlas DIR:all:1`, and a script over the
ROMs' coordinate tables):

| Area | Its maps' ramps |
| --- | --- |
| Green (0x92:0-1) | ladders one panel wide, two long, rising 32, both ways (16 of them); its grass stands at 32, its planks at 0 |
| CopyBot Comp (0x85:4) | two ladders one panel wide, two long, rising 32, towards -Y only; its plateaus at 32 |
| Sky (0x94:0-3) | two panels wide and two long both ways, one wide both ways, rising 32; longer ones rising 64 and 128 |
| Undernet (0x95:0-3) | Undernet 1's two by two rising 32; Undernet 3's ladders rising 64 over four panels; Undernet Zero's three panels wide |
| Graveyard (0x96:1) | ladders one wide, four long, rising 64, both ways; one three wide |
| Mr. Weather Comp (0x83:0-2, 0x85:2) | ladders one wide, four long, rising 64 |
| Aquarium Comp (0x81:0-2, 0x85:0) | stairs one panel long rising 16 to the glass pads; one three wide rising 32 |
| Underground (0x93:0-1) | a ramp in two slopes (28 over two panels, then 44 over three) |
| Central 3 (0x90:2) | rising 16, 44 and 124; not a map Central's layers learn from |
| Seaside, Robot Control, the Judge Tree, the comps and homepages | none |
| BN5 | End Area, Nebula Area and the Undernet: ladders rising 64 over four panels; Oran Area 1 two wide rising 64 |
| BN4 | Town Area and the countries: rising 64 and 32 |

No map has a ramp that climbs down the screen: types 0x12 and 0x15 (the
other two ways, which arrow cells use as shapes) stand in section 1 only
in four cells of BN5's End Area 2, none a stair. So a raised floor is
reached only from its front, grid +x or +y.

**Half a cell off the panels.** Every ramp lies 4 units off the panel
lattice towards its foot: Green Area 1's ladder runs from Y 0 to 64 where
its panels' edges are at -4, 28 and 60, so its foot reaches 4 units into
the walkway's panel, its top stops 4 short of the room's, and the step past
it (z 0, height 32, then the room's z 32) straddles that edge. Measured by
its middle, a ladder climbing towards -Y read three panels long, and none
of Green's, CopyBot's or the Graveyard's was learned; a ramp climbing
towards +X lies off the same way along X and its middle fell inside, which
is why only Sky's and the Undernet's two-by-two (both 0x13) were. A ramp's
place is now its middle moved half a cell towards its top (`middle`,
`src/map/stairs.c`).

## What a layer learns (`stairs_learn`)

Each way (0x13, 0x14) and width (two panels, one) keeps the first ramp
found that is straight: one or two panels wide, two to four long, rising a
whole panel's height or two at one even slope, a top row at its far end,
no other ramp in its box, and its tiles on the grid a generated map has
(an even number of tiles a side, the area's own map's panel edges). It is
looked for in the area's own map, then its other maps (`more`), the maps
it learns arrow panels from and those it takes side pieces from (Green
Area 2 lends Green its ladders climbing towards +X, Undernet 1 lends
Undernet Zero its two by two). A layer has one rise: a ramp of another
rise than those learned is left. Each is cut whole as before: its cells,
the walls and priorities in its box, and every tile its cells cover from
foot to top.

Twelve areas learn stairs (were two): Sky (two wide both ways), Green (ladders
both ways), the Graveyard and Mr. Weather Comp (ladders rising 64 both
ways), the Undernet and Undernet Zero (two by two), CopyBot Comp (a ladder
towards grid -x), BN5's End Area, Nebula Area and Undernet, and BN4's Town
Area and countries. Left, with why:

- **The Aquarium's** stairs rise 16, half a panel's height. A layer draws
  raised floor where flat floor whole panels up the screen would be
  (netmap.c, `floor_raw`), so its glass pads stay level with the water,
  drawn apart on their legs (side pieces).
- **The Underground's** ramp bends its slope halfway, and **Undernet
  Zero's** own and **Oran Area 1's** are three panels wide or have another
  ramp in their box.
- **Oran Area 1's** two-wide ramp draws its tiles off the grid of Oran
  Area 2, its lead map.
- **Sky's own ladders** are learned but not used: a way with a stair two
  wide keeps to it (`kit_stairs`, netmap.c). Sky Area 1's one-wide stairs
  stand among its catwalks at 32 and 64, and cut whole they came out
  notched where the catwalks' faces hang under their foot (seen in the
  tile test's stair layers); its two-wide ones draw clean, as before.

## Where a layer raises floor (`layer_raise_rooms`)

After the way is widened (the legalizer and the tiles come later), each
room but the arrival, the exit, the signature, the guardian's arena and
the room before it and a trail's legs is tried, in order:

1. **The room alone**, where every way into it comes from its +x or +y
   side: a dead end with one stair, a room on the way with a stair at each
   end.
2. **A terrace**: the floor joined to the room behind a line across the
   screen (cells with x + y, x or y under a bound one to four cells past
   the room's front), rooms and walkways together. Every way out of such
   a place crosses the line towards the front. It holds whole rooms only,
   none of those kept flat, and 45 in 100 of the layer's floor at most.

A place is raised where each of its ways in can take a stair: the
corridor straight on for the stair's length and a landing past it, void
on both sides of the stair (no floor meets a ramp mid-climb), no room's
floor under it and nothing raised or stepped on already. A corridor one
wide takes the area's ladder, one two wide its wide stair, each the other
where the area has only that (a ladder's corridor widened as before, a
wide one never narrowed: the way's widened walkways stay wide). And the
place must be clear on the screen: no other floor within a cell of where
it is drawn, nor of where its side faces hang on the way down (one panel
up the screen for a rise of 32, two for 64).

A widened landing must leave no one-panel pit beside it, on along the
corridor or across it: a stair two wide at the end of a one-wide walkway
left one, which the arrow's way at that mouth did not walk MegaMan past
(`test_arrow_mouths`), and a playtester was stuck beside such a pit in
session 76.

A place that can be raised is, in Green and CopyBot Comp (`terraced`:
their originals raise every platform over the walkways), and 85 in 100
elsewhere. Over the unit tests' 400 layers a two-wide kit raises floor on
174 of them (216 stairs), a ladder kit on 254 (375), and one rising 64
over four panels on 30 (70; its corridors must run five panels
straight); before, only dead ends with one stair two wide.

MegaMan walks the ramp as BN6 walks him: the stair is the originals' own
cells, walls and step, placed whole; nothing about his movement changes
(AGENTS.md; [the generator](generator.md), Navigation).

## Checked (15 November 2026)

- `build.py test` (`test_stairs`: every stair tops onto raised floor and
  stands on a ground landing across its width, no floor beside its ramp,
  no raised floor touching the ground but by a stair, no object on one;
  for a two-wide kit, a ladder kit and a kit rising 64).
- The atlas's stairs cropped area by area and looked over (the
  `stair pasted` lines of `CYBERWORLD_EMU_DEBUG` give each one's place on
  the picture and its world corner).
- Climbed in the game with `tools/cw.py play`, up and down, z read with
  `CYBERWORLD_STATE_POS=1`: Green's ladder (seed 2, depth 2, `--dev
  xroms=0`: 0 to 32 and back), CopyBot Comp's (the same seed: 0 to 32 and
  back), a four-panel ramp rising 64 (seed 2, depth 3 in the Graveyard's
  place, dressed as BN5's Nebula Area: 0 to 64 and back).
- The autopilot across Green's and CopyBot Comp's laddered layers (seed 2)
  to layer 3.
