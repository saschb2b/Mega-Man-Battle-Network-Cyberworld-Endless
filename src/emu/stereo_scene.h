/* The 3DS's 3D depth on the game's picture (docs/engine/stereo.md): with
 * Options' 3D depth on and the 3D slider up, what of each frame on a net
 * layer, in town or in battle is flat (src/gfx/stereo.h) is read as the
 * frame ends, and the 3DS's present draws each eye's rows from it
 * (present_3ds.h). Anywhere, CYBERWORLD_STEREO_SHOT=FRAME:PATH saves
 * that frame's two eyes side by side (left, then right) as a look at the
 * depth on a system without a 3D screen. */
#ifndef CW_STEREO_SCENE_H
#define CW_STEREO_SCENE_H

#include <stdbool.h>
#include <stdint.h>

/* The frame drawn (`px`, the picture as the canvas has it) in 3D where
 * the screen is */
void stereo_scene_frame(const uint32_t *px);
#ifdef __3DS__
/* Where the frame drawn is in 3D, the standing sprites' pixels (1 each;
 * NULL where it is not), and whether the moving background stands behind
 * (a layer's, a battle's): the 3DS's picture carries each pixel's part in
 * its alpha by them */
const uint8_t *stereo_scene_sprites(void);
bool stereo_scene_sky(void);
#endif

/* A pixel of the core's picture tagged by mGBA as BG3's: the moving
 * background (each tagged pixel BG3's or the backdrop where nothing is,
 * measured against the backgrounds decoded in 11 areas) */
static inline bool stereo_sky_tag(uint32_t c) { return (c >> 24 & 0x08u) && (c >> 28 & 3u) == 3; }

#endif
