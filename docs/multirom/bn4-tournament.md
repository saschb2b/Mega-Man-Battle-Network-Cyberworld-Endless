---
type: Research
title: "BN4's tournament screens"
description: "BN4 Red Sun's tournament presentation located in its ROM and driven in romlab: the bracket screen (main mode 0x24) and the RAM it draws from, the four tournaments, how a round's result shows, what opens it in a script, no versus screen found; the two ways into a run measured, the bracket moved into BN6 as its main mode 0x3C by a recipe and proved pixel for pixel against BN4's own, and how BN4 draws a plate, every BN6 and BN5 entrant's face made into one from his own game's art and proved on BN4's bracket."
tags: [multirom, bn4, tournament, research]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-05T04:30:00Z }
sources:
  - id: bn4-rom
    resource: "Mega Man Battle Network 4: Red Sun (USA), B4WE, SHA-1 a97e96a7da03abd70f7953328e511b9fa29179f1"
    title: "The owner's BN4 Red Sun ROM, read with a Thumb decoder and played in romlab"
  - id: bn6-rom
    resource: "Mega Man Battle Network 6: Cybeast Gregar (USA), SHA-1 89fe0bac4fd3d2ab1d2ca35e87ef8b1294a84cd6"
    title: "The player's BN6 ROM, its main modes read for a free slot, its mugshots and battle sprites for the plates"
  - id: bn5-rom
    resource: "Mega Man Battle Network 5: Team Colonel (USA), SHA-1 5f472f78d8de2df01d5039e045c043cb40969a39"
    title: "The player's BN5 ROM, its Navis' mugshots for the plates"
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
plates' places: found and proved below (Faces for every entrant). A
bracket of BN4's own Navis needs nothing more.

## Faces for every entrant

The owner's call 6 ([The tournament](../game/tournament.md), The owner's
calls): every bracket on BN4's screen, and no tournament until it can
show every entrant. The entrants are every moved Navi of the games
present (the same page, The entrants): BN6's sixteen (navis 1-10, 12-16
and 18) and BN5's six (24-29). This is how BN4 draws a plate, what each
of them is made from, the proof, and what the whole set costs.
`tools/bn4_faces.py` does all of it from the player's ROMs; the names are
in `src/emu/bn4_tournament.h` and the row in
[BN4 Red Sun's tables](../rom/bn4.md) (The bracket's plates). Nothing of
the games is committed: the faces exist in memory and the frames in
`.build/bn4faces/`.

### How BN4 draws a plate

| Question | Answer (read in BN4's code, checked by `tools/bn4_faces.py`) |
| --- | --- |
| Which plate | `BN4_PLATE_INIT` (`0x08104040`, the plates' type's kind 0) takes the entrant's id from the tournament (0xFF is drawn as 0x17), its animation from `BN4_PLATE_FRAMES`, its palette from `BN4_PLATE_PALETTES` plus a variant, and its sheet through `BN4_PLATE_SHEETS` and `BN4_PLATE_SPRITES` to `BN4_PLATE_SET` (`0x08104290`). Sheet A (`0x084BB768`) holds ids 0x00-0x0F, sheet B (`0x084BDB30`) ids 0x10-0x15 and MegaMan's. |
| Tiles, size, layout | BN's sprite format, an animation of one frame a plate: a 16x32 object at (-12, -16) and an 8x32 one at (4, -16), 12 tiles of 4 bits (384 bytes), 1D. The plate is 24x32: a frame of 22x30 from (1, 0), its outer line in colour index 14 and its inner in 15, the face an 18x26 window from (3, 2) in indexes 1-13 (11 to 13 colours on BN4's own). Plates 0x04 and 0x05 alone keep their frame in other indexes. |
| Palette | Four a plate: the sheet's palette `BN4_PLATE_PALETTES`[id] + the variant + the object's palette nibble (0, 4, 8, 12 by the animation's place in its four), so sheet A holds 64 palettes and B 28. Variant 0 is colour, 1 lighter with a green outer line (no code read chooses it), 2 grey, 3 colour with a purple inner line (`BN4_PLATE_SPECIAL`'s ids 0x02, 0x03, 0x06, 0x07, 0x0A and 0x0B in colour). |
| The grey "out" look | A palette, not a picture: the same tiles in variant 2, a sepia grey of the plate's own colours that BN4 made for each plate. Chosen where the entrant's flags hold neither `0x40` nor `0x80` (out before), or out this round when a script opened the screen. |
| Compressed | No: both sheets are plain (pointer bit 31 clear) and read in place when a plate's sprite is set. BN6's mugshots and battle sprites used below are plain too; BN5's mugshots are decoded either way. |
| An extra plate | Swapping an existing id's art needs no code: its 384 bytes of tiles and its four palettes (4 x 32 bytes) written at its place in the sheet (proved below). A new id needs an entry in each of `BN4_PLATE_FRAMES`, `BN4_PLATE_SHEETS` and `BN4_PLATE_PALETTES` (byte tables of 31 entries, ids 0x00-0x1E; 0x17-0x1D are drawn as 0x14's and 0x15's plates; past 0x1E they read the next routine's code), a value of `BN4_PLATE_SPRITES` naming a new entry of `BN4_MENU_SPRITE_PTRS`, and a sheet for it. Outside the plate, BN4's own round (`BN4_ADVANCE_ROUND`: ids under 0x14 over those above) and its opponents' battles (`BN4_OPPONENT_BATTLES`) go by id; a run settles its own rounds and reads neither. |

BN4's plates are drawn for the plate, not cut from its mugshots: the best
18x26 window of each mugshot matches its plate in 176 of 468 pixels for
GutsMan (0x42), 133 for Roll (0x41), 33 for SearchMan (0x59) and 8 for
AquaMan (0x51). They show the head about half the mugshot's size.

### The faces' sources

| Candidate | What it is | Taken |
| --- | --- | --- |
| Chat mugshots (sprite list 8) | 40x48, the face from the front filling it, on a ground of one colour, one palette of 16 | Yes, halved: the plates' look (a front face, the head filling the window) at the plates' scale. BN6 has them for 11 of its 16 entrants ([BN6's pictures](../rom/graphics.md), Mugshots), BN5 for its six (list 8, 69 and 71-75: [BN5's Navis](../rom/navis.md)). BN5's Liberation shows its Navis by these same faces; no other portrait sheet was searched for in either game. |
| Navi chips' pictures (chip records +0x24, +0x28) | 56x48, the Navi's whole body posed over a busy ground (HeatMan's flames) | No: a body, not a face, and the ground fights the frame |
| Battle sprites (list 0, 0x2E + the navi in BN6) | The whole Navi from the side, his head 16 to 27 pixels | For Falzar's five only: Gregar's mugshots for SpoutMan, TomahawkMan, TenguMan, GroundMan and DustMan are its black box (`mugshotBlack`). An 18x26 window round the head at its own scale (`HEAD_WINDOW` in the tool, chosen by eye on a grid of the standing frame) on a ground of his darkest colour. Halving the 40x48 portrait window of `src/layer/portrait.c` was tried first: SpoutMan came out whole and small. |

The control, run through the same pipeline: BN4's own GutsMan's mugshot
(0x42) halved into a plate sits beside BN4's own drawing of him at the
same scale and framing (by eye; they cannot match, as BN4 redrew it).

### The conversion

On the ROMs' bytes in memory, for a 40x48 face: each 2x2 block becomes
its commonest index (a tie goes to the darker); the middle 18 of the 20
columns and the 24 rows go into the window, a row of the face's ground
(its commonest edge index) above and below; at most 13 colours stay, the
least used merged into its nearest (18 of the 22 faces lose one or two);
the frame takes indexes 14 and 15. A head window at its own scale skips
the halving. Then the four palettes: variant 0 the face's colours; 1 each
channel plus 5 (against BN4's own plates' 269 face colours, a mean error
of 1.2 of 31); 2 a sepia grey, o = 5 + 0.7 x (0.25 r + 0.6 g + 0.15 b),
as (o + 1, o, o - 1) (a mean error of 2.2); 3 the colours again. The
frame's colours are BN4's own, read from GutsMan's plate (id 0x00) for
each variant. The 12 tiles are packed as the two objects read them. All
22 faces convert in 26 ms in Python.

### The proof

```
python3 tools/bn4_lab.py boot
python3 tools/bn4_faces.py show 1@1 26@5 bn4:0x42@8 6@0x0d
```

BN6's HeatMan (his mugshot 0x47) is written over plate 0x01, BN5's
NumberMan (list 8's 71) over 0x05, the control over 0x08 and BN6's
SpoutMan (his head window) over 0x0D, in romlab's copy of BN4's ROM. The
Den Battle Tournament then opens from Lan's room with MegaMan, those four,
GutsMan (0x00), AquaMan (0x03) and Roll (0x04). With every entrant in, BN4
draws all eight plates in colour (`.build/bn4faces/all_in.png`). After the
first round, MegaMan, NumberMan, SpoutMan and AquaMan stand one step up in
colour, and HeatMan and the control are grey beside BN4's own grey GutsMan
and Roll (`round1.png`). `tools/bn4_faces.py` alone (report) checks the
sheets, their objects, tiles and frame indexes, and lists every entrant's
source. There is no oracle for a face BN4 never had: the check is that
BN4's own code draws each in both looks. The lighter and purple-lined
variants were made but not seen on screen.

### The whole set

| | Estimate |
| --- | --- |
| Navis | 22: BN6's 16 (11 by mugshot, Falzar's 5 by head window) and BN5's 6 where its ROM is read. BN4's own Navis keep their plates (ids 0x00-0x16, GutsMan's 0x00 first) and MegaMan his (0x1E). |
| Bytes | 512 a plate (384 tiles, 128 palettes). In sheets as BN4's: one of 16 for BN6's, as sheet A (9,156 bytes), and one of 6 for BN5's, as sheet B (4,008 for its 7): about 13 KB of BN6's free ROM, and no RAM, as the tiles are read in place. |
| Time | Once at install, from the ROMs as read: a few hundred thousand operations for all 22, well under a millisecond in C. |
| Where | Beside BN4's plate sheets in the moved screen's art, a region of about 16 KB in `FREE_TAIL` (575 KB unclaimed, `src/emu/free_space.h`), named and claimed with the move. |
| Ids | The moved copy's tables grown (`BN4_PLATE_FRAMES`, `BN4_PLATE_SHEETS` and `BN4_PLATE_PALETTES` to 0x40 entries, `BN4_PLATE_SPRITES` and `BN4_MENU_SPRITE_PTRS` an entry for each new sheet): for example 0x20 + the run's navi number, BN6's 0x21-0x32 and BN5's 0x38-0x3D. The other way, seven of BN4's ids rewritten before each bracket (7 x 512 bytes) with no table grown, is an installer's write mid-run; the first keeps the install at boot. |

What does not fit yet:

- Halving loses fine lines: ElecMan's and EraseMan's faces read darker
  than their mugshots. A window per face at its own scale, as Falzar's
  five take, could replace the halving where it reads better.
- The head windows are by eye; TomahawkMan's (BN6's) cuts his mask at its
  edge.
- The proof writes over BN4's own ids in BN4's ROM; the new ids and the
  sheets in BN6's free space come with the screen's move, whose proof
  against the guest's bracket covers BN4's own plates only.
- What chooses the lighter variant (1) was not found.

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
2. Faces for BN6's and BN5's Navis from their own games: found and proved
   (Faces for every entrant), their sheets installed with the move.
3. Until then, a bracket of BN6's and BN5's Navis has no BN4 presentation:
   which acts' brackets are BN4's Navis is the tournament's design's call
   (its numbers, [Battle Network 4 in a run](bn4.md), The tournament).

## Moved into BN6

The owner's call 6 ([The tournament](../game/tournament.md)) took way (b):
BN4's bracket screen is moved into BN6 and runs there as a main mode of
BN6's own, on every build the 3DS included, proved against BN4's own screen
pixel for pixel.

**How it was moved.** By a recipe of numbers, `src/emu/xbracket_bn4.c`,
written by `tools/xrecipe_bracket.py` from both ROMs (`--check` says whether
it is in step). The generator walks the screen from its roots
(`BN4_BRACKET_SETUP`, `BN4_BRACKET_SCREEN` and its states,
`BN4_BRACKET_PLATE`, the menu sprites' runner and spawn, the Free
Tournament's two spawns) and takes its code with its literal pools, jump
tables and the data it reads (the palette cycles' lists, `BN4_BRACKET_MAPS`,
`BN4_MENU_SPRITE_TYPES`, the plates' byte tables) as blocks, each copied from
the player's ROM at install with its CRC32 checked (`src/emu/xbracket.c`,
through `xvirus_code_stage`, the virus families' own code mover). Its calls
out:

| Calls | How |
| --- | --- |
| 28 to BN6's routine of the same body | the project's pairs (`tools/bn4_twins.py`), the body search, a near-body search over BN6's mapped routines compared again (SAME): copies and fills, LZ77, BG maps, the sprite system, the fade, the palette cycle; the BG rect fill's veneer by its IWRAM routine (`BN4_BG_FILL_RECT` to BN6's `0x080018D0`: both games' IWRAM dumped in romlab, the same instructions; the body search alone took the copy's veneer, `0x080018C2`) |
| 1 audited | the music's start (`0x08000568`, BN6's `0x080005D4`): BN4's song 0x09 copied into a BN6 slot by `src/audio/xsong.c` as the screen installs, its `movs r0,#9` given the slot |
| 1 through a shim | `BN4_HBLANK_DMA_SET`: BN6's set (`BN6_HBLANK_DMA_SET`) takes a line buffer more |
| 8 the host's | BN4's logic around the screen: the next opponent, the chat box twice, the script's return, the warp back, the map's re-entry, MegaMan's overworld lock, the map's OBJ palette |

and three of BN4's engine routines moved with it, whose BN6 twins read other
data: `BN4_MENU_SPRITE_SET` (BN6's reads BN6's sprite table),
`BN4_BG_PRESETS_SET` (BN6's presets), the Free Tournament's spawns. Its 11
reads of eToolkit +0x3C (BN4's tournaments; BN6 keeps its game state there,
BN4's toolkit being BN6's from +0x3C shifted) are each made a call to a shim
that loads the tournaments' place (each routine pushes lr, read). Its RAM is
placed in `RAM_BRACKET`, which BN6's shop decompresses into as it runs while
the map waits (`src/emu/free_space.h`); its RAM twins come from the paired
bodies' literals (BN4's palette buffer `0x03002A50` is BN6's `0x03001960`,
the palette cycles', the fade's). Its art (the tiles, palettes and six maps,
`BN4_MENU_SPRITE_PTRS` 2-5: the lit lines' and the plates' sprites) is
copied from the player's BN4 ROM into `FREE_BRACKET` at install; nothing of
BN4's is in git.

**Its host and entry.** BN6 Gregar's main mode 0x3C (`XBR_MODE`), empty in
its table: the installer writes its entry. `xbracket_open(t, entrants)`
(`src/emu/xbracket.h`) writes the eight entrants and the tournament into the
moved RAM and makes a game call to `XBR_ENTER`, which leaves the map as BN6's
shop does (`BN6_MAP_TO_SHOP`'s calls read from it in turn, its fade test
skipped), then makes the calls `BN4_MAP_LEAVE` makes that BN6's way lacks
(their twins; `BN4_BG_PRESETS_SET` with the bracket's preset 0x13; DISPCNT
0x40), runs BN4's set-up and enters the mode. A, B or START fades it out;
the leave's return writes BN6's game mode with its map entered again
(`BN6_SUB_ENTER`). The mode's routine (`XBR_STUB_FRAME`) copies the line
blend values the frame before wrote for the HBlank DMA, then runs BN4's:
BN4's screen shows each frame the values written the frame before (measured:
the values the same byte for byte each frame on both sides, BN6's picture a
frame ahead on the blend's rows without the copy); why BN4's lags was not
read in its interrupt handler. The dev step `0:bracket T IIFFSS/...`
([A battle on demand](../devtools/battle-on-demand.md)) calls the entry.

**Proved.** `tools/xbracket.py compare` against BN4's own screen in romlab
from the same entrants, aligned on the screen's first frame in state 4: The
Den Battle Tournament, The Eagle Tournament, The Red Sun Tournament and the
Free Tournament at their first round, and The Den Battle Tournament after a
settled round (four through, their lines lit), each PASS over 238 frames,
every pixel, but the one allowed difference: the first frame, one colour on
both sides, the backdrop of the map each was opened from (Lan's room, a net
layer), every layer off until the screen's palette reaches PRAM. The
comparator's mutations (a palette colour, a plate a pixel lower, a frame
late) and the move's (`tournaments`: the shims give the next tournament's
record) FAIL. The frames are in `.build/xbracket/` (`bn4/`, `bn6/`), nothing
of them committed.

**Not reached.** The return to the map after the screen (written as BN6's
own screens return, not run past the fade); the song heard (its slot set, not
listened to); a tournament's bracket in a run (the tournament's design calls
the entry; faces for BN6's and BN5's Navis are their own unit, above); the
screen opened from a script (`BN4_BRACKET_FROM_SCRIPT`, its return the
host's stub); the Free Tournament's own list (`0x020014F0`) and its menu
sprite types 0x3C and 0x3D, which the move gives a stub (BN4 draws no plate
there either while the list is empty); why BN4's HBlank DMA shows the frame
before's values.

## Not reached

- The cutscene scripts beyond their tournament commands: the stage's
  intro and the announcer's lines were not decoded or watched.
- The plates' ids 0x01-0x02, 0x05-0x08, 0x0A-0x10 and 0x14-0x15 by name.
- The draw's placement rules and the pools' filling (`0x08043FE4`,
  `0x08044058`, `0x0804422C`, `0x0804428C`): read in part, not run.
- Main modes 0x10, 0x28, 0x2C and 0x3C; the Free Tournament's own flow.
- The plates' rise animation's length (`BN4_BRACKET` +4) and the text the
  screen opens at its step 4 (`0x08043B86`).
