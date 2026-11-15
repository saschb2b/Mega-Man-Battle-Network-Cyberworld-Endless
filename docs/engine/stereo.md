---
type: Feature
title: "3D depth: the 3DS's 3D screen"
description: "Options' Display's 3D depth on the 3DS (on by default, `depth_3d` in settings.ini) gives the picture depth through the 3DS's own 3D slider: the moving background deepest, the floor tilted back by its rows, what stands on it (MegaMan, viruses, people) upright at its feet's depth, the HUD flat at the screen; how each pixel's part is read as a frame ends and carried in its alpha, how the GPU draws each eye in layers from the frame's own texture, looked at on a computer, what it costs, the two versions before it, and what is still open."
tags: [engine, display, options, 3ds]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-15T01:30:00Z }
sources:
  - id: libctru
    resource: "https://libctru.devkitpro.org/gfx_8h.html"
    title: "libctru's gfx.h (gfxSet3D, gfxIs3D) and os.h (osGet3DSliderState), as installed in the 3DS build's image (docker/Dockerfile.3ds), read on 2026-11-14"
  - id: citro3d
    resource: "https://github.com/devkitPro/citro3d"
    title: "citro3d's C3D_RenderTargetSetOutput (a target for the top screen's right eye) and citro2d's C2D_DrawImageAt, as the 3DS's present already uses them"
---

# 3D depth: the 3DS's 3D screen

The owner, 2026-11-14, with [depth lighting](voxel.md) made fast: "the
perfect entry to add an additional way to enable 3d mode on a 3ds to give
everything some depth". Depth lighting itself is too heavy for the 3DS's
ARM11 cores; its 3D screen is the 3DS's own way to give the picture
depth, and needs far less.

On the 3DS, Options' Display's depth lighting line is **3D depth** ([the
panel's rules](ui.md)), on as it comes (the owner's call, 2026-11-14: the
slider is the switch a 3DS player reaches for, and costs nothing down),
kept as `depth_3d` in `settings.ini`, a key of its own: builds before it
wrote `depth_lighting = off` into every `settings.ini`, the 3DS's too,
where no line could set it, so the 3DS reads `depth_3d` alone. START's
DEFAULT puts it back on. On, the screen is put in 3D (`gfxSet3D`) and
the 3D slider sets how deep the picture goes; at the slider's bottom the
screen shows the left eye alone and nothing is sorted. Off, the screen is
not in 3D and the slider does nothing. Its notes: "The 3D slider sets the
world's depth" and "The picture flat, as BN6 draws it".

## What goes where

Nothing comes out of the screen: the picture goes back into it, so the
HUD, at the screen, is never behind what the world shows, and the
picture's edges never cut through something in front of them. The
owner, 2026-11-15, of the version that tilted the whole picture by its
rows: "it seems to just tilt the layer. yet no elements really reflect
their z axis state as 3d depth", and "we don't need pixel based 3d. just
giving the elements, player, and moving background some real depth".
So the picture is three kinds of thing, each at a depth of its own:

| What | Its depth (the eyes' parting with the slider at its top, in the frame's pixels) |
| --- | --- |
| The moving background: BG3 on a net layer and behind a battle's field (mGBA's tag on each pixel of the core's picture, BG3's or the backdrop's) | Tilted back by its rows behind the map: 5 at the last row, 10 at the first |
| The map: the floor's backgrounds, the sprites lying on it (priority 3, and the one-colour blob BN6 lays under one who stands), blended ones, and what the engine draws over the game | By its row, tilted back as BN6's camera looks down on it: 0 at the last row (at the screen, as the HUD), 7.5 at the first, the background 2.5 to 5 behind it at every row |
| What stands: each sprite drawn plainly (not blended, not lying), outside the HUD's places; sprites that touch or overlap joined into one body (a Navi's or a virus's parts) | Upright, the whole body at the map's depth under its feet (its box's last row): its head no deeper than its feet |
| The HUD's sprites: those in the HP's corner, in an open chat box's rows and in a battle's top 40 rows (depth lighting's rules) | Flat, at the screen, each by its box |
| BG0's tiles (the chat's text) and those of a BG3 in front of the map's backgrounds (a battle's Custom screen) | Flat, at the screen, each 8x8 cell of the frame with a pixel of theirs |

In town and Lan's HP the background stays the map's (BG3 there may be a
part of the room), so the map and what stands on it go back, and the
background with them. The kinds of scene are depth lighting's
(`voxel_scene_kind`): a net layer, another map (the town, Lan's HP) and a
battle are in depth; a menu, a battle beginning, a liberation's board, a
visit to another's Square, and a screen of BN6's menus whose HUD
backgrounds cover a quarter of it or more (the Custom screen opened, a
result screen) stay flat in both eyes.

## How a frame is drawn

`src/gfx/stereo.c` says what is flat and what stands;
`src/emu/stereo_scene.c` reads it; `src/emu/emu_picture.c` marks each
pixel's part; `src/core/present_3ds.c` draws the eyes.

1. **What is flat and what stands, as the frame ends.** On the New 3DS
   the GBA core runs the next frame on its own core while this one is
   drawn ([3ds/README.md](../../3ds/README.md)), so the drawing must not
   read the game: a read would wait for the next frame. A step after each
   frame the core ends (`emu_after_frame`, as the frame's picture is taken
   in) reads the sprites' table and the backgrounds' VRAM where the core
   keeps them (`emu_memory`: copying their 65 KB cost the 3DS a
   millisecond a frame) and the IO registers, and from them the HUD's
   sprites' boxes and the cells of BG0 and a BG3 in front that have a
   pixel (`stereo_flat`), then the standing sprites' pixels, each one's
   own (`gba_obj_pixels`), and their boxes joined into bodies where they
   touch (`stereo_sprites`, 48 at most, the rest joined to the last).
2. **Each pixel's part in its alpha.** The picture is turned into the
   3DS's texture order a pixel at a time already; in 3D each pixel's
   alpha says its part: a standing sprite's 0xFF, the moving
   background's 0x00, the map's 0x80. What the engine draws over the
   picture on the canvas lands at 0xFF.
3. **Each eye on the GPU, in layers.** The canvas's picture is the GPU's
   texture; each eye is drawn from it with replacing blending (the
   alpha the GPU tests, not one it blends by):
   - the background (an alpha test passing under 0x40): first as one
     quad sheared by the map's rows (citro2d's view shear: the first
     row's shift falling evenly to the last's), filling where the
     background's own move uncovers what no picture shows, then as one
     quad sheared by its own rows' depth;
   - the map and what stands (passing over 0x40), one quad sheared as
     the map; the gaps each world span leaves beside a flat one, filled
     with its edge's column drawn wide;
   - each body (passing over 0xC0: the sprites' own pixels, none of the
     floor in its box), unsheared at its feet's depth, over the map's
     copy of it a pixel away at most;
   - the flat spans where they are, rows of the same spans drawn as
     one.

   The left eye's picture goes to the top screen's own target, the
   right's to its right one.

A depth of 1 parts the eyes by 10 of the frame's pixels with the slider
at its top (15 of the screen's at the 3DS's 1.5x), the slider's share of
it below; each eye moves half of that, the left one left. A first parting
of 6 (9 of the screen's) left the slider's part in it too small for the
owner to see.

### The versions before

The first build of 2026-11-14 sorted every pixel on the main core: the
sprites' pixels read and joined into bodies, each standing at its feet's
depth, the engine's drawing found by the canvas's pixels against the
game's, the world behind a body filled, and three layers made and copied
to the GPU each frame. On the owner's New 3DS it held 30 frames a second
with the slider up (59 down): the drawing took 16 ms of the main core's
frame where it took 11, the present 17 where it took 5, against a
16.7 ms frame of which the main core had about half a millisecond to
spare (`log.txt`'s frame lines).

The second tilted the whole world by its rows, one sheared quad, the HUD
flat: it held 60, and the owner saw a tilted picture, nothing standing
in it. This one keeps its quad and gives the elements their depth on the
GPU: the main core marks pixels it already turns into the texture, and
the GPU tests their alpha, drawing the picture two more times and each
body once.

## A look on a computer

`CYBERWORLD_STEREO_SHOT=FRAME:PATH` saves that frame's two eyes side by
side (the left, then the right, at 2x, the slider at its top) on any
build, drawn as the 3DS's GPU draws them (`stereo_compose`, the same
layers in the same order, each pixel's part from mGBA's tags and the
standing sprites; nearest where the 3DS scales smoothly). Two eyes side
by side, or the pair flicked between, show the depth without a 3D
screen ([headless captures](../devtools/headless-shot.md)). On a layer of
2026-11-15 the pair parted the background by 10 of the frame's pixels,
MegaMan by 4 at his head and his feet alike, the floor under him by 4
and its nearest rows by 3, the HP's box by none; in a battle the field's
background stood behind the panels, MegaMan and the Gunner upright on
them. That pair's background was a wall at one depth, and its floor
tilted from 1.5 to 6: the owner, 2026-11-15, on the 3DS: "the moving
background looked good before in 3d. now it just flat". With the
background tilted by its rows again and the floor from the screen to
7.5, the pair parts the background's top by 9, MegaMan by 3 at head and
feet, the HP's box by none.

## Its cost

Nothing with the slider at its bottom (or the line off). With the slider
up, as each frame ends the main core reads the sprites' table and two
maps' cells in place and marks the standing sprites' pixels (a few
thousand), and sets each pixel's alpha as it already turns the picture
into the texture; the GPU draws each eye's picture three times (the
background twice, the map once) and each body's box once, with a few
quads for the HUD's spans and the gaps beside them. A screen of BN6's
menus drawn flat costs one quad an eye. `frame_log`'s line on the 3DS
says whether it holds 60 frames a second ([Frame
log](../devtools/frame-log.md)).

Measured on the owner's New 3DS, 2026-11-15: on a layer it held 59, the
GPU drawing a frame in 7 ms; in a battle and at a layer's card it fell
to 35-49, the GPU at 12-15 ms with only 1-3 bodies and 9-16 quads a
frame, and the canvas's blit into its texture taking twice its time
beside it. Neither the quads nor the layers were the cost: the GPU read
the canvas's texture from the main memory, where the GBA core's core and
the main core work, and read and wrote a depth buffer at each pixel it
drew. The texture now lives in VRAM and the targets have no depth
buffer (the picture is drawn in order).

## Open

- Measured on a New 3DS, by its owner: this version's frames on a layer
  and in a battle; how deep is comfortable.
- The raised floor of a layer is the map's by its row, not by its
  height (depth lighting's floor model would raise it, at a cost).
- What the engine draws over the game (a layer's name, the Net Square's
  visitors) is the map's by its row.
- A body's map copy under it, a pixel off at most, shows at its edge
  where the floor's tilt is steepest.

## Checked

`tests/test_stereo.c` (in `build.py test`): what is flat on video memory
of the test's own making (the HP's corner, a battle's top rows and a
chat box's rows by their sprites, BG0's tile and a BG3 in front by their
cells, a standing sprite not), each row's spans (the world, the HP's
box, the world; a row of many spans ending flat), the depths (the map
tilted short of the background, a body at its feet's map depth), the
standing sprites (their pixels marked, two touching joined into one
body, a lying one and a blended one not, the HP's corner not), and the
eyes (the background moved 5 each way, the map at its row's depth, a
body's head moved as its feet, the HP's box the same in both).
`build.py 3ds` builds it; `CYBERWORLD_STEREO_SHOT` was looked over on a
layer and in a battle (above).
