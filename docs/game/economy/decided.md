---
type: Decision Record
title: "Decided"
description: "The owner's yes (21 October 2026) to every economy decision: the opening plan's floors, traders picking codes as BN6 does, BN6's drops untouched, statistics' counts, bands a fifth either side, and the checks at switch-on."
tags: [game, economy]
generated: { by: claude-code/unrecorded, at: 2026-10-20T22:52:50Z }
sources:
  - id: owner
    resource: "The owner's decisions, given in the project's sessions and written down here"
    title: "The owner's decisions"
    author: human:saschb2b
---

# Decided (21 October 2026)

The owner said yes to every decision as proposed:

1. **The opening plan's shape and floors:** green data 0.5, a green
   data's chip 0.6, good chips 0.25, zenny 0.5, lean 0, bend 2; nothing
   above the flat plan anywhere.
2. **The short net** is back to the flat plan's by its own third act.
3. **The codes:** no lean on act 1's free finds; the dealer's answer keeps
   the folder's codes; the Chip and BugFrag Traders pick a prize's code as
   BN6 does (`src/layer/trader.c`: their pools' records kept as BN6 has
   them, [Chips, folders and the Library](../../rom/chips.md)'s Chip Traders, verified in Gregar,
   [Sources](../../research/sources.md)).
4. **BN6's battle drops** stay as BN6 has them: the busting levels'
   rewards uncapped.
5. **The statistics** carry the economy's counts (above).
6. **The bands** a fifth either side.

## Checked when it was switched on

- `build.py economy`: 0 of the opening plan's numbers outside their bands
  on either net; act 1 9.5 chips (from 12.2), 23% good (2.2 chips, from
  3.2), 11.6 Mystery Data, 2,630 zenny (from 3,550), three of one code
  by the guardian (from five); the sixth act as before.
- `build.py pacing`: 0 battles or guardians past their band.
- The autopilot, seeds 1-12, fresh data folders, before and after (the
  same build but the plan; the autopilot opens no Mystery Data and buys
  nothing, so it sees the battles and the layers' new shapes, not the
  thinner finds):

  | Autopilot, 12 seeds | Before (flat) | After (opening) |
  | --- | --- | --- |
  | Plain, 30,000 frames: deepest layer (median, most) | 3, 4 | 3, 3 |
  | ... deleted in act 1 | 11 of 12 (one in act 2) | 12 of 12 |
  | ... first guardian reached, beaten | 8, 1 | 11, 0 |
  | ... random battles won in act 1, their chips and zenny | 28, 11, 3,400 | 20, 13, 1,150 |
  | `god,onehit`, 40,000 frames: deepest layer (median, most) | 12, 15 | 13, 15 |
  | ... act 1: battles won, chips, zenny (guardians' included) | 38, 10, 15,000 | 36, 10, 12,800 |
  | ... act 2 | 36, 13, 18,000 | 35, 10, 14,540 |
  | ... act 3 | 35, 12, 17,350 | 31, 7, 13,780 |

  The plain autopilot loses act 1 either way (its known rate: about one
  first guardian in ten, the memory of earlier tallies); after, it
  reached the guardian more often and beat him once less, within that
  noise: a battle's timing reshuffles a seed's encounters. Its battles'
  chips are BN6's rows' and stay; its zenny falls with the guardians'
  chip-entry zenny (300 in act 1, from 600) and the battles fought. No
  sign of a starved act in what the autopilot can see; the finds it
  skips are the report's to measure.
- Not checked on hardware or in a playtest: a trade at a layer's Chip
  Trader under BN6's code rule, and how a first act feels to a player
  who opens every Mystery Data and shops (the playtest loop's next
  session is the place).
