---
type: Feature
title: "Security cubes"
description: "Security cubes (issue #45): BN6's cube across a pocket's mouth, opened by a P-Code one bystander knows or a zenny toll in Seaside (`layer_cube_kind`, `ng_place_teller`); no navi within three panels of any piece's A."
tags: [world, level-design]
generated: { by: claude-code/unrecorded, at: 2026-10-05T12:22:13Z }
---

# Security cubes (built, issue #45)

BN6's security cube (handler 3, sprite 7:0x03, its records looking the four
ways) stands across a walkway and vanishes when its condition is met. A
layer sets one where the act's plan calls for it, in every comp and
homepage, Central and Seaside (`layer_cube_kind` in `net_pieces.c`), in a
pocket's mouth as an obstacle stands (`plan_obstacle`), its one thing a
blue data of the best quality:

- **A P-Code** (comps, homepages, Central): four digits from the layer's
  seed, which one bystander knows (`ng_place_teller` in `net_set_pieces.c`). He
  stands a walk of eight panels or more from the cube and apart from
  anything else, so the key lies on another way than the lock: at the cube
  without it, MegaMan says somebody on this layer must know it, ask
  around; told it, A at the cube enters it and the cube opens. A layer
  with no such place for him takes a toll instead (a lock no one can open
  would close its pocket for good).
- **A toll** (Seaside): 200 zenny in act 1 and 100 more an act, half the
  Net Dealer's answer chip: the run's dialectic, prepare or press on, in
  zenny. The game's own zenny check takes it, or MegaMan says they don't
  have that much.

Where BN5 dresses the area ([Other games' ROMs](../../multirom/overview.md)), the cube is BN5's Security
Cube, ACDC Area's lock on a friend's homepage: a green cube with X eyes in
the walkway's mouth, centred on it as BN6's obstacles are, that opens its
eyes and flickers out. Only the look changes: the same lock, the same key.

Two rules came from the cube and hold for every piece spoken to:

- **No navi within three panels of a piece's A** (a cube's or an
  obstacle's mouth, a Rush stand; `navi_near`, `hush`): the engine's A
  turns to a navi within 52 units before a map's check, and a teller two
  panels from his cube took the cube's A. A piece is planned away from the
  navis standing, and the bystanders and the teller placed after it keep
  off the panels round it.
- **A mouth stays one panel wide:** the map's floor fix (`legal.c`), which
  toggles cells for the tiles, widened a cube's mouth on a comp to three
  panels, and MegaMan walked round the cube. The mouth, the void beside it
  and the floor before and after it are locked (`lock_pieces` in
  `netmap.c`).

Over the tests' 300 layers, 10 hold a P-Code cube, each teller eight
panels' walk or more from his, and 9 a toll; no navi stands within three
panels of a piece's A.
