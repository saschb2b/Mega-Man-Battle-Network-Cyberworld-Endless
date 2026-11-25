---
type: Design
title: "The PET's start menu"
description: "The PET's start menu of ours (issue #793): BN6's own menu laid out again as one grid of two columns from BN6's own pieces (its buttons, labels, icons, panel, PLACE and EXIT), EXIT its last place, over the field darkened as BN6 darkens it, its list folded so BN6's press counts hold, BN6's menu run underneath to open each entry as BN6 opens it; the owner's calls and four rounds of reviews."
tags: [game, pet, ui]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-25T18:10:00Z }
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
purist, a BN1-6 veteran and a BN6 speedrunner. The owner judged the third,
built: EXIT took a row of its own with nothing left of it, and the gaps
were all different. He also pointed at what BN6's menu shows through it,
the field (measured: BN6's PET takes 16 off each 5-bit channel of the
field's colours, its teal only the backdrop darkened). The fourth round
answered both, reviewed by the three and a visual designer; what each round
changed is in issue #793.

## What it shows

- **The field** as it stood on the map when START was pressed, darkened as
  BN6's PET darkens it, behind everything below the band.
- **BN6's band** with its emblem, as BN6 draws it. **PLACE's name** sits on
  the band's right stripe in the band's navy, its letters BN6's.
- **One grid of two columns**, BN6's 72 x 16 buttons with their labels at
  BN6's 16-pixel pitch, each with its BN6 row icon 3 pixels before it. A
  column holds as many rows as the entries and EXIT fill (six for ten
  entries), so no row stands half empty; the grid stands in the middle,
  19 pixels from each side. The cursor's row is lit as BN6 lights it: its
  label white and its icon stepped 8 pixels out. Forms' button is Save's
  with Forms' label set from the labels' own letters beside Options'
  (`pet_options.c`), its word's tiles the engine's own and drawn by the
  picture, never loaded into BN6's video memory, where the Chip Folder's
  screens use the tile past them as their blank
  ([The PET menu's labels](../rom/pet-and-second-screen.md)); its icon is BN6's Status icon, the one BN6 shows in
  MegaMan's row for another Navi. Colours' button ([MegaMan's look](looks.md),
  epic #854) is made the same way, its word set from the same letters (C
  and r ChipFolder's, l ChipFolder's, o Comm's, u and s Status's), its
  icon MegaMan's own, his colours being what it opens.
- **EXIT** in the grid's last place, under Save and beside Forms: BN6's
  capsule, its plain body repeated so it spans the cell from the icons'
  column to the buttons' right edge, its letters in the labels' column. It
  pulses white while the cursor is on it, as BN6's does.
- **One strip of HP, ZENNY and BUGFRAGS** the grid's width: BN6's panel cut
  to one frame, each label at its box's left as BN6 has it, each box its
  left end and the right end that holds its digits (HP's BN6's width, room
  for 1000/1000; BUGFRAGS its label's; ZENNY the rest).
- The band, the grid, the strip and the screen's foot stand an equal gap
  apart (13, 14 and 14 pixels with five rows).
- SELECT, a caption only, goes: the band's gap stays clear.

## The order

BN6's own list folded down the left column, then the right:

| Left | Right |
| --- | --- |
| ChipFolder | E-Mail |
| SubChip | KeyItem |
| Library | Options (Comm's place) |
| MegaMan | Save |
| Forms | EXIT |
| Colours | |

So BN6's counts from ChipFolder hold at both ends: Down 1-3 SubChip,
Library, MegaMan; Up 1-4 Save, Options, KeyItem, E-Mail. New entries go
only in the middle, after Forms and before E-Mail, and none is ever shown or
hidden during a run (Forms with no Cross yet opens to an empty screen).
Colours, the tenth, added the sixth row, room for eleven and EXIT, and the
gaps closed to 9, 8 and 8 pixels; Down 5 is Colours and Up 5 from
ChipFolder too, across the fold.

## The keys

- Up and Down walk the folded list and wrap (Colours to E-Mail, Save to
  ChipFolder, Up from ChipFolder to Save). EXIT stays out of it, as out of
  BN6's column, so BN6's counts hold.
- Left and Right hop to the same row in the other column. Right from Forms,
  beside EXIT, from Colours, under it, and from the right column goes to EXIT, as BN6's Right
  reaches EXIT from any row. Left from the left column does nothing.
- On EXIT, Up goes to Save, over it, and Left to Forms, beside it.
- A opens the entry; on EXIT it closes. B and START close, as BN6's do.
- START opens on ChipFolder every time, as BN6's menu does (measured); a
  screen left gives the menu back on its entry.

## Under it, BN6

BN6's menu runs underneath and draws everything the picture shows
(`src/director/pet_menu.c`, `pet_menu_frame.c`). Each frame the menu takes
input, BN6's cursor is set to the entry's own BN6 row (Forms and
Colours hold MegaMan's, EXIT BN6's 8), so BN6 lights that row and pulses EXIT itself;
the d-pad is ours and never reaches BN6. A on an entry is passed to BN6 on
its own row, so ChipFolder, SubChip, MegaMan, E-Mail and KeyItem open as BN6
opens them, with its sound and its transition, and Library, Options and Save
through `pet.c`'s hook as before; A on Forms opens the forms screen, on Colours the colours
screen (`src/looks/`). Where
BN6 moves its cursor itself (the menu opened, a screen given back) ours
follows it.

The picture is BN6's own frame of its menu laid out again
(`pet_menu_frame.c`), the way the run already recolours the core's picture
for a liberation's board: the buttons drawn from BN6's label tiles and
palette 15 in VRAM, the icons from their OBJ tiles at the height BN6 gives
each (its bob kept), the strip, PLACE's name and EXIT copied from BN6's
frame of its panel. The field behind is the core's picture kept on the map
while START is down there (in the player's hand or among the keys the game
was given), the last of them the one BN6's PET opens over, darkened by
BN6's 16 a channel once a menu; where none was kept, BN6's teal. Until
0.24.1 the map's picture was kept every other frame, 150 KB copied for a
menu opened once in a while. The picture reads nothing of the game as it is
drawn: the game's mode, BN6's menu's state and, while the menu is up, its
label tiles, icon tiles and palettes are read after the game's frame
(`pet_menu_see`), since a read while drawing waits for the next frame where
the core runs on its own thread ([The core on its own thread](../engine/emulation/thread.md),
Nothing drawn reads the game; issue #897).

What the picture costs matters where the GBA's own picture is drawn on the
same processor core, the New 3DS's: with the menu up a frame drew for 11
ms there against 5 on the map, and 43 to 48 were shown a second
(`log.txt`, 25 November 2026). Three whole frames were moved a frame: the
core's into the picture, the picture again for the menu to read BN6's
pieces from, the kept field over it. While the menu covers the frame
(`pet_menu_covers`) the picture takes the band's rows alone, the menu
takes its pieces' part straight from the core's frame (`pet_menu_under`:
columns 128 on, rows 24 on), and the field's is the one whole copy left.
That is exact because nothing else edits the picture then: the board's
tint, the dark way's mark and a Glitch layer's frame go by the map alone,
and the town's hour is 0 in the PET. BN6 loads the Status icon for Forms because the engine grows BN6's
lists of the menu's icons by a ninth, resting under the screen
(`pet_menu_install`; [The PET and the second screen](../rom/pet-and-second-screen.md),
the PET menu's icons). It stands while BN6's menu takes input, closes, fades to
an entry's screen or comes back from one; while BN6's slides in the first
frames are BN6's own, and the strip waits until BN6's panel is in its place.
In the Net Square the PET's menu stays BN6's own ([The PET in the Square](../online/square/pet.md)).

## Not yet

- The longest place name a run prints measured against the band's stripe
  (about 100 pixels, 12 letters), held by a test.
- BN6's held-key repeat matched at the list's ends, and the frames from
  START to the folder measured against BN6's.
