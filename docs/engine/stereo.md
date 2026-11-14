---
type: Feature
title: "3D depth: the 3DS's 3D screen"
description: "Options' Display's 3D depth on the 3DS (on by default, `depth_3d` in settings.ini) gives the picture depth through the 3DS's own 3D slider: the world tilted back into the screen row by row, the HUD flat at the screen; how the flat parts are read as a frame ends, how the GPU draws each eye from the frame's own picture, looked at on a computer, what it costs (a first version, sorting every pixel, halved the frame rate), and what is still open."
tags: [engine, display, options, 3ds]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-14T22:40:00Z }
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
picture's edges never cut through something in front of them.

| What | Its depth |
| --- | --- |
| The world: everything but the HUD (the map's backgrounds, the sprites on them, what the engine draws over the game) | By its row: the frame's last row at the screen, its first the deepest, the floor tilted back as BN6's camera looks down on it |
| The HUD's sprites: those in the HP's corner, in an open chat box's rows and in a battle's top 40 rows (depth lighting's rules) | Flat, at the screen, each by its box |
| BG0's tiles (the chat's text) and those of a BG3 in front of the map's backgrounds (a battle's Custom screen) | Flat, at the screen, each 8x8 cell of the frame with a pixel of theirs |

What stands goes back with its rows: MegaMan's head lies a third of a
pixel deeper than his feet with the slider at its top, which no eye
tells from standing upright. The kinds of scene are depth lighting's
(`voxel_scene_kind`): a net layer, another map (the town, Lan's HP) and a
battle are in depth; a menu, a battle beginning, a liberation's board and
a visit to another's Square stay flat in both eyes.

## How a frame is drawn

`src/gfx/stereo.c` says what is flat; `src/emu/stereo_scene.c` reads it;
`src/core/present_3ds.c` draws the eyes.

1. **What is flat, as the frame ends.** On the New 3DS the GBA core runs
   the next frame on its own core while this one is drawn
   ([3ds/README.md](../../3ds/README.md)), so the drawing must not read
   the game: a read would wait for the next frame. A step after each frame
   the core ends (`emu_after_frame`, as the frame's picture is taken in)
   copies the sprites' table, the IO registers and the backgrounds' part
   of VRAM, and reads from them the HUD's sprites' boxes (`gba_obj_boxes`:
   their places alone, none of their pixels) and the cells of BG0 and a
   BG3 in front that have a pixel (each map tile's pixels looked at once).
2. **Each row's spans.** From those, each row of the frame is spans,
   flat or the world's (eight at most, the rest of the row flat).
3. **Each eye on the GPU.** The canvas's picture is already the GPU's
   texture; each eye's picture is drawn from it a few rows at a time
   (rows of the same spans, four at most): the world's spans moved by
   their rows' depth, the gap a span leaves beside a flat one filled with
   its edge's column drawn wide, then the flat spans where they are. The
   left eye's picture goes to the top screen's own target, the right's to
   its right one.

The eyes' parting at the deepest is 6 of the frame's pixels with the
slider at its top (9 of the screen's at the 3DS's 1.5x), the slider's
share of it below; a row's is that times its depth, `(159 - row) / 159`.

### The first version, and why it went

The first build of 2026-11-14 sorted every pixel: the sprites' pixels
read and joined into bodies, each standing at its feet's depth, the
engine's drawing found by the canvas's pixels against the game's, the
world behind a body filled, and three layers made and copied to the GPU
each frame. On the owner's New 3DS it held 30 frames a second with the
slider up (59 down): the drawing took 16 ms of the main core's frame
where it took 11, the present 17 where it took 5, against a 16.7 ms
frame of which the main core had about half a millisecond to spare
(`log.txt`'s frame lines). A body standing at its feet's depth differs
from its rows' by under a pixel at this parting, so the bodies went with
the rest.

## A look on a computer

`CYBERWORLD_STEREO_SHOT=FRAME:PATH` saves that frame's two eyes side by
side (the left, then the right, at 2x, the slider at its top) on any
build, drawn the way the 3DS's GPU draws them (`stereo_compose`, nearest
where the 3DS scales smoothly); with the canvas the game's picture alone.
Two eyes side by side, or the pair flicked between, show the depth
without a 3D screen ([headless captures](../devtools/headless-shot.md)).

## Its cost

Nothing with the slider at its bottom (or the line off). With the slider
up, the main core copies 65 KB of video memory as each frame ends and
reads the HUD's sprites' boxes and two maps' cells from it, says each
row's spans, and the GPU draws each eye's picture from the texture it
has already: some fifty quads an eye where a row is the world's whole, a
few more where the HUD is. Not yet measured on a 3DS: `frame_log`'s line
there says whether it holds 60 frames a second.

## Open

- Measured on a New 3DS, by its owner: the frames' pacing with the slider
  up on a layer, in town and in battle; how deep is comfortable.
- The raised floor of a layer is the world's by its row, not by its
  height (depth lighting's floor model would raise it, at a cost).
- What stands goes back with its rows, not upright at its feet's depth.
- What the engine draws over the game (a layer's name, the Net Square's
  visitors) is the world's by its row: near the bottom, at the screen.

## Checked

`tests/test_stereo.c` (in `build.py test`): what is flat on video memory
of the test's own making (the HP's corner, a battle's top rows and a
chat box's rows by their sprites, BG0's tile and a BG3 in front by their
cells, a standing sprite not), each row's spans (the world, the HP's box,
the world; a row of many spans ending flat), the depth, and the eyes (the
top row moved three pixels each way, the HP's box the same in both, the
gap beside it its edge's colour, the last row unmoved). `build.py 3ds`
builds it; `CYBERWORLD_STEREO_SHOT` was looked over on a layer and in a
battle (the Custom screen flat, the field back).
