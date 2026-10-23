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
/* The net area a dark layer is drawn in: BN5's Liberation Missions' maps
 * (group 0x86) where BN5's ROM is there, else `tiles`, the act's own. */
int lib_layer_tiles(int tiles);
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
/* The board's sealed region opened and its route taken (docs/LIBERATION.md,
 * Routes that matter): the hook the act's next route reads. */
bool lib_route_found(void);
/* The exit stays shut: the Darkloid stands. */
bool lib_exit_shut(void);
/* Each frame on the layer's map, and each frame of a battle. */
void lib_update(void);
void lib_battle_frame(void);
/* The pad on its way to the game while the board is under way: the
 * board's own moves and menu take it; MegaMan's walk from panel to panel
 * is what reaches the game. */
uint32_t lib_keys(uint32_t keys);

/* (liberation_sound.c) The board's music and sounds, BN5's where its
 * board is drawn in BN5's look: the song a dark layer's map plays (its
 * board's, else `song`); one of BN5's songs started now (BN5_SONG_*); one
 * of BN5's sounds played. */
int lib_board_song(int song);
void lib_song(int bn5_song);
void lib_sound(int bn5_sound);

/* (liberation_camera.c) BN6's camera moved as BN5's moves it over its
 * board, where the board is in BN5's look: a frame of its pan to world
 * (wx, wy), true once there (and at once where it is not BN5's board); a
 * frame of its pan back to MegaMan, true once BN6's camera follows him
 * again; where it is while the board holds it (false: it follows him); and
 * after a CONTINUE, BN6's camera made to follow him. */
bool lib_camera_at(int wx, int wy);
bool lib_camera_home(void);
/* ... and moved `sx` pixels across and `sy` down the screen now (BN5's L view's MoveCam) */
void lib_camera_nudge(int sx, int sy);
bool lib_camera_place(int *wx, int *wy);
void lib_camera_mend(void);

/* (liberation_save.c) The board across a CONTINUE: saved with the run's
 * checkpoint (run.lib), and taken back once its state has loaded on the
 * same layer (false: none for this layer, the board begins afresh). */
void lib_save(void);
bool lib_resume(void);

/* (liberation_fence.c) The board's dark panels fenced with walls of BN6's
 * own, each panel's switched off as it is liberated: built after the
 * layer's map (netmap_build_layer), kept each frame. */
void lib_fence_build(void);
void lib_fence_update(void);

/* (liberation_look.c) The board in BN5's own look, on a layer drawn in
 * BN5's Liberation map: begun with the board, kept each frame on the map;
 * a panel freed dissolves as BN5's do. */
void lib_look_begin(void);
void lib_look_update(void);
void lib_look_freed(int x, int y);
bool lib_look_on(void);
/* ... a guardian of kind `kind` (UNIT_*) in BN5's own sprite where its ROM
 * is there: its list, number and animation set (else left as they are) */
void lib_look_unit(int kind, int *list, int *sprite, int *anim);

/* (liberation_auto.c) The test autopilot's keys on the board, frame
 * `frame` (autopilot.c). */
uint32_t lib_auto_keys(int frame);

/* (liberation_way.c) At home, the portal of the way to a dark layer in
 * BN5's dark panels' colours, in the core's frame `px` (240 x 160) */
void lib_way_tint(uint32_t *px);

/* (liberation_draw.c) What the drawing reads of the game, after each
 * frame: the camera, the sprites' colours. */
void lib_see(void);
/* (and director.h's director_tint_board and director_draw_board: the dark
 * panels recoloured in the frame, sprites and chat boxes left alone; the
 * phase and par, the Order Points, the banner and the command menu) */

#endif
