---
type: Feature
title: "3D depth: the 3DS's 3D screen"
description: "Options' Display's 3D depth on the 3DS (off by default, `depth_lighting` in settings.ini as on other systems) gives the picture depth through the 3DS's own 3D slider: the world tilted back into the screen row by row, what stands at its feet's depth, the HUD and the engine's drawing flat at the screen; how a frame is sorted from the video memory its frame ended with, drawn for each eye by the GPU, looked at on a computer, what it costs, and what is still open."
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
panel's rules](ui.md)), off as it comes (the owner's default for depth
lighting), kept as `depth_lighting = on` in `settings.ini`, the same key
as on every other system. On, the screen is put in 3D (`gfxSet3D`) and
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
| The world: the map's backgrounds, the sprites that lie on the floor (a warp pad, BN6's blob under who stands) and the blended ones (a shadow, a light) | By its row: the frame's last row at the screen, its first the deepest, the floor tilted back as BN6's camera looks down on it |
| A sprite that stands, joined with those it touches (8-connected) into a body | Its feet's row: a body stands upright at one depth, not tilted with the floor |
| A sprite behind a background in front of it | The world's (it shows the background) |
| The HUD: BG0 (the chat's text), BG3 in front of the map's (a battle's Custom screen), the sprites in the HP's corner, in an open chat box's rows and in a battle's top 40 rows | Flat, at the screen |
| Whatever the engine draws over the game (a pixel of the canvas unlike the game's picture), and the canvas outside the game's picture | Flat, at the screen |

Where a body or a flat pixel stood, the world behind takes the colours
beside it, half from each side: an eye sees a little round what stands
before the world, and there the frame has nothing. The kinds of scene are
depth lighting's (`voxel_scene_kind`): a net layer, another map (the
town, Lan's HP) and a battle are sorted; a menu, a battle beginning, a
liberation's board and a visit to another's Square stay flat in both
eyes. The rows a sprite counts as the HUD's are depth lighting's too.

## How a frame is sorted

`src/gfx/stereo.c` sorts and draws; `src/emu/stereo_scene.c` feeds it;
`src/core/present_3ds.c` shows it.

1. **The frame's memory, kept.** On the New 3DS the GBA core runs the
   next frame on its own core while this one is drawn
   ([3ds/README.md](../../3ds/README.md)), so the drawing must not read
   the game: a read would wait for the next frame. A step after each
   frame the core ends (`emu_after_frame`, as the frame's picture is
   taken in) copies its video memory (VRAM, OAM, the palette, the IO
   registers) and what the frame is of, while the slider is up.
2. **The sort, in the drawing.** The sprites are read from that copy
   ([depth lighting's reader](voxel.md), `gba_layers_sprites`), BG0 and a
   BG3 in front read a row at a time, a sprite's pixel looked under for a
   background in front of it, and the standing ones joined into bodies.
3. **The layers, at the present.** With the canvas whole (the engine's
   drawing over the game), three layers of the canvas's size are made in
   memory the GPU reads: the world (the holes filled), the bodies and the
   flat; a clear pixel beside a layer's own takes its colour, so the
   GPU's smooth scaling mixes an edge with its own colour, not black.
4. **Each eye.** The GPU draws each eye's picture from the layers: the
   world as 160 strips of a row each, each moved by its row's depth; then
   each body as one quad, deepest first, moved by its feet's; then the
   flat layer, unmoved. The left eye's picture goes to the top screen's
   own target, the right's to its right one.

The eyes' parting at the deepest is 6 of the frame's pixels with the
slider at its top (9 of the screen's at the 3DS's 1.5x), the slider's
share of it below; a row's is that times its depth, `(159 - row) / 159`.

## A look on a computer

`CYBERWORLD_STEREO_SHOT=FRAME:PATH` saves that frame's two eyes side by
side (the left, then the right, at 2x, the slider at its top) on any
build, drawn the way the 3DS's GPU draws them (`stereo_compose`, nearest
where the 3DS scales smoothly); with the canvas the game's picture alone.
Two eyes side by side, or the pair flicked between, show the depth
without a 3D screen ([headless captures](../devtools/headless-shot.md)).

## Its cost

Nothing with the line off or the slider at its bottom. With the slider up,
the 3DS's main core copies 98 KB of video memory a frame, sorts the
frame (its sprites' pixels, BG0's rows, a few backgrounds' pixels under
the sprites, a flood fill of the bodies), makes the three layers from the
canvas (each of its 38,400 pixels once, then the holes and edges), and
the GPU copies three textures where it copied one and draws about two
hundred quads an eye. Not measured on a 3DS: `frame_log`'s line says the
frames' pacing there.

## Open

- Measured on a New 3DS, by its owner: the frames' pacing with the slider
  up on a layer, in town and in battle; how deep is comfortable.
- The raised floor of a layer is the world's by its row, not by its
  height (depth lighting's floor model would raise it, at a cost).
- A body is one depth: a tall one's head stands with its feet; two bodies
  touching are one.
- The engine's own marks over the world (the Net Square's visitors, a
  liberation's pieces) are flat with the HUD.

## Checked

`tests/test_stereo.c` (in `build.py test`): the sort on video memory of
the test's own making (a standing sprite one body with its box, the HP's
corner, BG0 and a chat box's rows flat, a sprite behind a background or
blended the world's), the depth (the last row at the screen, the eyes
apart by the parting at the first), and the layers and the eyes (the
world behind a body filled from beside it, the engine's drawing flat in
both eyes, the top row moved three pixels each way, a body by its feet's
depth). `build.py 3ds` builds it; `CYBERWORLD_STEREO_SHOT` was looked over
on a layer, in town and in a battle (the Custom screen flat, the field and
the viruses back).
