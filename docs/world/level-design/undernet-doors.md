---
type: Feature
title: "The Undernet's doors"
description: "The Undernet's doors (issue #47): on Undernet layers a number door asking how many braziers burn, sealed by a wrong answer, or a skull door opened by BN6's WWW-ID from the Net Dealer (`layer_cube_kind`)."
tags: [world, level-design]
generated: { by: claude-code/unrecorded, at: 2026-10-02T17:51:06Z }
---

# The Undernet's doors (built, issue #47)

BN6's Undernet locks its spurs with its own doors, the security cube's
records with their own words ([The net's maps and their pieces](../../rom/net-maps.md), Link Navi obstacles and
security cubes): skull doors, which only a WWW-ID passes, and number
doors, which ask a count the map itself holds ("count the flames of
hatred": Undernet 2's braziers). A layer of the Undernet sets one where
the act's plan calls for a cube (`layer_cube_kind` in `net_pieces.c`):

- **A number door** on an act's first layer and on a dark warp's, else
  half the time: it asks how many braziers burn on the layer, three
  numbers in a row to pick from (the right one first, second or third, as
  the layer's seed has it). MegaMan warns that a wrong number seals it;
  B steps back to count. Wrong, it seals itself for the layer. The
  Undernet's layers line rows of two or three braziers past their rooms'
  rims (`brazier_row`), with the pair beside its statue six to nine in
  all; a layer with fewer than two asks a P-Code instead.
- **A skull door** otherwise: it opens for BN6's WWW-ID (key item 0x44),
  one of which opens every skull door of a run. The act's Net Dealer
  stocks it where a skull door lies ahead (at 2200 zenny in the Undernet's
  act, two layers' worth) and says what it opens; never on an act's first
  layer, before his. The keys now come before the SubChips on his list of
  eight: an Unlocker had taken the last place and left the WWW-ID off.
