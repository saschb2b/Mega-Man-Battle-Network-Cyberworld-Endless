/* What MegaMan and Lan say coming home after an act, and of Lan's HP's
 * portals (home_words.c, docs/HOME.md, docs/VOICE.md). */
#ifndef CW_HOME_WORDS_H
#define CW_HOME_WORDS_H

#include <stdbool.h>

/* Their words on arriving in Lan's HP, `beaten` the guardian just deleted
 * (NULL unknown), the run at the next act's first layer; its ways open,
 * `ports[0..ways)` the portals' names ("pink pad"), and `sealed` where a
 * dark way's stands shut (NULL none). */
const char *home_words(const char *beaten, int ways, const char *const ports[3], const char *sealed);
/* MegaMan at a portal (docs/HOME.md): what he reads through the link of
 * area `biome` (its data, never its name: he has not been there) and of
 * `navi` its guardian (his signal named where MegaMan has battled him);
 * `dark` the dark way into the Undernet, `sealed` that way still shut. */
const char *home_port_words(int biome, int navi, bool dark, bool sealed);
/* Lan's HP's portal `k` (lanhp.h) as MegaMan names it, after "the". */
const char *home_portal_name(int k);
/* MegaMan in Lan's HP the run's first time there. */
const char *home_hp_words(void);
/* L in Lan's HP: the pink pad `way` from MegaMan, the first time how to
 * jack out, and how far the Net's clock has run (`clock` its notches). */
const char *home_hp_status(const char *way, bool first, int clock);
/* MegaMan beside an older portal (docs/HOME.md, going back): where it
 * leads (an area won, so named) and its price on the Net's clock, all of
 * it until `taught`. */
const char *home_back_portal_words(int biome, bool taught, int clock);
/* Home again from a trip back, the Net's clock at `clock`. */
const char *home_back_words(int clock);

#endif
