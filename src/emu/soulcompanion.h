/* BN4's Souls' companions in BN6's engine (docs/multirom/bn4-souls.md,
 * NumSoul's companion): the object BN4 spawns beside MegaMan as he takes a
 * Soul's form, NumSoul's and FireSoul's (BN4_FORM_COMPANIONS by form; BN4's
 * 0x0800B7E8 at the unison and at the end), as BN6's companion overlay: BN6's
 * first-kind 0x56 is BN4's 0x57 with a mask by animation added (its update
 * 0x080C5C34 against BN4's 0x080CC46E), spawned, told and taken away as a
 * Cross's overlay is (BN6_T1_OVERLAY), of a kind of its own
 * (SOUL_OVERLAY_BN4) whose masks keep it at his depth always, as BN4's, and
 * whose sprite is the shared look room's (soullook.c,
 * SOUL_LIST0_BN4_COMPANION). FireMan's moved 0x57 (xnavi_bn4.c) is no road:
 * spawned beside a Soul it held BN6's battle on the Custom screen. */
#ifndef CW_SOULCOMPANION_H
#define CW_SOULCOMPANION_H

#include <stdbool.h>
#include <stdint.h>

/* The overlay's kind written in the Navis' grown tables where BN4's ROM is
 * read; true either way, none spawned where not */
bool soulcompanion_install(void);
/* BN4's k-th moved Soul's companion's sprite in BN4's ROM, its list and
 * number (BN4_FORM_COMPANIONS' word: the list's byte offset, the number);
 * false where he has none */
bool soulcompanion_sprite(int k, int *list, int *number);
/* Whether the proxy form's entry spawns a companion for BN4's k-th moved
 * Soul (BN6_FORM_OVERLAY_SPAWN), its word at *word; false where he has none
 * or it is not installed */
bool soulcompanion_enter(int k, uint32_t *word);
/* ... and its leave: BN6's own removal of the overlay, 0 where he has none */
uint32_t soulcompanion_leave(int k);

#endif
