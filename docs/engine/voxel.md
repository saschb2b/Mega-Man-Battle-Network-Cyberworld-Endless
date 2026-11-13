---
type: Feature
title: "Depth lighting: the world lit in 3D"
description: "Options' Display's Depth lighting (off by default, `depth_lighting` in settings.ini; the code's `voxel_` files, its first name) lights the world as the game runs, on the net, in town and in battle (a sun with soft shadows, shaded sides, a glow, what is far out of focus), and on a net layer rebuilds the picture as columns under a camera a little lower than BN6's: how a frame is read, lifted over the layer's own floor, lit and drawn at the screen's scale, where it runs, what it leaves alone, and what is still open."
tags: [engine, display, options, voxel]
generated: { by: claude-code/claude-fable-5-2, at: 2026-11-12T23:49:36Z }
sources:
  - id: reference-mod
    resource: "https://github.com/scottcandy34/DramaticShapeVoxelMod-latest"
    title: "Dramatic Shape Voxel Mod (a Lua mod for a Game Boy recompilation): the owner's reference for the look, its README and the heads of lib/Voxel3D.lua and lib/TileShape.lua read on 2026-11-12; no code of it is used (it forbids redistribution)"
  - id: gbatek
    resource: "https://problemkaputt.de/gbatek.htm#gbalcdvideocontroller"
    title: "GBATEK, LCD Video Controller: BGxCNT, the scroll, OAM's attributes, BLDCNT, BLDALPHA and BLDY, which gba_layers.c and gba_world.c read"
---

# Depth lighting: the world lit in 3D

The owner's first step of a visual sweep (2026-11-12): a mode, off by
default, that turns BN6's isometric world into something
three-dimensional and well lit, after a voxel diorama mod for another
game (the reference above). Options' Display has the line **Depth
lighting** ([the panel's rules](ui.md); it sits under [the GBA's
screen's lines](lcd.md)), kept as `depth_lighting = on` in
`settings.ini`; `--depth-lighting on|off|debug` on the command line sets
it over that. It is not offered on the 3DS or the Vita.

It began as "voxel mode", and the code keeps that name (`voxel.h`,
`voxel_scene.c`, `--voxel` and `voxel =` still read): the owner renamed
what players see on 2026-11-13, since no voxels are made, only each pixel
given a place and lit.

It is the owner's exception to two of AGENTS.md's rules, and only while it
is on: the picture is no longer BN6's 240x160 at a whole scale (it is
drawn anew at the screen's scale), and the map is drawn by us rather than
by BN6. Nothing of the game's is kept or embedded: every frame is made
from the core's video memory of that frame.

## Where it runs

Wherever BN6 shows a map or a battle (`voxel_scene.c`'s `scene_now`):

| Scene | How |
| --- | --- |
| A generated layer | its own floor known (`netmap_floor_at`): slabs, sides, stairs, the camera lower (25 degrees) |
| The town, Lan's room, Lan's HP, any other map | no floor of ours: all of the map lies flat under BN6's own camera (30 degrees, every pixel where BN6 puts it), its people and furniture standing on it and casting shadows, the town's hour on it |
| A battle | the same, the field from the backgrounds' screen maps (a battle has no tile map), nothing out of focus |

Not a battle's first frames, BN6's menus, a liberation's board (its
panels' tints are the picture's) or a visit to another's Square: those
show as they always do. The owner asked for the town and battles on
2026-11-13, after a first version that drew layers alone.

The HUD stays BN6's own pixels on the canvas, and so does everything the
engine draws over the picture; the canvas is clear where the world
shows, and the platform lays it over the voxel picture
(`platform_set_under`). The HUD is: BG0; BG3 where its priority puts it
before BG1 and BG2 (`gba_bg3_front`: a battle's Custom screen, measured
there as priority 1 over the field's 2 and 3, where a net map's BG3 is
the sky at priority 3); a sprite on either; a sprite all in the HP's
corner (48 x 24), in an open chat box's rows, or in a battle's first 40
rows (its gauge and emotion window).

## The frame read

mGBA's picture names each pixel's layer in its top byte, but not
reliably (measured on a layer's frame: the floor's flags alternate in
32x16 blocks), so two readers take the frame from the GBA's video memory
as it ends:

- `src/gfx/gba_layers.c`: which layer shows at each of the frame's
  pixels, by the backgrounds' tile maps and OAM (mode 0; regular and
  affine sprites, 1D and 2D mapping, their priorities). Voxel mode takes
  the sprites and the HUD from it.
- `src/gfx/gba_world.c`: BG1 and BG2 (the map's two layers) drawn from
  the map's **whole** tile map in RAM (`BN6_MAP_TILEMAP`), so that more
  of the world than the frame holds can be shown, with BG3 (the sky,
  wrapped) or the backdrop behind; BLDCNT's alpha and fades applied.

Where the frame lies on the tile map is found by matching: the screen
map BG1 shows (31 x 21 tiles) is looked for in the tile map, and the
place kept as an offset from the camera (verified on RoboDog Comp: the
screen's tiles equal the tile map's at +81, +2 tiles, no mismatch).
The camera itself is BG1's scroll taken as the place nearest MegaMan
modulo 256: a map's scroll is its camera's plus a multiple of 256 that
differs by area (seen: +256 on one area, +512 on another).

## The lift

The lattice is the frame's own: column x, and **ground row** gy, the row
a point shows in were it at height 0; a point h high shows h rows above.
`voxel_scene.c` lays the layer's floor over it (`netmap_floor_at`, each
level's panels), and `voxel_lift.c` sorts every map pixel:

| Kind | Rule |
| --- | --- |
| A floor's top | its ground point lies on a panel the layer draws as floor (the raised level first; walled-off floor too) |
| A stair | its ground point lies on a stair's two panels and the ramp stands there as high as the pixel shows it (`netmap_stair_height`: even from the foot to the rise) |
| A slab's side | art the floor does not explain, a run of under 28 rows down its column right under a top, the void under it, no deeper than nine in ten of the window's such runs (plus 2 rows), and no scenery within 8 columns beside it |
| Scenery | any other art the floor does not explain: the platforms pasted beside the way, lying flat on the ground |

A first rule took every unexplained pixel within 40 rows under an edge
for a side, and a second told floor art by its tiles; both left blocks
of wrong light on the scenery's platforms (the owner, 2026-11-12, on a
Sky Area layer with a stair), and stairs taken as floor of both levels
broke in two under the lower camera.

A sprite of the last priority (3) lies on the floor and is drawn as part
of it, under whoever stands there: BN6 gives a warp pad that priority
(measured on a Sky Area layer: the pad's two halves 3, MegaMan's parts
2, the HP's 1; the owner saw the pad stand up and float before this).
The others are joined into bodies (8-connected) standing on the ground row
of their lowest pixel; one in the air stands over the floor found within
28 rows under it. A blended sprite (BN6's shadow blob) is left out: the
sun casts its own.

## The light and the camera

`voxel_light.c`: a sun from the front left. Each pixel loses the sun to
what stands between it and the sun (the sprites' outlines, three rows
deep, a raised level's slabs, a stair), less the farther the shadow has
travelled (to a quarter over 120 steps), and the shadow's edge is spread
over two pixels each way; a shadow leaves 62 hundredths of the light.
Sides are darker than tops and darker the deeper under their edge, the
floor a little darker where a body stands. A first version cast hard
shadows that left half the light: too harsh (the owner, 2026-11-13).
The blob BN6 lays under who stands (a sprite of one colour, wider than
high) lies on the floor with the pads.
`voxel_draw.c`: each pixel becomes a face (a top a ground row deep, a
side or a sprite's pixel a row high) projected through a camera at
`--voxel-pitch` degrees (BN6's is 30; 25 as it comes), with a z-buffer;
seams of up to two voxels between a top and its side closed (they showed the sky in dots under the lower camera; the owner, 2026-11-13); then the sky softened and dimmed under the floor, a glow off the bright
panels, what is far from MegaMan out of focus, and a vignette. Past 4x
the picture is made at half the screen's scale.

Its passes run in bands of rows on a small pool of threads (three
workers and the game's own; none where a system has no threads): the
floor's grid, the map's window, the light and the picture's last pass.
Measured on the development machine at 1440x960: about 11 ms a frame.
Where a frame takes over 11 ms for a second, the picture is made a scale
smaller for the rest of the session and the screen scales it up
(`voxel_scene.c`).

The world's colours go through the GBA's screen's colour correction
([the GBA's screen](lcd.md)) as the frame's sprites do; its frame
blending is not applied to them.

## Open

- Speed on a handheld is not measured: the passes before the picture
  (the readers, the floor, the lift, the light) cost about half a frame
  here whatever the scale, and fixed-point for the last pass is not tried.
- Scenery that stands (a tower's art in the map's layers) is painted
  flat; a stair's ramp is taken as even, and casts no shadow.
- The world skips the frame blending, a liberation's panel tints and a
  Glitch layer's frame.
- The engine's own marks over the world (the Net Square's visitors, a
  liberation's pieces) are placed for BN6's camera, not the lower one.
- The wide-screen picture the whole tile map allows.

## Checked

`tests/test_voxel.c` (in `build.py test`): the two readers on video
memory of the test's own making (a tile, a sprite, priorities, a blended
sprite, a fade, the map's place), the lift (a top, a side, the scenery, a
stair's height, a body on its feet, the HUD), a shadow behind a body, and
the camera (BN6's angle leaves every pixel in place; a lower one brings
the floor's rows closer). On a layer the redrawn map equals the frame's
own pixels wherever a map layer shows (26,538 of 26,538 on a Sky Area
layer, 14,350 of 14,350 on RoboDog Comp, measured with a dev print since
removed).
