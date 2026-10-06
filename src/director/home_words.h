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
/* MegaMan in Lan's HP the run's first time there: what it is, or, to a
 * profile `taught` that, a word. */
const char *home_hp_words(bool taught);
/* L's words at home `words`, then once a visit what it holds besides the
 * way on: `requests` posted (none held), an order `order` AsterLand can
 * still make that the zenny held pays for, and which way AsterLand is
 * (`aster`, way_to's word; NULL unsaid). */
const char *home_errands_words(const char *words, bool requests, bool order, const char *aster);
/* L's words in AsterLand: where its counter takes orders and sells
 * SubChips, then `words`. */
const char *home_aster_words(const char *words);
/* L's words in Lan's house `words`, then where its front door is (`door`
 * from Lan, way_to's word). */
const char *home_door_words(const char *words, const char *door);
/* L in Lan's HP: the pink pad `way` from MegaMan, the first time how to
 * jack out, and how far the Net's clock has run (`clock` its notches). */
const char *home_hp_status(const char *way, bool first, int clock);
/* MegaMan beside an older portal (docs/HOME.md, going back): where it
 * leads (an area won, so named) and its price on the Net's clock, all of
 * it until `taught`. */
const char *home_back_portal_words(int biome, bool taught, int clock);
/* Home again from a trip back, the Net's clock at `clock`. */
const char *home_back_words(int clock);
/* The courier in Lan's HP (docs/HOME.md, piece 2), the most valuable
 * first, a box each: the pay waiting with asker `reward` (jobs.h, -1
 * none), else the places of the new requests posted (bit k asker k), and
 * what AsterLand's SubChip seller has in stock: `unlockers` Unlockers,
 * RushFood, a WWW-ID. "" for none. */
const char *home_courier_words(int reward, unsigned posted, int unlockers, bool rush_food, bool www_id);

#endif
