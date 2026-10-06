/* super_boss.h. Every number here is BN6 Gregar's own, read and checked
 * as docs/BOSSES.md (Super bosses, What the ROMs hold) says: the enemy
 * table's versions, the music of the story's battle records, the Gigas
 * Gregar pays for Bass SP and Bass BX. */
#include "super_boss.h"

#include "meta.h"
#include "pacing.h"
#include "rivals.h"

/* BN6's Giga chips they pay (the chip records' ids) */
enum { CHIP_BASS = 301, CHIP_COLFORCE = 304, CHIP_BUGRSWRD = 305, CHIP_BASSANLY = 306 };

int super_bass_form(int wins, bool beast_fallen) {
	if (wins <= 0) return SUPER_BASS_V1;
	return wins >= 2 && beast_fallen ? SUPER_BASS_BX : SUPER_BASS_SP;
}

int super_cybeast_form(int loop) { return loop > 0 ? SUPER_CYBEAST_SP : SUPER_CYBEAST_V1; }

int super_form(int navi, int depth) {
	if (navi == SUPER_BASS) return super_bass_form(rival(SUPER_BASS)->megaman_won, rival(SUPER_CYBEAST)->megaman_won > 0);
	if (navi == SUPER_CYBEAST) return super_cybeast_form(pacing_loop(depth));
	return 0;
}

int super_song(int navi) { return navi == SUPER_CYBEAST ? 0x17 : 0x16; }

const char *super_suffix(int navi, int version) {
	if (navi == SUPER_BASS) return version == SUPER_BASS_BX ? " BX" : version == SUPER_BASS_SP ? " SP" : "";
	return navi == SUPER_CYBEAST && version == SUPER_CYBEAST_SP ? " SP" : "";
}

int super_chip(int navi, int version) {
	/* (Bass SP's and Bass BX's are Gregar's own for them; Bass's first
	 * the other version's Bass Giga; the Cybeast's Gregar's own bug-born
	 * blade) */
	if (navi == SUPER_BASS) return version == SUPER_BASS_BX ? CHIP_COLFORCE : version == SUPER_BASS_SP ? CHIP_BASS : CHIP_BASSANLY;
	return navi == SUPER_CYBEAST ? CHIP_BUGRSWRD : 0;
}

int super_nest_master(void) { return SUPER_CYBEAST; }

int super_secret_master(uint16_t marks, int own) { return marks & MARK_SECRET ? SUPER_BASS : own; }

bool super_reserved_ai(int ai) {
	/* (Bass 19, Gregar 20, Falzar 21, GBeast 23, FBeast 24; the cut
	 * Navi's places 17 and 22) */
	return ai == 17 || (ai >= 19 && ai <= 24);
}
