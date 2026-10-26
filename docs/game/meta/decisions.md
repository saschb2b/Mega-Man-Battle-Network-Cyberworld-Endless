---
type: Design
title: "Decisions and phase one"
description: "The meta layer's decisions (options only, a short net by default, helpers counting toward unlocks, a setup screen) and phase one as built: unlocks, threat rungs 1-5, helpers, the Blade and Storm folders."
tags: [game, meta]
generated: { by: claude-code/unrecorded, at: 2026-10-06T09:52:44Z }
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
- The helpers are two more HPMemory from the gift, a heal on every layer,
  and gentle battles (the lower half of the band all run); since issue
  #18 a fourth, All *, every chip in * (below).
- `--setup NET,FOLDER,THREAT,HELPERS` sets a headless run's (HELPERS a bit
  each: 1 HP+, 2 Heals, 4 Gentle, 8 All *, 16 BeastOut); the weak
  autopilot wins a short net (layers 1-10).
