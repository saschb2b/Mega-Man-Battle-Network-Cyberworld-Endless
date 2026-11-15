/* The 3DS's 3D depth on the game's picture (docs/engine/stereo.md): with
 * Options' 3D depth on and the 3D slider up, what of each frame on a net
 * layer, in town or in battle is flat (src/gfx/stereo.h) is read as the
 * frame ends, and the 3DS's present draws each eye's rows from it
 * (present_3ds.h). Anywhere, CYBERWORLD_STEREO_SHOT=FRAME:PATH saves
 * that frame's two eyes side by side (left, then right) as a look at the
 * depth on a system without a 3D screen. */
#ifndef CW_STEREO_SCENE_H
#define CW_STEREO_SCENE_H

#include <stdint.h>

/* The frame drawn (`px`, the picture as the canvas has it) in 3D where
 * the screen is */
void stereo_scene_frame(const uint32_t *px);

#endif
