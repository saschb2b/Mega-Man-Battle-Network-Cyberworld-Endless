/* ROM-free checks of voxel mode's readers and renderer (src/gfx/gba_layers.c,
 * gba_world.c, voxel_*.c) on video memory of the test's own making,
 * returning their failure count. */
#ifndef CW_TEST_VOXEL_H
#define CW_TEST_VOXEL_H

int test_voxel(void);

#endif
