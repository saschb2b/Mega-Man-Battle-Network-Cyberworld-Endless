---
type: Design
title: "Arrivals"
description: "Where MegaMan arrives on a layer and which way he faces (issue #106): BN6's 54 arrivals measured by `--atlas arrivals`, then a side per layer (`net_plan.c`) and a facing along the way (`arrival_facing`)."
tags: [world, level-design]
generated: { by: claude-code/unrecorded, at: 2026-10-06T13:05:29Z }
---

# Arrivals (built, issue #106)

The owner, after #98: the arrival point and the way MegaMan faces there
were a low hanging fruit for more variety, and players don't always go
top to bottom. The study of #98 had MegaMan arrive at the top of the
screen on 87 to 100% of layers, facing straight down.

## What BN6's maps do

`--atlas arrivals` (`src/dev/navstudy.c`, [Dev tools](../../devtools/overview.md)) reads where
the game itself puts MegaMan: every jack-in destination (the 43 records,
[The net's maps and their pieces](../../rom/net-maps.md), Jack-in) and every entry of every map's warp list (the
real world's groups and the internet's, [The net's maps and their pieces](../../rom/net-maps.md), Warp pads) that
lands on one of the net areas' maps (groups 0x90-0x96), but a teleport
within one map and a jack-out; each place and facing once, however many
warps lead there. Each is set against its map's floor on the screen: the
side whose edge lies within a quarter of the floor's box, else the middle;
and its facing (the WarpData's byte: BN6's eight, UP first, the order of
the player's facing probes, [Scripts, text and shops](../../rom/scripts-and-shops.md), Talking reach) against the
floor's middle. Verified on the maps drawn from the ROM, the places marked:
Central Area 1's three are its left pad (from ACDC's homepage) facing
up-right, its right pad (from the comps) facing up-left and the top beside
the arrow to Central Area 2 facing down-left, each along its walkway into
the map.

| | Top | Bottom | Left | Right | Middle |
| --- | --- | --- | --- | --- | --- |
| BN6's net areas' maps (54 places) | 11 | 12 | 13 | 12 | 6 |
| The layers before (the study's 3420) | 88% | 0% | 9% | 2% | 0% |

- **Every side about as often.** The middle's six are the pads inside a
  map that links and the Underground's holes lead to.
- **Facing along a world axis**, the screen's diagonals: up-right 12,
  down-right 13, down-left 13, up-left 15; once straight up (a comp's link
  onto Green Area 1), never straight down, left or right. The layers'
  arrivals all faced straight down.
- **Into the map**: 47 of 54, along the walkway that leaves the pad or the
  edge; the others face along an edge or stand on a pad inside.
- **Where a way leads in**: on the links' pads and beside the edge arrows
  to the next area.

## The design

Through the game-design skill: the dialectic is #98's, the familiar and
the fresh, at the walk's scale (where a layer is entered sets which way
it is walked); the loop layers the walk (MegaMan faces where the way
goes, so the first steps need no reading) and the act (three layers
walked three ways). The patterns are a shuffle bag again, with memory
across the act's border, and an honest telegraph: the facing points along
the way on. Nothing to learn, nothing BN6 lacks (its own arrivals and
facings), and MegaMan's walk untouched.

## Built

- **A side per layer** (`net_plan.c`, `LayerPlan.side`): an act's three
  layers at three of the four sides, drawn in an order from the act's
  seed, turned on by one where an act would open at the side the act
  before closed at; a side layer's from its own seed.
- **The arrival** (`choose_arrival`, `net_gen.c`): the pad (else the
  room) nearest that side of the screen: the least x + y for the top, the
  most for the bottom, the least x - y for the left, the most for the
  right. The exit and a guardian's arena are still the rooms farthest from
  it past the signature, so the way still runs through it.
- **The facing** (`arrival_facing`): along the way's first leg, the
  grid's axis it runs along most over its first five panels, as BN6's
  facing for it (grid +x DOWN+RIGHT, +y DOWN+LEFT, -x UP+LEFT, -y
  UP+RIGHT), `layer.arrive_face`.
- **Everywhere he enters a layer** the director puts him so
  (`D.start_face`): the jack-in from the exit pad of the layer before,
  Lan's HP's portals, a CONTINUE at the layer's start (elsewhere as he
  stood) and the way back from a map the run does not use.
- The atlas report names each layer's arrival side and facing; the tests
  hold all of it (`test_arrivals`).

## Measured

| | Before | After |
| --- | --- | --- |
| Arrival side (the study: 60 runs per area, acts at depths 4-6) | top 88%, bottom 0%, left 9%, right 2% | 32%, 29%, 19%, 18%; the middle 2% |
| Facing | straight down, all | up-right 23%, down-right 28%, down-left 24%, up-left 25% |
| Facing into the layer | 100% (down from the top) | 97% |
| The walk from the arrival to the goal | 35.1 panels | 33.8 (from the top or bottom 35, from the left or right 32) |
| The way through the signature | 96% | 96% |
| An act's three layers at three sides (the tests' 40 runs) | | every act; no act opening at the side the last closed at |
| An act repeating a layer: its layout, signature and side | the Aquarium's and Mr. Weather's always | none |
| Tiles (`build.py tiles`, the same 170 layers) | 24558 seams, 776 tiles drawn off | 24536, 759: the arrival joins a layout's pieces from its own room, so a layer's bridges and seams move, and per area by -5 to +10% (Mr. Weather's four layers; over six seeds each +0.5%) |

A side planned left or right is the room nearest that edge, and on a
window 29 panels across and 53 down (in walking steps) that room lies
near a corner a third of the time: the study counts such an arrival at
the top or the bottom, so left and right come out at a fifth each.

The Aquarium's and Mr. Weather's acts no longer build one layer three
times: their three layers are still one layout round one signature, but
entered from three sides and walked three ways.

## Left for later

- **Arrivals in the middle**: BN6's six (a link's pad or a hole inside a
  map) have none in the layers, which start at an edge.
- **Shorter walks from the side**: a left or right arrival's walk is a
  tenth shorter, the window's width; the act's two of them a twentieth.
