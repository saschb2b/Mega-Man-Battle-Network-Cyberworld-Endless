---
type: Feature
title: "The GBA's screen: colour correction, frame blending, lcd3x"
description: "Options' Display can show the game as a GBA's own screen did (issue #265): mGBA's libretro core's Color Correction and Interframe Blending on BN6's picture, step for step (`lcd_frame.c`), the correction on what the engine draws over it too (`lcd_canvas.c`), and RetroArch's lcd3x shader over the screen (`lcd_grid.c`); what each does, where it runs, and how it was checked against mGBA's own code."
tags: [engine, display, options]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-02T00:41:13Z }
sources:
  - id: mgba-libretro
    resource: "https://github.com/libretro/mgba/blob/master/src/platform/libretro/libretro.c"
    title: "mGBA's libretro core: _initColorCorrection and the videoPostProcess functions; libretro_core_options.h's mgba_color_correction and mgba_interframe_blending (read on 2026-11-02)"
  - id: gba-color
    resource: "https://github.com/libretro/glsl-shaders/blob/master/handheld/shaders/color/gba-color.glsl"
    title: "The gba-color shader (Pokefan531, hunterk; public domain)"
  - id: lcd3x
    resource: "https://github.com/libretro/glsl-shaders/blob/master/handheld/shaders/lcd3x.glsl"
    title: "The lcd3x shader and its preset handheld/lcd3x.glslp (Gigaherz; public domain)"
  - id: issue-265
    resource: "https://github.com/saschb2b/Mega-Man-Battle-Network-Cyberworld-Endless/issues/265"
    title: "Issue #265: the mGBA core's Color Correction, LCD Ghosting and the LCD 3x shader, asked by a player on a KONKR Pocket Advanced"
---

# The GBA's screen: colour correction, frame blending, lcd3x

A GBA's screen was dark, unlit and slow: its colours came out softer than
the numbers in the game, and each frame faded into the next. Players who
know the game from RetroArch turn on what the mGBA core and RetroArch's
shaders offer for it, and issue #265 asked for the same three here. They
are three lines of Options' Display ([The PET's
screens](ui.md) has the panel's rules), each off by default, kept in
`settings.ini`:

| Line | settings.ini | Values | What it is |
| --- | --- | --- | --- |
| Colour correction | `colour_correction` | `off`, `gba` | mGBA's Color Correction, its GBA setting |
| Frame blending | `interframe_blending` | `off`, `simple`, `smart`, `lcd_ghosting`, `lcd_ghosting_fast` (mGBA's `mix` and `mix_smart` read too) | mGBA's Interframe Blending: Simple, Smart, LCD Ghosting (Accurate) and (Fast) |
| LCD 3x | `lcd3x` | `off`, `on` | RetroArch's lcd3x shader (not on the 3DS) |

`--colour-correction`, `--interframe-blending` and `--lcd3x` on the command
line set them over `settings.ini` (a headless capture reads no
`settings.ini`: [Headless shots](../devtools/headless-shot.md)).

## mGBA's two, on BN6's picture (`src/gfx/lcd_frame.c`)

The libretro build of mGBA, the core RetroArch runs, draws its frames in
16-bit RGB565 and runs both options on each frame it hands the frontend:
the blending first, on the frame's raw colours, then each colour through
the correction's table. `lcd_frame.c` takes the same steps on the same
numbers:

- **The frame.** The picture's 8-bit channels back to mGBA's 565 (each
  channel's top five bits; green's sixth bit 0, as `mColorFrom555` leaves
  it), and the result shown as a GPU shows a 565 texture (a channel over
  31, green over 63).
- **Color Correction** is Pokefan531's GBA matrix (the gba-color shader's,
  in the public domain): each channel to light at gamma 3.2 (2.2 and the
  GBA's 1.0 more), mixed by the matrix (red 0.82, 0.24 of green, -0.06 of
  blue; green 0.125, 0.665, 0.21; blue 0.195, 0.075, 0.73), times 0.94,
  back at gamma 1/2.2 and rounded to five bits. A table of every colour,
  made once when it is turned on, as mGBA's `ccLUT`.
- **Simple** mixes each frame half and half with the one before; **Smart**
  only where a pixel alternates (the same as two frames back, not as the
  last); both with blargg's packed mix, each channel's half rounded up.
- **LCD Ghosting (Accurate)** moves each channel toward each of the four
  frames before by 0.333, then 0.333 squared, cubed and to the fourth
  (Harlequin's exponential drop-off). **Fast** keeps one running frame,
  each new one mixed into it half and half (mGBA raises its response from
  0.333 to 0.5 so its trail looks as long).

Both run on the core's picture: `emu_picture.c` (the town's hour and a
liberation's panels on it first) and the Arena's `arena_view.c`; the
correction also on what the engine draws over it (below). Each picture the core makes is blended once (`emu_pictures`, a count
of them): a frame the loop plays to catch up makes none, and while the
game waits under Options the last frame shows again, as a frontend shows
its last while the core runs no frame. Two differences from mGBA, both at
a start: its buffers start white (Accurate's four) or black (Fast's), ours
from the first picture, so turning one on mid-game flashes nothing; and in
fast forward only the batch's last picture is drawn, so only it blends.

## The engine's colours, on the same screen (`src/gfx/lcd_canvas.c`)

A frontend corrects the one frame the core hands it, and all a GBA's
screen showed came from that frame. Here the engine draws over BN6's
picture too (the map, the counts, its PET look's screens, Options
itself, the title), and with the picture alone corrected those stood out
brighter than BN6's own menus beside them, and Options could not show
what its line does. So Colour correction takes the engine's colours
through the same table, each where it enters the canvas:

- a fill's colour (`fill_rect`, `fill_rects`: the PET look's panels, the
  map, the 3x5 letters), the battle font's and the pixel letters' colours
  (`pixfont.c`), a fade's (`platform_apply_effects`) and the error
  screen's ground;
- the pixels of what is drawn from a texture: sprites' frames and ROM
  tiles (`gfx.c`), BN5's tiles (`xtiles.c`), the chat font's two tones
  (`chatbox.c`), the battle font's shade, the title's OPTIONS
  (`title_menu.c`);
- the title's and the start's whole pictures (the logo over an area's
  backdrop, the notice), before they go to their textures.

A colour goes as `lcd_frame` takes the picture's: its channels' top five
bits (a colour from a palette, `bgr555`, gives back its own five), mGBA's
table, and back as a GPU reads 565 (`lcd_correct_argb`). A texture made
from colours is made again when the correction comes on or off
(`lcd_canvas_gen`), so the line turned in Options recolours the panel it
is on, and the picture behind it.

Two differences from correcting one frame. What lies in front is
corrected and then laid over the corrected picture, where a frontend
would lay it over first: for an opaque pixel the two are the same, and a
translucent fill (a dim, a ghost) differs by little. And the battle
font's shade is drawn as the corrected shade times the text's corrected
colour, as its atlas is tinted, where one frame would correct the
product: white text's shade a few hundredths darker than one frame's.

Kept out, as none of them is a GBA's screen: a screen of its own
(`platform_own_taps`: the ROMs screen, SAVES, the Net Square's text
field), whose canvas is the app's; the touch controls, drawn at the
phone's own pixels (`touch_art.c`); and the second screen, drawn into
memory (`gfx_draw_into`). The title and the start are in: they are BN6's
own pixels at the GBA's 240x160 (its logo, the areas' backdrops, PRESS
START, the chat box), and Options opens over the title, where its
preview has the game behind it. Frame blending stays on the core's
picture. With the line off nothing changes: every colour and texture as
before, the title, Options' Display, the PET and a layer's map checked
pixel for pixel against the build before.

## lcd3x, over the screen (`src/gfx/lcd_grid.c`)

The lcd3x preset runs its one pass at the viewport's size, sampling the
game's frame nearest: each screen pixel is the frame's colour times its
column's red, green and blue factors, `(4 + sin(2 pi x + offsets)) / 5`
with offsets pi (1/2, 1/2 - 2/3, 1/2 - 4/3), and its row's, `(16 + sin(2
pi y)) / 17`, x and y the screen pixel's centre counted in the game's
pixels. So each game pixel has a red, a green and a blue stripe and a
darker line through it, and the picture is about a quarter darker. The
factors are a column's times a row's, so two strips (a row of the
columns' and a column of the rows', in 255ths) multiplied over the
picture with SDL's MOD blending draw it: after the picture lands on the
screen (`platform.c`'s `show_frames`, after smooth motion's mix and a
Fill's scaling), at the screen's own pixels as the preset draws it, and
under the touch controls. Not on a screen of its own (the ROMs screen,
SAVES), nor on the 3DS, whose top screen its own GPU fills
(`present_3ds.c`).

## Checked against mGBA

mGBA's own functions, cut from `libretro.c` into a scratch build beside
`lcd_frame.c` (kept out of the repository), gave the same results: all
65,536 entries of the correction's table, and 240 frames of a 240x160
scene (still noise, a flickering band, a moving bar) through each of the
four blendings, with the correction and without, every pixel equal.
`tests/test_core.c`'s `test_lcd_frame` keeps a few of mGBA's results: ten
corrected colours, and each blending of a pixel turning white and one
flickering red and blue, frame by frame. lcd3x was checked by its formula
and by eye at a 1280x960 screen's 5x; no GPU here ran the shader itself.
