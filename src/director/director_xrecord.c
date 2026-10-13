/* The xrecord step (director.h, director_dev_xrecord; issue #82): one of
 * BN5's battle records fought in BN6's own engine at once, as xbattles.h
 * translates it, the dev steps' battle (director_dev.c). */
#include <stdio.h>

#include "director.h"
#include "director_dev.h"
#include "director_state.h"
#include "xbattles.h"

/* what a reason names (xbattles.h, XB_*), for the log */
static const char *why_name(int why) {
	static const char *const names[] = { "fightable", "no record there", "a story's battle", "a Navi of its game's", "a virus family BN6 lacks",
		"a field BN6 has no panels for", "an object of a kind BN6 never sets", "more than a battle of BN6's holds" };
	return why >= 0 && why < (int)(sizeof names / sizeof *names) ? names[why] : "?";
}

bool director_dev_xrecord(const char *spec) {
	unsigned addr = 0;
	int up = 0;
	if (sscanf(spec, "%x %d", &addr, &up) < 1 || up < 0 || up > 5) {
		printf("xrecord: %s names no record (its address in BN5's ROM in hex, then the versions up)\n", spec);
		return false;
	}
	Encounter e = D.next;
	int what = 0, why = xbattle_encounter(XROM_BN5_COLONEL_US, addr, (XScale){ up, 5 }, &e, &what);
	if (why != XB_FIGHTABLE) {
		printf("xrecord: BN5's record %08X is not fought in BN6's engine: %s (%#x)\n", addr, why_name(why), what);
		return false;
	}
	printf("xrecord: BN5's record %08X in BN6's engine, field %#x, MegaMan at %02x:", addr, e.field, e.player ? e.player : 0x22);
	for (int i = 0; i < e.nfoes; ++i) printf(" %#x@%d%d", e.foes[i].id, e.foes[i].col + 1, e.foes[i].row + 1);
	for (int i = 0; i < e.nobj; ++i) printf(" obj %02x@%02x", e.obj[i].kind, e.obj[i].panel);
	if (e.gem_own) printf(" (its Mystery Data's finds %04X %04X %04X %04X %04X %04X %04X %04X)", e.gem_row[0], e.gem_row[1], e.gem_row[2],
		e.gem_row[3], e.gem_row[4], e.gem_row[5], e.gem_row[6], e.gem_row[7]);
	printf("\n");
	director_dev_due(&e);
	return true;
}
