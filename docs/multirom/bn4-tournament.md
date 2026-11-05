---
type: Research
title: "BN4's tournament screens"
description: "BN4 Red Sun's tournament presentation located in its ROM and driven in romlab: the bracket screen (main mode 0x24) and the RAM it draws from, the four tournaments, how a round's result shows, what opens it in a script, no versus screen found; the two ways into a run (BN4's own code on the guest core, or moved into BN6) measured, and the move recommended."
tags: [multirom, bn4, tournament, research]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-05T01:20:00Z }
sources:
  - id: bn4-rom
    resource: "Mega Man Battle Network 4: Red Sun (USA), B4WE, SHA-1 a97e96a7da03abd70f7953328e511b9fa29179f1"
    title: "The owner's BN4 Red Sun ROM, read with a Thumb decoder and played in romlab"
  - id: bn6-rom
    resource: "Mega Man Battle Network 6: Cybeast Gregar (USA), SHA-1 89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6"
    title: "The player's BN6 ROM, its main modes read for a free slot"
---

# BN4's tournament screens

The owner's call 2 makes BN4's tournament an act's climax: some acts' arenas
hold a bracket of three Navis instead of one guardian ([Battle Network 4 in a
run](bn4.md), The tournament). This is what BN4 Red Sun itself shows of a
tournament, where its code and data live, how it was made to show from
outside, and what each way of bringing it into a run costs. Every address is
named in `src/emu/bn4_tournament.h` and has its row in
[BN4 Red Sun's tables](../rom/bn4.md) (The tournaments). Nothing of BN4's is
committed: the pictures below stay in `.build/`.

## What BN4 shows

| Screen | What it is | Found and verified |
| --- | --- | --- |
| The bracket | Main mode 0x24 (`BN4_MODE_BRACKET`, `0x0804395C`): the tournament's title, a tree of eight with CHAMPION and two crowns at its top, a plate per entrant (his face) at the place his rounds won reach, the lines he climbed lit and flashing. Its song is 0x09 (`BN4_SONG_BRACKET`), not 0x17. A press of A, B or START fades it out. | The main loop's 16 modes read; each map sub-state that enters one written in romlab from Lan's room, the bracket captured. Then driven: entrants poked in, the bracket drawn by BN4 as set (below). |
| The four tournaments | `BN4_BRACKET` +5: 0 The Den Battle Tournament, 1 The Eagle Tournament, 2 The Red Sun Tournament, 3 Free Tournament, each its own title and colours (`BN4_BRACKET_MAPS`' third map, `BN4_PALETTE_ANIM`'s lists). The Free Tournament takes its entrants from elsewhere (`0x020014F0`, its own mode 0x34) and was not driven further. | All four drawn in romlab. That the Free Tournament is the link's is assumed. |
| A round's result | No screen of its own: the bracket again. An entrant still in has `BN4_ENTRANT_IN` (his plate in colour), one beaten this round `BN4_ENTRANT_OUT` (shown so once, the bit cleared as the screen leaves), one beaten before neither (his plate grey); his rounds won are bits 1, 2 and 4 (`BN4_ENTRANT_ROUNDS`), which place his plate and light his lines (`BN4_BRACKET_LINES`). | Read in the result's routine (`BN4_ADVANCE_ROUND`) and the plates' (`BN4_BRACKET_PLATE`); in romlab, plates in colour with 0x40 and grey without, MegaMan's plate under a crown, in the final, with rounds 1 and 2; the out-this-round bits cleared as the screen left. |
| The intro, the ceremony | No screen of its own: the stage's cutscene, BN4's chat box over its map, its commands starting the tournament (`52 t`, `BN4_CS_START_TOURNAMENT`: the pool, the draw, the next opponent) and showing the bracket (`54 t 0 0`). | The cutscene engine's table read (`BN4_CUTSCENE_COMMANDS`, `0x0802C140`, command 0x54 the bracket's opener); the commands found in BN4's scripts by their bytes: `52 00` then `54 00 00 00` at `0x08087680`, The Den Battle Tournament's start. The scripts' other commands were not decoded. |
| Before a match | No versus or announcement screen found. A match is the stage's cutscene, the opponent's arena chosen (`55 0`, `BN4_CS_OPPONENT_ARENA`: his net map by `BN4_NEXT_OPPONENT`'s battle), the jack-in (mode 0x0C) and the battle. After a win, `54 t 1 0` asks for the bracket and `53 t` settles the round (`BN4_CS_ADVANCE_ROUND`); the opener only sets the map's sub-state, so the round is settled before the screen draws and it shows the new results (read, not watched). | The search that came up empty: the 16 main modes (the start's 0x00 and 0x14 assumed, the map 0x04, the battle 0x08, the jack-in 0x0C, GAME OVER 0x18, the PET 0x1C, the city map 0x20, the bracket 0x24 and 0x34, the Chip Trader 0x30, a play-time screen 0x38 with song 0x1C; 0x10, 0x28, 0x2C and 0x3C unidentified, 0x28 and 0x2C drawn wrong without their set-up); the cutscene commands beside the tournament's; the battle modes 0x46-0x4C (the link's rounds, whose 0x4A-0x4C play song 0x17). The order after a win from the script's bytes (`54 00 01 00 53 00` at `0x08087720` and `0x080877C8`, `54 01 01 01 53 01` at `0x080890CA`, `54 02 01 02 53 02` at `0x0808ACE3`), not watched. |

The text engine opens the bracket too (`FC 01 t script argument`,
`BN4_TEXT_TOURNAMENT`, its commands at `0x0804E900` by byte - 0xE5). A
menu of the developers' (`0x0802E104` and its neighbours) starts a
tournament (`0x0802E14C`), settles its round (`0x0802E15C`) and opens its
bracket.

## What it draws from

The tournaments live at eToolkit +0x3C, `0x02001460` (fixed), 0x20 bytes
each: eight entrants of four bytes in bracket order, a first round's pairs
0-1, 2-3, 4-5 and 6-7. An entrant is its id (`BN4_ENTRANT_ID`), its flags
(above), its seed (`BN4_ENTRANT_RANK`: of two Navis the lower wins) and its
mark (what the screen last showed). MegaMan is id 0x1E; ids 0x00-0x16 are
BN4's plates, 20 Navis' in two sprites of its own (`BN4_PLATE_SPRITES`: 16
plates, then 7, MegaMan's the last): 0x12 and 0x13 are numbered
placeholders ("18", "19") with no battle (`BN4_OPPONENT_BATTLES`), and 0x16
shows 0x14's plate. By
sight, 0x00 is GutsMan, 0x03 AquaMan, 0x04 Roll, 0x09 SearchMan and 0x11
KendoMan; the rest are not named yet (their battles through
`BN4_OPPONENT_BATTLES` would name them).

The screen's own state is `BN4_BRACKET` (`0x0200ACC0`): +5 the tournament,
+6 whether a script waits on it, +7 what it hands back. Its opener
(`BN4_OPEN_BRACKET`, `0x08005792`, r0-r2 those three) keeps MegaMan's place
and sets the map's sub-state 0x1C, which fades the map out, sets the screen
up (`BN4_BRACKET_SETUP`) and enters mode 0x24. BN4's own logic around it, a
run would not need: the pools (`0x020073A0`, 12 bytes a tournament), the
draw (`BN4_DRAW_BRACKET`), the round's settling and MegaMan's next opponent.

## Shown in romlab

From `tools/bn4_lab.py boot`'s state (Lan's room, game state `0x0200218C`),
these lines to `tools/romlab` (its ROM BN4's, `.build/` mounted at `/src/.build`)
ask for The Den Battle Tournament at its final: MegaMan (0x1E) and 0x04 in,
two rounds won each (`43`), the semi-finalists 0x02 and 0x06 out this round
(`81`), the rest out before (`00`); then the bracket's three bytes and the
game state's +0 made 0x1C:

```
load /src/.build/bn4lab/room.state
run 2
pokes 02001460 1e43000001000000028100000300000004430000050000000681000007000000
poke8 0200acc5 0
poke8 0200acc6 0
poke8 0200acc7 0
poke8 0200218c 1c
run 400
shot /src/.build/bn4tour/semi_400.ppm
```

BN4 draws that bracket: MegaMan's and Roll's plates in colour under the
crowns, the six others grey at the places their rounds reached, the lines
climbed lit. It takes 3 frames to set up after the map's fade (probes on
its states: the setup, the start, the lines, then state 4 each frame), holds
until a key, and an A press fades it out in 23 frames back to Lan's room as
he stood, the `81`s now `01` (the out-this-round bit cleared as it leaves).
Opened from a script (`BN4_BRACKET_FROM_SCRIPT`), an entrant out this round
shows in colour first. All four tournaments were drawn so, and ids
0x00-0x16 and 0x1E each in colour for their plates. The frames are in
`.build/bn4tour/` (`semi_400.png` this one, `t0.png` to `t3.png` the four
tournaments, `sheet_ids.png` the plates by id); nothing of them is
committed.

Entered from the title screen instead (mode 0x24 written without the
map's set-up), the plates and lines draw but the backgrounds do not: the
map's sub-state 0x1C does the set-up the screen relies on, so a caller goes
through the map, as BN4's scripts do.

## The two ways in

Both obey the project's rule (where a game has a screen, its own code shows
it) and both read BN4's art from the player's ROM at run time.

### (a) On the guest core

The guest core (`src/emu/guest.c`) already boots BN4 to Lan's room for the
differential and keeps the state (`xdiff-bn4.state`). A tournament's
bracket in a run would be: that state loaded, the eight entrants and the
three bytes poked, the sub-state set, the guest's frames shown in place of
BN6's (`guest_video`) with the player's keys passed to it until it fades
out, BN6 paused meanwhile. The poke is the whole interface: six writes.

| Cost | Measured or derived |
| --- | --- |
| The 3DS | None: its heap holds no second core (`guest.h`). A climax every other build shows, it would lack. |
| First boot | 12,440 BN4 frames (the boot's presses, `bn4_boot`) once per data folder: about 6 s on a desktop and 9 s on a Retroid Nova, scaled from BN5's measured 7,210 frames in 3.4-3.6 s and 5 s ([Guest battles](guest-battles.md)) |
| Memory | BN5's guest took the browser page from 97 to 153 MB ([Guest battles](guest-battles.md)); BN4's ROM is as large |
| Per showing | A state load, 3 frames and the map's fade in, the player's time, 23 frames out; the guest's frame about 0.5-0.7 ms while shown, by the same rates |
| Sound | The guest's is dropped (`guest_frame_quiet`); song 0x09 played by BN6 instead, already copied (`src/audio/xsong.c`, every BN4 song proved) |
| Run code | A presentation path issue #82 took out of runs with the guest's battles: the boot in the background, a fallback while it boots and on the 3DS, the frame and key hand-over |

### (b) Moved into BN6

| Part | Size |
| --- | --- |
| BN4's code to move | 3,728 bytes of Thumb: the screen and its set-up (`0x080438F8`-`0x08043F1F`, 1,576), the plates' type (`0x08103FFC`-`0x0810470B`, 1,808), the menu sprites' runner and spawn (`0x08053F68`-`0x080540BF`, 344) |
| Routines it calls outside itself | 41: some 30 of BN4's engine (copies and fills, LZ77, BG maps, OAM, 14 of the sprite system, the fade and its test, the scanline gradient, the palette cycle, PlayMusic) to twin with BN6's or shim, as the virus families' were; the rest BN4's own logic or its map's return, which a run replaces |
| RAM | About 2.4 KB of free space: the menu sprites (24 of 0x58, 2,112 bytes), the screen's state (0x2C), the tournament (0x20), the palette cycles, the per-line blend values (0x140, sent to BLDALPHA each line); and a buffer for the tiles (9,924 bytes unpacked) |
| Art, from the player's BN4 ROM | The tiles (LZ77), a palette of 0xC0 bytes and its cycles, three BG maps of 1,200 bytes (the title's by tournament), the two plates' sprites |
| A mode of its own | BN6 Gregar's main modes 0x3C and 0x40 are empty (its table at `0x0800034C`), so the screen can run as BN6 runs its shop (0x2C), entered from the map and back to it |
| The 3DS | Included, as every moved family is |
| Its proof | The guest core as the oracle: each tournament's bracket from the same entrants, frame by frame, as `build.py xdiff` proves a family |

### The faces, either way

BN4's bracket draws BN4's plates only: 20 of its Navis and MegaMan. The
design's first brackets draw from the Navis a run already moves, BN6's and
BN5's, and neither game has a plate. Whichever way is taken, a bracket of
those Navis needs their faces from their own games' art composed into the
plates' places, a unit of research of its own (candidates to look for:
BN5's Liberation team faces, BN6's Cross select faces). A bracket of BN4's
own Navis needs nothing more.

## Recommendation

Move it (b). The guest's way is six writes and no moved code, but it leaves
the 3DS without the act's climax, costs a 9-second first boot on the
reference device and some 56 MB in the browser, and brings back into runs
the second core issue #82 took out of them. The move is 3.7 KB of BN4's
code and about 30 engine routines to twin, it runs everywhere, and the
guest serves as its oracle, which is what the guest is kept for. In order:

1. The bracket screen moved, BN4's own plates, MegaMan and BN4's Navis as
   they join (GutsMan's move is under way), proved against the guest's
   bracket frame by frame, in a free BN6 main mode.
2. Faces for BN6's and BN5's Navis from their own games, once found.
3. Until then, a bracket of BN6's and BN5's Navis has no BN4 presentation:
   which acts' brackets are BN4's Navis is the tournament's design's call
   (its numbers, [Battle Network 4 in a run](bn4.md), The tournament).

## Not reached

- The cutscene scripts beyond their tournament commands: the stage's
  intro and the announcer's lines were not decoded or watched.
- The plates' ids 0x01-0x02, 0x05-0x08, 0x0A-0x10 and 0x14-0x15 by name.
- The draw's placement rules and the pools' filling (`0x08043FE4`,
  `0x08044058`, `0x0804422C`, `0x0804428C`): read in part, not run.
- Main modes 0x10, 0x28, 0x2C and 0x3C; the Free Tournament's own flow.
- The plates' rise animation's length (`BN4_BRACKET` +4) and the text the
  screen opens at its step 4 (`0x08043B86`).
