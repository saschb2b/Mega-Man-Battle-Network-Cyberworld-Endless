/* Voxel mode on the game's picture (docs/engine/voxel.md): where MegaMan
 * walks a layer's map, the frame rebuilt as columns over the layer's own
 * floor (src/gfx/voxel.h) at the screen's scale, which the platform shows
 * under the canvas (platform_set_under); the canvas keeps the HUD and
 * what the engine draws, clear where the world shows. */
#ifndef CW_VOXEL_SCENE_H
#define CW_VOXEL_SCENE_H

#include <stdbool.h>
#include <stdint.h>

/* This frame of the core's (`shown`: its colours as the picture shows
 * them) made into the voxel picture, where display.h's voxel is on and
 * MegaMan walks a layer's map. True where it was: `hud` then holds 1 for
 * each pixel the canvas keeps. */
bool voxel_scene_frame(const uint32_t *shown, uint8_t *hud);

#endif
