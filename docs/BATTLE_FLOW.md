# Battle flow

The battle follows the original frame by frame. The timings below were
measured from a random virus battle recorded in `tools/romlab` (every frame
with its IO registers and the MP2K player state; see "Recording" at the end)
and are the numbers `src/battle.c` and `src/scene_net.c` use. Frames are
1/60 s. Sound ids are MP2K song numbers.

## Backgrounds

Each area uses one of the game's battle backgrounds (records at 0x82058,
animations at 0x822E0): Central 0x07, Seaside 0x0B, Sky 0x04, Green 0x0D,
Graveyard 0x14, Undernet 0x0F, Secret 0x13, Cybeast Nest 0x15. Most drift
right one pixel every 2 frames and down one every 4; 0x06 only down, 0x0A
and 0x0C only right, 0x0F and 0x11 stand still, 0x12 rises one every 4 and
0x14 drifts left one every 16.

## Custom emblem

Picking a chip spins the emblem above the chip column once, zooming up to
1.23x, over 17 frames from the second frame after the press (OAM affine
values recorded frame by frame).

## Encounter

| Frame | Original | Engine |
| --- | --- | --- |
| 0 | Sound 0x78, the area music stops | `begin_battle` in `scene_net.c` |
| 1-15 | Horizontal mosaic (BG block n+1, sprites n+1 square) while palettes fade to white by n/16 | `P.fx_mosaic`, `P.fx_fade` |
| 16 | Screen white; the battle starts | `battle_begin` |

## Start of battle

Counted from the first white frame.

| Frame | Event |
| --- | --- |
| 0-72 | White (the game is loading) |
| 73 | Battle music (0x15 viruses, 0x16 navis) and a 16-frame fade in. The field, MegaMan, the HP box and the emotion window are already there |
| 90 | The first enemy materialises: sound 0x94, then 32 frames in which it fades in by 1/16 every 2 frames under an OBJ mosaic shrinking from 16 to 2 pixels. The next enemy starts when one finishes |
| last + 1 | Enemy HP numbers appear |
| last + 5 | Custom window, sound 0x79 |

## Custom screen

- The window slides in 12 pixels a frame over 10 frames while the field sinks
  1.5 pixels a frame (15 in all). The HP box and emotion window ride on the
  window's right edge. Enemy names appear once it is in, on dark tabs
  (0x6E4020) in the battle font.
- Cursor 0x7F, pick 0x81, OK 0x82. The cursor pulses between two frames
  (0x6E3540 and 0x6E3560) every 8 frames. A picked chip leaves its dark slot
  and code letter and stays shown in the chip window.
- With the cursor on OK the art slot shows "CHIP DATA TRANSMISSION"
  (0x7204F0).
- R opens the description one frame later in a 27x8 text box over the
  bottom of the screen, one line a frame, the arrow with the third (0x9C).
  R, A or B closes it (0x9E): the arrow goes, then the box folds to 6, 4 and
  2 rows; the cursor returns two frames after.
- A on OK: one frame later the window slides out 12 pixels a frame. The frame
  it is gone the gauge appears.

## BATTLE START

Every turn, not only the first, starts the same way. From the frame the
gauge appears the world is frozen:

| Frame | Event |
| --- | --- |
| 51 | Chip icons above MegaMan |
| 57 | BATTLE START banner: affine OBJs scaled vertically, inverse scale 768, 640, 512, 384, 256, 208, 224, 256; closing 320 to 896; 58 frames in all |
| 116 | The fight starts; the gauge counts from the next frame and the chip name appears two frames in |

The chip name is drawn in 8x16 cells at the bottom left, its power in the HP
box's yellow digits (glyphs 12-21).

## Fight

- Gauge: +1 a frame, full at 512, drawn 1 pixel per 4 (127 at most). Full:
  sound 0x8F, the bar cycles four patterns every 7 frames and the "L or R"
  label changes colour every 8 (tiles from 0x6E2A20).
- L or R with a full gauge: the fight runs 8 more frames, the gauge and chip
  icons go on the 9th, the window starts in on the 10th.
- START pauses (0x9F): PAUSE at (100, 63), world frozen, the gauge animation
  and HP rolls carry on.
- HP shown rolls toward the real value by an eighth of the gap plus 4 a frame
  (MegaMan) or plus 2 (enemies). MegaMan's box uses the damage palette
  0x6DFC3C for 17 frames after a hit; enemy digits use the red glyphs (10 on)
  while they roll. At 1 HP the original beeps (0x84) every 45 frames and
  at half it does not; the engine beeps from a quarter down.
- Hits: 0x6D on an enemy, 0x6B on MegaMan. MegaMan blinks 2 frames on,
  2 off while invulnerable.
- Buster: the shot leaves 4 frames after B is released (sound 0x6A, spark
  hit sprite 5). Charge (level 1): lines (GUI sprite 162) and sound 0x71 11
  frames into the hold, charged at 101 with 0x72 and the pink palette
  0x3AB1B0; each level takes 12 frames off. A charged shot leaves 8 frames
  after release, MegaMan raising the buster at 7, and bursts in hit sprite 4.
- Mettaur: sound 0x183 as it raises its pickaxe, the shockwave (effect
  sprite 3) 50 frames later, one panel every 22 frames with 0xA6 each step.
  The panel under the wave turns solid yellow (colour 14 of 0x6DE5BC) unless
  the wave hits someone there.

## Chips

Counted from the A press.

- Cannon: attack sprite 1 (anim 1 for HiCannon and M-Cannon) draws the
  barrel, flash and blast at MegaMan + (24, -25); sound 0xAE at 15, the hit
  at 17 with hit sprite 1.
- MiniBomb: the bomb in MegaMan's hand (attack sprite 2, anim 0) from 2 with
  sound 0xB2; from 13 it flies 37 frames to the panel three ahead, x from 13
  px past MegaMan to 4 short of the target, 58 + 16t/15 - t²/15 above the
  row, its shadow on the ground (the two objects of anim 1 drawn apart); it
  bursts with sound 0x70 in the explosion sprite.
- Swords: sound 0xB0 at 5, the slash and hit at 14; attack sprite 20, anim
  2 Sword, 0 WideSwrd, 1 LongSwrd, at the next panel + (2, -11).

## Deletion and victory

- A deleted enemy flashes white every other two frames for 33 frames. The
  explosion (GUI sprite 155, 22 frames, sound 0x6F) goes off at 2 frames and
  again at 18, the second 12 left and 7 up.
- Three frames after the last enemy is gone: ENEMY DELETED (same banner
  timing), the win music (0x19), and the gauge, emotion window and chips
  disappear.
- 48 frames after the banner the RESULT window slides in from the left, 16
  pixels a frame, landing at x 24 on frame 13. PRESS A BUTTON (0x730AF0)
  blinks 8 on, 8 off from frame 19.
- A: the reward art appears one 8x8 tile a frame in random order from 5
  frames later, a tick (0x7E) every 4 frames from 6, then the name at 76 with
  0x95. Zenny uses the picture at 0x730D90.
- A again: 22 frames, a 16-frame fade to black, the music stops at 42, and at
  62 the net fades back in over 16 frames with its music.

## MegaMan deleted

Sound 0x6C. MegaMan stays a white silhouette for 22 frames, then fades out
over 32 under a growing mosaic. MEGAMAN DELETED at 57, a fade to black from
153, the music stops at 167, and the game over screen follows at 177.

The GAME OVER screen (tiles in LZ77 block 0x6C211C, palette 0x6C25AC, the
two layers recorded as layouts) fades in over 10 frames, starts its music
(0x1B) at 6, races its streaked grid right 16 pixels a frame and fades out
over 32 frames from 180. The engine then shows the run summary.

## Recording

`tools/romlab/romlab.c` has `rec N PREFIX` (a PPM per frame and the IO
registers in `PREFIX.io`), `watch ADDR LEN` (memory appended per frame, used
on the MP2K players at 0x2010480) and `pokes`. A random battle was forced by
patching the encounter check (0x5A98, a branch straight to the roll) and the
roll itself (0xABD30, returning a battle-settings record written to free ROM
space at 0x7FE400). The player table at 0x159DC8 maps each MP2K player to its
`MusicPlayerInfo`; a change of song header or a restart of the clock marks a
sound. `tools/romlab/labtrace.py` resolves captured tiles, palettes and OBJs
to ROM offsets and sprites.
