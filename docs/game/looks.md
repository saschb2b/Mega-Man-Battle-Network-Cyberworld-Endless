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
    resource: "docs/rom/megaman-colours.md"
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
  gold), the channels his art separates (step 1, [MegaMan's
  colours](../rom/megaman-colours.md)). Each takes any colour of any
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

## The colours screen

Colours, a row of [the PET's start menu](pet-menu.md) after Forms, opens
it outside battle. BN6 has no screen for it, so it is ours, in the PET's
look and from its pieces ([The PET's screens](../engine/ui.md)), like
[the forms screen](meta/forms-screen.md) beside it.

Reasoned with the game-design skill:

- **The dialectic** is the epic's, what you chose against what you did:
  free picks for expression, earned palettes as trophies. The screen
  shows both: every owned colour is one press away, and a palette not
  owned is in sight with where it comes from.
- **The loop layer** is the meta layer's (a milestone's reward that is not
  power) and the Square's social one. Nothing on the screen touches a run.
- **The patterns:** meta-progression that unlocks variety, not power;
  feedforward (the cursor recolours him before A commits); undo over
  confirm (B in the grid puts the colour back, and every pick can be
  picked again); and a locked goal shown, not hidden, its state said in
  words and a mark, never by colour alone.

What it shows, at 240 x 160:

- **The band** names it COLOURS.
- **STATUS's slot** on the left holds BN6's own MegaMan, read from the
  player's ROM, on a loop that shows him whole (`looks_preview.c`): his
  field sprite walking round all eight directions, front, side and back,
  then his battle body standing, firing the buster, throwing and firing a
  charged shot; each sprite in its own palette as the look makes it
  (`look_palette_of`), the saved look's letter under him.
- **The list** in a window: `Look` with the look worn (A, B or C) between
  Options' orange pointers, then each colour channel (Armour, Suit,
  Accent) with its colour in a swatch.
  The orange arrow is on the cursor's row.
- **The palette's window**: the palette's name on its band between the
  Library's green page arrows, its sixteen colours in a 4x4 grid, and
  PLAIN under them, the channel as BN6 draws it. The channel's colour is
  framed white, as STATUS frames its slot; in the grid EQUIP's orange
  corners are the cursor. A palette not owned is dimmed with LOCKED
  across it.
- **KeyItem's INFORMATION box** says what the cursor is on: the look worn;
  a channel's parts and colour; a colour's palette and where that palette
  came from; a locked palette's name and where it comes from (the words
  in `src/looks/looks_screen_text.c`).

The keys, as BN6's screens use them:

- Up and Down walk the list; Left and Right on `Look` change the look
  worn, at once. A on a channel goes into the grid on its colour.
- In the grid the d-pad moves the cursor, four across and four down, and
  Down from the last row reaches PLAIN. MegaMan takes the colour under the
  cursor as it moves, in the slot and in the game. A picks it and goes
  back to the list; B puts the colour back as it was.
- L and R page through all twelve palettes, on the list and in the grid.
  A on a locked palette's colour buzzes, and he keeps his colour.
- B on the list (or START) leaves, the look kept, as BN's screens keep
  what was set. Each press has BN6's sound: the cursor's, A's, B's, the
  buzz.
- A tap takes a row, a cell or a page arrow; a tap on the grid's cursor
  is A.

Each change is applied in the screen's update (`look_apply`, never in its
draw), so the field behind the PET and the chat box's face wear it as soon
as the screen closes. The second screen (the 3DS's bottom, a phone's
panel) shows nothing of it: it keeps what it shows under the PET. The
state in words has `look A armour Classic/13 suit plain accent plain`
(the look worn, always) and, while the screen is up, `looks at armour grid
page Classic cell 13`; the scenario `tests/scenarios/home_looks.py` picks a
colour and tries a locked one ([Scenarios](../devtools/scenarios.md)).
