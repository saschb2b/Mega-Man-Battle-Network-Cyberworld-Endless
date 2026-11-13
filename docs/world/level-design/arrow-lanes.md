---
type: Feature
title: "Arrow lanes"
description: "Arrow lanes (issue #43): BN6's one-way arrow panels as a layer's quick way back from a far detour (`plan_lane`, `carve_lane`), each area's own arrow art (`props_learn_arrow`), and BN6's own ride."
tags: [world, level-design]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-09T10:50:00Z }
---

# Arrow lanes (built, issue #43)

BN6's arrow panels carry MegaMan one way, input held, four units a frame,
until he is past them: Seaside Area 2 is a field of them, Sky Area 3 rings
a platform with one-way lanes, Green's and Seaside Area 1's are single
panels set into a walkway. A layer's lane is the quick way back
(`plan_lane`, `carve_lane` in `net_set_pieces.c`):

- **Where:** from a free ground panel five or more off the way (where a
  long detour ends, by its data) straight across one to five void panels,
  the void beside each, to free ground floor three or less off the way,
  the walk it saves five panels or more, the most saved first. It is
  carved last, once the rest stands: neither end on an island past a gap
  or a teleport's void, in a pocket a lock closes or in the arena. So the
  walk in is long and the ride out short; walked against its arrows, its
  first panel carries MegaMan back, so it is never a way in.
- **Which areas:** by the act's plan, Seaside's the most, then Green's,
  the Undernet's, the Underground's under the Nest and the Judge Tree
  comp's; Sky's seldom (its layers seldom leave a short gap from a far
  floor back to the way: one in ten found room), none in Robot Control's
  (two ways drawn, its layers too close).
- **The art** (`props_learn_arrow` in `props.c`): each area's own arrow
  panel for each way, cut from its maps where one closes a lane a panel
  wide (the end cells on its far edge): the tiles of both layers whose
  middle lies on its diamond. An area whose maps draw a way's panel alone
  gets lanes that way only (`LayerKit.arrows`); Sky's and Green's also
  come from Sky Area 3 and Green Area 2 (`net_area.arrow_maps`). BN5's
  areas cut theirs from BN5's own maps, which mark arrow panels with the
  same values: SciLab's and the Undernet's every way, End Area's, Oran
  Area's and Nebula Area's three, ACDC Area's none ([BN5's maps, songs,
  text and objects](../../rom/bn5-world.md), Its arrow panels and ramps);
  a lane on a layer dressed as one of them is BN5's art over BN6's ride.
- **Not a walkway's look** (`find_arrows` in `area_src.c`,
  `learn_tile` in `tiles_learn.c`): a map's arrow lanes' panels are listed
  as it is loaded (`AreaSrc.arrow`: from each start cell's panel along its
  way to the panel an end cell closes), and no tile on one is learned as
  the floor's, nor is the panel lent whole. Their art is an arrow's: a
  walkway drawn from it showed an arrow where nothing carries MegaMan.
  The panels beside one still count it as the floor it is (BN5's Undernet
  joins its rooms to its walkways by single arrow panels, some thirty a
  map; read as floor of another kind they took the rooms' open rims with
  them, its breaks up by a quarter). A field of them, a 2 x 2 block or
  more, is left as it was: Seaside Area 2's and 3's currents are 150 and
  250 panels, a floor of their own, and without them Seaside's walkways
  took another map's planks (its breaks up by a fifth). Breaks per 100
  panels over all areas 89.9 to 89.6, Green 67 to 60, none worse.
- **The ride** (`nm_lanes_place` in `netmap_extra.c`) is BN6's own: three start
  cells across the lane's near edge and three end cells across its far one
  ([The net's maps and their pieces](../../rom/net-maps.md), Arrow panels), as Robot Control Comp 1's lane has
  them; the game's player update carries MegaMan from one to the other.
  The lane, its ends and the void beside it are kept as generated
  (`lock_pieces`).
- **The way on** (`layer_step_ok`): the guide's walk and the autopilot's
  step onto and along a lane only the way it runs.

Over the tests' 300 layers, 20 hold a lane (of 36 planned), 35 panels in
all.
