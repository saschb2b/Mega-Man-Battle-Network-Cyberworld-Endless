/* BN4's Navis' sounds BN6 has no song of (the owner's call 11, 5 November
 * 2026; docs/multirom/bn4.md, docs/multirom/bn4-navis.md, GutsMan built):
 * a pool of BN6's empty song slots the Navis share, a Navi's k-th sound its
 * k-th slot (tools/xnavirecipe_bn4.py SOUND_POOL, its recipe's songs), so
 * his code names the same slots whichever Navi fights. Each Navi's songs
 * are packed from the player's BN4 ROM into his art's room at his install
 * (xsong.h's copy), and before each frame the pool's slots are pointed at
 * the songs of the BN4 Navi the battle holds (one a battle: a tournament's
 * three rounds are three battles), BN6's song table's entries rewritten in
 * the core's copy only where they point elsewhere. The families keep their
 * fixed slots (0x34-0x3F, xsong.c's XSOUNDS). */
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
 * pointed at them, each slot BN6's empty song still or another BN4 Navi's song of the pool; the step failed (xv_fail)
 * where one is not a pool slot, holds another song or does not pack. */
void xnavi_bn4_songs_set(const XVirusFamily *f, int versions);
/* Its songs as its install made them, kept for the pool's pointing (once the install is done) */
void xnavi_bn4_songs_noted(void);
/* Before each frame of BN6's core (emu_frame): the pool pointed at the songs of the BN4 Navi among the battle's first-kind
 * objects, where it points at another's */
void xnavi_bn4_sounds_frame(void);

#endif
