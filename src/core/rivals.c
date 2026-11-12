#include "rivals.h"

#include <string.h>

#include "save_blob.h"

#define RIVALS_MAGIC 0x43575235u /* "CWR5": Nebula's LarkMan's and GridMan's records too (RIVAL_NAVIS) */
#define RIVALS_MAGIC_4 0x43575234u /* "CWR4": Team ProtoMan's records too, navis 0 to RIVAL_NAVIS_4 - 1 */
#define RIVALS_MAGIC_3 0x43575233u /* "CWR3": BN4's Navis' records too, navis 0 to RIVAL_NAVIS_3 - 1 */
#define RIVALS_MAGIC_2 0x43575232u /* "CWR2": a record with how the last battle ended, navis 0 to RIVAL_NAVIS_2 - 1 */
#define RIVALS_MAGIC_1 0x43575231u /* "CWR1": who won and how often, alone, as many */
#define RIVAL_NAVIS_4 65           /* the navis the fourth version kept */
#define RIVAL_NAVIS_3 59           /* the navis the third version kept */
#define RIVAL_NAVIS_2 32           /* the navis the first two versions kept */
_Static_assert(RIVAL_NAVIS_2 <= RIVAL_NAVIS_3 && RIVAL_NAVIS_3 <= RIVAL_NAVIS_4 && RIVAL_NAVIS_4 <= RIVAL_NAVIS, "each version's navis within the next's");

static Rival rivals[RIVAL_NAVIS];
static bool loaded;

static void store(void) { save_write_blob("rivals.sav", RIVALS_MAGIC, rivals, sizeof rivals); }

/* The first version's records, their counts kept and how their last
 * battles went not known; false where there is none */
static bool load_first(void) {
	struct { int met, megaman_won, navi_won, last; } old[RIVAL_NAVIS_2];
	if (!save_read_blob("rivals.sav", RIVALS_MAGIC_1, old, sizeof old)) return false;
	memset(rivals, 0, sizeof rivals);
	for (int i = 0; i < RIVAL_NAVIS_2; ++i)
		rivals[i] = (Rival){ old[i].met, old[i].megaman_won, old[i].navi_won, old[i].last, 0, 0, 0 };
	store();
	return true;
}

/* The second version's records, navis 0-31, the third's, 0-58, or the
 * fourth's, 0-64, each kept whole, none yet of the navis past them (BN4's,
 * Team ProtoMan's, Nebula's); false where there is none */
static bool load_kept(uint32_t magic, int navis) {
	Rival old[RIVAL_NAVIS_4];
	size_t n = (size_t)navis * sizeof *old;
	if (!save_read_blob("rivals.sav", magic, old, n)) return false;
	memset(rivals, 0, sizeof rivals);
	memcpy(rivals, old, n);
	store();
	return true;
}

void rivals_load(void) {
	if (!save_read_blob("rivals.sav", RIVALS_MAGIC, rivals, sizeof rivals) && !load_kept(RIVALS_MAGIC_4, RIVAL_NAVIS_4) &&
		!load_kept(RIVALS_MAGIC_3, RIVAL_NAVIS_3) &&
		!load_kept(RIVALS_MAGIC_2, RIVAL_NAVIS_2) && !load_first())
		memset(rivals, 0, sizeof rivals);
	loaded = true;
	/* Colonel was kept as navi 17, the enemy table's unnamed navi, before
	 * this version: his record moves to 18 */
	if (rivals[17].met && !rivals[18].met) { rivals[18] = rivals[17]; memset(&rivals[17], 0, sizeof rivals[17]); store(); }
}

const Rival *rival(int navi) {
	static const Rival blank;
	if (!loaded) rivals_load();
	return navi >= 0 && navi < RIVAL_NAVIS ? &rivals[navi] : &blank;
}

void rival_met(int navi) {
	if (navi < 0 || navi >= RIVAL_NAVIS) return;
	if (!loaded) rivals_load();
	rivals[navi].met++;
	store();
}

void rival_result(int navi, int result, const RivalEnd *how, int run) {
	if (navi < 0 || navi >= RIVAL_NAVIS) return;
	if (!loaded) rivals_load();
	Rival *r = &rivals[navi];
	if (result == RIVAL_MEGAMAN_WON) r->megaman_won++;
	if (result == RIVAL_NAVI_WON) r->navi_won++;
	r->last = result;
	/* (the winner's HP left rounded up: a winner standing is never 0) */
	int hp = how && how->hp < how->max ? how->hp : how ? how->max : 0;
	r->form = how && how->form > 0 && how->form < 256 ? (uint8_t)how->form : 0;
	r->left = how && hp > 0 ? (uint8_t)((hp * RIVAL_LEFT_FULL + how->max - 1) / how->max) : 0;
	r->run = run > 0 && run < 0x10000 ? (uint16_t)run : 0;
	store();
}
