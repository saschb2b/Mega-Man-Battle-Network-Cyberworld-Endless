---
type: Feature
title: "Invisible paths"
description: "Invisible paths (issue #46): floor drawn as void from a walkway's tip to a lonely pad with blue Mystery Data, as BN6 hides HPMemory and AirShoes, cued by a nearby navi (`place_hidden`, `carve_hidden`, `path_panel`)."
tags: [world, level-design]
generated: { by: claude-code/unrecorded, at: 2026-10-05T12:22:13Z }
---

# Invisible paths (built, issue #46)

BN6 hides floor drawn as void in Seaside Area 1, Sky Area 2, Underground 1
and Undernet 2, from a stub's tip to a lonely pad: an HPMemory, MegaCannon
S, AirShoes. A layer hides one the same way (`place_hidden`,
`carve_hidden` in `net_set_pieces.c`):

- **Where:** a Rush gap's site (`plan_gap`: a walkway's tip, else a
  platform's edge, off the way, aimed across one to three void panels at
  a 3 x 3 pad of its own), planned once the islands stand; in Seaside,
  Sky, the Undernet and the Nest, from the second act on, by the act's
  plan. Its pad holds a blue data of the best kind.
- **Drawn as void, walked as floor** (`path_panel` in `netmap_extra.c`): its
  panels stay void in the layout, so the tiles draw the void there, and
  count as floor to the map's walls, as a Rush gap's do once Rush lies in
  it, with no wall across its mouths; the legalizer leaves them as they
  are (`lock_pieces`). The guide and the maps never show it.
- **Its cue, never none:** the tip aimed at a lonely pad, and a navi two
  to eight panels from it, off the way, who says he saw a Navi walk out to
  that pad over nothing; where no navi may stand, there is no path.

Laying it turned up a bug in `cuts`, which keeps a navi from cutting the
floor: it counted every floor panel, so once any island stood (a Rush
gap's, a teleport's, this one's) every navi placed after looked as if it
cut the floor, and a P-Code's teller gave way to a toll. It now counts the
floor reached before.

With every layer made to hold one (`--dev pieces=64`), 59 of the tests'
300 layers find a site and a navi for it.
