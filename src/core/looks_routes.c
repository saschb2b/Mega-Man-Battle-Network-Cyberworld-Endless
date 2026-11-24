/* looks_routes.h: the three ways a profile earns a palette (issue #854,
 * docs/game/looks.md, Earning). */
#include "looks_routes.h"

#include "looks.h"
#include "meta.h"
#include "rivals.h"
#include "run.h"
#include "save.h"

/* (the endless net's fifth act is the Undernet's, layers 13-15: run.c's
 * biome_order[4]; a best dive past 12 went into it) */
#define UNDERNET_DEPTH 13
/* (BN5's Darkloids' navi numbers, guardians.h's GUARDIAN_DARKLOID_FIRST and
 * _LAST: ShadeMan, Cosmo's three) */
#define DARKLOID_FIRST 30
#define DARKLOID_LAST 33

static bool quiet(void) { return save_slot_weekly(); }

static bool darkloid_beaten(void) {
	for (int navi = DARKLOID_FIRST; navi <= DARKLOID_LAST; ++navi)
		if (rival(navi)->megaman_won > 0) return true;
	return false;
}

/* (as meta.c's beast_earned: Bass's mark, or a Nest cleared past the short
 * nets' wins, which the endless one's checkpoint counts as it falls) */
static bool cybeast_fallen(void) { return (profile.marks & MARK_NEST) || profile.nest_clears > (int)profile.short_wins; }

unsigned looks_milestones(void) {
	if (quiet()) return 0;
	const struct { int palette; bool held; } m[] = {
		{ LOOK_UNDERNET, profile.best_depth >= UNDERNET_DEPTH },
		{ LOOK_GREGAR, cybeast_fallen() },
		{ LOOK_PROTOMAN, rival_clearance() >= 2 },
		{ LOOK_NEBULA, darkloid_beaten() },
		{ LOOK_HAZARD, (profile.marks & MARK_THREAT) != 0 },
	};
	unsigned got = 0;
	for (unsigned i = 0; i < sizeof m / sizeof *m; ++i)
		if (m[i].held && looks_grant(m[i].palette)) got |= 1u << m[i].palette;
	return got;
}

bool looks_milestone(int palette) { return !quiet() && looks_grant(palette); }

int looks_shop_offer(int out[LOOKS_SHOP_COUNT]) {
	int n = 0;
	if (quiet()) return 0;
	for (int p = LOOK_CENTRAL; p < LOOK_CENTRAL + LOOKS_SHOP_COUNT; ++p)
		if (!looks_owned(p)) out[n++] = p;
	return n;
}

static bool glitch_run(void) {
	for (int depth = 1; depth <= CYCLE_LAYERS; ++depth)
		if (run_glitch_at(depth)) return true;
	return false;
}

int looks_find_palette(void) {
	if (quiet()) return -1;
	if (profile.look_find_run == run.seed && (profile.look_find == LOOK_MYSTERY || profile.look_find == LOOK_GLITCH)) return profile.look_find;
	if (!looks_owned(LOOK_GLITCH) && glitch_run()) return LOOK_GLITCH;
	return looks_owned(LOOK_MYSTERY) ? -1 : LOOK_MYSTERY;
}

static unsigned find_hash(unsigned k) { k ^= k >> 16; k *= 0x45D9F3Bu; k ^= k >> 16; k *= 0x45D9F3Bu; return k ^ (k >> 16); }

int looks_find_depth(void) {
	/* (an act's first or last layer of 4-7: its second may be its dark
	 * layer, a Liberation's board with no Mystery Data, docs/game/
	 * liberation/; never the Spin's layer, whose best data is the Spin's) */
	static const int depths[] = { 4, 6, 7 };
	int k = (int)(find_hash(run.seed ^ 0x434F4C52u) % 3u);   /* "COLR" */
	return depths[k] == meta_spin_depth() ? depths[(k + 1) % 3] : depths[k];
}

void looks_find_taken(int palette) {
	if (quiet()) return;
	profile.look_find_run = run.seed;
	profile.look_find = (uint8_t)palette;
	if (!looks_grant(palette)) profile_save();
}

unsigned looks_untold(void) {
	unsigned owned = 0;
	for (int p = LOOK_CLASSIC + 1; p < LOOK_PALETTES; ++p)
		if (looks_owned(p)) owned |= 1u << p;
	return owned & ~(unsigned)profile.looks_told;
}

void looks_told(unsigned palettes) {
	profile.looks_told |= (uint16_t)palettes;
	profile_save();
}
