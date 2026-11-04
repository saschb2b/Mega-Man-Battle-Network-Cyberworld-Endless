/* The run's Glitch layer (docs/game/glitch-layer.md): a rare side layer
 * behind a glitching warp, a normal layer of its area that comes apart the
 * further MegaMan explores it, and the copy at its heart. The director's
 * part: the layer's stage, the glitches it schedules and draws, its
 * battles, the copy's fight and prize (glitch.c), and the other areas'
 * looks flipped onto its floor (glitch_look.c). */
#ifndef CW_GLITCH_H
#define CW_GLITCH_H

#include <stdbool.h>
#include <stdint.h>

/* How far a Glitch layer has come apart: 0 on arrival to GLITCH_STAGES - 1
 * at its heart, by the share of its floor MegaMan has come near; it never
 * falls. */
#define GLITCH_STAGES 5

/* A layer built (any kind): the glitch's state for it, its warp's place
 * where the layer holds one */
void glitch_layer_begin(void);
/* Each director frame on the map: the stage, the next glitch scheduled,
 * the stage's words */
void glitch_update(void);
/* The next random battle on a Glitch layer: its depth (an act deeper from
 * stage 3) and whether it brings a visitor of another area or net (from
 * stage 2); nothing changed elsewhere */
void glitch_battle(int *depth, bool *visitors);
/* The copy's Yes: its fight begins */
void glitch_copy_fight(void);
/* Whether the battle under way is the copy's */
bool glitch_copy_fighting(void);
/* ... and back from it: won, its prize waits and the glitch settles */
void glitch_copy_over(bool won);

/* (glitch_look.c) The layer's other looks, made as it begins: the same
 * floor in up to three other areas' tiles, from the areas that draw the
 * run's (not `own`, the layer's); and freed */
void glitch_looks_make(int own);
void glitch_looks_free(void);
int glitch_looks(void);
/* ... look `k` flipped in (-1: the layer's own back), its tiles and colours
 * now and its tile map at the next update; the look on screen (-1 its own) */
void glitch_look_flip(int k);
int glitch_look_shown(void);
/* Each director frame on the map: a flip's second step */
void glitch_look_update(void);

#endif
