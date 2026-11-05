/* BN4's Navis' sounds BN6 has no song of (the owner's call 11, 5 November
 * 2026; docs/multirom/bn4.md, docs/multirom/bn4-navis.md, GutsMan built):
 * a pool of BN6's empty song slots the Navis share, a Navi's k-th sound its
 * k-th slot (tools/xnavirecipe_bn4.py SOUND_POOL, its recipe's songs). Each
 * Navi's songs are packed from the player's BN4 ROM into its art's room at
 * its install (xsong.h's copy) and the pool's slots pointed at them; the
 * families keep their fixed slots (0x34-0x3F, xsong.c's XSOUNDS). With one
 * BN4 Navi installed the pool is his; the pool pointed per battle at the
 * Navi it holds, as the call asks, is still to come. */
#ifndef CW_XNAVI_BN4_SOUNDS_H
#define CW_XNAVI_BN4_SOUNDS_H

#include <stdbool.h>
#include <stdint.h>

#include "xvirus_recipe.h"

/* The pool: BN6's empty song slots no other track takes (docs/rom/sound.md, BN6's empty song slots), the five a movs or a
 * byte can name first */
#define XNAVI_BN4_POOL_SLOTS 16
extern const uint16_t xnavi_bn4_pool[XNAVI_BN4_POOL_SLOTS];

/* Its songs (XVirusFamily.songs: each a pool slot) staged (xvirus_tx.h): packed into their rooms and the pool's slots
 * pointed at them, each slot BN6's empty song still; the step failed (xv_fail) where one is not a pool slot, is taken or
 * does not pack. */
void xnavi_bn4_songs_set(const XVirusFamily *f);

#endif
