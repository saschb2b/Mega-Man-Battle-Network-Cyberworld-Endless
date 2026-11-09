/* A moved family's sounds BN6 has none of (XVirusFamily.songs, xvirus.h's
 * install): each song copied whole (xsong.h) into the art's room the recipe
 * gives it, and listed at its fixed one of BN6's empty song slots (every
 * track's copy of the same sound the same slot, xsong.c's XSOUNDS), on the
 * players its own game's entry names (BN6's 32 are BN5's), as the Navis'
 * copies are; a late BN4 family's at the pool's slots, pointed by battle
 * (xnavi_bn4_sounds.h). */
#ifndef CW_XVIRUS_SONGS_H
#define CW_XVIRUS_SONGS_H

#include "xvirus_recipe.h"

/* Its sounds staged (xvirus_tx.h) into their slots, each slot BN6's empty
 * song still or its game's same song another install copied there first;
 * the step failed (xv_fail) where one is not. */
void xvirus_songs_set(const XVirusFamily *f);

/* Its pool songs as its staging packed them, kept once its install is
 * committed (a late BN4 family's, xnavi_bn4_sounds.h). */
void xvirus_songs_staged(const XVirusFamily *f);

/* Its sounds noted at their slots for the other tracks' installs and the
 * differential (xsong.h), once installed; a late BN4 family's pool songs
 * with the pool (xnavi_bn4_sounds.h). */
void xvirus_songs_noted(const XVirusFamily *f);

#endif
