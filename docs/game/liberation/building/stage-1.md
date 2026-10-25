---
type: Feature
title: "Stage 1, as built"
description: "Stage 1 as built: the dark layer behind an act's fork (`run_dark_area`), its board made from the seed (`liberation_board.c`), the phases' course, fences, BN5's look, camera and music, and `run.lib` across a CONTINUE."
tags: [game, liberation, bn5]
generated: { by: claude-code/unrecorded, at: 2026-10-25T01:11:12Z }
---

# Stage 1, as built (23 October 2026)

**Where a dark layer stands.** An act's second layer, from the second act,
behind one of the two ways its fork offers in Lan's HP (the act's own area
at the pink pad, or the other way): two acts in three that have a fork hold
one, either way by a coin of the run's seed (`run_dark_area`,
`src/core/run.c`). Nothing is saved for it: the fork is worked out again
from the seed (the plan's area, and the other way as `run_route_alt`
offered it), so a CONTINUE knows. MegaMan says so at the way's portal
("Part of it's gone dark. Dark panels,like Nebula's!"), and the act's card
says "A layer here has gone dark" in its motto's place, in Nebula's purple.
The short net meets one in acts 2 and 3, the endless net in acts 2 to 4.
`--dev dark` makes every act's second layer from the second a dark one.
With BN5's ROM (without it there is no dark layer, nothing to mark) the
way's portal reads dark at a glance before MegaMan steps on it: its own
colours where it is drawn (the pink pad's magenta, a link square's glass)
take the shades of BN5's dark panels' swirl, palette 9 read from BN5's
ROM and stepped on BN5's timing (`BN5_LIB_DARK_FRAMES`: at rest 120
frames, then 16, 16, 32, 16, 16), its lightest the ramp's middle, its
darkest its end (`src/director/liberation_way.c`). The frame alone is
recoloured, within the portal's zone (its cells and its art, `lanhp.c`),
placed by BN6's own camera (eToolkit +0x0C's render point), so BN6's
palettes and their scripts, and the sprites over it, are as they were;
with MegaMan at the portal the mark goes and his sprite stays whole.

**The layer.** `layer_generate` with `layer_make_dark` (`src/net/net_gen.c`)
builds the layer's floor, flat, its arrival and exit, and nothing else of a
normal layer (services, data, bystanders, set pieces), so it gives up the
second layer's Net Dealer and Recovery Mr. Prog: the price of the way.
`LAYER_MAKE` 97.

**The board** (`src/director/liberation_board.c`, ROM-free, unit-tested):
the layer's floor, one cell a panel (32 world units), from the arrival's
reach (the floor of the layer's map as built since: The board on the map's
floor, below); its kinds placed by `tools/liberation.py`'s rules from the layer's
seed: the start zone clean (two panels' walk), the Darkloid's 3x3 at 60% of
the farthest walk or more, the DarkHoles in whole rings of floor, a third of
the walk on, six apart and four off the Darkloid, each ring dark and its
guardian on it (BigBrute the first); dark panels thickening with the walk
and gathered by one majority pass; BonusPanels at dead ends; items in the
dark's corners first (one of Order Points always, then hearts and Order
Points); roaming guardians from act 3. A smaller team than BN5's meets a
DarkHole less for every two Navis short and dark five points thinner each
(MegaMan alone: one DarkHole in act 2, two in acts 3 and 4, three in 5
and 6). Rejected and the next seed tried as the prototype's guarantees say
(nothing to liberate in the first phase, a guardian within three of the
start, no room for the Darkloid or the holes); a layer whose eight seeds
fail is made plain. Over 30 layers an act: 190-240 panels, 40-47% dark.

**The par** (stage 1's estimate, `lib_par`, the dark panels on the way at
two a phase) gave act 2 12 (8-21); the planner's replaced it (The quality
bar, as built).

**The course** (`src/director/liberation.c`): after the arrival, Lan and
MegaMan name the board ("Like Nebula's dark areas,Lan!", its DarkHoles and
par), then PLAYER PHASE. MegaMan walks the clean panels as BN6 walks him,
as BN5's Navis walk its board (AGENTS.md: no movement of the engine's):
every panel not walked as the board is made (dark, an item's, a
DarkHole, the Darkloid's ring and his panel) is fenced with walls of BN6's
own on its cells that meet another panel's floor (`liberation_fence.c`).
A wall whose value has bit 7 set is skipped while event 0x1640 + its low
bits is set (Rush's gaps' walls, [ROM data](../../../rom/overview.md)), and the fence's walls
name one of two: 0x16A0, never set, where the wall stands, or 0x16A1, set
each frame (entering a map clears it), where it is skipped (the Link Navi
obstacles' walls take 0x1640-0x1647 and Rush's gaps' 0x16BE-0x16BF). As a panel is
liberated, or darkened again, its walls are switched between the two in
BN6's coordinate data in RAM (`BN6_MAP_COORDS`, which BN6 reads in place
and decompresses again at each map entry, after a battle too), as BN5
raises and lowers its border cells' heights per panel
(`BN5_LIB_EDGES`): no board has more fences than BN6's events allow,
however many of its panels are dark. (A panel the map left without floor
at its middle, netmap's legal.c, was off the board; the board is made on
the map's floor since.) A beside the panel he faces (the one
his facing points at, a little ahead of him) opens the command menu on it: **Liberation** (a battle of BN6's: its
virus battles' easier half with two dark panels round it or fewer, the
band with three to six, the next act's band one version up with seven or
eight, a DarkHole or its guardian; won, the panel is liberated, and the
eight round it where no second Custom screen opened, BN5's one-turn win),
**LongSwrd** (an Order Point: two panels in a line, one battle) or
**Pass** (half his max HP back, BN5's guides; a fifth since stage 3,
BN5's as played). Item panels give Order
Points (3 to 8) or a heart (30% of max HP); a BonusPanel stepped on spun
four of BN5's roulette's slots (since BN5's own: BonusPanels as BN5's, as
built, below). Then DARKLOID PHASE: each guardian
steps toward him over the dark (BigBrute two and hits all in one,
TinHawk three and one in three, Bladia two, darkening what it lands on),
the Darkloid hits him within his reach (two or three); the board's harm
never takes his last HP (alone, a deletion off a battle would end the run
unfought). A DarkHole sends its deleted guardian back as BN5 does, at the
Darkloids' phase's end while MegaMan stands within three panels of it
(`lib_guardian_return`; The Darkloids' phase as BN5 shows it). Every DarkHole down, the Darkloid's ring can be liberated, and
the Darkloid fought from it: a Navi of BN6's whose HP fits the act's band,
no act's guardian (`run_darkloid`), in his own battle. Deleted, the exit
opens where he stood and the par's prize is given, BN5's tiers (The par's
prize, as built, below); the board's battles pay BN6's own rewards beside
it. No random battles on the board.

**On the map, first look.** The units are the layer's own map objects (`OBJ_UNIT`:
HeelNavis for the guardians, the Darkloid in his own overworld sprite),
moved by their objects' places. The dark panels are the layer's own floor
recoloured as it shows (`src/director/liberation_draw.c`), in Nebula's
purple, the Darkloid's ring redder, a DarkHole near black, a BonusPanel
gold, the panel MegaMan faces lit: pixel by pixel in the frame, from the
map layer's scroll (BG1's: `x + y + 256` and `(y - x) / 2 - 256` of the
world at the picture's middle, mod 512, read on captures as MegaMan was
moved), a pixel in a sprite's colour or under a chat box left alone; BN5's
own board tiles are not read yet. The phase, par and Order Points in the
corner, the banners and the command menu in BN6's PET look
(`src/gfx/pet_look.h`); every word in BN6's chat boxes
(`src/director/liberation_words.c`).

**On the map: BN5's own look** (`src/director/liberation_look.c`, with
BN5's ROM). A dark layer is drawn in a BN5 Liberation map (0x86:00 at
first; its own mission's since: [Each mission's own look](mission-look.md)), as an
older net's area is (a held area of BN5's, never a run's dressing): its
floor learned from the board's own panels, which the mission's record lists
(`area_src_board`: the map's walls cover its walkways alone, the board
moving its units itself), at the record's origin (`+6`, `+7`, the panels'
edges at X = -o6 and Y = -o7, mod 32), not its pads' look; its tiles and
its board's colours (`BN5_LIB_BOARD_PAL_ACDC`, loaded over the map's as BN5's
board loads them: the frames grey) copied in. Each panel's kind lies over
it on BG2 as BN5 lays it: one of BN5's twelve looks (`BN5_LIB_LOOKS`,
picked by the kind as `BN5_LIB_LOOK_OF` picks it: clean cyan, the dark
swirl, the item's stud, the DarkHole's pit, the BonusPanel's "?", the
Darkloid's ring a grey grid until the DarkHoles fall, the start clean's:
BN5's start look is blank over its map's pad, and ours stands among its
clean panels on a panel's frame, which bare read as a hole, the owner
found; the start's 21 panels of mission 1 are left out of the floor
learned, as their pad's pieces drew lilac round the start), its 8 x 4 entries at the panel's centre less 32 and 16 pixels and 8
down (fit on the dumped map: the looks over the frames' dark middles, 1.6%
over the void), their tiles copied from BN5's ROM to where BN5's board
copies them (tile 864 on, VRAM taking words), and the board's palettes
animated as BN5 times them (traced): the dark panels' swirl (9) at rest 120
frames, then three frames down and up, 216 frames round; the DarkHoles'
(10) four frames there and back, 20 frames a step; the barriers' glint
(11) every 132 frames; the BonusPanel's breath (12) six frames there and
back. BN6 reads the tile map in place and builds each BG's
screen map in IWRAM, copied to VRAM every frame (`src/emu/bn6_map.h`): a
look goes into the map's layer 1, and BG2's screen map is made from it for
the picture a while after the looks change (the map's layer 1 and BG2
cleared first, as BN5's BG2 holds the looks alone; again after a battle,
whose return loads the tile map afresh). A panel freed blinks as BN5's
do (traced): its old look while the board fades back in (27 frames), then
clean and old by turns every 4 frames, 14 times (`BN5_LIB_BLINK`,
`BN5_LIB_BLINKS`), then clean. The panel MegaMan faces, where A
takes a command there and while the menu is open, is outlined by BN5's
own sprite (list 7's 15, `BN5_LIB_OUTLINE`, read from its ROM: its
yellow blink, 24 frames lit and 24 not, from the moment he turns to it).
The backdrop is the mission's: map 0x86:00's own animated tiles in its
purple (`BN5_LIB_BACKDROP`), as BN5's map group 0x86 loads them while a
mission is on (its LoadBGAnim and LoadGFXAnims each choose a mission's
table by `BN5_LIB_ON_MAP`; `src/gfx/xbackdrop.c` reads both shapes and
takes the mission's). The mission's list moves the board's palettes too
(banks 9-12, `BN5_LIB_ANIM_*`, their writes moved to BN6's palette
buffer): BN5's own scripts are their one writer, so the host map's own
(the Central Area map the layer takes over, whose bank 9 flashed teal
and red every 16 frames) no longer runs there. BN5's own pieces are drawn over it from BN5's ROM
(`src/director/liberation_pieces.c`, its tiles through
`src/gfx/xtiles.c`), each where BN5 lays it (traced): the Order Points
gauge at the top right (its six OBJs, the frame's tiles from its pieces,
the bar masked by the count as BN5 masks it, filling from the right),
the L view while L is held ("PHASE" and "TRGT PHASE" with their numbers,
without BN5's corner arrows: BN6's camera stays on MegaMan), the command
window in BN5's frame on BG0's place with its cursor (its words in BN6's
menu font, white, a command not to be taken in the paler colour of
BN5's letters' edges), "LIBERATE SUCCESS!" or "1 TURN LIBERATE!" squashed in and out
over the board once it has faded back in (27 frames, then 58; MegaMan's
words wait for it), and each item freed popped over its panel in its own
icon until the words of it end, BN5's jingle (0x16D) as they begin.
The phases' banners are BN5's (traced, How the board looks and moves):
the band blended over the board (`xtiles_blend_rect`, the colour whole
and the board at half, as BLDALPHA's 16 and 8), the words' pieces at
BN5's places, sliding in with BN5's sound and out on its frames, 104 in
all, begun once nothing else shows (a CONTINUE's act card first); the L
view's panel behind its phases the same. The guardians stand in BN5's
own sprites (list 6's 83-85, copied into BN6's list 7 at numbers Gregar
leaves on a placeholder, `src/layer/xnavi.c`), the Darkloid in his own.
BN6's own camera moves as BN5's (`src/director/liberation_camera.c`):
held, its mode is 0 (it no longer copies MegaMan's place) and its place
is written each frame, panning at BN5's pace to the Darkloid at the
opening (his entrance: held there a second, then back before the
briefing) and before DARKLOID PHASE, to each guardian as it moves in his
phase (resting BN5's 15 frames on each before it moves), back to MegaMan
before PLAYER PHASE; under L the D-pad moves it at BN5's 4 and 8 pixels a
frame, kept over the board, BN5's arrows and cross over it; let go, it
follows MegaMan again. Stage 3's R, BN5's switch between Navis, pans with
the same `lib_camera_at` to the Navi taken. MegaMan walks as BN6 walks him throughout: it moves only
while he stands. The board's music is BN5's: its song 0x22 the dark layer's map's own
(`lib_board_song`), 0x1C at the briefing, 0x22 again at PLAYER PHASE
01, copied into BN6 after the areas' songs and themes (`xsong.c`, each
keeping its place) and started by BN6's PlayMusic
(`liberation_sound.c`); BN5's one-turn win 0x24 at a battle won in its
first turn, and 0x18 as the board is won, the board's own again once it
ends (`liberation_cues.c`; why, [BN5's music in a run](../../../multirom/music.md)).
Without BN5's ROM the board keeps stage 1's first look: the act's own floor
recoloured in Nebula's purple, pixel by pixel. The units, the corner, the
banners and the menu are as above (BN5's own still to come).

**Checked:** `tests/test_core.c` (`test_liberation_board`: 150 dark layers
over acts 2-6, the same board twice from a seed, the Darkloid's ring whole,
each hole's ring dark, the start clean, no guardian by it, nine boards in
ten made); the autopilot plays the board (`liberation_auto.c`: the dark
panel that brings it nearest the next target, Pass below two fifths of its
HP): `CYBERWORLD_AUTOPILOT=1 build.py shot --scene emu --run-depth 5 --dev
dark,god,onehit --frames 48000` (the autopilot walking MegaMan with the
pad as a player does) liberated seed 1's act 2 board in 23
phases against its par 14 (the autopilot never wins in one turn), deleted
CircusMan, took the prize (Recov80 *) and the exit, and went on through act
2's guardian, home and act 3's dark layer.

**Across a CONTINUE** (`src/director/liberation_save.c`): the board's
panels, units (where each stands, which are deleted and when a DarkHole
sends one back), the phase, the step and the Order Points are saved
beside the run's checkpoint as `run.lib` ([Portable saves](../../../engine/saves.md)) and taken back
once the checkpoint's state has loaded on the layer its seed made again
(the same layer only: another build's or area's starts afresh, as the
layer does). What was being shown begins again: a banner from its start,
a talk's step its next (no chat box is open as a checkpoint is saved),
the menu closed. Checked: the autopilot's board quit at PLAYER PHASE
04's banner (SIGTERM, the quit's checkpoint) and continued: the same
board, phase 4, the panels it had freed clean and its Order Points as
they were, the banner shown again once the act's card was gone.

**Left for later:** a guardian's HP under it (the Darkloid's came with
The board's HP as BN5's); the economy report and pacing for
dark layers; the second screen's board; stages 2 to 5. (Barriers, keys,
the sealed region, traps, chips and zenny in item panels and the
planner's par came with the quality bar, below.)

Moving BN5's own board code (0x12000 bytes of Thumb, its tables, text and
graphics) into BN6's core instead was weighed: its exact rules for free,
but most of it calls BN5's overworld engine at other addresses, and the
generated boards need none of BN5's missions. Its code is read for the
rules where the guides are unsure (the commands' shapes, the Darkloid's
heal).
