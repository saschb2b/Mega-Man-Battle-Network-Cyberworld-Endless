---
type: Feature
title: "Teleports"
description: "Teleports (issue #44): BN6's paired gem pads in Green, Sky and Central, a quick way back from a long detour or the only way to a 4x4 island with blue Mystery Data (`ng_plan_teleport`, `mapslot_teleport`)."
tags: [world, level-design]
generated: { by: claude-code/unrecorded, at: 2026-10-05T12:22:13Z }
---

# Teleports (built, issue #44)

BN6's gem marks a teleport pad: step on it and MegaMan beams to its pair
within the map, the camera scrolling along (warp-list departure 12; 18 gems
in the surface maps, all in pairs: Green Area 1's four pairs, Sky's three,
Central's one). A layer's pair (`ng_plan_teleport`, `ng_carve_teleport_island` in
`net_set_pieces.c`) is one of two things, in Green, Sky and Central, where the
act's plan calls for one (`net_pieces.c`) and the area's maps give the gem's
art:

- **A quick way back:** one gem where a long detour ends (7 panels off the
  way or more), the other 2 to 6 off the way, the walk between them 16
  panels or more, so the long way in has a short way out.
- **Else an island of its own,** as ACDC's homepage keeps an isolated square
  reached by its teleport alone: a 4x4 pad past the void (two void panels
  round it, inside the camera's window, the nearest such to the gem near
  the way, so it is seen from there), its gem a panel in from its top
  corner and one blue Mystery Data on its bottom corner, straight below
  where MegaMan lands. He lands on the gem's rim below it (the warp's
  facing 4), still beside its trigger, so a step up and to either side
  beams him back: on a 3x3 island every corner lay that way, its data
  reached only round the gem's rim, half over the void.

Each gem stands on a panel with floor all round it (its art draws its own
diamond over them) and never on or beside the way (its trigger reaches a
cell into the panels round it): no one is warped walking by. The pads'
triggers take warps 2 and 3, each leading to the other's middle
(`mapslot_teleport`, the layer map's warp list). Over the tests' 300
layers, 12 hold a pair, 11 of them to an island (21 before the obstacles
shared the act's budget).
