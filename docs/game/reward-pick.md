---
type: Design
title: "The reward pick's screen"
description: "The run's picks of one reward among several (a guardian's NaviCust draft, a vault's, a Server's and a Glitch copy's three chips, an official gate's, Mr.Prog's first gift) on a screen of ours in BN6's PET look: each choice on the Library's card with its picture and details, the skip a row of its own, and how a pick's script opens it (`src/director/reward_pick.c`)."
tags: [game, ui, navicust, rewards]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-10T23:40:00Z }
---

# The reward pick's screen

Every pick of one reward among several in a run was the chat box's
options: a name a line, its colour or code beside it, a box or two of
words before it about what one did. The owner asked for the same move the
NaviCust vendor's BugFrag shop made, from a text select to a screen
(10 November 2026): room for each choice's picture and more of what it
is. This page is the design, reasoned with the game-design skill, and
what is built.

## Where it stands

| Pick | Its choices | B | Screen's name |
| --- | --- | --- | --- |
| A guardian's Guardian Data ([the NaviCust](navicust.md), 2) | up to three programs, then the BugFrags of none | to the BugFrags' row | PROGRAM DATA |
| A collector's vault ([phase two](meta/phase-two.md)) | three chips | leaves, the vault keeps | VAULT |
| An official gate ([the rival](rival.md)) | three chips | leaves, the gate keeps | CHIP ORDER (level 1), OFFICIAL VAULT (level 2) |
| A won Server's pick | three chips | leaves | SERVER DATA |
| A Glitch layer's copy ([the Glitch layer](glitch-layer.md)) | three chips | leaves | GLITCH DATA |
| Mr.Prog's gift on the first layer | HPMemory x2, a chip, a program | nothing: it waits for a pick | GIFT |

BN6 hands its rewards out one at a time (RESULT's chip, a Mystery Data's
find, a shop's buy) and has no screen for a pick of one among several;
none of the PET's screens measured for [the PET's rules](../engine/ui.md)
is one (assumed from that list, no screen of the kind found). So this one
is ours to draw, and it composes BN6's own pieces rather than new art.

## The design

- **The dialectic.** What earns a place: a program's cells on the board
  or a chip's slot in the folder, against what the run already holds.
  The draft is a card draft with a skip (the pattern of Slay the
  Spire's pick of one or none): the BugFrags of none are a choice with
  a value, so they are a row of their own, under the choices, not a key
  named in a line.
- **The loop layer.** The session's: a guardian's or a vault's reward is
  what the next layers are fought with. A pick read by its name alone
  was a guess (two playtesters picked their vault chips by the names,
  session 55's notes in `reward_words.c`).
- **Loadout as budget.** A program's cost is its shape on the board; a
  capacity limit makes a choice only where the cost is seen at the
  choice. So the card shows the shape, in its colour, where a chip's
  card shows its picture.
- **Telegraph.** What each choice is stays on screen while the cursor
  moves; the words before the pick that said it (Mr.Prog's two boxes on
  the gift's chip and program, a vault chip's description and a "Take
  TrplShot J? We only get one!" before each take) are gone.

## What a choice shows

Laid out as BN6's Library, whose card a hardcore player reads at a
glance: the card at the left, the list in the window at the right, the
information box under the window (`reward_pick_draw.c`). What MegaMan
already holds is shown as BN6's shops show it, read from captures of
the Net Dealer's chip shop and the NaviCust vendor's program shop (10
November 2026): their ITEMS box beside the list holds each code the chip
comes in (an orange letter), or the program's colour (its square), and
under each in a navy cell how many he holds of it; the code stands after
the name in the list, nothing else beside it.

| Choice | Its row | The card | The information box |
| --- | --- | --- | --- |
| A chip | its icon, its name, its code in yellow in a column of its own, E-Mail's NEW where the Library lacks it | the Library's card: BN6's picture, its code at the head's foot as the folder editor's card sets it, element, power; under the head the shops' ITEMS box, each of its codes and the Folder's and Pack's copies in it | BN6's description, as the shops show it on R |
| A program | its colour's square, its name | its shape in its colour where a chip's picture stands; the ITEMS box, each colour it comes in and its copies on the board | what it does in MegaMan's words of the NaviCust; its part (program, plus) on the box's tag as KeyItem's box carries its tag |
| HPMemory | HPMemory | the max HP it adds | MegaMan's max HP now and with it |
| The draft's skip | BugFrags | their count | the data breaks into them |

The code in the row and on the card is set in heavy capitals: the list's
and the chat font's O is a zero's oval, and an O read as a zero beside
HPMemory's x2 in the first build (the owner's catch, 10 November 2026);
the heavy capitals' zero is slashed. The holdings are counted as the
screen opens.

Everything is read from the player's ROM as it is drawn (the chip's
picture, icon and description, the program's name and shape); nothing of
it is kept in the repository.

## Keys

Up and Down move round the rows, A takes, B as the table above. Keys
rest for half a second as it opens (its fade in among them), and A
counts only after it has rested a third of a second on the screen: A
pressed through the lines before the chat box's options once took the
first of three for good (session 55), and in a capture a mash through
Mr.Prog's lines ran on into the screen past its fade. No confirmation
follows A: the screen is the look a question gave before.

## How it is built

- A pick's script (`reward_words.c`) says its lines, then sets the kind's
  bits (event flags 0x1467-0x1469) and `REWARD_PICK_FLAG` (0x147C), and
  ends. Its offer (the choices, the script that gives each, B's script)
  is made with the script (`reward_pick_offer`), so a layer's offers are
  made again with its scripts from `run.layer_seed`.
- After the frame, once the chat box has closed, the director opens the
  screen (`director_pick_watch`) and clears the flags; the game's core
  holds still under it as under the Library (`scene_emu.c`), its keys
  the screen's.
- A choice closes it and starts the script that gives it through
  `talk_script`, in BN6's own "MegaMan got:" box; the taken flag is set
  there as before, so a guardian's exit opens and a vault stands empty
  as they did.
- The autopilot presses A with rests on it, taking the first row, as it
  took the chat box's first option.
- The state's `screen` line says `reward-pick` while it is up.
