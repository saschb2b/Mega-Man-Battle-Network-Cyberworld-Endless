/* The tournament's bracket as a layer holds it (bracket.h). */
#include "bracket.h"

#include <string.h>

#include "chips.h"
#include "data.h"
#include "guardians.h"
#include "loot.h"
#include "pacing.h"
#include "rom.h"
#include "xguardian.h"

/* A BN6 or BN5 Navi's plate on BN4's screen: 0x20 + his navi number, the
 * contract of the faces composed for it (docs/multirom/bn4-tournament.md,
 * Faces for every entrant), which xbracket.h names XBR_PLATE_NAVI once
 * they are built. Until then an id past BN4's own (0x1E) reads past its
 * plates' tables, and the screen, and the map after it, show stripes
 * (measured with the dev step, 5 November 2026): no tournament for players
 * (the owner's call 6: none until it can show every entrant), and the dev
 * route's (run_debug_bracket) shows BN4's own plates in their places. */
#ifdef XBR_PLATE_NAVI
#define BRACKET_FACES 1
#define BRACKET_PLATE(n, place) XBR_PLATE_NAVI(n)
#else
#define BRACKET_FACES 0
#define BRACKET_PLATE(n, place) ((place) - 1)
#endif

bool bracket_here(int depth) { return (BRACKET_FACES || run_debug_bracket) && XR[XROM_BN4_RED_SUN_US].data && run_bracket_at(depth); }

/* The Navis moved of the games present (The entrants): BN6's guardians but
 * ProtoMan (Chaud's own), navi 17 and the super bosses; BN5's six where
 * its ROM brings them into BN6's engine, the older nets on. Never the
 * Darkloids or Duo. (BN4's join here as each is moved and proved:
 * docs/multirom/bn4-navis.md) */
static void pool_make(BracketPool *p) {
	static const uint8_t bn6[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 14, 15, 16, 18 };
	memset(p, 0, sizeof *p);
	for (unsigned i = 0; i < sizeof bn6; ++i) {
		p->navi[p->n] = bn6[i];
		for (int v = 0; v < 4; ++v) p->hp[p->n][v] = (int16_t)(v < 3 ? navi_hp(bn6[i], v) : -1);
		++p->n;
	}
	if (!run_extra(EXTRA_OLDER)) return;
	for (int navi = GUARDIAN_OLDER_FIRST; navi <= GUARDIAN_OLDER_LAST && p->n < BRACKET_POOL; ++navi) {
		if (!xguardian_fightable(navi)) continue;
		p->navi[p->n] = (uint8_t)navi;
		p->older[p->n] = true;
		for (int v = 0; v < 4; ++v) p->hp[p->n][v] = (int16_t)xguardian_hp(navi, v);
		++p->n;
	}
}

bool bracket_of(int depth, int final, RunBracket *b) {
	if (!bracket_here(depth) || final <= 0) return false;
	BracketPool p;
	pool_make(&p);
	bool older = guardian_older(final);
	int v = older ? xguardian_version(final, depth) : make_boss(depth, run.biome, final).foes[0].version;
	int hp = older ? xguardian_hp_fought(final, v, depth) : navi_hp(final, v);
	run_bracket_draw(depth, final, v, hp, &p, b);
	return b->navi[0] > 0 && b->navi[1] > 0;
}

int bracket_chip(int navi, int version, int depth, int *code) {
	*code = 26;
	if (guardian_older(navi)) return xguardian_chip(navi, depth, code);
	int chip = navi_chip(navi, version);
	if (chip <= 0) return 0;
	/* (its letter where the folder holds it, else its *, as a guardian's
	 * Navi chip comes: guardian_objs.c) */
	ChipInfo ci;
	chip_info(chip, &ci);
	char c = ci.ncodes ? ci.codes[0] : '*';
	bool held = false;
	for (int k = 0; k < 3 && run.codes[k]; ++k) held |= c == 'A' + run.codes[k] - 1;
	if (!held && memchr(ci.codes, '*', (size_t)ci.ncodes)) c = '*';
	*code = c == '*' ? 26 : c - 'A';
	return chip;
}

int bracket_tournament(int act) {
	return act >= 4 ? BN4_TOURNAMENT_RED_SUN : act == 3 ? BN4_TOURNAMENT_EAGLE : BN4_TOURNAMENT_DEN;
}

void bracket_entrants(const RunBracket *b, int won, XbrEntrant e[BN4_ENTRANTS]) {
	/* (the places in bracket order, a first round's pairs 0-1, 2-3, 4-5 and
	 * 6-7, and the rounds each wins: MegaMan all three, the second round's
	 * Navi one, the final's two, the other half's best one) */
	const int navi[BN4_ENTRANTS] = { 0, b->navi[0], b->navi[1], b->others[0], b->navi[2], b->others[1], b->others[2], b->others[3] };
	static const int wins[BN4_ENTRANTS] = { 3, 0, 1, 0, 2, 0, 1, 0 };
	for (int i = 0; i < BN4_ENTRANTS; ++i) {
		int rounds = 0;
		for (int r = 0; r < won && r < wins[i]; ++r) rounds |= 1 << r;
		int in = wins[i] >= won ? BN4_ENTRANT_IN : wins[i] == won - 1 ? BN4_ENTRANT_OUT : 0;
		uint8_t id = i == 0 ? BN4_ENTRANT_MEGAMAN : navi[i] > 0 ? (uint8_t)BRACKET_PLATE(navi[i], i) : 0xFF;
		e[i] = (XbrEntrant){ id, (uint8_t)(rounds | in), (uint8_t)(3 - wins[i]) };
	}
}
