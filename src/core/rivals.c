#include "rivals.h"

#include <string.h>

#include "save_blob.h"

#define RIVALS_MAGIC 0x43575231u /* "CWR1" */

static Rival rivals[RIVAL_NAVIS];
static bool loaded;

static void store(void) { save_write_blob("rivals.sav", RIVALS_MAGIC, rivals, sizeof rivals); }

void rivals_load(void) {
	if (!save_read_blob("rivals.sav", RIVALS_MAGIC, rivals, sizeof rivals)) memset(rivals, 0, sizeof rivals);
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

void rival_result(int navi, int result) {
	if (navi < 0 || navi >= RIVAL_NAVIS) return;
	if (!loaded) rivals_load();
	if (result == RIVAL_MEGAMAN_WON) rivals[navi].megaman_won++;
	if (result == RIVAL_NAVI_WON) rivals[navi].navi_won++;
	rivals[navi].last = result;
	store();
}
