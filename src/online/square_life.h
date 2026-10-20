/* The Net Square's life (square_life.c, docs/ONLINE.md, The Square): two
 * Navis walking their routes by the wall clock, so every client whose
 * clock agrees sees them in the same place; BN6's log-in beam where a
 * Navi logs in or out, and the greeter's word on it; the greeter's and the
 * referee's words as the channel stands; what A finds facing a walker or
 * a prop; and the Square answering MegaMen who emote together at the
 * landmark or on the stage. Nothing in it makes a run stronger. */
#ifndef CW_SQUARE_LIFE_H
#define CW_SQUARE_LIFE_H

#include <stdbool.h>

#include "online_types.h"

/* As MegaMan arrives in the Square. */
void square_life_reset(void);
/* After each frame there, with the snapshot shown. */
void square_life_frame(const OnlineSnapshot *s);
/* A pressed by MegaMan at world (x, y) facing `face` (0-7), no peer nor
 * Mr.Prog ahead: a walker's or a prop's words; true where one spoke. */
bool square_life_talk(int x, int y, int face);
/* L's Look around chosen: MegaMan says where Trade Central and the Arena
 * lie from where he stands (square_look_words); false where no talk
 * began. */
bool square_life_look(void);
/* A word over someone's head this frame (world x, y, z); false none. */
bool square_life_bark(const char **text, int *x, int *y, int *z);

#endif
