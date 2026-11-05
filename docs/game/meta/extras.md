---
type: Feature
title: "The extras"
description: "The extras: what a run adds to BN6 that a player may leave out (Liberation Missions, the tournament, the Glitch layer, the Virus Battler, the DarkChips' flames, the older nets), each on or off on the JACK-IN SETUP's EXTRAS page, all on by default, kept for the run and named on its summary; the owner's calls of 5 November 2026."
tags: [game, meta, setup]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-05T16:30:00Z }
sources:
  - id: owner
    resource: "The owner's brief and calls, given in the project's sessions on 5 November 2026"
    title: "A way to opt out of version-specific additions"
    author: human:saschb2b
  - id: code
    resource: "src/core/run.h (EXTRA_*, run_extra), src/core/extras.c, src/core/extras_text.c, src/scenes/title_setup.c, src/core/run_tourney.c"
    title: "The extras as built"
---

# The extras

The owner's brief (5 November 2026): not every player wants the
additions the run makes to BN6 from other versions and of its own, so a
player may switch them off for a run. The JACK-IN SETUP holds them on a
page of their own, EXTRAS, beside the run's choices and the
[helpers](decisions.md).

## The six, and where each is honoured

| Extra | What it brings | Off: where it is held back |
| --- | --- | --- |
| Liberation | BN5's Liberation Missions on a dark layer past some forks ([Liberation Missions](../liberation/overview.md)) | `run_dark_area` (`src/core/run.c`) and `lib_way_dark` (`src/director/liberation_way.c`): the act's second layer is a plain one, as in the acts that hold no dark layer; no special route can be found |
| Tournament | Three Navis in a row in act 3's arena, in some runs ([the tournament](../tournament.md)) | `run_bracket_at` (`src/core/run_tourney.c`), where the bracket hooks in as it is built: the owner's calls 1 and 2 placed there, and none with the switch off |
| Glitch | The rare glitching warp to a [Glitch layer](../glitch-layer.md) | `run_glitch_at` (`src/core/run_glitch.c`): the layer keeps the dark warp its seed rolled |
| Virus Battler | BN6's [Virus Battler](../../world/level-design/virus-battler.md) at a side path's end | `fill_layer` (`src/net/net_gen.c`): placed as ever, then taken away, so every other object stands where it would (`test_extras`) |
| DarkChips | The flame of darkness offering a DarkChip on each act's middle layer ([DarkChips](darkchips.md), [BN6's own](bn6-darkchips.md)) | `dark_flame_setup` (`src/director/director_dark.c`): no flame, BN5's or BN6's |
| Older nets | Other games' areas, Navis and viruses in BN6's net ([other games' ROMs](../../multirom/overview.md)) | `run_dress`, `run_xguardian_at`, `run_darkloid` (`src/core/run.c`), `visitors_here` (`src/core/visitors.c`), `loot_layer_nets` (`src/core/loot.c`): BN6's own areas, guardians, Darkloids, battles and finds; no Soul, as no Navi of BN5's is met |

Older nets is shown only where another game's ROM is read (`extra_shown`,
`src/core/extras.c`): without one there is nothing of theirs to leave out.
The tournament's line is shown though the bracket is not in runs yet, its
note saying so (the owner's call).

## The calls (5 November 2026)

1. **Which extras.** The four the owner named (Liberation, the
   tournament, the Glitch layer, DarkChips), and of those proposed, the
   older nets and the Virus Battler. The rival's duels stay without a
   switch (never forced, the duel starts on No, and his clearance opens
   official gates and TagChips), as do the Mystery Data's pieces (the
   economy's, held to its bands).
2. **The tournament's line shown now**, before the bracket reaches runs.
3. **A run with extras off counts as any run**: toward unlocks, marks
   and records, as a helped run does ([Decisions](decisions.md), 3); its
   summary names what it left out and the statistics carry it.
4. **The screen in tabs** (the owner's direction, as Options' pages):
   RUN, HELP and EXTRAS, L and R between them, NEXT at each page's foot
   and JACK IN! at the last's.

## The design

Reasoned with the game-design skill:

- **The dialectic** is the meta layer's, what you bring against what you
  find ([the design](design.md)), turned on the run's own content: a
  player decides what the net may hold before it is found. Each switch
  takes a piece of what is found out of the run's scope, never a power
  from MegaMan's: DarkChips and the older nets' Souls are power the run
  offers, and switching them off makes a run poorer, not easier. The
  tournament is the one switch that eases a run (act 3's guardian alone),
  which is why the run's summary names what was off.
- **The loop layer** is the meta's setup, feeding the run's: the choice
  is made once, before the first layer, and holds for the whole run
  (`run.extras_off`, set at JACK IN! and never changed after). A switch
  that could turn mid-run would let a player read act 3's card and switch
  its bracket away.
- **The patterns**: Celeste's Assist Mode for granular opt-outs over one
  "classic" toggle (each addition a player may dislike is its own line);
  Slay the Spire's custom runs for a run's rules set before it begins;
  defaults that ship the game as designed, all on, so a player who never
  opens the page plays today's game.
- **Against the core loop**: it adds no system and no decision inside
  the run, so the main path's pace is untouched. The review asks that a
  new system meet a player gentle or opt-in first ([the core
  loop](../core-loop.md), Outside the loop); this is the same courtesy
  from the other side, a way out of what has already shipped, and the
  statistics' `off` shows which additions players leave out.

## Records and statistics

- `run.extras_off`, a bit each (`1 << EXTRA_*`), 0 for all on: the run's
  save grew with it (`RUN_MAGIC` "CWEB"; a "CWEA" run continues with every
  extra on, `save_legacy.c`). The profile keeps the last run's
  (`last_extras_off`), which the next setup starts from.
- The summary's band names them after the helpers, "HELP: HP+  OFF:
  Glitch", or their counts where the names run long
  (`src/scenes/title_summary.c`).
- The anonymous statistics' run-start and run-end events carry `off`, the
  extras left out by name ("Glitch, DarkChips"), "none" for none
  (`setup_json`, `src/analytics/analytics.c`).
- The week's net plays with every extra on, its setup standard
  ([the week's net](weekly-net.md)).

## What a seed makes

A layer seed makes the same layer with every extra on as before, so
`LAYER_MAKE` stays. With one off, only what it holds back changes: the
Glitch warp's layer keeps its dark warp, a dark layer's act its plain
second layer, a Battler's layer loses him alone (`test_extras` makes
each Battler's layer both ways and compares the rest), and with the older
nets off the acts take BN6's own areas, as a run whose seed dressed none
does.

## The screen

The JACK-IN SETUP in tabs as Options' pages ([the engine's
screens](../../engine/ui.md)): `look_tabs` draws both. RUN holds the net,
the folder, the Cross and the threat rung; HELP the helpers; EXTRAS the
six, each On or Off between BN6's small arrows. Three rows are in sight,
the rest scrolled with the window's bar, so that the information box
keeps its three lines. A starts on each page's NEXT, so A by A goes
through every page to JACK IN!; START jacks in from any page, its hint
on the band. `--setup NET,FOLDER,THREAT,HELPERS,CROSS,OFF` sets a headless
run's extras (OFF a bit each, in EXTRA_*'s order: 1 Liberation, 2
Tournament, 4 Glitch, 8 Virus Battler, 16 DarkChips, 32 Older nets).
