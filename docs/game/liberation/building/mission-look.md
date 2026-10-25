---
type: Feature
title: "Each mission's own look, as built"
description: "Each dark layer in the BN5 mission matching its area (`src/director/liberation_look.c`, `lib_look_mission`): what differs between BN5's nine boards (map, backdrop, two colour sets), which BN6 area takes which mission, and the boards' measures."
tags: [game, liberation, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-24T23:52:21Z }
---

# Each mission's own look, as built (25 October 2026)

Every dark layer stood in BN5 mission 1's map. Each now takes the BN5
mission that matches its area, its map, backdrop and colours read from
BN5's ROM as the layer is made (`src/director/liberation_look.c`,
`lib_look_mission`; [BN5's Liberation Missions](../../../rom/liberation.md), Each Liberation Mission's own look).

**What differs between BN5's nine boards** (each opened in romlab on the
plain ROM, as mission 1 was: the mission set, BN5's own warp into its map,
A tapped through its opening to the board): less than their maps suggest.
The nine maps (0x86:00-08) share one tile set and BN5's board one overlay
and its twelve looks, so the framed panels, the swirl and the looks are the
same in all nine. A mission's own is:

- **its map**: the board's shape, and the pieces round its panels where
  the map draws them (End 2's walkway joins, Nebula's stone slabs);
- **its backdrop**, one of five: ACDC 3's purple diamonds (mission 1),
  Oran 3's with green specks (2), SciLab's purple binary digits (3),
  End's magenta diamonds (4 and 5), the Undernet's and Nebula Area's purple
  static (6-9), each the map's own backdrop in a mission's colours
  (`BN5_LIB_MISSION_BACKDROPS`), and the map's own tile animation;
- **its board's colours**, two sets: the mission's GFXAnim list
  (`BN5_LIB_MISSION_ANIMS`, the lists group 0x86 takes while a mission is
  on) loads banks 1-12 over the map's as it is entered, missions 1-5 from
  `BN5_LIB_BOARD_PAL_ACDC`, 6-9 from `BN5_LIB_BOARD_PAL_NEBULA` (the two
  differ in banks 1 and 3, the frames' and pieces' shades; the panels'
  banks 8-12 are the same).

The music is the same in all nine: the board's song 0x22, started by an
immediate in the board's code and read on the music player in missions 2,
3, 5, 7 and 9 (1 before).

**Which mission an area takes.** A BN6 area takes the missions of the BN5
net area that dresses it in a run (rom.c's `like`; [Other games' ROMs](../../../multirom/overview.md)): each
mission's map is one of its areas' maps by BN5's own labels and by the
maps BN5's random battles name for that area (`NetAreaDef.xbattles`:
0x86:00 ACDC Area's, 0x86:01 Oran's, 0x86:02 SciLab's, 0x86:03 End's,
0x86:05 the Undernet's, 0x86:06 and 07 Nebula's).

| BN6 area | Dressed by | Missions |
| --- | --- | --- |
| Central Area, ACDC HP | ACDC Area | 1 (ACDC 3) |
| Green Area, Green HP | Oran Area | 2 (Oran 3) |
| Sky Area, Sky HP | SciLab Area | 3 (SciLab) |
| Seaside Area | End Area | 4 (End 2) in act 2, 5 (End 5) from act 3 |
| Undernet | Undernet | 6 |
| Graveyard | Nebula Area | 7 (Nebula 1) in act 2, 8 (Nebula 3) in act 3, 9 (Nebula 5) from act 4 |
| Undernet Zero, the Cybeast Nest | none: deeper than BN5's net | 9, BN5's last |
| the comps and homepages | none | 1, BN5's first |

An area with several takes a later one in a later act, as BN5 meets them
(the homepages named for an area take that area's by their name alone).

**How.** Each of the nine maps is a held net area of BN5's (`rom.c`,
`XAREAS_MAX` 16 for them), so what an area brings comes with it: its floor
learned from its own board's panels (`area_src_board`, already per
record), its backdrop and animations copied by `xbackdrop_map` (the
mission's lists and records, as BN5's group 0x86 takes them on a board),
its colours over the map's (`area_src.c`'s `board_colours`: the list's
script that writes bank 1's copy, 0x180 bytes, its frame's source).
`xbackdrop` copies each script and record once however many maps share it
(the nine share five backdrops and their scripts; each its own filled the
64 KB range by the fifth): the nine take 14 KB beside the six areas' 37.
Nothing is kept: every byte is read from the player's ROM.

**The boards.** A mission's map draws more of a layer's generated floor
than mission 1's did (End 2's and Nebula's maps learn joins 0x86:00 never
shows), so the boards on them are bigger (a tenth to a fifth more panels; the planner's par within three phases of before, either way), and in acts 3
and 4 the generator's barriers fail a few more of them in all 16 tries.
Where none fits on its mission's map, the layer is made again on mission
1's map, where every dark layer stood before, and only then plain
(`dark_again`, `src/director/director_layer.c`). The debug log of 8 seeds
each, `--dev dark` (kept boards; before, every one on mission 1's map):

| Mission | Area, act | Kept, before | Now (on mission 1's map) | Panels, before / now | Par, before / now |
| --- | --- | --- | --- | --- | --- |
| 1 | Central, 2 | 8 of 8 | 8 (0) | 160 / 160 | 12.5 / 12.5 |
| 2 | Green, 2 | 8 of 8 | 8 (1) | 166 / 178 | 12.2 / 12.5 |
| 3 | Sky, 2 | 8 of 8 | 8 (0) | 159 / 182 | 13.1 / 11.5 |
| 4 | Seaside, 2 | 8 of 8 | 8 (0) | 185 / 223 | 13.1 / 11.8 |
| 5 | Seaside, 3 | 8 of 8 | 8 (1) | 194 / 221 | 13.5 / 11.1 |
| 6 | Undernet, 2 | 8 of 8 | 8 (0) | 177 / 195 | 14.2 / 12.2 |
| 7 | Graveyard, 2 | 8 of 8 | 8 (0) | 168 / 176 | 11.2 / 13.8 |
| 8 | Graveyard, 3 | 5 of 8 | 5 (2) | 189 / 193 | 12.6 / 14.0 |
| 9 | Graveyard, 4 | 8 of 8 | 8 (3) | 195 / 206 | 21.1 / 19.1 |
| 9 | Undernet Zero, 2 | 8 of 8 | 8 (0) | 171 / 197 | 11.4 / 11.4 |
| all | | 77 of 80 | 77 (7) | | |

`LAYER_MAKE` 101.

**Checked:** BN5's nine boards beside ours, one mission an area (the
pull request's contact sheet); mission 1's boards the same as before on
every seed measured (the control); autopilot runs from depth 5, seed 1
(`--dev dark,god,onehit`, 36000 frames), each through its area's first
dark board to the exit and on: Central (mission 1) to depth 8, Seaside
(4, then 5 in act 3) to 10, the Graveyard (7; its act 3 board plain on
this seed, as before) to 11, SciLab dressing Sky (`x1`, 3) to 9, Oran
dressing Green (`x4`, 2) to 8, the Undernet (6) with `hp=2000` to 8
(without it MegaMan was deleted in a board's battle at 100 HP, as stage
2's runs found).
