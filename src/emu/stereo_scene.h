/* The 3DS's 3D depth on the game's picture (docs/engine/stereo.md): with
 * Options' 3D depth on and the 3D slider up, each frame on a net layer, in
 * town or in battle is sorted by depth (src/gfx/stereo.h) from the video
 * memory its frame ended with, and the 3DS's present draws each eye from
 * it (present_3ds.h). Anywhere, CYBERWORLD_STEREO_SHOT=FRAME:PATH saves
 * that frame's two eyes side by side (left, then right) as a look at the
 * depth on a system without a 3D screen. */
#ifndef CW_STEREO_SCENE_H
#define CW_STEREO_SCENE_H

#include <stdint.h>

/* The frame's picture as the canvas has it (`px`, drawn at the canvas's
 * (x, y)), sorted for the 3D screen where it is in 3D */
void stereo_scene_frame(const uint32_t *px, int x, int y);

#endif
