/* BN6's save holding the game's RAM masked (bn6_save.h; docs/rom/game-state.md,
 * The save's mask; issue #734): from just before its mask until its unmask
 * has returned, the bytes from 0x02000000 read XORed with the save's key,
 * some eight frames as it writes the cartridge, and a write there comes out
 * inverted by the key's bits once it unmasks. What follows a frame of the
 * game (scene_emu.c's after_frame, the director among it) and the keys on
 * their way to the next wait for it: hooks on both cores (mGBA's and gpSP's)
 * mark the window, the loops at its ends inside it however a frame falls
 * on them. */
#ifndef CW_SAVE_MASK_H
#define CW_SAVE_MASK_H

#include <stdbool.h>
#include <stdio.h>

/* The hooks, once (game_hooks.c) */
void save_mask_install(void);
/* Whether BN6's save holds the RAM masked now, the frame in flight ended
 * first (emu_sync) */
bool save_masked(void);
/* A state loaded into BN6's core, or a reset: its CPU elsewhere, the mark
 * gone (emu.c, emu_gpsp.c) */
void save_mask_loaded(void);
/* The state's lines (scenarios): `savemasks N`, the masks BN6's saves made
 * since the start (one a save: a byte written into it masked fails the
 * save's sum, and BN6 masks and saves it all again), and `savemasked yes`
 * or `no` */
void save_mask_describe(FILE *f);

#endif
