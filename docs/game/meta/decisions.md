---
type: Design
title: "Decisions and phase one"
description: "The meta layer's decisions (options only, a short net by default, helpers counting toward unlocks, a setup screen), phase one as built (unlocks, threat rungs 1-5, helpers, the Blade and Storm folders) and the designed Z-Canon, Busting and Lean folders."
tags: [game, meta]
generated: { by: claude-code/claude-opus-5-5, at: 2026-10-26T21:20:00Z }
sources:
  - id: owner
    resource: "The owner's decisions, given in the project's sessions and written down here"
    title: "The owner's decisions"
    author: human:saschb2b
---

# Decisions and phase one

## Decisions (28 September 2026)

1. **Stance: options only.** Unlocks add folders, Crosses, routes and
   threat rungs; MegaMan's strength resets every run.
2. **Run length: a short net by default.** Three acts and the Nest, about
   an hour; the endless cycle unlocks after a first clear.
3. **Helpers count toward unlocks.** They are an accessibility dial; the
   summary names them.
4. **Phase one starts with a setup screen** after NEW GAME, in the PET's
   style; the town hub comes later.

## Phase one, in order (built)

1. Returning players hear less: the arrival's words, the town's opening
   and the gift's offers shorten after the first runs (a playtester called
   a restart after a long run a chore).
2. The profile's unlocks and the run's setup (folder, threat, helpers,
   short or endless), saved with the run (`RUN_MAGIC` changes once).
3. The short net: three acts and the Nest, paced in [Progression](../progression/overview.md)'s
   bands; the endless cycle once the Nest has fallen.
4. Threat rungs 1 to 5 and three helpers, on the engine's pacing knobs.
5. Starting folders: Standard and two to unlock, written into the game's
   folder at the run's start (its RAM layout from bn6f, noted in
   [ROM data](../../rom/overview.md)).
6. The setup screen after NEW GAME, and the summary's line for what a run
   unlocked and what is closest.

Built as planned, with these particulars:

- The setup screen is skipped on a profile's very first run, and remembers
  the last run's choices. A run given up for a NEW GAME counts no run and
  has no summary, but what it opened opens the setup, marked NEW: a first
  run that deleted ElecMan, given up, had jacked in with no setup, so
  neither his Cross nor the Blade folder could be chosen.
- The two folders to unlock are Blade, once any guardian falls, and
  Storm, once an Aqua guardian (DiveMan, SpoutMan) does, in any run. They
  first opened with SlashMan and ElecMan, whom the net brings in act 3 or
  the Nest: a playtester's setup offered nothing to choose until a first
  win. The milestones now come in the first or second run and still
  teach the folder's style (Storm's Elec answers Aqua). BN6's five other preset folders ([ROM data](../../rom/overview.md)) wait for later
  phases: one is a Program Advance folder, one a Navi-chip folder.
- The design's other three, Z-Canon, Busting and Lean, came on 26
  October 2026 ([the core loop](../core-loop.md), Phase 2: folders are
  the meta axis players name most, and of 251 runs the statistics saw
  135 brought Standard, 76 Storm, 40 Blade). Below.
- The helpers are two more HPMemory from the gift, a heal on every layer,
  and gentle battles (the lower half of the band all run); since issue
  #18 a fourth, All *, every chip in * (below).
- `--setup NET,FOLDER,THREAT,HELPERS` sets a headless run's (HELPERS a bit
  each: 1 HP+, 2 Heals, 4 Gentle, 8 All *, 16 BeastOut); the weak
  autopilot wins a short net (layers 1-10).

## The designed three (26 October 2026)

Each a different way to play, none a stronger one (the design's stance):
its chips are BN6's commonest, in codes BN6's records give them (read at
`0x021DA8` + id x 0x2C, [ROM data](../../rom/overview.md); `test_folders`
holds each folder to them, five of a chip at most), and each gives up
something Standard keeps.

| Folder | Its 30 | What it plays like | What it gives up | Opens |
| --- | --- | --- | --- | --- |
| Z-Canon | Cannon A A B B C, CrakShot A x2, TankCan1 A, DublShot C x2, Rflectr1 C x2, AirShot * x2, MiniBomb B x2, AreaGrab B x2, HolyPanl B, Barrier A x2, Recov10 A x2 * x2, Atk+10 * x3, PanlGrab * x2 | GigaCan1 (Cannon A, B, C in a row) waits in every draw; the rest are shots in its codes | The design's few answers behind cover: two MiniBombs its only throw (that its shots stop at the first foe in the row is assumed from play, not measured) | A Program Advance used, any, in any run |
| Busting | TankCan1 R x4, HiCannon L x3, WideSwrd L x3, Sword L x3, MiniBomb L x3 R x2, Thunder R x3, Barrier R x2, AreaGrab * x2, PanlGrab *, Atk+10 * x4 | The heaviest chips act 1 allows, in two codes; Thunder holds a foe for TankCan1's wind-up | No Recovery chip: every heal is the net's (a Mr.Prog's, a find's). No LongSwrd, so LifeSword stays Blade's | An act cleared with no Recovery chip used in it (its guardian deleted; the Secret Area's master ends no act) |
| Lean | Cannon B x5, MiniBomb B x5, Vulcan1 B x5, Thunder B x5, AreaGrab B x4, Atk+10 * x3, Recov10 * x3 | Seven kinds, all in B or *: any five chips make a hand, every turn | Every chip weak and near the same; and the code lean (phase two) brings its finds in B too, so a strong find off B breaks the hands it is built on | Layer 10 reached in any run (the best depth, so a veteran's profile has it) |

The game-design reasoning, the part the design left open:

- **The dialectic** is the meta layer's, what you bring against what you
  find: Z-Canon brings a combo and finds cover; Busting brings damage and
  finds its heals in the net, which sharpens the run's prepare or press on
  (a Mr.Prog's heal is worth a detour); Lean brings whole hands and finds
  chips that would break them, a loadout as budget where the budget is
  the code.
- **Costed power, not more of it.** Standard's Recov10s heal 40 HP a
  battle at most, so Busting's cost is in its chips too: TankCan1's
  wind-up, Sword and WideSwrd's reach, and no LongSwrd. Lean's chips are
  Standard's weakest kinds; its five-chip turns are the power, its lack
  of anything sharp the cost. Z-Canon's Cannons are 40 each, GigaCan1 the
  payoff of a draw.
- **Milestones that teach the style** (the design's rule, never a count):
  a Program Advance is what Z-Canon is for, a dry act is Busting's whole
  run, and layer 10 is the middle of the net, where Lean's weak chips
  begin to need the finds it leans.
- **Measured:** the statistics' `folder` (the run's start event) names
  each; a folder nobody brings needs a reason to exist (the design's
  Measuring it).

Two milestones are seen in battle: BN6's chip use's last step
(`BN6_DARK_AFTER`, `src/director/darkbn6.c`, its one hook there) reports
MegaMan's Program Advances and Recovery chips (`meta_chip_watched`) to
`meta_chip_used`, which keeps them in the profile (`folder_feats`, and the
act of the last Recovery chip with its run's seed); a guardian deleted
asks `meta_guardian_deleted` whether its act had one. Checked in the game
with a folder of GigaCan1 and one of Recov10 (`--dev folder=320`,
`folder=154`): the profile's bit and act written as each ran. `--dev
folders` opens every folder for a capture of the setup
([the dev menu](../../devtools/dev-menu.md)).
