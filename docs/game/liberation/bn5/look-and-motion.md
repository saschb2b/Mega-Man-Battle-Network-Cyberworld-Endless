---
type: Research
title: "How the board looks and moves"
description: "How BN5's Liberation board looks and moves, recorded in romlab: the overlay diamonds and palettes (`BN5_LIB_LOOKS`, `BN5_LIB_ANIMS`), faced-panel outline, item pops, banners, HUD and command window, phase bands, music and camera."
tags: [game, liberation, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-25T01:11:12Z }
---

# How the board looks and moves (play, ROM)

The owner's direction (23 October 2026): "I expect a near mmbn5 like
result. Take the original as hard reference." What BN5 shows, recorded in
romlab on the plain ROM (mission 1, from the board's start through a won
liberation and an item's find; the recordings are in the pull request):

- **Two layers.** BG1 is the mission's map itself (group 0x86): a floor
  built as a grid of framed panels, one a 32-unit board cell, with BN5's
  walkway trim at the start. BG2 lays each panel's kind over it as an
  isometric diamond of tiles (two, four, four and two across): cyan for a
  clean panel (palette 8), a magenta swirl for a dark one (palette 9, which
  changes every frame: the dark panels' pulse), a pit for a DarkHole.
  An item panel is a dark panel with a teal stud. The start is the map's
  own floor, no overlay.
- **The faced panel** gets a yellow outline, a ripple, while the command
  window is open over a panel to liberate (6 frames after A; never while
  walking, never on a clean panel); the command window (Liberation, the
  Navi's command, Pass) greys what cannot be used there.
- **A won liberation:** "LIBERATE SUCCESS!" over the battle (or "1 TURN
  LIBERATE!" for a one-turn win), a fade back to the board, the freed
  panel turning from the swirl to grey to cyan, then the Navi's line in
  his chat box ("We liberated it!").
- **An item's find:** the item pops out of its freed panel as an icon (a
  zenny coin, a heart, Order Points and so on), its shadow on the panel:
  up 8 pixels after 3 frames, down 2 after 5 more, held while the Navi
  says what he found ("I found some Zennys!"). The board queues it as an
  event (`BN5_LIB_EVENT`, 12 bytes each from `0x02001AB0`). Each kind's
  icon is a sprite of list 7, its index the item kinds' table's
  (`BN5_LIB_ITEM_KINDS` +5: 0x18 a heart, 0x19 a key, 0x25 zenny, 0x26
  BugFrags, 0x29 Order Points, 0x53 a chip), its animation 0 with its
  shadow.
- **The tiles** are the mission maps' own: the overlay's tiles stand from
  tile 865 of map 0x86:00's tile set, uncompressed at `0x08623130` (its
  tile t at `0x0861C510` + 32t, copied whole into the BG's characters),
  and the diamonds in twelve looks after them (`BN5_LIB_LOOKS`, 8 x 4
  tile-map entries each, named by the mission record's first block, the
  same in all nine, a panel's kind its look's index): clean, dark, a
  paled dark (kind 2, the dissolve's middle),
  an item's stud, a "+5", BarrierPanels 1-4 (numbered, a keyhole), a
  BonusPanel's "?", a DarkHole, a grey grid (kind 0x0B, the Darkloid's
  ring until the DarkHoles fall); then three more: the start's (kind
  0x0C), every entry blank, as a team starts on its map's own walkway
  ends or (mission 1) its lilac pad, never on a panel, and two diamonds
  on palette 7 whose kinds (0x0D, 0x0E) no record lists; palettes 8-12
  the board's own. Each
  panel's four look bytes are its corners, each the three neighbours
  around it the Navi in play can walk (`BN5_LIB_CORNERS`); they choose
  height modifiers on the panel's border cells in the map's collision
  (`BN5_LIB_EDGES`), so a Navi stops where he cannot walk: no drawing.
- **The palettes move.** The dark panels' (9) rest 120 frames, dim
  over three frames and come back, 216 frames round; 10 steps there and
  back over four, 20 frames a step; the barriers' (11) glint every 132
  frames; the BonusPanel's (12) breathes over six, 12 frames a step.
  They are the mission map's own GFXAnim scripts (`BN5_LIB_ANIMS`), the
  engine BN6 runs too, so in BN6 they run as BN5's own scripts copied,
  in place of the host map's: a host map's script left running writes
  its colours into the same banks every few frames and flickers (PR
  #202's first walk: Central Area 1's bank 9 cycle, every 16 frames).
- **The freed panel** blinks once the board has faded back in: its
  look toggles between its old kind and clean every 4 frames, 14 times,
  then stays clean.
- **The sprites:** the faced panel's outline and the items' icons are
  BN5 sprites (list 7: the outline 15, the icons 24, 25, 37, 38, 41 and
  83). The outline shows only while the command window is open over a
  dark panel faced, never while walking.
- **The banners** ("LIBERATE SUCCESS!", "1 TURN LIBERATE!", "TURN n
  START!") are five 32x16 affine OBJs at y 64, built from 8x16 letters
  (`BN5_LIB_LETTERS`, digits `BN5_LIB_BIG_DIGITS`, palette 11):
  they open from squashed to full size in 8 frames, hold 43 and close in
  8.

- **The HUD and the window:** the Order Points gauge is six OBJs at the
  top right (`BN5_LIB_HUD_OAM`), its frame, bar and digits BN5's
  (`0x087E4xxx`, OBJ palette 12); the L view's PHASE and TRGT PHASE are
  OBJs on palette 13; the command window is a frame on BG0 (nine tiles,
  `BN5_LIB_WINDOW_TILES`) with its text in 21 OBJs of 16x16 drawn by
  BN5's code (palette 15), 14 pixels a line, BN5's menu cursor beside
  it; the gauge's bar is 6 pixels an Order Point (`BN5_LIB_HUD_MASKS`);
  the battle's
  banners come from BN5's battle-message table (`BN5_LIB_MESSAGES`),
  the engine BN6 also has.
- **The backdrop** is the mission map's own animated backdrop, recoloured
  purple (BG palette 0 from `0x0861BF20`): outside a mission the same map
  is BN5's teal ACDC Area 3.
- **The phases' banners** ("PLAYER PHASE 02", "DARKLOID PHASE"): a band
  across the board, BG0 filled with one dark colour and shown in WIN1
  alone (rows 56-87), blended so the board shows through at half; it
  opens over three frames from its middle row, and 18 frames on the
  words slide in from the right, 32 pixels a frame, with sound 0xD6, hold
  65 frames and slide out to the left; the band closes 104 frames after
  it opened. The words are OBJs of letters' pieces (`BN5_LIB_PHASE_WORDS`,
  each phase its own palette), the player's with the phase in two
  digits. Before each, BN5's camera pans to the unit about to act (the
  Darkloid, a guardian, the Navi).
- **The L view's panels** are the same band's fill, in WIN1 behind PHASE
  and TRGT PHASE (20-107 by 120-147) and in WIN0 behind the keys' help
  (123-219 by 12-42).
- **The music:** song 0x22 is the board's own in both phases (its clock
  runs on from one to the next: no change of song at a phase), 0x1C plays
  at the mission's opening (the board's routine 1, its tour), the
  liberations' battles BN5's virus battle song 0x15; a battle won in its
  first turn ends on 0x24 in place of the win's usual song, and the
  mission won plays 0x18 over the Colonel's tally of its phases, once
  (its routine 6; [BN5's maps, songs, text and objects](../../../rom/bn5-world.md), Where BN5 plays its songs).
- **The camera** pans to the unit about to act: to the Darkloid before
  DARKLOID PHASE (about 20 pixels a frame, then a quarter of what is left
  each frame, some 32 frames, the band 3 frames after it stops), to each
  guardian as it moves and back to the Navi before PLAYER PHASE; at the
  mission's opening it tours the board to the Darkloid. In the Darkloids'
  phase each pan takes some 22 frames and the camera rests 15 on the unit
  before its turn. Under L the D-pad moves it, 4 pixels a frame, 8 with B
  ("MoveCam(F)"), eight arrows at the picture's edges (their second look 8
  frames of every 32) and a cross at its middle.
- **The guardians on the board** are sprites of BN5's list 6: 83 BigBrute
  (a red, spiked beast), 84 TinHawk, 85 Bladia, each standing in its
  animation by the way it faces (down-left, 5) with its shadow; how they
  move and strike: [The Darkloids' phase as BN5 shows it](darkloids-phase.md#the-darkloids-phase-as-bn5-shows-it-play-rom).

Still to trace: the window's font, when an item's pop shows and goes,
the sounds of a DarkHole closing and of LIBERATE SUCCESS, the Navis' own
board sprites beside MegaMan. (The other eight missions' maps carry the
same sets, but for their backdrops and colours: Each mission's own look,
as built, below.) The mission's end, the L view's keys and the Darkloid's
first word are traced below (The board's words and banners as BN5's, as
built).

So the engine's board is to be BN5's: its overlay tiles, diamonds and
palettes read from BN5's ROM at run time (never kept), its pulse, its
outline, its dissolve, its banners and its item icons, over a floor of
the mission maps' panel grid where BN5's ROM is read. The dark layer's
first build (stage 1) tints BN6's floor instead and has none of the
animations; its rework follows this section.
