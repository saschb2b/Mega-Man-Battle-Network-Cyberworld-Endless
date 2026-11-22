---
type: Design
title: "The forms screen"
description: "The forms screen (issue #774): a PET screen of ours that lists the run's Crosses in BN6's folder editor's two windows, CROSSSELECT's five and the spares, swapped freely at no cost, and the run's Souls with their kind; CROSSSELECT then lists the five in BN6's order."
tags: [game, meta, pet, ui]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-22T15:10:00Z }
sources:
  - id: issue-774
    resource: "https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/774"
    title: "MegaMan's version for a run: the owner's calls of 22 November 2026, the forms screen among them"
  - id: bn6-rom
    resource: "Mega Man Battle Network 6: Cybeast Gregar (USA), SHA-1 89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6"
    title: "CROSSSELECT's list, count, rows' art and palettes"
---

# The forms screen

A run holds up to ten Crosses (Gregar's five and Falzar's, since issue
#706) and BN6's CROSSSELECT lists five rows. Before this screen it listed
the first five held in BN6's order, so a sixth Cross won could never be
worn. Issue #774 gives each run one of MegaMan's forms
([the meta layer's design](design.md)); the owner's call there is a screen
of ours in the PET where the run's forms are seen and five Crosses are
marked for CROSSSELECT, changed freely and at no cost. It works in today's
runs too, which hold Crosses and [Souls](souls.md) at once.

## The design

Reasoned with the game-design skill.

- **The dialectic** is the meta layer's, what you bring against what you
  find, at the run's scale: the run finds its Crosses, and the five
  chosen are what each battle's Custom screen brings.
- **The loop layer** is the run's (its identity: which Crosses CROSSSELECT
  offers in every battle). Nothing reaches the meta layer.
- **The patterns:** a loadout as a budget (five rows over up to ten, BN6's
  own five); an opportunity cost (a sixth Cross is a choice, not an
  addition); a deck editor with no cost to change, the owner's call, the
  price being paid in the battle (one Cross a turn, each once a battle).
- **What could go wrong:** complexity on a first run (none: five or fewer
  held are all CROSSSELECT's, nothing to choose, and an empty run's
  screen says so); a deck left short (a change is a swap, one in for one
  out, so the five are always five where five or more are held, and no
  question is asked); a Cross won going unseen (it joins the spares, in
  sight the next time the PET opens).

## The screen

BN6 has no screen for it, so it is drawn in BN6's PET look
([The PET's screens: Battle Network's rules](../../engine/ui.md)) from
BN6's own pieces, no new art:

- **The band** names it FORMS; at its right end the count of
  CROSSSELECT's rows filled in a pill (`5/5`), as FOLDER EDIT's 30/30.
  Where the run holds both Crosses and Souls, their pages stand on the
  band as tags (CROSS, SOUL), as the Library's entries; L and R turn
  between them.
- **The Crosses' page is the folder editor's two windows**: CROSSSELECT's
  five on the left, the other Crosses held on the right (SPARE), each in
  CROSSSELECT's order. Each row is CROSSSELECT's own row, read from the
  player's ROM as it is drawn (Gregar's five; Falzar's made from it as the
  core starts, `cross_rows.c`): in colour among the five, grey among the
  spares, as CROSSSELECT greys the rows off its cursor, and in its
  "chosen" palette while it is taken up. The orange arrow is on the
  cursor's row.
- **A swap is the folder editor's**: A takes the cursor's Cross up and the
  cursor goes to the other window; A there swaps the two; B puts the
  Cross back. Left and Right go between the windows, Up and Down along
  one. With five or fewer held nothing is a spare, and A buzzes.
- **The Souls' page** lists each Soul the run holds, BN5's and then BN4's
  in their games' numbers, by its name and the kind of chip it takes to
  unite, nothing to equip (a run never holds two
  of a kind under issue #774's forms).
- **KeyItem's INFORMATION box** under them says what the cursor's row is:
  a Cross's name, the attacks that hit it twice as hard and what it does
  more; a Swap question while a Cross is taken up; a Soul's chip.
- B leaves, and START as it leaves BN6's Library; a tap takes a row, and
  a tap on the cursor's row is A.
- **On a second screen** the panel follows the cursor: the Cross with its
  weakness and what it does, and CROSSSELECT's five as they stand with
  each one's weakness; or the Soul with its kind.

## Where it opens

Forms, a row of the PET's start menu of ours, after MegaMan
([The PET's start menu](../pet-menu.md), issue #793). BN6's menu has no room
for a row of ours (its eight rows fill the column to the screen's foot), so
the owner chose a menu of our own, BN6's laid out again in two columns.

## CROSSSELECT's five

`src/emu/crossselect.c` lists a held Cross only where it is among the
five (`crossselect_active`, a hook on the list's store), and its count
(which decides whether the Custom screen shows CROSSSELECT's button)
counts only those (a hook on the count's step), so the button never opens
an empty list. The five are the director's (`src/director/pet_forms.c`):
those marked that MegaMan holds, then the first held in BN6's order until
five are. None marked is BN6's own rule, the first five held, so a run
that never opens the screen plays as before; once five are marked, a Cross
won later waits among the spares. The run keeps the marks
(`run.cross_active`, with its checkpoint), so a CONTINUE lists the same
five, and a new run starts with none. The addresses
are in [MegaMan's forms and BN5's Souls in BN6's engine](../../rom/souls.md),
CROSSSELECT.

## Not yet

- A row of its own in the PET's menu, "Forms" (the owner's call, 22
  November 2026), in place of R: BN6's eight rows are tiles of BG0 on the
  8-pixel grid, so a ninth either moves the column up a tile row (BN6
  drawing it) or is drawn by the engine at a closer pitch.
