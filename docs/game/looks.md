---
type: Design
title: "MegaMan's look"
description: "Each profile's MegaMan in colours of its own (epic #854): his colour channels, palettes earned by milestones, the home shop and rare finds, three saved looks, and the colours screen in the PET where a look is picked, previewed on BN6's own MegaMan."
tags: [game, meta, pet, ui, look]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-24T16:30:00Z }
sources:
  - id: issue-854
    resource: "https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/854"
    title: "MegaMan's look: the owner's calls of 24 November 2026"
---

# MegaMan's look

Each profile's MegaMan wears a look of its own, on Warframe's model:
colour channels on his plain body, each coloured from palettes the profile
owns. The owner's calls of 24 November 2026 (epic #854) are the design:
looks only, no play effect; palettes earned by milestones, zenny at the
home shop and a rare find, Classic free; free picks, the outline and skin
fixed; three saved looks a profile; a Cross, a Soul or BeastOut in its own
colours. Where BN6 keeps his colours and which of them a channel takes is
measured in [MegaMan's colours](../rom/megaman-colours.md).

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
- **STATUS's slot** on the left holds BN6's own MegaMan, his battle sprite
  read from the player's ROM, in his battle palette as the look makes it,
  the saved look's letter under him.
- **The list** in a window: `Look` with the look worn (A, B or C) between
  Options' orange pointers, then each colour channel (Armour, Suit, and
  Accent where the core gives it colours) with its colour in a swatch.
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
  in `src/looks/looks_text.c`).

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
