---
type: Design
title: "The PET's start menu"
description: "The PET's start menu of ours (issue #793): BN6's own menu laid out again in two columns of six from BN6's own pieces (its buttons, labels, icons, panel, PLACE and EXIT), its list folded so BN6's press counts hold, BN6's menu run underneath to open each entry as BN6 opens it; the owner's calls and three rounds of reviews by BN experts."
tags: [game, pet, ui]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-22T15:10:00Z }
sources:
  - id: issue-793
    resource: "https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/793"
    title: "Our own PET start menu: the owner's calls, the settled design"
  - id: bn6-rom
    resource: "Mega Man Battle Network 6: Cybeast Gregar (USA), SHA-1 89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6"
    title: "The PET menu: its BG0 buttons, its icons' OBJs, its panel, PLACE and EXIT, measured"
---

# The PET's start menu

BN6's PET menu is one column of eight rows down to EXIT, and the run
needed a ninth, Forms ([The forms screen](meta/forms-screen.md)). A ninth
row squeezed into BN6's own menu (its column a tile row higher, SELECT
moved off) was built and the owner judged it on sight: it looked wrong. The
owner's call (22 November 2026): a start menu of our own, BN6's pieces
reused and BN6's own screens opened from it, with room to grow. Of three
layouts offered (two columns, one scrolling column, grouped sub-lists) the
owner chose two columns. It is the second of the owner's exceptions to the
rule that a screen BN6 has is driven, never redrawn, after
[the Library](meta/library.md).

## How it was settled

Three rounds of mockups composed from BN6's own frame of its menu, each
reviewed by three players of the series in their own roles: a BN6 pixel
purist, a BN1-6 veteran and a BN6 speedrunner. They agreed in the end on
everything below; what each round changed is in issue #793.

## What it shows

- **BN6's band** with its emblem, as BN6 draws it. **PLACE's name** sits on
  the band's right stripe in the band's navy, its letters BN6's.
- **Two columns of six places**, twelve in all, BN6's 72 x 16 buttons with
  their labels at BN6's 16-pixel pitch from y 17, each with its BN6 row
  icon 3 pixels before it (x 15 and 127). The cursor's row is lit as BN6
  lights it: its label white and its icon stepped 8 pixels out. Forms'
  button is Save's with Forms' label set from the labels' own letters
  beside Options' (`pet_options.c`); its icon is MegaMan's for now (BN6's
  Status icon, the reviewers' choice, waits on its tiles).
- **One strip of HP, ZENNY and BUGFRAGS** at y 113-138: BN6's panel cut to
  one frame, each label over its box, each box its left end and the right
  end that holds its digits.
- **BN6's foot line** straight across at y 140-141 and **EXIT's capsule** at
  BN6's own y 144-156, which pulses white while the cursor is on it, as
  BN6's does.
- SELECT, a caption only, goes: it is the price of the sixth row.

## The order

BN6's own list folded down the left column, then the right:

| Left | Right |
| --- | --- |
| ChipFolder | E-Mail |
| SubChip | KeyItem |
| Library | Options (Comm's place) |
| MegaMan | Save |
| Forms | |

So BN6's counts from ChipFolder hold at both ends: Down 1-3 SubChip,
Library, MegaMan; Up 1-4 Save, Options, KeyItem, E-Mail. New entries go
only in the middle, after Forms and before E-Mail, and none is ever shown or
hidden during a run (Forms with no Cross yet opens to an empty screen).

## The keys

- Up and Down walk the folded list and wrap (Forms to E-Mail, Save to
  ChipFolder, Up from ChipFolder to Save).
- Left and Right hop to the same row in the other column; Right where that
  row is empty, and from the right column, goes to EXIT. Left from the left
  column does nothing.
- Left or Up from EXIT returns to the entry the cursor came from.
- A opens the entry; on EXIT it closes. B and START close, as BN6's do.
- START opens on ChipFolder every time, as BN6's menu does (measured); a
  screen left gives the menu back on its entry.

## Under it, BN6

BN6's menu runs underneath and draws everything the picture shows
(`src/director/pet_menu.c`, `pet_menu_frame.c`). Each frame the menu takes
input, BN6's cursor is set to the entry's own BN6 row (Forms holds
MegaMan's, EXIT BN6's 8), so BN6 lights that row and pulses EXIT itself;
the d-pad is ours and never reaches BN6. A on an entry is passed to BN6 on
its own row, so ChipFolder, SubChip, MegaMan, E-Mail and KeyItem open as BN6
opens them, with its sound and its transition, and Library, Options and Save
through `pet.c`'s hook as before; A on Forms opens the forms screen. Where
BN6 moves its cursor itself (the menu opened, a screen given back) ours
follows it.

The picture is BN6's own frame of its menu laid out again
(`pet_menu_frame.c`), the way the run already recolours the core's picture
for a liberation's board: the buttons drawn from BN6's label tiles and
palette 15 in VRAM, the icons from their OBJ tiles at the height BN6 gives
each (its bob kept), the strip, PLACE's name and EXIT copied from BN6's
frame of its panel. It stands while BN6's menu takes input, closes, fades to
an entry's screen or comes back from one; while BN6's slides in the first
frames are BN6's own, and the strip waits until BN6's panel is in its place.
In the Net Square the PET's menu stays BN6's own ([The PET in the Square](../online/square/pet.md)).

## Not yet

- Forms' icon as BN6's Status icon (MegaMan's row's for another Navi),
  which BN6 loads only for another Navi: its tiles from the UI sprites' list
  grown by a slot, or from the ROM's sprite.
- The longest place name a run prints measured against the band's stripe
  (about 100 pixels, 12 letters), held by a test.
- BN6's held-key repeat matched at the list's ends, and the frames from
  START to the folder measured against BN6's.
