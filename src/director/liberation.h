/* A liberation on a dark layer (docs/LIBERATION.md, Building it: stage 1):
 * the layer's floor a board of BN5's dark panels (liberation_board.h),
 * MegaMan moving over it panel by panel as BN5's board moves its units,
 * liberating a panel beside him in a battle of BN6's, resting, spending
 * Order Points; the DarkHoles' guardians' and the Darkloid's phase after
 * his; the phase against the par. Its words in liberation_words.c, its
 * drawing in liberation_draw.c. */
#ifndef CW_LIBERATION_H
#define CW_LIBERATION_H

#include <stdbool.h>
#include <stdint.h>

/* Whether the layer the run is about to build is a dark one: an act's
 * second layer, from the second act, behind the way at its fork that the
 * run's seed darkened (run_dark_area); every act's second from the second
 * with --dev dark. */
bool lib_layer_dark(void);
/* ... whether act `act`'s way into area `biome` holds its dark layer (the
 * fork's words and the act's card say so) */
bool lib_way_dark(int act, int biome);
/* The dark layer just generated (net.h's `layer`): its board made from its
 * seed, its units added as its objects and its exit put where the
 * Darkloid stands; false where no board fits (the layer is made again as
 * a plain one). */
bool lib_layer_made(void);
/* The layer installed: the board begins (or nothing, on a plain layer). */
void lib_begin(void);
/* The board is under way on this layer: MegaMan moves as its units do. */
bool lib_on(void);
/* The exit stays shut: the Darkloid stands. */
bool lib_exit_shut(void);
/* Each frame on the layer's map, and each frame of a battle. */
void lib_update(void);
void lib_battle_frame(void);
/* The pad on its way to the game while the board is under way: the
 * board's own moves and menu take it; MegaMan's walk from panel to panel
 * is what reaches the game. */
uint32_t lib_keys(uint32_t keys);

/* (liberation_auto.c) The test autopilot's keys on the board, frame
 * `frame` (autopilot.c). */
uint32_t lib_auto_keys(int frame);

/* (liberation_draw.c) What the drawing reads of the game, after each
 * frame: the camera, the sprites' colours. */
void lib_see(void);
/* (and director.h's director_tint_board and director_draw_board: the dark
 * panels recoloured in the frame, sprites and chat boxes left alone; the
 * phase and par, the Order Points, the banner and the command menu) */

#endif
