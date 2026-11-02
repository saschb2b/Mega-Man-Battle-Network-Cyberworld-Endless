---
type: Feature
title: "The GBA's screen: colour correction, frame blending, lcd3x"
description: "Options' Display can show the game as a GBA's own screen did (issue #265): mGBA's libretro core's Color Correction and Interframe Blending on BN6's picture, step for step (`lcd_frame.c`), libretro's handheld colour shaders (VBA, DS, PSP) over that picture as RetroArch runs them, the chosen colours on what the engine draws over it too (`lcd_canvas.c`), and RetroArch's lcd3x shader over the screen (`lcd_grid.c`); what each does, where it runs, and how it was checked against mGBA's own code and the shaders' math."
tags: [engine, display, options]
generated: { by: claude-code/claude-opus-5-5, at: 2026-11-02T01:16:00Z }
sources:
  - id: mgba-libretro
    resource: "https://github.com/libretro/mgba/blob/master/src/platform/libretro/libretro.c"
    title: "mGBA's libretro core: _initColorCorrection and the videoPostProcess functions; libretro_core_options.h's mgba_color_correction and mgba_interframe_blending (read on 2026-11-02)"
  - id: gba-color
    resource: "https://github.com/libretro/glsl-shaders/blob/master/handheld/shaders/color/gba-color.glsl"
    title: "The gba-color shader (Pokefan531, hunterk; public domain)"
  - id: handheld-color
    resource: "https://github.com/libretro/glsl-shaders/tree/master/handheld/shaders/color"
    title: "The vba-color, nds-color and psp-color shaders (Pokefan531, hunterk; public domain), read at master on 2026-11-02: nds-color last changed in 77697c58 (2020-08-12), vba-color and psp-color in 27ae0954 (2019-04-18, from Pokefan531's shader pack)"
  - id: pokefan531-thread
    resource: "https://forums.libretro.com/t/real-gba-and-ds-phat-colors/1540"
    title: "Pokefan531's thread on libretro's forums, \"Real GBA and DS Phat colors\", where the profiles come from"
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
| Colour correction | `colour_correction` | `off`, `gba`, `vba`, `nds`, `psp` (`ds` read too, and `on` as `gba`) | mGBA's Color Correction, its GBA setting; libretro's vba-color, nds-color and psp-color shaders |
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
correction also on what the engine draws over it (below). Each picture
the core makes is blended once (`emu_pictures`, a count of them): a
frame the loop plays to catch up makes none, and while the game waits
under Options the last frame shows again, as a frontend shows
its last while the core runs no frame. Two differences from mGBA, both at
a start: its buffers start white (Accurate's four) or black (Fast's), ours
from the first picture, so turning one on mid-game flashes nothing; and in
fast forward only the batch's last picture is drawn, so only it blends.

## The handheld colour shaders (`src/gfx/lcd_frame.c`)

Colour correction's VBA, DS and PSP are three of libretro's handheld
colour shaders (glsl-shaders, `handheld/shaders/color`): Pokefan531's
colour profiles in hunterk's Color Mangler, in the public domain, the
same pass as `gba-color`, whose numbers mGBA's own correction took.
**vba-color** is a GBA's colours as VisualBoyAdvance showed them (the
slang version's comment: "based from VBA-M and No$GBA"), **nds-color** a
GBA game on the first DS's screen (its comment: "a Nintendo DS Phat"),
**psp-color** the PSP 1000's and 2000's screen. RetroArch players pick
them for a different screen than the GBA's, and a Vita's player can see
the game as a PSP's screen showed it. The slang-shaders versions of the
same names are newer and differ (colour modes, the PSP's sRGB, DCI and
Rec2020 profiles, VBA's darkening from 0): the glsl ones are these.

Each runs one pass, at every pixel, in a GPU's 32-bit float: each channel
of the frame to light at `target_gamma` (with `darken_screen`, its
`#pragma parameter`'s default, where it has one); mixed with grey by
`1 - contrast` (contrast 1: nothing); times `lum` and kept within 0 and
1; mixed by the colour matrix (`mat4(r, rg, rb, 0, gr, g, gb, 0, br, bg,
b, 0, ...)`, column by column: red out is `r` of red, `gr` of green and
`br` of blue; the saturation matrix it is multiplied by, `sat` 1 and the
black levels 0, leaves it as it is); back at `1 / display_gamma`:

| | target gamma | display gamma | lum | red out (of R, G, B) | green out | blue out |
| --- | --- | --- | --- | --- | --- | --- |
| VBA | 1.45 + 1.7 x darken 1.0 = 3.15 | 1.45 | 1.0 | 0.73, 0.27, 0 | 0.085, 0.675, 0.24 | 0.085, 0.24, 0.675 |
| DS | 1.91 | 1.91 | 0.89 | 0.87, 0.255, -0.125 | 0.10, 0.645, 0.255 | 0.10, 0.17, 0.73 |
| PSP | 2.21 | 2.2 | 1.0 | 0.98, 0.20, -0.18 | 0.04, 0.795, 0.165 | 0.01, 0.01, 0.98 |
| (gba-color) | 2.2 + darken 1.0 = 3.2 | 2.2 | 0.94 | 0.82, 0.24, -0.06 | 0.125, 0.665, 0.21 | 0.195, 0.075, 0.73 |

So VBA darkens the middle tones most (a mid grey shown at 60 of 255,
against GBA's 94 and Off's 132) and softens its colours (red 205, 47,
47); DS keeps the middle tones, dims white to 240 and softens (red 223,
72, 72); PSP keeps both and leans warm (red 253, 59, 31; green 121, 226,
31).

**A shader against mGBA's table.** mGBA reads each channel's five bits
over 31 (green's top five), works in float with C's `pow` and rounds the
light back to five bits of its 565 frame, so its screen has 32 levels a
channel (green's top one 251 of 255). A shader runs on the GPU over the
frame the core hands RetroArch, which for mGBA's libretro core is that
same 565 frame: the GPU reads a channel over 31 and green over 63 (a
white's green 62 of 63; a blended frame's can be odd) and writes 8 bits
a channel, kept within 0 and 1 and rounded to the nearest. So a shader's
table is the shader's pass over each of the 65,536 colours of the 565
frame (blended or not), at 8 bits a channel, in 32-bit float (`powf`) in
the shader's order: the frame holds no other colour, so the table is
the pass, pixel for pixel. Where a matrix's negative share takes a
channel below 0 (DS's and PSP's red of a pure blue), GLSL leaves the
`pow` undefined; a GPU's NaN is written as 0, and 0 it is here, as
mGBA's own correction keeps it. A channel past 1 (DS's and PSP's red of
a full yellow) is kept at 1, as the framebuffer keeps it. GBA stays
mGBA's own, its table now of what the screen shows too.

The table is made when a colour is chosen, its 65,536 entries in 256 KB
(each channel's light in once a value, three `powf` an entry out): about
1.3 ms on the desktop here, more on a handheld, once a choice. Options
turns the line Off, GBA, VBA, DS, PSP (Left the other way): GBA first,
the game's own machine and the likeliest choice, VBA beside it as the
GBA again, then the two later handhelds. A presentation choice only, it
feeds no loop of the game: the picture as the game stores it against
the screen a player remembers, each applied at once and named on the
panel by the screen it shows ([The PET's screens](ui.md)).

## The engine's colours, on the same screen (`src/gfx/lcd_canvas.c`)

A frontend corrects the one frame the core hands it, and all a GBA's
screen showed came from that frame. Here the engine draws over BN6's
picture too (the map, the counts, its PET look's screens, Options
itself, the title), and with the picture alone corrected those stood out
brighter than BN6's own menus beside them, and Options could not show
what its line does. So Colour correction takes the engine's colours
through the chosen colour's table (GBA's or a shader's), each where it
enters the canvas:

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
bits as a 565 colour (a colour from a palette, `bgr555`, gives back its
own five), and that colour's entry in the table, what the screen shows
(`lcd_correct_argb`). A texture made from colours is made again when the
colour it takes changes (`lcd_canvas_gen`: Off, another of the five, a
screen of its own), so the line turned in Options recolours the panel it
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

## Checked against the shaders' math

A scratch Python evaluation (kept out of the repository) reads the four
`.glsl` files' own text, their `#define`s, their `#pragma parameter`
defaults and the exponent of `main()`'s first `pow`, checks the pass's
steps are as above, and runs them over all 65,536 colours of the 565
frame, each operation rounded to 32-bit float, and again in 64-bit. The
C tables equal its 32-bit results in every entry, all three; against the
64-bit ones VBA's and DS's equal, PSP's differ in 3 entries by 1 of 255,
values on a rounding's edge (a GPU's `pow` is its own approximation, so
there too such a value may land a 255th either way). No GPU here ran the
shaders. `test_lcd_frame` keeps 11 of their colours (white, red, green
and grey; blue, whose red goes below 0; yellow at green's top, whose red
passes 1) and one through the frame. GBA's colours, now in the same kind
of table, were checked beside the earlier `lcd_frame.c`: the same picture
for all 32,768 colours of an unblended frame.
