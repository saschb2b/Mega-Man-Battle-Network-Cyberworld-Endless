---
type: Design
title: "MegaMan's look"
description: "Each profile's MegaMan in colours of its own, on Warframe's model (issue #854): three channels of his plain body (armour, suit, accent) each coloured from one of twelve palettes the profile owns, earned by milestones, the home shop and rare finds, looks only; the owner's calls of 24 November 2026 and what is built."
tags: [game, design, meta, megaman]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-24T16:20:00Z }
sources:
  - id: issue
    resource: "https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/854"
    title: "Issue #854: MegaMan's look, the design and the owner's calls of 24 November 2026"
    author: human:saschb2b
  - id: colours
    resource: "https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/pull/855"
    title: "MegaMan's colours as BN6 keeps them (step 1)"
---

# MegaMan's look

Every MegaMan was the same blue one, in a run and in the Net Square, where
only a name told players apart. A profile's MegaMan now has a look of its
own: his plain body's colour channels, each coloured from a palette the
profile owns. The model is Warframe's. It belongs to the meta layer
([the meta layer's design](meta/design.md): variety, never power) and to
the Square's social layer.

## As decided (the owner, 24 November 2026)

- **Looks only.** No trait and no play rule; folder, FORM and NaviCust stay
  the playstyle.
- **Channels and palettes.** His plain body has three channels: 0 the armour
  (BN6's light blue ramp), 1 the suit (the dark blue), 2 an accent (the
  gold), the channels his art separates (step 1, the map of
  his colours). Each takes any colour of any
  palette the profile owns. The outline and the skin never change.
- **Twelve palettes of sixteen colours,** named and coloured for where
  they are earned: Classic free (BN6's own blue and a starter spread);
  Undernet, Gregar, ProtoMan, Nebula, Tournament and Hazard by milestones;
  Central Town, Seaside and Sky at the home shop; Mystery and Glitch as rare
  finds in a run.
- **Three saved looks** a profile, free picks among them.
- **Shown everywhere:** runs, battles, chat boxes, the Square, NetBattles. A
  Cross, a Soul or BeastOut keeps its own colours; the look returns with the
  plain body.
- **The screen** is an entry of the PET's start menu ([the PET's start
  menu](pet-menu.md)).

## Built

**The model** (`src/core/looks.h`): a look is six bytes, a palette and a
colour a channel, `LOOK_NONE` (0xFF) in both for the channel as BN6 draws
it; it is what a profile keeps and what the Square's wire carries. The
palettes' colours are ours, authored as RGB for each theme in
`src/core/looks.c` and kept as the GBA's BGR555; their names are in
`looks_text.c`. `look_valid` checks a look read from a file or a peer
(ranges alone, not ownership).

**The profile** (`src/core/looks_profile.c`) keeps, at its end, the palettes
owned (a bit each, Classic always), the three looks, the slot worn and a
byte saying they are kept. A profile saved before looks reads that byte 0
and takes Classic owned, every slot BN6's own colours and slot 0, once.
`looks_grant` gives a palette and saves the profile; `looks_set` stores a
valid look in a slot (the screen offers owned palettes alone);
`looks_worn` is the selected slot's look, or the dev switch's for the
session.

**The apply** (`src/emu/look_apply.h`): `look_apply` writes a look onto his
plain palettes in the core's ROM copy, which BN6 reads raw each frame;
`look_palette_of` gives a look's sixteen colours for one of his sprites,
for a peer's MegaMan and a NetBattle's two sides. In this first part both
are declared and do nothing: BN6's colours stand.
