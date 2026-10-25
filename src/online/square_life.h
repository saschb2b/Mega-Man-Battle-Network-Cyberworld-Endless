/* The Net Square's life (square_life.c, docs/online/square/dropped-link.md, The Square): two
 * Navis walking their routes by the wall clock, so every client whose
 * clock agrees sees them in the same place; BN6's log-in beam where a
 * Navi logs in or out, and the greeter's word on it; the greeter's and the
 * referee's words as the channel stands; what A finds facing a walker or
 * a prop; and the Square answering MegaMen who emote together at the
 * landmark or on the stage. Nothing in it makes a run stronger. */
#ifndef CW_SQUARE_LIFE_H
#define CW_SQUARE_LIFE_H

#include <stdbool.h>
#include <stddef.h>

#include "online_types.h"

/* As MegaMan arrives in the Square. */
void square_life_reset(void);
/* After each frame there, with the snapshot shown. */
void square_life_frame(const OnlineSnapshot *s);
/* A pressed by MegaMan at world (x, y) facing `face` (0-7), no peer nor
 * Mr.Prog ahead: a walker's or a prop's words; true where one spoke. */
bool square_life_talk(int x, int y, int face);
/* L's Look around chosen: MegaMan says where the warps go from where he
 * stands, and who is in which part (square_look_words); false where no
 * talk began. */
bool square_life_look(void);
/* The label over warp gem `i` (warp i / 2's, in the plaza for an even i,
 * in its part for an odd one): where it goes and who is there now
 * (square_warp_label), and its world middle; false none, a gem of
 * another part than MegaMan's, or under a chat box. */
bool square_life_warp(int i, char *text, size_t n, int *x, int *y);
/* A word over someone's head this frame (world x, y, z): `k` 0 a walker's
 * or a Mr.Prog's, 1 the greeter's call on a big cheer; false none. */
bool square_life_bark(int k, const char **text, int *x, int *y, int *z);
/* BN6's "!!" over world (x, y, z) a moment, and BN6's sound `sound` (0
 * none): an answered wave's, a Surprised emote's (square_emote.c). */
void square_life_mark(int x, int y, int z, int sound);

#endif
