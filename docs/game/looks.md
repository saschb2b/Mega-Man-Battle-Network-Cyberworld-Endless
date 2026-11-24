---
type: Design
title: "MegaMan's look"
description: "Each profile's MegaMan in colours of its own, on Warframe's model (issue #854): three channels of his plain body (armour, suit, accent) each coloured from one of twelve palettes the profile owns, earned by milestones, the home shop and rare finds, looks only; the owner's calls of 24 November 2026 and what is built."
tags: [game, design, meta, megaman]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-24T21:00:00Z }
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
- **Classic only at first:** the other palettes come by milestones, the
  home shop ([the home shop](../world/home/home-shop.md)) and rare finds,
  each once.

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
`looks_worn` is the selected slot's look. A slot set or selected calls the
apply below through a callback it registered (`looks_on_change`), so the
look on him changes at once.

**The apply** (`src/emu/look_apply.c`): BN6 reads MegaMan's palettes raw
from the core's ROM copy every frame ([MegaMan's
colours](../rom/megaman-colours.md)), so `look_apply` writes a look there:
the battle body's palettes 0, 1 (paled) and 4 (Full Synchro), the field's
palette 0, the mugshot's and the emotion window's face 0 (through the word
`soullook.c` repoints). Each art has its own indexes for a channel:

| Art | Armour | Suit | Accent | Never touched |
| --- | --- | --- | --- | --- |
| Battle (0, 1, 4) | 2-5 | 6-9 | 11 | 1 outline, 10 glints, 12 emblem, 13-15 skin |
| Field | 2-5 | 6-8 | 10 | 1 outline, 9 white, 11 red, 12-15 skin |
| Mugshot | 1-3 (the helmet) | (not drawn) | 11 | the green behind, red, skin, black |
| Emotion face | 11-13 (the helmet) | (not drawn) | (none) | 1-2, the window's border and fill |

BN6's own bytes are read from the player's ROM image, never from what was
written, so a look is computed afresh each time and a channel at
`LOOK_NONE` is BN6's bytes again. It is applied as the game's scene starts
(a run's start, a CONTINUE: `game_hooks.c`), and again when the worn look
changes. A Cross, a Soul and BeastOut draw palettes of their own and keep
their colours; the plain body takes the look again the frame the form ends.
`look_palette_of` gives the same sixteen colours for any look and art,
for a peer's MegaMan in the Square and a NetBattle's two sides.

### How a pick colours a ramp

A pick sets a channel's average. The mean hue, saturation and value of
BN6's ramp (from the plain palette) become the pick's, and each shade
keeps its place around that mean: its hue offset from the mean; its
saturation scaled by the pick's over the mean's; its value levelled (below
the mean scaled toward black, above it toward full, so a light pick keeps
its highlights). A blue near BN6's own gives his ramp back, a pale pick a
pale ramp, a deep one a deep ramp. A **grey, white or black** pick has no
saturation, so every shade loses its own and its hue no longer matters:
the ramp comes out as BN6's shading in grey, from the pick's value (white
a near-white ramp shaded to grey, black a near-black one). A single index
(the accent) takes the pick itself. The arithmetic is integer, so both
games of a NetBattle compute the same bytes on any system.

### The accent

The accent channel is the gold of the helmet's fins and the emblem's ring
(battle 11, field 10, the mugshot's 11): a few pixels, but a capture with a
cyan accent against BN6's gold showed it reading in battle and on the
field, so it ships as channel 2. The emblem's red stays BN6's.

### The palettes

Twelve of sixteen colours, ours, authored as RGB in `looks.c`: each runs
lights, mids, darks and an accent or two, coloured for where it is earned
(Classic a spread around BN6's blue; Undernet violets and ash; Gregar the
Cybeast's oranges and bone; ProtoMan red, white and silver; Nebula dark
violets with red and cyan glows; Tournament gold, silver, bronze and
banners; Hazard yellow and black, rust and toxic green; Central Town brick,
park and sky; Seaside aqua, sand and coral; Sky blues from dawn to night;
Mystery the crystals' greens, blues and violets; Glitch pure colours and
static greys). `python3 tools/looks_sheet.py OUT.png [FRAMES]` draws them
as a contact sheet, with whole frames of the game beside them.

### In a NetBattle

Each player's MegaMan wears that player's look on both GBAs of both games
([Looks in a NetBattle](../online/arena/looks.md)): both looks, computed by
`look_palette_of` from BN6's own bytes, go into eight palettes of his
battle block BN6 never picks, in every Arena core before it starts, and
three hooks answer BN6's picks of his palette by the object's side. Both
MegaMen and each GBA's own emotion face take their player's look; a Cross,
a Soul and BeastOut keep their colours; the looks are in the pair's hash,
so two games given different ones part before the first frame. Each game
takes the two looks from the Square's wire (its own as last sent, the
other's as the Square knows him), and so do the stands; `--dev
arenalook=MINE/THEIRS` gives them for tests; two MegaMen a side keep
BN6's.

### In the Net Square

A player's look travels with his name (protocol v30's LOOK, in every
SNAPSHOT), and the other players' MegaMen are drawn in theirs ([Peers'
looks](../online/square/looks.md)): as many looks as the part of the
Square has free OBJ palette slots (the plaza three besides one's own),
the nearest first, the rest in BN6's blue, never in a stale slot's
colours. The Square shows forms on nobody.

### For tests and the other parts

- `--dev look=P.C,P.C,P.C` sets the worn slot's look and saves it, a
  channel `-` for BN6's own (`3.2,4.4,5.1`: ProtoMan's red armour,
  Nebula's violet suit, Tournament's gold). A CONTINUE without the switch
  keeps it.
- The state in words: `look P.C,P.C,P.C slot N`, then `lookseen` with the
  arts whose looked palette the frame's OBJ palettes hold (`battle`,
  `field`, `mugshot`, `face`, `pale`, `synchro`; `none`).
- `tests/scenarios/look_worn.py`: the look set and seen on arrival's chat
  box, the field, the Custom screen and a battle; HeatCross in BN6's own
  colours (`lookseen none`), the look back in the next battle, and kept by
  a quit and CONTINUE.

## The colours screen

Colours, a row of [the PET's start menu](pet-menu.md) after Forms, opens
it outside battle. BN6 has no screen for it, so it is ours, in the PET's
look and from its pieces ([The PET's screens](../engine/ui.md)), like
[the forms screen](meta/forms-screen.md) beside it. It went through two
rounds of review and a final pass by three experts in their own roles (a
BN6 pixel purist, a game UX designer, a Warframe fashion player), each
round's findings built before the next; a count of the palettes owned and
marks for each page were tried and taken out (no room under the grid; read
as a page number).

Reasoned with the game-design skill:

- **The dialectic** is the epic's, what you chose against what you did:
  free picks for expression, earned palettes as trophies. The screen
  shows both: every owned colour is one press away, and a palette not
  owned can be tried on him, never kept, its origin in words.
- **The loop layer** is the meta layer's (a milestone's reward that is not
  power) and the Square's social one. Nothing on the screen touches a run.
- **The patterns:** meta-progression that unlocks variety, not power;
  feedforward (the cursor recolours him before A commits); undo over
  confirm where undo is free (B in the grid puts the colour back) and a
  question, No chosen, where something would be lost (a look replaced);
  and no colour told by its colour alone (every colour has a plain name).

What it shows, at 240 x 160:

- **The band** names it COLOURS, `L/R:PALETTE` at its right end.
- **STATUS's slot** on the left holds BN6's own MegaMan, read from the
  player's ROM, each art in its palette as the look makes it
  (`look_palette_of`, `looks_preview.c`): his chat box's face at the top
  in the NAVIGATOR box's outline, and his body under it. On the list his
  body runs a loop that shows him whole (his field sprite walking round
  all eight directions, front, side and back; then his battle body
  standing, firing the buster, throwing and firing a charged shot). In
  the grid it holds the battle stance, the body that shows every channel
  largest, with a shot of the buster on a change (one under way is let
  finish), and left alone it turns round slowly on the field, as the
  Square sees him. A channel chosen blinks on him, its colours turned
  over twice.
- **The list** in a window, LOOK A on its band: `Look` with the look worn
  between Options' orange pointers, then Armour, Suit and Accent, each
  with the colour he wears in a swatch (BN6's own shade inset in the
  field's navy where it is plain, so plain reads by its shape). The orange arrow is on the cursor's row; in the grid it holds
  still and dim.
- **The palette's window**: the palette's short name on its band between
  the Library's green page arrows, its sixteen colours in a 4x4 grid, and
  PLAIN under them, the channel as BN6 draws it. The channel's colour is framed
  white, as STATUS frames its slot; EQUIP's orange corners in the gutters
  are the grid's cursor. A palette not owned has its cells dimmed and
  LOCKED where PLAIN stands.
- **ChipFolder's SELECT pop-up** under the Look row, SELECT on its band and
  its rows in capitals as SELECT's: COPY TO B, COPY TO C, ALL PLAIN.
- **KeyItem's INFORMATION box** says what the cursor is on: the look worn,
  channel by channel; a channel's parts and colour; a colour's palette,
  number and plain name and where the palette comes from; a locked
  palette's origin; on the pop-up what the look it goes over holds now;
  its question; and what was done (the words in
  `src/looks/looks_screen_text.c`, in BN6's own way: no space after a
  comma, no colons).

The keys, as BN6's screens use them:

- Up and Down walk the list and wrap; Left and Right on `Look` change the
  look worn, at once. A on a channel goes into the grid on its colour,
  worn at once.
- In the grid the d-pad moves the cursor, four across and four down, and
  Down from the last row reaches PLAIN. MegaMan takes the colour under the
  cursor as it moves, in the slot and in the game; a locked palette's
  colour he takes in the slot alone. A picks it (a locked one buzzes),
  the swatch's frame flashing; B or START puts the colour back.
- L and R page through all twelve palettes, on the list and in the grid.
- A on `Look` opens the pop-up. A copy over a look that is not plain, or
  All plain on one that is not, asks first in the INFORMATION box, Yes
  and No behind BN6's green triangle, No chosen. A copy is worn at once,
  to be changed from there.
- B on the list (or START) leaves, the look kept, as BN's screens keep
  what was set. Each press has BN6's sound: the cursor's, A's, B's, the
  buzz.
- A tap takes a row, a cell or a page arrow; a tap on the grid's cursor
  is A; on the Look row the letter turns to the next look and the word
  opens the pop-up; a tap outside the pop-up is B.

Each change is applied in the screen's update (`look_apply`, never in its
draw), so the field behind the PET and the chat box's face wear it as soon
as the screen closes. The second screen (the 3DS's bottom, a phone's
panel) shows nothing of it: it keeps what it shows under the PET. The
state in words has the core's `look` line (the look worn, above) and,
while the screen is up, `looks at armour grid page Classic cell 13` (and
`popup N`, `asking yes|no`); the scenario `tests/scenarios/home_looks.py`
picks a colour, tries a locked one and copies a look over another
([Scenarios](../devtools/scenarios.md)).

Not yet: a palette's exact way to earn it named (the routes' work), and a
mark on what is new since the last visit (the profile does not yet keep
what was seen).

## Earning

How each palette is earned, as built (the six milestones, AsterLand's
three, the two rare finds, MegaMan's word and Dad's Colors mail):
[MegaMan's palettes, earned](looks-earning.md).
